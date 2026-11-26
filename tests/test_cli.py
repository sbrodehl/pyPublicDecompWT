"""Exercise the command line wrapper against the reference xRIT files."""

from pathlib import Path
import io
import re
import runpy
import shutil
import subprocess
import sys
import sysconfig

import pytest
import pyPublicDecompWT


ROOT = Path(__file__).resolve().parents[1]
# Exercise the installed script shipped in the wheel.
SCRIPT = Path(sysconfig.get_path("scripts")) / "xRITDecompress"
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
    inputs = tmp_path / "inputs"
    inputs.mkdir()
    copied = inputs / "renamed-input"
    shutil.copyfile(str(source), str(copied))

    result = run_cli(tmp_path, *option, copied)

    assert result.returncode == 0, result.stderr
    output_name = source.name.replace("-C_", "-__")
    expected = ROOT / "test-data" / "uncompressed_ref" / output_name
    assert (tmp_path / output_name).read_bytes() == expected.read_bytes()
    assert list(inputs.iterdir()) == [copied]
    assert result.stdout == ""


@pytest.mark.parametrize("destination", ["relative", "absolute", "existing"])
def test_cli_output_directory(tmp_path, destination):
    source = COMPRESSED[0]
    output = tmp_path / "output" / "nested"
    if destination == "existing":
        output.mkdir(parents=True)
    argument = output if destination == "absolute" else output.relative_to(tmp_path)

    result = run_cli(tmp_path, "--output", argument, source)

    assert result.returncode == 0, result.stderr
    output_name = source.name.replace("-C_", "-__")
    expected = ROOT / "test-data" / "uncompressed_ref" / output_name
    assert (output / output_name).read_bytes() == expected.read_bytes()
    assert not (tmp_path / output_name).exists()
    assert result.stdout == result.stderr == ""


@pytest.mark.parametrize("nested", [False, True])
def test_cli_invalid_output_directory(tmp_path, nested):
    blocked = tmp_path / "not-a-directory"
    blocked.write_bytes(b"existing content")
    output = blocked / "nested" if nested else blocked

    result = run_cli(tmp_path, "--output", output, COMPRESSED[0])

    assert result.returncode == 2
    assert "Cannot use output directory" in result.stderr
    assert "Traceback" not in result.stderr
    assert "Failed to decompress" not in result.stderr
    assert blocked.read_bytes() == b"existing content"


def test_cli_no_input_is_an_argument_error(tmp_path):
    result = run_cli(tmp_path, "--output", "unused-output")

    assert result.returncode == 2
    assert "No compressed HRIT/LRIT file(s) specified" in result.stderr
    assert not (tmp_path / "unused-output").exists()


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

    assert result.returncode == 1
    assert result.stderr.splitlines() == [
        "missing-file: Input file does not exist or is not a regular file",
        "Failed to decompress 1 of 1 file(s).",
    ]
    assert "Traceback" not in result.stderr


@pytest.mark.parametrize("failure", ["missing", "directory", "corrupt", "empty", "output"])
def test_cli_continues_after_file_failure(tmp_path, failure):
    assert len(COMPRESSED) >= 3, "Reference files are missing; run git lfs pull."
    first, middle, last = COMPRESSED[:3]
    output = tmp_path / "output"
    failed_input = tmp_path / "failed-input"
    if failure == "directory":
        failed_input.mkdir()
    elif failure == "corrupt":
        failed_input.write_bytes(b"not an xRIT file")
    elif failure == "empty":
        failed_input.touch()
    elif failure == "output":
        failed_input = middle
        (output / middle.name.replace("-C_", "-__")).mkdir(parents=True)

    result = run_cli(tmp_path, "--verbose", "--output", output, first, failed_input, last)

    assert result.returncode == 1
    assert str(failed_input) + ": " in result.stderr
    assert result.stderr.splitlines()[-1] == "Failed to decompress 1 of 3 file(s)."
    assert "Traceback" not in result.stderr
    assert result.stdout.count("Decompressed file: ") == 2
    for source in (first, last):
        output_name = source.name.replace("-C_", "-__")
        expected = ROOT / "test-data" / "uncompressed_ref" / output_name
        assert (output / output_name).read_bytes() == expected.read_bytes()


