import ctypes
from pathlib import Path

if __name__ == '__main__':
    lib_path = Path("../dist/lib/libxRIT.so").resolve().absolute()
    assert lib_path.exists()
    libxRIT = ctypes.CDLL(str(lib_path))

    class xRIT(object):
        def __init__(self, data):
            libxRIT.xRITWrapper_Constructor.argtypes = (ctypes.POINTER(ctypes.c_char), ctypes.c_int)
            libxRIT.xRITWrapper_Constructor.restype = ctypes.c_void_p

            libxRIT.xRITWrapper_getOutputLength.argtypes = [ctypes.c_void_p]
            libxRIT.xRITWrapper_getOutputLength.restype = ctypes.c_int

            libxRIT.xRITWrapper_write.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
            libxRIT.xRITWrapper_write.restype = ctypes.c_void_p

            num_bytes = len(data)
            array_type = (ctypes.c_char * num_bytes)(*data)
            self.xRITWrapper = libxRIT.xRITWrapper_Constructor(array_type, ctypes.c_int(num_bytes))

        def output(self):
            _num_bytes = libxRIT.xRITWrapper_getOutputLength(self.xRITWrapper)
            _string_buffer = ctypes.create_string_buffer(_num_bytes + 1)  # account for NULL terminator
            _pointer = ctypes.c_char_p(ctypes.addressof(_string_buffer))
            libxRIT.xRITWrapper_write(self.xRITWrapper, _pointer)
            return _string_buffer.raw

    # this is a valid, but random sample xRIT file
    in_buffer = b'......'  # not a valid xRIT file
    # output_buffer = xRIT(in_buffer).output()
