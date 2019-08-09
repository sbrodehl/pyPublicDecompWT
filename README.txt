OVERVIEW
This product contains the source-code for the xRITDecompress tool. 
The tool can be build under Windows, Linux and Solaris (see build instructions).
Note that it only decompresses compressed XRIT files.
It includes decoder for JPEG,WT and T4.


BUILD INSTRUCTIONS
Unzip the source-code package in an empty directory (called <root>)

Linux and Solaris:
Required software:
	GNU-GCC (g++, version 4.2.x or previous version 3.x)
	GNU-make (make, version 3.79.1)
	ar program
	ranlib program
Instructions:
Change to the <root>/xRITDecompress folder and call 'make'
The compiled image will be stored in the <root>/xRITDecompress directory (image name xRITDecompress)

Windows:
Required software: Eclipse 3.2.2 or CygWin
Eclipse Instructions:
Select as workspace the folder 'Extras'. The project 'Eclipse_project' will be loaded automatically. 
It can be rebuilt or executed directly choosing the executble placed in the folder 'Release'.
Notice that one image should be used as argument. 

CygWin Instructions:
Change to the <root>/xRITDecompress folder and call 'make'
The compiled image will be stored in the <root>/xRITDecompress directory (image name xRITDecompress)


USAGE
You can specify:
 --help: prints this list of options, stops execution
 --version: prints current version, stops execution
 -s:* source file (compulsory)
 BITS=32 for 32 bits platforms or BITS=64 for 64 bits platforms.
Alternatively you can invoke the program with just the XRIT file name:
Call: xRITDecompress(.exe under Windows) <XRITFilename>
Output: decompressed XRITFile on the current directory (same file name without extension 'C')


FOLDERS
Data: some example uncompressed XRITFiles (original files before compression) as well as their
 compressed counterparts are provided. Decompressed files should be identical to uncompressed ones.
Image: compiled images for Linux, Solaris, CygWin and Windows.
-CygWin
-Linux_32bits
-Linux_64bits
-Windows
-Solaris9_32bits
-Solaris10_32bits


SYSTEM DEPENDENCIES:
The precompiled versions on the Image directory require the following resources:

Linux 32 bits:
Compiled for Linux Red Hat, Intel Processor 32 bits
libpthread.so.0 
libstdc++.so.5       
libm.so.6         
libgcc_s.so.1         
libc.so.6 
/lib/ld-linux.so.2 

Linux 64 bits:
Compiled for Linux 2.4.0, AMD x86 64 bits.
libpthread.so.0       
libstdc++.so.6        
libm.so.6        
libgcc_s.so.1        
libc.so.6 
/lib64/ld-linux-x86-64.so.2 



Windows:
Compiled for Windows XP. 
CygWin or Eclipse SDK 3.2.2 (MinGW 3.4.2 + Eclipse c/c++ Development Tools)
Cygwin1.dll required for Cygwin compilations.


TEST:
Under Linux/Unix/Cygwin you can run make testDx to check if it works.
Under Windows you should execute the program with the segments under Data/compressed_ref folder and
 compare the results with data at Data/decompressed

