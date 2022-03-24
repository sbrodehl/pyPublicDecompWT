# pyPublicDecompWT
[![python versions](https://img.shields.io/pypi/pyversions/pyPublicDecompWT)](https://pypi.org/project/pyPublicDecompWT)
[![wheel](https://img.shields.io/pypi/wheel/pyPublicDecompWT)](https://pypi.org/project/pyPublicDecompWT/#files)
[![CI build](https://github.com/sbrodehl/pyPublicDecompWT/actions/workflows/trigger.yml/badge.svg)](https://github.com/sbrodehl/pyPublicDecompWT/actions/workflows/trigger.yml)

This package provides python bindings for some of [EUMETSAT’s PublicDecompWT][2] tools.

With [pyPublicDecompWT][1] compressed xRIT (HRIT/LRIT) files can now be decompressed in-memory, directly with python!  
Currently, only bindings for the tool `xRITDecompress` are available, if you need other classes / libraries please open a [pull-request][3].

This repository contains / mirrors the source code for the complete [PublicDecompWT][2] tool, as well.
Additionally, we provide CMake build files.
We tested the tool with linux and mac-os, but did not manage to build it with windows (happy to change that, please open a [pull-request][3]!)

## Installation

Install pyxRITDecompress via pip:
```bash
pip install pyPublicDecompWT
```

The following command will verify if the installation succeeded:

```bash
python -c "import pyPublicDecompWT"
```

## Requirements
Building from sources requires C/C++, including compiler and standard library support.

- C/C++ Compiler, e.g. GNU-GCC
- `CMake`

## Usage

```python
from pyPublicDecompWT import xRITDecompress

compressed_file = "test-data/compressed_ref/H-000-MSG4__-MSG4________-HRV______-000012___-202105260000-C_"
with open(compressed_file, mode="rb") as fh:
    buf = fh.read()

xRIT = xRITDecompress(buf)
uncompressed = xRIT.data()
```

### `xRITDecompress`

To comply with the original PublicDecompWT project, the pyPublicDecompWT package provides a [`xRITDecompress` executable](scripts/xRITDecompress) as well.
Our `xRITDecompress` script is a near drop-in replacement for the original.

There are two changes:
- We are not writing `Decompressed file ...` to `stdout`
- As per UNIX convention, the short option `-s` is separated by a space, not by a colon.

The former can be enabled by passing `-v` to `xRITDecompress`, however, the latter can't be emulated.
Thus, the command line call has to be changed.

In addition to the `-s` option we added a positional argument `files` to enable batch processing of (multiple) files in a single call.

```
$ xRITDecompress --help
usage: xRITDecompress [-h] [--version] [-v] [-s FILE] [files ...]

Command line tool for manual decompression of HRIT/LRIT files.

positional arguments:
  files          Name of compressed HRIT/LRIT file(s)

options:
  -h, --help     show this help message and exit
  --version      show program`s version number and exit
  -v, --verbose  Verbose mode
  -s FILE        Name of compressed HRIT/LRIT file
```

## Versioning Scheme

The versioning scheme of the python bindings is the PublicDecompWT version with the additional version of the python bindings.

Example: version 1 of the python bindings for PublicDecompWT version 2.8.1 is 2.8.1.1

[1]: https://github.com/sbrodehl/pyPublicDecompWT
[2]: https://gitlab.eumetsat.int/open-source/PublicDecompWT
[3]: https://github.com/sbrodehl/pyPublicDecompWT/pulls
