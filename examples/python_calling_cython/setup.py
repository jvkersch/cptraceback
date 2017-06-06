from setuptools import setup
from distutils.extension import Extension

setup(
    name="Examples",
    ext_modules=[
        Extension(
            'cymod',
            sources=['cymod.pyx'])
    ]
)
