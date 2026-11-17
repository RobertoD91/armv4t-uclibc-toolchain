#include <stdio.h>
#include <sys/utsname.h>

int main(void) {
    struct utsname u;
    puts("Hello from the ARMv4T uClibc cross-toolchain!");
    if (uname(&u) == 0)
        printf("  kernel: %s %s   machine: %s\n", u.sysname, u.release, u.machine);
    puts("  target: ARMv4T soft-float (ARM7TDMI/ARM720T/arm920t), uClibc-ng");
    return 0;
}
