#include "lykosapi.h"

void main(void) {
  while (1) {
    KeyEvent ev;
    i64 ret = get_key_event(&ev);
    if (ret == 0) {
      sleep(100);
      continue;
    }
    if (ev.key == 'a') {
      write("entered a!\n");
    }
  }
}
