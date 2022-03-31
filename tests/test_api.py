from pathlib import Path

from pyPublicDecompWT import xRITDecompress

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


def test_legacy_decompression():
    compressed_dir = Path("../test-data/compressed_ref")
    uncompressed_dir = Path("../test-data/uncompressed_ref")
    for c_pt in compressed_dir.glob("*-C_"):
        uc_pt = next(iter(uncompressed_dir.glob(f"{c_pt.name[:-2]}*")))
        xrit = xRITDecompress(str(c_pt))
        pyxrit_uc_pt = Path(xrit.getAnnotationText())
        assert pyxrit_uc_pt.exists()
        with open(pyxrit_uc_pt, mode="rb") as fh:
            pyxrit_un_buf = fh.read()
        with open(uc_pt, mode="rb") as fh:
            un_buf = fh.read()
        assert pyxrit_un_buf == un_buf
        pyxrit_uc_pt.unlink()
        assert not pyxrit_uc_pt.exists()


def test_inmemory_decompression():
    compressed_dir = Path("../test-data/compressed_ref")
    uncompressed_dir = Path("../test-data/uncompressed_ref")
    for c_pt in compressed_dir.glob("*-C_"):
        uc_pt = next(iter(uncompressed_dir.glob(f"{c_pt.name[:-2]}*")))
        with open(c_pt, mode="rb") as fh:
            buf = fh.read()
            xrit = xRITDecompress()
            xrit.decompress(buf)
            pyxrit_un_buf = xrit.data()
        with open(uc_pt, mode="rb") as fh:
            un_buf = fh.read()
        assert pyxrit_un_buf == un_buf


def test_output_length():
    compressed_dir = Path("../test-data/compressed_ref")
    for c_pt in compressed_dir.glob("*-C_"):
        with open(c_pt, mode="rb") as fh:
            buf = fh.read()
            xrit = xRITDecompress()
            xrit.decompress(buf)
            pyxrit_un_buf = xrit.data()
            output_length = xrit.getOutputLength()
            assert len(pyxrit_un_buf) == output_length


def test_annotation_text():
    compressed_dir = Path("../test-data/compressed_ref")
    for c_pt in compressed_dir.glob("*-C_"):
        with open(c_pt, mode="rb") as fh:
            buf = fh.read()
            xrit = xRITDecompress()
            xrit.decompress(buf)
            annotation_text = xrit.getAnnotationText()
            assert annotation_text == c_pt.name.replace("-C_", "-__")


def test_total_header_length():
    compressed_dir = Path("../test-data/compressed_ref")
    for c_pt in compressed_dir.glob("*-C_"):
        with open(c_pt, mode="rb") as fh:
            buf = fh.read()
            xrit = xRITDecompress()
            xrit.decompress(buf)
            total_header_length = xrit.getTotalHeaderLength()
            assert total_header_length == 49584  # TODO: Is this always the same?


def test_file_type_code():
    compressed_dir = Path("../test-data/compressed_ref")
    for c_pt in compressed_dir.glob("*-C_"):
        with open(c_pt, mode="rb") as fh:
            buf = fh.read()
            xrit = xRITDecompress()
            xrit.decompress(buf)
            file_type_code = xrit.getFileTypeCode()
            assert file_type_code == 0  # 0 == Image data file


def test_segment_seq_no():
    compressed_dir = Path("../test-data/compressed_ref")
    for c_pt in compressed_dir.glob("*-C_"):
        with open(c_pt, mode="rb") as fh:
            buf = fh.read()
            xrit = xRITDecompress()
            xrit.decompress(buf)
            segment_seq_no = xrit.getSegmentSeqNo()
            assert segment_seq_no == int(c_pt.name.split('-')[-3].replace("_", ""))


def test_spectral_channel_id():
    compressed_dir = Path("../test-data/compressed_ref")
    for c_pt in compressed_dir.glob("*-C_"):
        with open(c_pt, mode="rb") as fh:
            buf = fh.read()
            xrit = xRITDecompress()
            xrit.decompress(buf)
            spectral_channel_id = xrit.getSpectralChannelID()
            assert spectral_channel_id == E_SC_CHAN_ID[c_pt.name.split('-')[-4].replace('_', '')]
