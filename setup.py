import os
import re
import subprocess
import sys
import codecs

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

HERE = os.path.abspath(os.path.dirname(__file__))
PROJECT_URL = "https://github.com/sbrodehl/pyPublicDecompWT"
KEYWORDS = [
    "PublicDecompWT",
    "Wavelet Decompression",
    "xRITDecompress",
    "xRIT",
    "Meteosat",
    "EUMETSAT",
]
CLASSIFIERS = [
    "Development Status :: 4 - Beta",
    "Natural Language :: English",
    "License :: OSI Approved :: Apache Software License",
    "Operating System :: MacOS :: MacOS X",
    "Operating System :: POSIX :: Linux",
    "Programming Language :: Python",
    "Programming Language :: Python :: 3 :: Only",
    "Programming Language :: Python :: 3",
    "Programming Language :: Python :: 3.7",
    "Programming Language :: Python :: 3.8",
    "Programming Language :: Python :: 3.9",
    "Programming Language :: Python :: 3.10",
    "Intended Audience :: Developers",
    "Intended Audience :: Science/Research",
    "Topic :: Scientific/Engineering :: Atmospheric Science",
    "Topic :: Scientific/Engineering :: Image Processing",
]


def read(*parts):
    """
    Build an absolute path from *parts* and return the contents of the
    resulting file.  Assume UTF-8 encoding.
    """
    with codecs.open(os.path.join(HERE, *parts), "rb", "utf-8") as f:
        return f.read()


def find_eumetsat_version():
    """
    Extract __*meta*__ from META_FILE.
    """
    meta_match = re.search(
        r"version\s*:\s*[\'\"]*((?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)(?:-(?:(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+(?:[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?)[\'\"]*",
        read("PublicDecompWT.ABOUT"), re.M
    )
    if meta_match:
        return meta_match.group(1)
    raise RuntimeError("Unable to find VERSION_NUMBER string.")


def find_version():
    """
    Extract __*meta*__ from META_FILE.
    """
    meta_match = re.search(
        r"version\s*:\s*[\'\"]*((?:0|[1-9]\d*))[\'\"]*",
        read("pybind/VERSION"), re.M
    )
    if meta_match:
        return ".".join([find_eumetsat_version(), meta_match.group(1)])
    raise RuntimeError("Unable to find VERSION_NUMBER string.")


# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        # required for auto-detection & inclusion of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        # Set Python_EXECUTABLE instead if you use PYBIND11_FINDPYTHON
        # EXAMPLE_VERSION_INFO shows you how to pass a value into the C++ code
        # from Python.
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
        ]
        if not sys.platform.startswith("darwin"):
            # exclude RPATH for mac-os, see https://stackoverflow.com/questions/60384985/problem-loading-dylib-libraries-in-python-with-macos
            cmake_args += [
                "-DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE",  # https://stackoverflow.com/a/58175755 (E.g. https://github.com/sizmailov/pyxmolpp2/blob/master/setup.py#L42-L44)
                "-DCMAKE_INSTALL_RPATH={}".format("$ORIGIN"),
            ]

        build_args = []
        # Adding CMake arguments set as environment variable
        # (needed e.g. to build for ARM OSx on conda-forge)
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        # Pass in the version to C++.
        cmake_args += [f"-DPYXRITDECOMPRESS_VERSION_INFO={find_version()}"]

        if self.compiler.compiler_type != "msvc":
            # Using Ninja-build since it a) is available as a wheel and b)
            # multithreads automatically. MSVC would require all variables be
            # exported for Ninja to pick it up, which is a little tricky to do.
            # Users can override the generator with CMAKE_GENERATOR in CMake
            # 3.15+.
            if not cmake_generator:
                try:
                    import ninja  # noqa: F401

                    cmake_args += ["-GNinja"]
                except ImportError:
                    pass

        else:

            # Single config generators are handled "normally"
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})

            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", PLAT_TO_CMAKE[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"
                ]
                build_args += ["--config", cfg]

        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS - respect ARCHFLAGS if set
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                # CMake 3.12+ only.
                build_args += [f"-j{self.parallel}"]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp
        )
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=self.build_temp
        )


if __name__ == "__main__":
    setup(
        name="pyPublicDecompWT",
        version=find_version(),
        author="Sebastian Brodehl",
        author_email="foss@sbrodehl.de",
        description="Python bindings for EUMETSAT's PublicDecompWT",
        license="Apache-2.0 License",
        url=PROJECT_URL,
        project_urls={
            "Documentation": PROJECT_URL,
            "Source": PROJECT_URL,
            "Tracker": PROJECT_URL + "/issues",
        },
        long_description_content_type="text/markdown",
        long_description=read("README.md"),
        ext_modules=[CMakeExtension("pyPublicDecompWT")],
        cmdclass={"build_ext": CMakeBuild},
        include_package_data=True,
        zip_safe=False,
        python_requires=">=3.7",
        extras_require={"test": ["pytest"]},
        classifiers=CLASSIFIERS,
        scripts=['scripts/xRITDecompress'],
    )
