# Makefile for set6x86, a program to get/set Cyrix 6x86 CPU config settings 
# from GNU/Linux, modified for emx for OS/2
CC = gcc
CFLAGS = -O2 -Wall
VERSION=1.5
#############################################################################
_VERSION = -DVERSION=\"$(VERSION)\"

all: set6x86 get6x86 6x86_reg
 
set6x86: set6x86.c
	$(CC) $(CFLAGS) $(_VERSION) -o set6x86.exe set6x86.c

get6x86: set6x86
	copy set6x86.exe get6x86.exe

6x86_reg: 6x86_reg.c
	$(CC) $(CFLAGS) $(_VERSION) -o 6x86_reg.exe 6x86_reg.c

clean:
	del get6x86.exe set6x86.exe 6x86_reg.exe *.o
