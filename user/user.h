#include "kernel/types.h"

#define SBRK_ERROR ((char *)-1)

struct stat;

/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * User-visible structure mirroring kernel/proc.h:proccostinfo.
 * This must stay layout-compatible with the kernel definition
 * so that the proccost() syscall ABI remains correct.
 * ========================================================= */

// ===== FEATURE 2 START: Expensive Process Analysis =====

struct proccostinfo {
	int    pid;                 // Process ID
	char   name[16];            // Process name (truncated)

	uint64 cpu_ticks;           // CPU ticks while RUNNING
	uint64 sched_count;         // Number of times scheduled
	uint64 disk_writes;         // Approximate disk-backed write() count

	// Cost score as computed by the kernel:
	//   cost = cpu_ticks + 5 * disk_writes
	uint64 cost;
};

// ===== FEATURE 2 END: Expensive Process Analysis =====

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(const char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sys_sbrk(int,int);
int pause(int);
int uptime(void);
int setpowermode(int, int);
int setcpulimit(int, int);
int getcputicks(int);

/* =========================================================
 * FEATURE 2: EXPENSIVE PROCESS ANALYSIS
 * User-facing prototype for the proccost() syscall.
 *
 * proccost(info, max) fills up to max entries in the array
 * pointed to by info and returns the number of entries
 * written, or -1 on error.
 * ========================================================= */

// ===== FEATURE 2 START: Expensive Process Analysis =====
int proccost(struct proccostinfo *info, int max);
// ===== FEATURE 2 END: Expensive Process Analysis =====
struct syshealth;
int getsyshealth(struct syshealth*);

// usage accounting
struct usagestats {
	unsigned long cpu_ticks;
	unsigned long cpu_ticks_limit;
};
int setusagelimit(int pid, int max_ticks);
int getusagestats(int pid, struct usagestats *st);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
char* gets(char*, int max);
unsigned int strlen(const char*);
void* memset(void*, int, unsigned int);
int atoi(const char*);
int memcmp(const void *, const void *, unsigned int);
void *memcpy(void *, const void *, unsigned int);
char* sbrk(int);
char* sbrklazy(int);

// printf.c
void fprintf(int, const char*, ...) __attribute__ ((format (printf, 2, 3)));
void printf(const char*, ...) __attribute__ ((format (printf, 1, 2)));

// umalloc.c
void* malloc(unsigned int);
void free(void*);
