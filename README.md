# PublicDecompWT
This repository contains the source code for the PublicDecompWT tool.
The tool can be build in Windows, Linux and Solaris.
Note that it only decompresses compressed XRIT files.
It includes a decoder for JPEG, WT and T4.

# Build instructions
## Linux and Solaris
### Required software
- GNU-GCC (`g++`, tested with version 4.2.x or previous version 3.x)
- GNU-make (`make`, tested with version 3.79.1)
- `ar`
- `ranlib`

### Instructions
Navigate into `xRITDecompress` and call `make`.
The compiled image will be stored in the `xRITDecompress` directory (image name `xRITDecompress`).

## Windows
### Required software
- Eclipse (tested with 3.2.2) or CygWin

### Eclipse Instructions
Select as workspace the folder `Extras`. The project `Eclipse_project` will be loaded automatically.
It can be rebuilt or executed directly choosing the executable placed in the folder `Release`.
Notice that one image should be used as argument.

### CygWin Instructions
Build the software as described for Linux and Solaris.

# Usage
You can specify:

```
--help: prints this list of options, stops execution
--version: prints current version, stops execution
-s:* source file (compulsory)
BITS=32 for 32 bits platforms or BITS=64 for 64 bits platforms.
```

Alternatively you can invoke the program with just the XRIT input file name:
```bash
$ xRITDecompress $XRITFilename
```

In Windows the program must be invoked with the `.exe` extension, i.e.

```bash
$ xRITDecompress.exe $XRITFilename
```

The output is the decompressed XRITFile in the current directory (same file name without extension `C`).

# Testing
Under Linux/Unix/Cygwin you can run `make testDx` to verify the software works correctly.