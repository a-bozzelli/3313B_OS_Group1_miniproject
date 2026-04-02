/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * User program: proccost
 *
 * This program queries the kernel for per-process cost data
 * via the proccost() system call and prints the most
 * expensive processes in a simple table.
 * ========================================================= */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// ===== FEATURE 2 START: Expensive Process Analysis =====

// Maximum number of processes we expect. This should be
// at least as large as NPROC in the kernel.
#define MAX_PROCS 64

// Use a global buffer instead of a large stack-allocated
// array to avoid overflowing xv6's small user stack.
static struct proccostinfo g_info[MAX_PROCS];

// Fixed column widths for pretty-printed table output.
#define PID_COL_WIDTH    4
#define NAME_COL_WIDTH  14
#define CPU_COL_WIDTH   10
#define SCHED_COL_WIDTH 12
#define DISK_COL_WIDTH  12
#define COST_COL_WIDTH  10

// Print n space characters to standard output.
static void
print_spaces(int n)
{
  for(int i = 0; i < n; i++)
    printf(" ");
}

// Print an unsigned 64-bit integer right-aligned in a field
// of the given width using only basic printf features.
static void
print_uint64_padded(uint64 x, int width)
{
  char buf[32];
  int i = 0;

  do {
    buf[i++] = '0' + (x % 10);
    x /= 10;
  } while(x != 0 && i < (int)sizeof(buf));

  // Left pad with spaces up to the requested width.
  for(int k = i; k < width; k++)
    printf(" ");

  // Digits are stored in reverse order; print them back out.
  for(int j = i - 1; j >= 0; j--)
    printf("%c", buf[j]);
}

// Print a process name left-aligned in a fixed-width field,
// truncating if necessary and padding with spaces otherwise.
static void
print_name_padded(const char *name, int width)
{
  int len = strlen(name);
  if(len > width)
    len = width;

  for(int i = 0; i < len; i++)
    printf("%c", name[i]);
  for(int i = len; i < width; i++)
    printf(" ");
}

// Simple in-place sort by descending cost score.
static void
sort_by_cost(struct proccostinfo *info, int n)
{
  for(int i = 0; i < n; i++) {
    int max_idx = i;
    for(int j = i + 1; j < n; j++) {
      if(info[j].cost > info[max_idx].cost) {
        max_idx = j;
      }
    }
    if(max_idx != i) {
      struct proccostinfo tmp = info[i];
      info[i] = info[max_idx];
      info[max_idx] = tmp;
    }
  }
}

int
main(int argc, char *argv[])
{
  // Ask the kernel to fill in per-process accounting data.
  int n = proccost(g_info, MAX_PROCS);
  if(n < 0) {
    printf("proccost: syscall failed\n");
    exit(1);
  }
  if(n > MAX_PROCS)
    n = MAX_PROCS;

  // Sort processes by descending cost so the
  // most expensive ones appear first.
  sort_by_cost(g_info, n);

  // Print a simple header and one line per process.
  printf("PID ");
  print_spaces(PID_COL_WIDTH - 3);
  printf("  NAME");
  print_spaces(NAME_COL_WIDTH - 4);
  printf("  CPU_TICKS");
  print_spaces(CPU_COL_WIDTH - 9);
  printf("  SCHED_COUNT");
  print_spaces(SCHED_COL_WIDTH - 11);
  printf("  DISK_WRITES");
  print_spaces(DISK_COL_WIDTH - 11);
  printf("  COST\n");

  for(int i = 0; i < n; i++) {
    print_uint64_padded((uint64)g_info[i].pid, PID_COL_WIDTH);
    printf("  ");
    print_name_padded(g_info[i].name, NAME_COL_WIDTH);
    printf("  ");
    print_uint64_padded(g_info[i].cpu_ticks, CPU_COL_WIDTH);
    printf("  ");
    print_uint64_padded(g_info[i].sched_count, SCHED_COL_WIDTH);
    printf("  ");
    print_uint64_padded(g_info[i].disk_writes, DISK_COL_WIDTH);
    printf("  ");
    print_uint64_padded(g_info[i].cost, COST_COL_WIDTH);
    printf("\n");
  }

  exit(0);
}

// ===== FEATURE 2 END: Expensive Process Analysis =====
