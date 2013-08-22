REM This command file will enable the various performance and power-saving
REM features of your Cyrix CPU.  Once you have tested the settings on your
REM system, simply call this file from your STARTUP.CMD file located at
REM the root of your OS/2 system drive (create one if it isn't there) and
REM the registers will be set automatically on system startup.
REM
REM set6x86 version 1.2 or newer required

echo Cyrix 6x86MX: Suspend-on-Halt,
set6x86 -r 0xC2 -s 0x08

REM Enable protection against the Cyrix Coma bug
echo NO_LOCK,
set6x86 -r 0xC1 -s 0x10

REM MAPEN (access to CCR4/5 enabled -- needed for settings below)
set6x86 -r 0xC3 -s 0x10

echo Fast IORT,
set6x86 -r 0xE8 -s 0x07

echo Enable WT_ALLOC,
set6x86 -r 0xE9 -s 0x01

REM This enables some performance features for the memory mapped LVFB -
REM Linear Video Frame Buffer, assuming it's on address 0xE0000000 and its size
REM is 4Mb. 
REM
REM Also assumed a power-of-two amount of main memory.
REM
REM Improves CPU/video memory bandwidth by anything from 20 to 50%.
REM On the original author's Linux system, memory bandwidth goes from 
REM 44 MB/sec to 78 MB/sec.  DIVE performance under OS/2 gets a nice
REM boost from this if your video card can handle it.  My S3 765 couldn't
REM but my Matrox Millennium can.
REM
REM It is disabled here, because this is very system-dependent.
REM
REM echo Fast Lin. VidMem.
REM set6x86 -r 0xD6 -d 0xE0
REM set6x86 -r 0xD7 -d 0x00
REM set6x86 -r 0xD8 -d 0x0B
REM set6x86 -r 0xE2 -d 0x09

REM MAPEN (disable access to CCR4/5)
set6x86 -r 0xC3 -c 0x10