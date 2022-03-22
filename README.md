# pyPublicDecompWT
[![CI build](https://github.com/sbrodehl/pyPublicDecompWT/actions/workflows/trigger.yml/badge.svg)](https://github.com/sbrodehl/pyPublicDecompWT/actions/workflows/trigger.yml)

This package provides python bindings for some of [EUMETSAT’s PublicDecompWT][2] tools.

With [pyPublicDecompWT][1] compressed XRIT files can now be decompressed in-memory, directly with python!  
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

[1]: https://github.com/sbrodehl/pyPublicDecompWT
[2]: https://gitlab.eumetsat.int/open-source/PublicDecompWT
[3]: https://github.com/sbrodehl/pyPublicDecompWT/pulls
