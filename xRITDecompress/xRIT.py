import ctypes
from pathlib import Path

if __name__ == '__main__':
    lib_path = Path("../dist/lib/libxRIT.so").resolve().absolute()
    assert lib_path.exists()
    _LIBxRIT = ctypes.CDLL(str(lib_path))

    class xRIT(object):
        def __init__(self, data):
            _LIBxRIT.xRITWrapper_Constructor.argtypes = (ctypes.POINTER(ctypes.c_char), ctypes.c_int)
            _LIBxRIT.xRITWrapper_Constructor.restype = ctypes.c_void_p

            _LIBxRIT.xRITWrapper_getOutputLength.argtypes = [ctypes.c_void_p]
            _LIBxRIT.xRITWrapper_getOutputLength.restype = ctypes.c_int

            _LIBxRIT.xRITWrapper_getSpectralChannelID.argtypes = [ctypes.c_void_p]
            _LIBxRIT.xRITWrapper_getSpectralChannelID.restype = ctypes.c_int

            _LIBxRIT.xRITWrapper_getSegmentSeqNo.argtypes = [ctypes.c_void_p]
            _LIBxRIT.xRITWrapper_getSegmentSeqNo.restype = ctypes.c_int

            _LIBxRIT.xRITWrapper_getTotalHeaderLength.argtypes = [ctypes.c_void_p]
            _LIBxRIT.xRITWrapper_getTotalHeaderLength.restype = ctypes.c_int

            _LIBxRIT.xRITWrapper_write.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
            _LIBxRIT.xRITWrapper_write.restype = ctypes.c_void_p

            _LIBxRIT.xRITWrapper_getTimeStamp.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
            _LIBxRIT.xRITWrapper_getTimeStamp.restype = ctypes.c_void_p

            num_bytes = len(data)
            array_type = (ctypes.c_char * num_bytes)(*data)
            self.xRITWrapper = _LIBxRIT.xRITWrapper_Constructor(array_type, ctypes.c_int(num_bytes))

        def output(self):
            _num_bytes = _LIBxRIT.xRITWrapper_getOutputLength(self.xRITWrapper)
            _string_buffer = ctypes.create_string_buffer(_num_bytes + 1)  # account for NULL terminator
            _pointer = ctypes.c_char_p(ctypes.addressof(_string_buffer))
            _LIBxRIT.xRITWrapper_write(self.xRITWrapper, _pointer)
            return _string_buffer.raw

        def getTimeStamp(self):
            _string_buffer = ctypes.create_string_buffer(14 + 1)  # account for NULL terminator
            _pointer = ctypes.c_char_p(ctypes.addressof(_string_buffer))
            _LIBxRIT.xRITWrapper_getTimeStamp(self.xRITWrapper, _pointer)
            return _string_buffer.value.decode("utf-8")

        def getTotalHeaderLength(self):
            return _LIBxRIT.xRITWrapper_getTotalHeaderLength(self.xRITWrapper)

        def getSpectralChannelID(self):
            return _LIBxRIT.xRITWrapper_getSpectralChannelID(self.xRITWrapper)

        def getSegmentSeqNo(self):
            return _LIBxRIT.xRITWrapper_getSegmentSeqNo(self.xRITWrapper)

    # this is a valid, but random sample xRIT file
    # in_buffer = b'......'  # not a valid xRIT file
    # output_buffer = xRIT(in_buffer)
