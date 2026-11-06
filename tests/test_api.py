from pathlib import Path

import pytest

from pyPublicDecompWT import xRITDecompress

TEST_DATA = Path(__file__).resolve().parents[1] / "test-data"
COMPRESSED_FILES = sorted((TEST_DATA / "compressed_ref").glob("*-C_"))
UNCOMPRESSED_FILES = sorted((TEST_DATA / "uncompressed_ref").glob("*-__"))
LFS_POINTER = b"version https://git-lfs.github.com/spec/v1"

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


@pytest.fixture(scope="module", params=COMPRESSED_FILES, ids=lambda path: path.name)
def reference(request):
    path = request.param
    expected_path = TEST_DATA / "uncompressed_ref" / path.name.replace("-C_", "-__")
    return path, read_fixture(path), read_fixture(expected_path)


@pytest.fixture
def decoded(reference):
    xrit = xRITDecompress()
    xrit.decompress(reference[1])
    return xrit


def test_legacy_decompression(reference, tmp_path, monkeypatch):
    path, _, expected = reference
    monkeypatch.chdir(tmp_path)
    xrit = xRITDecompress(str(path))
    output_path = Path(xrit.getAnnotationText())
    assert output_path.read_bytes() == expected


def test_inmemory_decompression(reference, decoded):
    assert decoded.data() == reference[2]


def test_output_length(reference, decoded):
    output = decoded.data()
    assert decoded.getOutputLength() == len(output) == len(reference[2])


def test_annotation_text(reference, decoded):
    assert decoded.getAnnotationText() == reference[0].name.replace("-C_", "-__")


def test_total_header_length(decoded):
    assert decoded.getTotalHeaderLength() == 49584  # Bits, for these reference files.


def test_file_type_code(decoded):
    assert decoded.getFileTypeCode() == 0  # Image data file.


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
    assert xrit.data() == expected
    assert xrit.data() == expected
    assert xrit.getOutputLength() == len(expected)