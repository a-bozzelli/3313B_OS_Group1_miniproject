/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * User test program: writer
 *
 * Repeatedly opens/creates a file and writes to it in a loop,
 * generating disk-backed write() activity that should show up
 * in the disk_writes and cost columns reported by proccost.
 *
 * Usage:
 *   $ writer &          # run indefinitely (original behavior)
 *   $ writer 300 &      # run for ~300 iterations then exit
 *
 * The optional numeric argument lets you cap how long writer
 * runs so it doesn't dominate the system for too long when
 * you're experimenting with proccost.
 * ========================================================= */

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// ===== FEATURE 2 START: Expensive Process Analysis =====

static const char *filename = "writer.log";

// Parse a positive decimal integer from a string. Returns
// -1 on error, or the parsed value (> 0) on success.
static int
parse_positive_int(const char *s)
{
  int n = 0;

  if(s == 0 || *s == 0)
    return -1;

  while(*s){
    if(*s < '0' || *s > '9')
      return -1;
    n = n * 10 + (*s - '0');
    s++;
  }

  if(n <= 0)
    return -1;
  return n;
}

int
main(int argc, char *argv[])
{
  char buf[128];
  int i;

  // Optional limit on the number of write iterations.
  // If no argument is given, run indefinitely (original behavior).
  int max_iters = -1;
  if(argc >= 2){
    max_iters = parse_positive_int(argv[1]);
    if(max_iters < 0){
      printf("writer: invalid limit '%s'\n", argv[1]);
      exit(1);
    }
  }

  // Prepare a deterministic buffer so that each write() has
  // some non-trivial size but still fits easily in a block.
  for(i = 0; i < sizeof(buf); i++) {
    buf[i] = 'A' + (i % 26);
  }

  // Loop, optionally bounded by max_iters if provided.
  int iter = 0;
  for(;;) {
    if(max_iters > 0 && iter >= max_iters)
      break;

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

    iter++;
  }

  // Exit cleanly once the requested number of iterations completes.
  exit(0);
}

// ===== FEATURE 2 END: Expensive Process Analysis =====
