/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * User test program: spin
 *
 * A simple CPU-bound loop intended to generate measurable
 * cpu_ticks for the calling process when run under proccost.
 *
 * Usage:
 *   $ spin &
 * ========================================================= */

#include "kernel/types.h"
#include "user/user.h"

// ===== FEATURE 2 START: Expensive Process Analysis =====

int
main(int argc, char *argv[])
{
  // Busy-loop forever. The body is intentionally trivial so
  // the process simply burns CPU time until killed.
  while(1) {
    // Prevent the compiler from optimizing away the loop body.
    asm volatile("");
  }

  // Not reached.
  exit(0);
}

// ===== FEATURE 2 END: Expensive Process Analysis =====
