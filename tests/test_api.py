from datetime import datetime, timedelta, timezone
import json
import os
from pathlib import Path
import subprocess
import sys

import pytest

from pyPublicDecompWT import xRITDecompress

TEST_DATA = Path(__file__).resolve().parents[1] / "test-data"
# One timestamp covers each format: HRV/VIS006, plus uncompressed prologue/epilogue.
COMPRESSED_FILES = sorted((TEST_DATA / "compressed_ref").glob("*-202105260000-C_"))
UNCOMPRESSED_FILES = sorted((TEST_DATA / "uncompressed_ref").glob("*-202105260000-__"))
LFS_POINTER = b"version https://git-lfs.github.com/spec/v1"
RESULT_ACCESSORS = (
    "data",
    "getOutputLength",
    "getTotalHeaderLength",
    "getSpectralChannelID",
    "getSegmentSeqNo",
    "getFileTypeCode",
    "getTimeStamp",
    "getAnnotationText",
)

assert COMPRESSED_FILES, "Missing compressed test data; fetch the Git LFS fixtures."
assert UNCOMPRESSED_FILES, "Missing uncompressed test data; fetch the Git LFS fixtures."


E_SC_CHAN_ID = {
    "NO_CHANNEL": 0,
    "VIS006": 1,
    "VIS008": 2,
    "IR_016": 3,
    "IR_039": 4,
    "WV_062": 5,
    "WV_073": 6,
    "IR_087": 7,
    "IR_097": 8,
    "IR_108": 9,
    "IR_120": 10,
    "IR_134": 11,
    "HRV": 12,
    "UnknownChannel": 13,
}


def read_fixture(path):
    data = path.read_bytes()
    assert not data.startswith(LFS_POINTER), f"Fetch the Git LFS content for {path}."
    return data


def header_offset(data, record_type):
    offset = 0
    while offset < int.from_bytes(data[4:8], "big"):
        if data[offset] == record_type:
            return offset
        offset += int.from_bytes(data[offset + 1:offset + 3], "big")
    pytest.fail(f"Reference file has no header of type {record_type}")


def read_reference(path):
    expected_path = TEST_DATA / "uncompressed_ref" / path.name.replace("-C_", "-__")
    return path, read_fixture(path), read_fixture(expected_path)


@pytest.fixture(scope="module")
def reference():
    # Accessor and lifecycle checks do not need every reference image.
    return read_reference(COMPRESSED_FILES[0])


@pytest.fixture
def decoded(reference):
    xrit = xRITDecompress()
    xrit.decompress(reference[1])
    return xrit


def test_legacy_decompression(reference, tmp_path, monkeypatch):
    path, _, expected = reference
    monkeypatch.chdir(tmp_path)
    xrit = xRITDecompress(str(path))
    assert xrit.getOutputLength() == len(expected)
    output_path = Path(xrit.getAnnotationText())
    assert output_path.read_bytes() == expected


@pytest.mark.parametrize("path", COMPRESSED_FILES, ids=lambda path: path.name)
def test_inmemory_decompression(path):
    _, data, expected = read_reference(path)
    decoder = xRITDecompress()
    decoder.decompress(data)
    assert decoder.data() == expected


def test_output_length(reference, decoded):
    assert decoded.getOutputLength() == len(reference[2])
    output = decoded.data()
    assert decoded.getOutputLength() == len(output) == len(reference[2])


def test_annotation_text(reference, decoded):
    assert decoded.getAnnotationText() == reference[0].name.replace("-C_", "-__")


def test_total_header_length(decoded):
    assert decoded.getTotalHeaderLength() == 49584  # Bits, for these reference files.


def test_file_type_code(decoded):
    assert decoded.getFileTypeCode() == 0  # Image data file.


def test_timestamp_matches_utc_header(reference, decoded):
    offset = header_offset(reference[1], 5)
    days = int.from_bytes(reference[1][offset + 4:offset + 6], "big")
    milliseconds = int.from_bytes(reference[1][offset + 6:offset + 10], "big")
    expected = datetime(1958, 1, 1, tzinfo=timezone.utc) + timedelta(days=days, milliseconds=milliseconds)
    assert decoded.getTimeStamp() == expected.strftime("%Y%m%d%H%M%S")


@pytest.mark.parametrize("tz", ["UTC0", "EST5EDT"])
def test_timestamp_is_independent_of_timezone(tmp_path, tz):
    epoch = datetime(1958, 1, 1, tzinfo=timezone.utc)
    dates = (
        epoch,
        datetime(2025, 1, 15, 12, 34, 56, 789000, tzinfo=timezone.utc),
        datetime(2025, 7, 15, 12, 34, 56, 789000, tzinfo=timezone.utc),
        epoch + timedelta(days=65535, hours=23, minutes=59, seconds=59, milliseconds=999),
    )
    source = read_fixture(COMPRESSED_FILES[0])
    offset = header_offset(source, 5)
    cases = []
    for index, date in enumerate(dates):
        elapsed = date - epoch
        milliseconds = elapsed.seconds * 1000 + elapsed.microseconds // 1000
        data = bytearray(source)
        data[offset + 4:offset + 6] = elapsed.days.to_bytes(2, "big")
        data[offset + 6:offset + 10] = milliseconds.to_bytes(4, "big")
        path = tmp_path / str(index)
        path.write_bytes(data)
        cases.append((str(path), date.strftime("%Y%m%d%H%M%S")))

    script = """
import json, os, sys, time
from pathlib import Path
from pyPublicDecompWT import xRITDecompress
if hasattr(time, "tzset"):
    time.tzset()
for path, expected in json.loads(sys.argv[1]):
    source = Path(path).read_bytes()
    decoder = xRITDecompress()
    decoder.decompress(source)
    assert decoder.getTimeStamp() == expected, (os.environ["TZ"], expected, decoder.getTimeStamp())
    offset = int(sys.argv[2])
    assert decoder.data()[offset + 4:offset + 10] == source[offset + 4:offset + 10]
assert os.environ["TZ"] == sys.argv[3]
"""
    result = subprocess.run(
        [sys.executable, "-c", script, json.dumps(cases), str(offset), tz],
        cwd=tmp_path, env={**os.environ, "TZ": tz}, capture_output=True, text=True,
        timeout=30,
    )
    assert result.returncode == 0, result.stdout + result.stderr


