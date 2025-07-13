# setup.py
from setuptools import setup, Extension
from Cython.Build import cythonize

extensions = [
    Extension(
        "libkeypass",
        sources=[
            "Crypto/BLAKE2s.cpp",
            "Crypto/BlockCipher.cpp",
            "Crypto/ChaCha.cpp",
            "Crypto/Cipher.cpp",
            "Crypto/Crypto.cpp",
            "Crypto/Hash.cpp",
            "String.cpp",
            "_keypass.cpp",
            "crypto.cpp",
            "importexport.cpp",
            "libkeypass.pyx",
            "utils.cpp",
        ],
        include_dirs=[".", "Crypto"],  # Include current directory
        extra_compile_args=["-W", "-Wall", "-pedantic"],
        language="c++",
    )
]

setup(
    ext_modules=cythonize(extensions),
)
