#include "BlockDevice.h"
#include "Callback.h"
#include "EventQueue.h"
#include "Semaphore.h"
#include "ThisThread.h"
#include "database.h"
#include "jukebox.h"
#include "mbed.h"
#include "mbed_wait_api.h"
#include "ui/ui.h"
#include "webserver.h"
#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <functional>

#define MAX_THREAD_STACK 384
#define SAMPLE_TIME_MS 1000

Thread thread_main;
Thread thread_web_server;
Thread thread_jukebox;
Thread thread_sd;

UI::Display *display;

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Semaphore s_db(1);

InterruptIn button(D2);

void button_handler() {
  if (Jukebox::is_playing()) {
    Jukebox::stop();
    return;
  }

  Jukebox::play();
}

void jukebox_task() { button.rise(button_handler); }

void webserver_task(WebServer *webServer) {
  while (true) {
    webServer->tick();
  }
}

/*  SD Start   */

// Maximum number of elements in buffer
#define BUFFER_MAX_LEN 10
#define FORCE_REFORMAT true
// This will take the system's default block device
BlockDevice *bd = BlockDevice::get_default_instance();

#include "FATFileSystem.h"
FATFileSystem fs("fs");

// Set up the button to trigger an erase
InterruptIn irq(BUTTON1);
void erase() {
  printf("Initializing the block device... ");
  fflush(stdout);
  int err = bd->init();
  printf("%s\n", (err ? "Fail :(" : "OK"));
  if (err) {
    error("error: %s (%d)\n", strerror(-err), err);
  }

  printf("Erasing the block device... ");
  fflush(stdout);
  err = bd->erase(0, bd->size());
  printf("%s\n", (err ? "Fail :(" : "OK"));
  if (err) {
    error("error: %s (%d)\n", strerror(-err), err);
  }

  printf("Deinitializing the block device... ");
  fflush(stdout);
  err = bd->deinit();
  printf("%s\n", (err ? "Fail :(" : "OK"));
  if (err) {
    error("error: %s (%d)\n", strerror(-err), err);
  }
}

static auto erase_event =
    mbed_event_queue() -> make_user_allocated_event(erase);

