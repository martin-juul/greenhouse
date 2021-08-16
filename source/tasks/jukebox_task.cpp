#include "jukebox_task.h"
#include "jukebox.h"

InterruptIn button(D2);

void _button_handler() {
  if (Jukebox::is_playing()) {
    Jukebox::stop();
    return;
  }

  Jukebox::play();
}

void Tasks::Jukebox::start() {
  button.rise(_button_handler);
}