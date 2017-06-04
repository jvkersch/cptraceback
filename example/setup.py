from setuptools import setup
from Cython.Build import cythonize

setup(
    name="Example",
    ext_modules=cythonize('cymod.pyx'),
)
