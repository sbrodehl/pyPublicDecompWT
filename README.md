# pyPublicDecompWT
[![CI build](https://github.com/sbrodehl/pyPublicDecompWT/actions/workflows/trigger.yml/badge.svg)](https://github.com/sbrodehl/pyPublicDecompWT/actions/workflows/trigger.yml)

This package provides python bindings for [PublicDecompWT@eumetsat.int](https://gitlab.eumetsat.int/open-source/PublicDecompWT).

With [pyPublicDecompWT](https://github.com/sbrodehl/pyPublicDecompWT) compressed XRIT files can now be decompressed in-memory, directly with python!  
Currently, only bindings for the tool `xRITDecompress` are available, if you need other classes / libraries please open a [pull-request](https://github.com/sbrodehl/pyPublicDecompWT/pulls).

This repository contains / mirrors the source code for the complete [PublicDecompWT](https://gitlab.eumetsat.int/open-source/PublicDecompWT) tool, as well.
Additionally, we provide CMake build files.
We tested the tool with linux and mac-os, but did not manage to build it with windows (happy to change that, please open a [pull-request](https://github.com/sbrodehl/pyPublicDecompWT/pulls)!)

## Install

Install pyPublicDecompWT via pip:
```bash
pip install pyPublicDecompWT
```

The following command will verify if the installation succeeded:

```bash
python -c "import pyPublicDecompWT"
```

### Dependencies
- GNU-GCC (`g++`, tested with version 4.2.x or previous version 3.x)
- GNU-make (`make`, tested with version 3.79.1)
- `ar`
- `ranlib`
- `nmake` as part of the Native Tools for VS
