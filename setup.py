from setuptools import setup
from distutils.extension import Extension

setup(
    name="cptraceback",
    ext_modules=[
        Extension(
            'cptraceback', sources=['cptraceback_module.c'])
    ]
)
