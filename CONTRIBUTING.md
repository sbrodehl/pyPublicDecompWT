# Contributing

Bug fixes, tests, documentation improvements, and new functionalities are welcome!
For substantial features or changes to supported platforms, please open an issue first to discuss the approach.

## Reporting bugs

Include:
- Steps to reproduce the problem.
- Expected and actual behavior.
- Your operating system, Python version, and package version.
- Relevant error messages.

Provide a small reproducer or sample input when possible.

## Development setup

Use Python 3.11 or newer in a virtual environment.
Building requires a C++14-capable compiler and native build tools; see the [README](README.md) for build requirements.

From the repository root:

```sh
git lfs install
git lfs pull
python -m pip install ".[test]"
python -m pytest tests
```

Test fixtures use Git LFS. Tests need the actual files, not LFS pointers.
After changing C++ code, make sure to reinstall the package before running tests.

## Making changes

- Keep each PR focused on one problem.
- Follow the surrounding code's conventions.
- Add or update tests for behavior changes and bug fixes.
- Update documentation when usage or requirements change.
- Keep unrelated refactoring and dependency updates in separate PRs.
- Preserve documented Python and platform support unless the change explicitly addresses it.

## Pull requests

Explain the problem and resulting behavior briefly.
Include the checks you ran and their results, and mention anything you could not test.

A few sentences and a short testing summary are usually enough.
Add detail when it helps reviewers understand a decision or tradeoff.
Mention breaking changes and link related issues when applicable.

## Responsibility for contributions

Review and understand everything you submit, including AI-assisted code and descriptions.
Verify technical claims and report testing accurately.
Contributors are responsible for addressing review feedback.
