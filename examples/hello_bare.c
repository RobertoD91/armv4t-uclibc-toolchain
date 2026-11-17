/* Freestanding ARMv4T hello world: no libc, raw Linux EABI syscalls (svc #0,
 * syscall number in r7). Guaranteed to run on the ARMv4T (ARM7TDMI/ARM720T (ARMv4T))
 * because it depends on nothing but the kernel syscall ABI. */

static inline long arm_syscall3(long nr, long a, long b, long c) {
    register long r7 asm("r7") = nr;
    register long r0 asm("r0") = a;
    register long r1 asm("r1") = b;
    register long r2 asm("r2") = c;
    asm volatile ("svc #0" : "+r"(r0) : "r"(r7), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static unsigned slen(const char *s) { unsigned n = 0; while (s[n]) n++; return n; }

void _start(void) {
    static const char msg[] =
        "Hello ARMv4T! (bare-metal ARMv4T, raw Linux syscalls, no libc)\n";
    arm_syscall3(4, 1, (long)msg, slen(msg));  /* write(1, msg, len) */
    arm_syscall3(1, 0, 0, 0);                   /* exit(0) */
    for (;;) {}
}
