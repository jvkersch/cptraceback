from setuptools import setup
from distutils.extension import Extension


setup(
    name="cytraceback",
    ext_modules=[
        Extension(
            'cytraceback.cytraceback',
            sources=['cytraceback/cytraceback.pyx',
                     'cytraceback/backtrace.cc',
                     'cytraceback/cbacktrace.cc'],
            language="c++")
    ]
)
