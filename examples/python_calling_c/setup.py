from setuptools import setup
from distutils.extension import Extension

from cytraceback import get_include


setup(
    name="Examples",
    ext_modules=[
        Extension(
            'tb',
            sources=['tb.c'],
            include_dirs=[get_include()],
            libraries=[''],
            library_dirs=[get_include()],
        )
    ]
)
