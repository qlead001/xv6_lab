struct stat;
struct rtcdate;

// system calls
int debug(void);
int fork(void);
int exit(int status) __attribute__((noreturn));
int wait(int *status);
int waitpid(int pid, int *status, int options);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int setprior(int prior);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

// Macros

// proc
#define W_NOHANG	1

#define	MAX_PRIOR	16
#define	MIN_PRIOR	1
#define	DEF_PRIOR	4
// exit
#define	E_FINE	0
#define	E_ERR	1
#define	E_WARN	2
