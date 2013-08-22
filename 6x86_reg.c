/*  6x86_reg: a tool for displaying Cyrix 6x86 configuration registers
 *  and ARRs in human-readable format.
 *
 *  Copyright (C) 1996, 1997  Koen Gadeyne
 *  Modifications for OS/2 port by Greg Kondrasuk, 1998.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Updated 12/97 by Andrew D. Balsa
 *  (see http://www.tux.org/~balsa/linux/cyrix)
 *
 ******************************************************************************
 *
 *  Usage: no parameters! Just typing 6x86_reg at the prompt will dump all 6x86(L)
 *  documented registers to the console. Now of course if you run it on another
 *  processor model don't expect any meaningful results...
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <asm/io.h>  not needed by OS/2

// start of OS/2 section added
#include <sys/hw.h>
#define outb(data,port) _outp32(port,data)
#define inb(port) _inp32(port)
#define ioperm(x,y,z) (0)
#define VERSION 1.5
// end of OS/2 section

typedef int bool;
#ifndef TRUE
#  define TRUE (1)
#endif
#ifndef FALSE
#  define FALSE (0)
#endif
char *CommandName;
char info[256];
void get_IO_range(int start, int len)
{
  if (ioperm(start, len, 1) != 0)
  {
    perror("I/O Permissions");
    fprintf(stderr,"Cannot get I/O permissions for hardware address range 0x%x-0x%x.\n\
             You must be superuser, or the program must be setuid root!\n", start, start+len-1);
    exit(1);
  }
}
char* int_to_bin(int num, int bits)
{
  static char binstr[sizeof(long int)+1];
  int i;
  for (i=0; i<bits; i++) binstr[i] = 0x30; /* char '0' */
  binstr[bits] = 0x00;
  for (i=0; i<bits; i++) binstr[bits-1-i] += ((num >> i) & 0x00000001);
  return(binstr);
}
char *size_str(int kb)
{
  static char size[20];
  if (kb < 1024) sprintf(size, "%d KB", kb);
  else if (kb < 1024*1024) sprintf(size, "%d MB", kb >> 10);
  else sprintf(size, "%d GB", kb >> 20);
  return size;
}
/* the original bogomips code from the Linux kernel */
static __inline__ void delay(int loops)
{
  __asm__(".align 2,0x90\n1:\tdecl %0\n\tjns 1b": :"a" (loops):"ax");
}
/* a short routine to read /proc/cpuinfo */
void read_cpuinfo(void) {
  FILE *in;
  char things[256];
  extern char info[256];
  if ((in=fopen("/proc/cpuinfo","r"))==NULL)
      strcpy(info,"unknown");
  else {
   while (strcmp(things,"bogomips"))
     fscanf(in, "%s", things);   /* skip to the bogomips line */
   fscanf(in, "%s", things);  /* skip the ":" */
   fscanf(in, "%s", info);    /* now this should be the bogomips */
   fclose(in);
  }
  return;
}
/* Register index defines */
#define ARR_BASE 0xC4
#define RCR_BASE 0xDC
#define CCR0 0xC0
#define CCR1 0xC1
#define CCR2 0xC2
#define CCR3 0xC3
#define CCR4 0xE8
#define CCR5 0xE9
#define DIR0 0xFE
#define DIR1 0xFF
/*****************************************************************************/
/* This was a nice short routine before I began messing with it
 * It still has a very linear structure, but the registers on the 6x86 line
 * of CPUs are so irregular that the program got a little bit messy.
 * - Andrew
 */