void sd_test() {
  printf("--- Mbed OS filesystem example ---\n");

  // Setup the erase event on button press, use the event queue
  // to avoid running in interrupt context
  irq.fall(std::ref(erase_event));

  // Try to mount the filesystem
  printf("Mounting the filesystem... ");
  fflush(stdout);
  int err = fs.mount(bd);
  printf("%s\n", (err ? "Fail :(" : "OK"));
  if (err || FORCE_REFORMAT) {
    // Reformat if we can't mount the filesystem
    printf("formatting... ");
    fflush(stdout);
    err = fs.reformat(bd);
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
      error("error: %s (%d)\n", strerror(-err), err);
    }
  }

  // Open the numbers file
  printf("Opening \"/fs/numbers.txt\"... ");
  fflush(stdout);
  FILE *f = fopen("/fs/numbers.txt", "r+");
  printf("%s\n", (!f ? "Fail :(" : "OK"));
  if (!f) {
    // Create the numbers file if it doesn't exist
    printf("No file found, creating a new file... ");
    fflush(stdout);
    f = fopen("/fs/numbers.txt", "w+");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
      error("error: %s (%d)\n", strerror(errno), -errno);
    }

    for (int i = 0; i < 10; i++) {
      printf("\rWriting numbers (%d/%d)... ", i, 10);
      fflush(stdout);
      err = fprintf(f, "    %d\n", i);
      if (err < 0) {
        printf("Fail :(\n");
        error("error: %s (%d)\n", strerror(errno), -errno);
      }
    }
    printf("\rWriting numbers (%d/%d)... OK\n", 10, 10);

    printf("Seeking file... ");
    fflush(stdout);
    err = fseek(f, 0, SEEK_SET);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
      error("error: %s (%d)\n", strerror(errno), -errno);
    }
  }

  // Go through and increment the numbers
  for (int i = 0; i < 10; i++) {
    printf("\rIncrementing numbers (%d/%d)... ", i, 10);
    fflush(stdout);

    // Get current stream position
    long pos = ftell(f);

    // Parse out the number and increment
    char buf[BUFFER_MAX_LEN];
    if (!fgets(buf, BUFFER_MAX_LEN, f)) {
      error("error: %s (%d)\n", strerror(errno), -errno);
    }
    char *endptr;
    int32_t number = strtol(buf, &endptr, 10);
    if ((errno == ERANGE) ||         // The number is too small/large
        (endptr == buf) ||           // No character was read
        (*endptr && *endptr != '\n') // The whole input was not converted
    ) {
      continue;
    }
    number += 1;

    // Seek to beginning of number
    fseek(f, pos, SEEK_SET);

    // Store number
    fprintf(f, "    %d\n", number);

    // Flush between write and read on same file
    fflush(f);
  }
  printf("\rIncrementing numbers (%d/%d)... OK\n", 10, 10);

  // Close the file which also flushes any cached writes
  printf("Closing \"/fs/numbers.txt\"... ");
  fflush(stdout);
  err = fclose(f);
  printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
  if (err < 0) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  // Display the root directory
  printf("Opening the root directory... ");
  fflush(stdout);
  DIR *d = opendir("/fs/");
  printf("%s\n", (!d ? "Fail :(" : "OK"));
  if (!d) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  printf("root directory:\n");
  while (true) {
    struct dirent *e = readdir(d);
    if (!e) {
      break;
    }

    printf("    %s\n", e->d_name);
  }

  printf("Closing the root directory... ");
  fflush(stdout);
  err = closedir(d);
  printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
  if (err < 0) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  // Display the numbers file
  printf("Opening \"/fs/numbers.txt\"... ");
  fflush(stdout);
  f = fopen("/fs/numbers.txt", "r");
  printf("%s\n", (!f ? "Fail :(" : "OK"));
  if (!f) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  printf("numbers:\n");
  while (!feof(f)) {
    int c = fgetc(f);
    printf("%c", c);
  }

  printf("\rClosing \"/fs/numbers.txt\"... ");
  fflush(stdout);
  err = fclose(f);
  printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
  if (err < 0) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  // Tidy up
  printf("Unmounting... ");
  fflush(stdout);
  err = fs.unmount();
  printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
  if (err < 0) {
    error("error: %s (%d)\n", strerror(-err), err);
  }

  printf("Mbed OS filesystem example done!\n");
}

