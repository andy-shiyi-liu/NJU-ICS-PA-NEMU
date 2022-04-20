#include "trap.h"

int fs_write(int fd, void *buf, int len)
{
	nemu_assert(fd <= 2);

    // here this asm code will triger push_i_b bug without call push_i_b?
	asm volatile(".byte 0x82"
				 : "=a"(len)
				 : "a"(4), "b"(fd), "c"(buf), "d"(len));
	return len;
}

int main()
{
	static char buf[256];
	fs_write(1, buf, 1);

    HIT_GOOD_TRAP;
    return 0;
}