/*****************************************************************************/
int main (int argc, char* argv[])
{
  int i;
  unsigned int addr, size, rcr, ccr, dir;
  int mapen;
  unsigned long loops_per_sec = 1;
  unsigned long ticks;
/*
 * start doing something useful
 */
   get_IO_range(0x22, 2);
   printf("\n6x86 (Classic/L/MX) Register Dump utility\n\n");
  /* MAPEN **************************************************************/
  outb(0xC3, 0x22); mapen = inb(0x23);
  outb(0xC3, 0x22); outb(mapen | 0x10, 0x23);
  /* DIR0 ***************************************************************/
  outb(DIR0, 0x22); dir = inb(0x23);
  printf("6x86 DIR0: 0x%x\t", dir);
  /* core/bus clock ratio analysis **************************************/
  if ((dir & 0x70) == 0x50)   /* 6x86MX */
  switch (dir & 0x07) {
   case 0x01:
      { printf("2X core/bus clock ratio");
        break;
      }
   case 0x02:
      { printf("2.5X core/bus clock ratio");
        break;
      }
   case 0x03:
      { printf("3X core/bus clock ratio");
        break;
      }
   case 0x04:
      { printf("3.5X core/bus clock ratio");
        break;
      }
  }
  else            /* 6x86(L) */
  switch (dir & 0x05) {
   case 0x00:
      { printf("1X core/bus clock ratio");
        break;
      }
   case 0x01:     /* practically the only one used */
      { printf("2X core/bus clock ratio");
        break;
      }
   case 0x05:
      { printf("3X core/bus clock ratio");
        break;
      }
   case 0x04:
      { printf("4X core/bus clock ratio");
        break;
      }
  }
  printf("\n");
  /* DIR1 ***************************************************************/
  outb(DIR1, 0x22); dir = inb(0x23);
  printf("     DIR1: 0x%x\t", dir);
  /* Processor identification *******************************************/
  switch (dir) {
   case 0x14:
      { printf("6x86 Rev. 2.4");
        break;
      }
   case 0x15:
      { printf("6x86 Rev. 2.5");
        break;
      }
   case 0x16:
      { printf("6x86 Rev. 2.6");
        break;
      }
   case 0x17:
      { printf("6x86 Rev. 2.7 or 3.7");
        break;
      }
   case 0x22:
      { printf("6x86L Rev. 4.2");
        break;
      }
   case 0x03:
      { printf("6x86MX Rev. 1.3");
        break;
      }
   case 0x04:
      { printf("6x86MX Rev. 1.4");
        break;
      }
  }
  /* BogoMIPS calculation ***********************************************/
  printf("\n\nWait a moment...\n");
  /* Now let's take a look at the two bogomips readings */
  /* First we calculate the bogoMIPS reading using exactly
   * the same code as the Linux kernel
   */
   while ((loops_per_sec <<= 1)) {
    ticks = clock();
    delay(loops_per_sec);
    ticks = clock() - ticks;
    if (ticks >= CLOCKS_PER_SEC) {
      loops_per_sec = (loops_per_sec / ticks) * CLOCKS_PER_SEC;
      printf("Calculated BogoMIPS:\t%lu.%02lu\n",
        loops_per_sec/500000,
        (loops_per_sec/5000) % 100
        );
      break;
      }
  }
  /* and now the kernel bogoMIPS, calculated at boot time and
   * reported by /proc/cpuinfo.
   */
  read_cpuinfo();
  printf("Kernel BogoMIPS:\t%s\n", info);
  printf("\n");
  /* CCR ****************************************************************/
  outb(CCR0, 0x22); ccr = inb(0x23);
  printf("6x86 CCR0: 0x%x\t", ccr);
  if (ccr & 0x02) printf("NC1 set (address region 640Kb-1Mb non-cacheable)\n");
  else printf("NC1 reset (address region 640Kb-1Mb cacheable)\n");
  outb(CCR1, 0x22); ccr = inb(0x23);
  printf("     CCR1: 0x%x\t", ccr);
  if (ccr & 0x10) printf("NO_LOCK set\n");
  else printf("NO_LOCK reset\n");
  outb(CCR2, 0x22); ccr = inb(0x23);
  printf("     CCR2: 0x%x\t", ccr);
  if (ccr & 0x08) printf("SUSP_HLT set (low power suspend mode enabled)\n");
  else printf("SUSP_HLT reset (low power suspend mode disabled)\n");
  outb(CCR3, 0x22); ccr = inb(0x23);
  printf("     CCR3: 0x%x\n", ccr);
  outb(CCR4, 0x22); ccr = inb(0x23);
  printf("     CCR4: 0x%x\t", ccr);
  if (dir & 0xf0)    /* if this is a 6x86 or 6x86L */
    if (ccr & 0x10) printf("DTE cache enabled, ");
    else printf("DTE cache disabled, ");
  switch (ccr & 0x07) {
   case 0x00:
      { printf("1 clock cycle I/O recovery time");
        break;
      }
   case 0x01:
      { printf("2 clock cycles I/O recovery time");
        break;
      }
   case 0x02:
      { printf("4 clock cycles I/O recovery time");
        break;
      }
   case 0x03:
      { printf("8 clock cycles I/O recovery time");
        break;
      }
   case 0x04:
      { printf("16 clock cycles I/O recovery time");
        break;
      }
   case 0x05:
      { printf("32 clock cycles I/O recovery time");
        break;
      }
   case 0x06:
      { printf("64 clock cycles I/O recovery time");
        break;
      }
   case 0x07:
      { printf("no I/O recovery time");
        break;
      }
  }
  printf("\n");
  outb(CCR5, 0x22); ccr = inb(0x23);
  printf("     CCR5: 0x%x\t", ccr);
  if (dir & 0xf0)    /* if this is a 6x86 or 6x86L */
    if (ccr & 0x02) printf("slow LOOP enabled, ");
    else printf("slow LOOP disabled, ");
  if (ccr & 0x01) printf("allocate cache lines on write misses\n\n");
  else printf("default cache line allocation policy\n\n");
  /* ARR ****************************************************************/
  printf("6x86 Address Region Register dump:\n");
  for (i=0; i<8; i++)
  {
    outb(ARR_BASE + i*3, 0x22); addr = inb(0x23) << 24;
    outb(ARR_BASE + i*3 + 1, 0x22); addr |= inb(0x23) << 16;
    outb(ARR_BASE + i*3 + 2, 0x22); size = inb(0x23);
    addr |= (size & 0xF0) << 8;
    size &= 0x0F;
    printf("  ARR%d: ", i);
    if (size == 0)
    {
      printf("disabled\n");
      continue;
    }
    else
      printf("address = 0x%X , size = ", addr);
    if (i<7)
    {
      if (size == 15) printf("%s\n", size_str(4*1024*1024));
      else printf("%s\n", size_str(2 << size));
    }
    else
    {
      printf("%s\n", size_str(128 << size));
    }
    /* RCR **************************************************************/
    outb(RCR_BASE + i, 0x22); rcr = inb(0x23);
    printf("    RCR = 0x%X : ", rcr);
    if (i != 7)
      printf("%scached",  (rcr & 0x01) ? "not " : "");
    else
      printf("%scached",  (rcr & 0x01) ? "" : "not ");
    if (dir & 0xf0)     /* if this is a 6x86 or 6x86L */
      if (rcr & 0x2) printf(", weak write ordering");
    if (rcr & 0x4) printf(", weak locking");
    if (rcr & 0x8) printf(", write gathering");
    if (rcr & 0x10) printf(", write through");
    if (rcr & 0x20) printf(", no LBA#");
    printf("\n");
  }
  /* Disable MAPEN ******************************************************/
  outb(0xC3, 0x22); outb(mapen, 0x23);
  return(0);
}