def test_segment_seq_no(reference, decoded):
    assert decoded.getSegmentSeqNo() == int(reference[0].name.split("-")[-3].replace("_", ""))


def test_spectral_channel_id(reference, decoded):
    channel = reference[0].name.split("-")[-4].replace("_", "")
    assert decoded.getSpectralChannelID() == E_SC_CHAN_ID[channel]


def test_repeated_data_does_not_append(reference, decoded):
    first = decoded.data()
    assert isinstance(first, bytes)
    for _ in range(3):
        repeated = decoded.data()
        assert repeated is first
        assert repeated == reference[2]
        assert decoded.getOutputLength() == len(first)


@pytest.mark.parametrize("path", UNCOMPRESSED_FILES, ids=lambda path: path.name)
def test_uncompressed_passthrough(path):
    expected = read_fixture(path)
    xrit = xRITDecompress()
    xrit.decompress(expected)
    assert xrit.getOutputLength() == len(expected)
    assert xrit.data() == expected
    assert xrit.getOutputLength() == len(expected)


@pytest.mark.parametrize("arguments", [(), (b"",)], ids=["omitted", "empty"])
def test_decompression_requires_input(arguments):
    with pytest.raises(ValueError, match="Input file or buffer not specified"):
        xRITDecompress().decompress(*arguments)


def test_invalid_buffer_raises():
    with pytest.raises(RuntimeError, match="Opening/reading input file failed"):
        xRITDecompress().decompress(b"not an xRIT file")


def test_missing_input_path_raises(tmp_path):
    with pytest.raises(RuntimeError, match="Opening/reading input file failed"):
        xRITDecompress(str(tmp_path / "missing-input"))


@pytest.mark.parametrize("accessor", RESULT_ACCESSORS)
def test_result_requires_successful_decompression(accessor):
    with pytest.raises(RuntimeError, match="No decompressed result is available"):
        getattr(xRITDecompress(), accessor)()


@pytest.mark.parametrize("cache_output", [False, True])
def test_decoder_reuse_replaces_result(cache_output):
    decoder = xRITDecompress()
    for path in (COMPRESSED_FILES[0], COMPRESSED_FILES[-1], COMPRESSED_FILES[0]):
        expected = read_fixture(TEST_DATA / "uncompressed_ref" / path.name.replace("-C_", "-__"))
        decoder.decompress(read_fixture(path))
        assert decoder.getOutputLength() == len(expected)
        assert decoder.getAnnotationText() == path.name.replace("-C_", "-__")
        assert decoder.getSegmentSeqNo() == int(path.name.split("-")[-3].replace("_", ""))
        assert decoder.getSpectralChannelID() == E_SC_CHAN_ID[path.name.split("-")[-4].replace("_", "")]
        if cache_output:
            assert decoder.data() == expected
    assert decoder.data() == expected


@pytest.mark.parametrize("cache_output", [False, True])
@pytest.mark.parametrize("failure", ["empty", "invalid", "unsupported_compression"])
def test_failed_decompression_invalidates_result_and_recovers(reference, cache_output, failure):
    decoder = xRITDecompress()
    decoder.decompress(reference[1])
    if cache_output:
        previous_output = decoder.data()

    invalid = b"" if failure == "empty" else b"not an xRIT file"
    if failure == "unsupported_compression":
        invalid = bytearray(reference[1])
        offset = header_offset(invalid, 128)  # Segment identification header.
        size = int.from_bytes(invalid[offset + 1:offset + 3], "big")
        invalid[offset + size - 1] = 255  # Unknown data-field representation.
        invalid = bytes(invalid)

    error = ValueError if failure == "empty" else RuntimeError
    with pytest.raises(error):
        decoder.decompress(invalid)
    for accessor in RESULT_ACCESSORS:
        with pytest.raises(RuntimeError, match="No decompressed result is available"):
            getattr(decoder, accessor)()

    decoder.decompress(reference[1])
    assert decoder.getOutputLength() == len(reference[2])
    assert decoder.getAnnotationText() == reference[0].name.replace("-C_", "-__")
    assert decoder.getTotalHeaderLength() == 49584
    assert decoder.data() == reference[2]
    if cache_output:
        assert previous_output == reference[2]


def test_failed_file_reuse_invalidates_result(reference, tmp_path, monkeypatch):
    source = tmp_path / "input"
    source.write_bytes(reference[1])
    monkeypatch.chdir(tmp_path)
    decoder = xRITDecompress(str(source))
    assert decoder.data() == reference[2]
    source.unlink()

    with pytest.raises(RuntimeError, match="Opening/reading input file failed"):
        decoder.decompress()
    for accessor in RESULT_ACCESSORS:
        with pytest.raises(RuntimeError, match="No decompressed result is available"):
            getattr(decoder, accessor)()
