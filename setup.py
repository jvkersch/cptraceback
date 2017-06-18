import platform

from setuptools import setup
from distutils.extension import Extension


if platform.system().lower() == 'windows':
    extra_compile_args = ['-Zi', '/EHsc']
    extra_link_args = ['/DEBUG', 'dbghelp.lib']
else:
    extra_compile_args = ['-g']
    extra_link_args = []

setup(
    name="cptraceback",
    ext_modules=[
        Extension(
            'cptraceback',
            sources=['cptraceback_module.c'],
            extra_compile_args=extra_compile_args,
            extra_link_args=extra_link_args)
    ]
)
