/*  get/set6x86: a tool for changing Cyrix 6x86 configuration registers
 *
 *  Copyright (C) 1996  Koen Gadeyne
 *  Modifications for OS/2 port by Greg Kondrasuk, 1997.
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
 */


/***
 *** This is just a hacking tool! Use at your own risk. It was NOT intended to be
 *** idiot proof! If you don't understand all this, then don't bother trying to use it.
 ***
 ***/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/hw.h>

#define outb(data,port) _outp32(port,data)
#define inb(port) _inp32(port)
#define ioperm(x,y,z) (0)
#define VERSION 1.5

#ifndef TRUE
#  define TRUE (1)
#endif
#ifndef FALSE
#  define FALSE (0)
#endif

typedef int bool;

char *CommandName;


//////////////////////////////////////////////////////////////////////////
// Function: get_IO_range
//////////////////////////////////////////////////////////////////////////
void get_IO_range(int start, int len)
{
   if (ioperm(start, len, 1) != 0) {
      perror("I/O Permissions");
      fprintf(stderr,"Cannot get I/O permissions for hardware address range 0x%x-0x%x.\n\
             You must be superuser, or the program must be setuid root!\n", start, start+len-1);

      exit(1);
   }
}

//////////////////////////////////////////////////////////////////////////
// Function: check_int_range
//////////////////////////////////////////////////////////////////////////
void check_int_range(int cvalue, int lmin, int lmax, char* descstr)
{
   if (cvalue<lmin || cvalue>lmax) {
      fprintf(stderr,"%s = %d (0x%x) out of range [%d..%d]!\n", descstr, cvalue, cvalue, lmin, lmax);

      exit(1);
   }
}

//////////////////////////////////////////////////////////////////////////
// Function: getint
// Description: Converts the int in 'instring' into an integer. Must be
//              within specified limits 'lmin' and 'lmax'. 'descrstring'
//              contains a description of the number to be parsed, used
//              in the error message.
//////////////////////////////////////////////////////////////////////////
int getint(char* instring, char* descrstring, int lmin, int lmax)
{
   char** errstr=NULL;
   int readint;
   readint = strtol(instring,errstr,0);

   if (errstr) {
      fprintf(stderr,"Illegal character '%s' in %s: '%s'\n", *errstr, descrstring, instring);
      exit(1);
   }

   check_int_range(readint, lmin, lmax, descrstring);

   return(readint);
}

//////////////////////////////////////////////////////////////////////////
// Function: int_to_bin
//////////////////////////////////////////////////////////////////////////
char* int_to_bin(int num, int bits)
{
   static char binstr[sizeof(long int)+1];
   int i;

   for (i=0; i<bits; i++)
      binstr[i] = 0x30; // char '0'

   binstr[bits] = 0x00;

   for (i=0; i<bits; i++)
      binstr[bits-1-i] += ((num >> i) & 0x00000001);

   return(binstr);
}

//////////////////////////////////////////////////////////////////////////
// Function: usage
// Description: displays program usage information.
//////////////////////////////////////////////////////////////////////////
void usage(bool setreg)
{
   printf("Inside the usage function.\n");
   printf("%s %.1f (c) 1996 Koen Gadeyne.\n", CommandName, VERSION);
   printf("Modified for OS/2 by Greg Kondrasuk.");
   printf("\n"
   "  Usage: %s [-hp] [-r register_index] %s\n\n", CommandName, (setreg) ? "[-d data] | [-sc bitmask]" : "");
   printf("  Options: -h  print usage information\n"
   "%s"\
   "\n"
   "  register_index: An index to the specified Cyrix 6x86 config register,\n"
   "                  In decimal (e.g. '24'), hex ('0x18') or octal ('030').\n"
   "\n"
   "%s\n",
   (setreg) ? \
   "           -p  silent mode: don't spread the news (no output)\n"\
   "           -r  `register_index' specifies the register to use\n"\
   "           -d  `data' is a raw value to program into the register\n"\
   "           -s  `data' is a bitmask for setting bits in the register\n"\
   "           -c  `data' is a bitmask for clearing bits in the register\n"\
   : \
   "           -p  pipeable output: only hex register contents are printed\n",\
   (setreg) ? \
   "  data: for the -d option, the raw data to program into the specified\n"\
   "        register (dec|hex|oct), or the bitmask for the -s and -c options.\n"\
   : \
   "");
}


/***********************************************************************************************************/

int main (int argc, char* argv[])
{
   int c;
   int tmpbyte=0;
   int regnum=0;
   bool setreg = FALSE;
   bool silent = FALSE;
   bool arg_is_setmask=FALSE;
   bool arg_is_clearmask=FALSE;
   char* commandfilename;
   int data=0;


   // See what action is required: read or write

   CommandName = argv[0];
   commandfilename = strrchr(CommandName, '\\');

   if (commandfilename)
      commandfilename++;
   else
      commandfilename = CommandName;

   setreg = (!strnicmp(commandfilename,"set",3));



   // command-line argument parsing

   while ((c = getopt (argc, argv, "hpr:d:s:c:")) != EOF) {
      switch (c) {

         case 'h': usage(setreg);
                   exit(0);
                   break;
         case 'p': silent = TRUE;
                   break;
         case 'r': regnum = getint( optarg, "register index", 0, 255 );
                   break;
         case 'd': data = getint( optarg, "register data", 0, 255 );
                   break;
         case 's': arg_is_setmask = TRUE;
                   data = getint( optarg, "register data", 0, 255 );
                   break;
         case 'c': arg_is_clearmask = TRUE;
                   data = getint( optarg, "register data", 0, 255 );
                   break;
         case '?': usage(setreg);
                   exit(1);
                   break;
         default:  fprintf(stderr,"getopt returned unknown token '%c'.\n",c);
                   exit(1);
      }
   }

   get_IO_range(0x22, 2);

   if (setreg) {
      if (arg_is_clearmask || arg_is_setmask) {

         outb(regnum, 0x22); tmpbyte = inb(0x23);  // read original data

         if (arg_is_clearmask)
            data = tmpbyte & ~data;
         if (arg_is_setmask)
            data = tmpbyte | data;
      }

      outb(regnum, 0x22); outb(data, 0x23);          // write data
   }

   outb(regnum, 0x22); tmpbyte = inb(0x23);         // read back data

   if (!silent)
       printf("Cyrix 6x86 config register, index %d (=0x%x) contains %d (=0x%02x =b%s)\n",
               regnum, regnum, tmpbyte, tmpbyte, int_to_bin(tmpbyte,8));

   else if (!setreg)                // "getreg" needs to output _something_
           printf("0x%x\n", tmpbyte);


   return(0);

}  // end main
