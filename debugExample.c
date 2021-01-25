// Run the debug syscall

#include "types.h"
#include "user.h"

int
main(void)
{
  debug();
  exit(0);
}
