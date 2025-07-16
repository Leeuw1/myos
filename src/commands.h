#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "core.h"

i32 syscall_command(u8 argc, const char* argv[]);
i32 shutdown(u8 argc, const char* argv[]);
i32 fat_test(u8 argc, const char* argv[]);

#endif //_COMMANDS_H
