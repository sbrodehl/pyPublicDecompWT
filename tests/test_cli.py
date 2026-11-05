"""Exercise the command line wrapper against the reference xRIT files."""

from pathlib import Path
import re
import subprocess
import sys

import pyPublicDecompWT


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "xRITDecompress"


def run_cli(tmp_path, *arguments):
    return subprocess.run(
        [sys.executable, str(SCRIPT), *map(str, arguments)],
        cwd=str(tmp_path),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )


def test_cli_version(tmp_path):
    result = run_cli(tmp_path, "--version")

    assert result.returncode == 0, result.stderr
    assert re.fullmatch(r"\d+\.\d+\.\d+\.\d+", pyPublicDecompWT.__version__)
    assert result.stdout.strip() == "xRITDecompress " + pyPublicDecompWT.__version__
