#include "storage_task.h"

#include "mbed.h"
#include <stdio.h>
#include <errno.h>
#include <functional>
#include <string>

#include "BlockDevice.h"
#include "FATFileSystem.h"
#include "ui.h"

// Maximum number of elements in buffer
#define BUFFER_MAX_LEN 10
#define FORCE_REFORMAT true

BlockDevice *bd = BlockDevice::get_default_instance();

FATFileSystem fs("fs");

void Tasks::Storage::example(UI::Display *display) {
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

  printf("Path: / \n");
  display->log((uint8_t *)"Path: /");
  while (true) {
    struct dirent *e = readdir(d);
    if (!e) {
      break;
    }

    printf("    %s\n", e->d_name);

    std::string file_name = std::string("    ");
    file_name.append(e->d_name);
    file_name.append("\n");
    display->log((uint8_t *)file_name.c_str());
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