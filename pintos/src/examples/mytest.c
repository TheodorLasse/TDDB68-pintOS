/* I cleaned this file up a bit, but it is still not pretty. -- ME */
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <stdarg.h>
#include "tests/lib.h"
#include "tests/main.h"
int main(void)
{  
    wait (exec ("printf"));
}
