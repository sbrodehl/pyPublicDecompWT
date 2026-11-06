"""Exercise the command line wrapper against the reference xRIT files."""

from pathlib import Path
import re
import shutil
import subprocess
import sys

import pytest
import pyPublicDecompWT


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "xRITDecompress"
COMPRESSED = sorted((ROOT / "test-data" / "compressed_ref").glob("*-C_"))


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


@pytest.mark.parametrize("option", [[], ["-s"]])
def test_cli_decompresses_file(tmp_path, option):
    assert COMPRESSED, "Reference files are missing; run git lfs pull."
    source = COMPRESSED[0]
    copied = tmp_path / source.name
    shutil.copyfile(str(source), str(copied))

    result = run_cli(tmp_path, *option, copied)

    assert result.returncode == 0, result.stderr
    output_name = source.name.replace("-C_", "-__")
    expected = ROOT / "test-data" / "uncompressed_ref" / output_name
    assert (tmp_path / output_name).read_bytes() == expected.read_bytes()
    assert result.stdout == ""


def test_cli_decompresses_batch_verbose(tmp_path):
    assert len(COMPRESSED) >= 2, "Reference files are missing; run git lfs pull."
    copied = []
    for source in COMPRESSED[:2]:
        target = tmp_path / source.name
        shutil.copyfile(str(source), str(target))
        copied.append(target)

    result = run_cli(tmp_path, "--verbose", *copied)

    assert result.returncode == 0, result.stderr
    for source in copied:
        output_name = source.name.replace("-C_", "-__")
        expected = ROOT / "test-data" / "uncompressed_ref" / output_name
        assert (tmp_path / output_name).read_bytes() == expected.read_bytes()
        assert "Decompressed file: " + output_name in result.stdout


def test_cli_missing_file(tmp_path):
    result = run_cli(tmp_path, "missing-file")

    assert result.returncode == 2
    assert "Input file does not exist" in result.stderr
    assert "Traceback" not in result.stderr
