#ifdef COLOUR_ON

#define OUT_RED     "\x1B[31m"
#define OUT_GREEN   "\x1B[32m"
#define OUT_YELLOW  "\x1B[33m"
#define OUT_BLUE    "\x1B[34m"
#define OUT_MAGENTA "\x1B[35m"
#define OUT_CYAN    "\x1B[36m"
#define OUT_WHITE   "\x1B[37m"

#define OUT_BOLD    "\x1B[1m"
#define OUT_REV     "\x1B[7m"
#define OUT_UNDER   "\x1B[4m"

#define OUT_RESET   "\x1B[0m"

#else

#define OUT_RED     ""
#define OUT_GREEN   ""
#define OUT_YELLOW  ""
#define OUT_BLUE    ""
#define OUT_MAGENTA ""
#define OUT_CYAN    ""
#define OUT_WHITE   ""

#define OUT_BOLD    ""
#define OUT_REV     ""
#define OUT_UNDER   ""

#define OUT_RESET   ""

#endif