void sd_read_test(UI::Display *display) {
  // Try to mount the filesystem
  display->log((uint8_t *)"Mounting the filesystem...");
  fflush(stdout);
  int err = fs.mount(bd);
  printf("%s\n", (err ? "Fail :(" : "OK"));
  if (err) {
    display->log((uint8_t *)"Could not mount SD card");
    return;
  }

  display->log((uint8_t *)"SD Card mounted successfully");

  // Open the numbers file
  printf("Opening \"/fs/numbers.txt\"... ");
  fflush(stdout);
  FILE *f = fopen("/fs/numbers.txt", "r+");
  printf("%s\n", (!f ? "Fail :(" : "OK"));
  if (!f) {
    // Create the numbers file if it doesn't exist
    printf("No file found, creating a new file... ");
    fflush(stdout);
    f = fopen("/fs/numbers.txt", "w+");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
      error("error: %s (%d)\n", strerror(errno), -errno);
    }

    for (int i = 0; i < 10; i++) {
      printf("\rWriting numbers (%d/%d)... ", i, 10);
      fflush(stdout);
      err = fprintf(f, "    %d\n", i);
      if (err < 0) {
        printf("Fail :(\n");
        error("error: %s (%d)\n", strerror(errno), -errno);
      }
    }
    printf("\rWriting numbers (%d/%d)... OK\n", 10, 10);

    printf("Seeking file... ");
    fflush(stdout);
    err = fseek(f, 0, SEEK_SET);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
      error("error: %s (%d)\n", strerror(errno), -errno);
    }
  }

  // Go through and increment the numbers
  for (int i = 0; i < 10; i++) {
    printf("\rIncrementing numbers (%d/%d)... ", i, 10);
    fflush(stdout);

    // Get current stream position
    long pos = ftell(f);

    // Parse out the number and increment
    char buf[BUFFER_MAX_LEN];
    if (!fgets(buf, BUFFER_MAX_LEN, f)) {
      error("error: %s (%d)\n", strerror(errno), -errno);
    }
    char *endptr;
    int32_t number = strtol(buf, &endptr, 10);
    if ((errno == ERANGE) ||         // The number is too small/large
        (endptr == buf) ||           // No character was read
        (*endptr && *endptr != '\n') // The whole input was not converted
    ) {
      continue;
    }
    number += 1;

    // Seek to beginning of number
    fseek(f, pos, SEEK_SET);

    // Store number
    fprintf(f, "    %d\n", number);

    // Flush between write and read on same file
    fflush(f);
  }
  printf("\rIncrementing numbers (%d/%d)... OK\n", 10, 10);

  // Close the file which also flushes any cached writes
  printf("Closing \"/fs/numbers.txt\"... ");
  fflush(stdout);
  err = fclose(f);
  printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
  if (err < 0) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  // Display the root directory
  printf("Opening the root directory... ");
  fflush(stdout);
  DIR *d = opendir("/fs/");
  printf("%s\n", (!d ? "Fail :(" : "OK"));
  if (!d) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  printf("root directory:\n");
  display->log((uint8_t *)"root directory:");
  while (true) {
    struct dirent *e = readdir(d);
    if (!e) {
      break;
    }

    printf("    %s\n", e->d_name);
  }

  display->log((uint8_t *)"Closing the root directory");
  fflush(stdout);
  err = closedir(d);
  printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
  if (err < 0) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  // Display the numbers file
  display->log((uint8_t *)"Opening /fs/numbers.txt");
  fflush(stdout);
  f = fopen("/fs/numbers.txt", "r");

  if (!f) {
    display->log((uint8_t *)"Operation failed");
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  display->log((uint8_t *)"numbers:");
  while (!feof(f)) {
    int c = fgetc(f);
    printf("%c", c);
  }

  printf("\rClosing \"/fs/numbers.txt\"... ");
  fflush(stdout);
  err = fclose(f);
  printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
  if (err < 0) {
    error("error: %s (%d)\n", strerror(errno), -errno);
  }

  // Tidy up
  printf("Unmounting... ");
  fflush(stdout);
  err = fs.unmount();
  printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
  if (err < 0) {
    error("error: %s (%d)\n", strerror(-err), err);
  }
}

/*  SD END   */

void print_cpu_stats() {
  mbed_stats_cpu_t stats;
  mbed_stats_cpu_get(&stats);

  display->update_stats(&stats, SAMPLE_TIME_MS);
}

int main(void) {
  display->init();

  thread_jukebox.start(jukebox_task);

  int id;
  Thread *thread_stats;
  EventQueue *stats_queue = mbed_event_queue();
  id = stats_queue->call_every(SAMPLE_TIME_MS, print_cpu_stats);

  thread_stats = new Thread(osPriorityNormal, MAX_THREAD_STACK);

  Database db;
  WebServer webServer = WebServer(&db);

  display->log((uint8_t *)"[webserver]: starting");

  int status = webServer.start();
  if (status == 0) {
    display->log((uint8_t *)"[webserver]: error - No network interface found");
    return 1;
  }

  display->log((uint8_t *)"[webserver]: IP: 192.168.1.100");

  display->log((uint8_t *)"[host]: spawning webserver_thread");
  thread_web_server.start(callback(webserver_task, &webServer));

  sd_test();

  return 0;
}