def test_cli_counts_multiple_failures(tmp_path):
    missing = tmp_path / "missing"
    corrupt = tmp_path / "corrupt"
    corrupt.write_bytes(b"not an xRIT file")

    result = run_cli(tmp_path, missing, COMPRESSED[0], corrupt, COMPRESSED[1])

    assert result.returncode == 1
    assert str(missing) + ": " in result.stderr
    assert str(corrupt) + ": " in result.stderr
    assert result.stderr.splitlines()[-1] == "Failed to decompress 2 of 4 file(s)."
    for source in COMPRESSED[:2]:
        output_name = source.name.replace("-C_", "-__")
        expected = ROOT / "test-data" / "uncompressed_ref" / output_name
        assert (tmp_path / output_name).read_bytes() == expected.read_bytes()


@pytest.mark.parametrize("failure", ["resolve", "construct", "read", "annotation", "serialize", "write", "close"])
def test_cli_recovers_from_processing_exceptions(tmp_path, monkeypatch, capsys, failure):
    """Inject failures that are difficult to produce portably with real files."""
    inputs = []
    for name in ("first", "failed", "last"):
        path = tmp_path / name
        path.write_bytes(name.encode())
        inputs.append(path)
    existing_output = tmp_path / "failed.out"
    existing_output.write_bytes(b"preserve existing output")

    class Decoder:
        instances = 0

        def __init__(self):
            Decoder.instances += 1
            if failure == "construct" and Decoder.instances == 2:
                raise RuntimeError("injected construction failure")

        def decompress(self, data):
            self.name = data.decode()

        def getAnnotationText(self):
            if failure == "annotation" and self.name == "failed":
                raise RuntimeError("injected annotation failure")
            return self.name + ".out"

        def data(self):
            if failure == "serialize" and self.name == "failed":
                raise RuntimeError("injected serialization failure")
            return b"decoded " + self.name.encode()

    original_resolve = Path.resolve

    def resolve(path, *args, **kwargs):
        if failure == "resolve" and path == inputs[1]:
            raise OSError("injected resolution failure")
        return original_resolve(path, *args, **kwargs)

    class FailingOutput(io.BytesIO):
        def write(self, data):
            if failure == "write":
                raise OSError("injected write failure")
            return super().write(data)

        def __exit__(self, *args):
            result = super().__exit__(*args)
            if failure == "close":
                raise OSError("injected close failure")
            return result

    original_open = open

    def open_file(path, mode="r", *args, **kwargs):
        if failure == "read" and Path(path) == inputs[1] and mode == "rb":
            raise PermissionError("injected read failure")
        if failure in ("write", "close") and Path(path) == existing_output and mode == "wb":
            return FailingOutput()
        return original_open(path, mode, *args, **kwargs)

    monkeypatch.chdir(tmp_path)
    monkeypatch.setattr(sys, "argv", [str(SCRIPT)] + list(map(str, inputs)))
    monkeypatch.setattr(pyPublicDecompWT, "xRITDecompress", Decoder)
    monkeypatch.setattr(Path, "resolve", resolve)
    monkeypatch.setattr("builtins.open", open_file)

    with pytest.raises(SystemExit) as error:
        runpy.run_path(str(SCRIPT), run_name="__main__")

    assert error.value.code == "Failed to decompress 1 of 3 file(s)."
    captured = capsys.readouterr()
    assert str(inputs[1]) + ": injected " in captured.err
    assert "Traceback" not in captured.err
    assert (tmp_path / "first.out").read_bytes() == b"decoded first"
    assert (tmp_path / "last.out").read_bytes() == b"decoded last"
    if failure == "serialize":
        assert existing_output.read_bytes() == b"preserve existing output"
