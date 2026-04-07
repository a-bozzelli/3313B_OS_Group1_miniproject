/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * User test program: writer
 *
 * Repeatedly opens/creates a file and writes to it in a loop,
 * generating disk-backed write() activity that should show up
 * in the disk_writes and cost columns reported by proccost.
 *
 * Usage:
 *   $ writer &
 * ========================================================= */

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// ===== FEATURE 2 START: Expensive Process Analysis =====

static const char *filename = "writer.log";

int
main(int argc, char *argv[])
{
  char buf[128];
  int i;

  // Prepare a deterministic buffer so that each write() has
  // some non-trivial size but still fits easily in a block.
  for(i = 0; i < sizeof(buf); i++) {
    buf[i] = 'A' + (i % 26);
  }

  // Loop forever creating write-heavy activity.
  for(;;) {
    int fd = open(filename, O_CREATE | O_WRONLY);
    if(fd < 0) {
      printf("writer: failed to open %s\n", filename);
      exit(1);
    }

    // Perform several write() calls per iteration so that the
    // per-process disk_writes counter grows quickly.
    for(int w = 0; w < 8; w++) {
      if(write(fd, buf, sizeof(buf)) < 0) {
        printf("writer: write error on %s\n", filename);
        close(fd);
        exit(1);
      }
    }

    close(fd);
  }

  // Not reached.
  exit(0);
}

// ===== FEATURE 2 END: Expensive Process Analysis =====
