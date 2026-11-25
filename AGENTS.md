# Agent instructions

Follow CONTRIBUTING.md and the build requirements in README.md.

## Project layout

- `pybind/`: Python bindings for the C++ library.
- `scripts/xRITDecompress`: Python command-line interface.
- `COMP/` and `DISE/`: mirrored PublicDecompWT C++ sources.
- `tests/`: pytest tests for the Python API and CLI.
- `test-data/`: reference fixtures stored with Git LFS.
- `.github/workflows/`: build, test, and release automation.

## Making changes

- Keep changes focused on the requested problem.
- Follow existing conventions and reuse existing helpers.
- Avoid unrelated refactoring, formatting, and dependency updates.
- Preserve documented Python versions, platforms, and public behavior unless changing them is part of the task.
- Do not touch the mirrored upstream sources.
- Add or update tests for changed behavior; reuse existing fixtures.
- Update documentation when usage or requirements change.

## Build and test

Run from the repository root in a virtual environment:

```sh
git lfs install
git lfs pull
python -m pip install ".[test]"
python -m pytest tests
```

A C++ compiler and native build tools are required.
Reinstall the package after changing C++ code.
Tests require actual Git LFS fixture contents, not pointer files.

## Reporting results

- Report the checks you ran and their results accurately.
- State which relevant checks could not be run and why.
- Keep PR descriptions brief: the problem, resulting behavior, and testing.
- Explain unusual decisions or compatibility changes when relevant.
- Avoid repeating the diff or making unsupported claims.