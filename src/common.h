#pragma once

#include <unistd.h>

namespace tty {

inline void red(int fd) {
  if (isatty(fd)) {
    fputs("\x1b[1;31m", fd == STDOUT_FILENO ? stdout : stderr);
  }
}

inline void sgr0(int fd) {
  if (isatty(fd)) {
    fputs("\x1b[m", fd == STDOUT_FILENO ? stdout : stderr);
  }
}

}  // namespace tty
