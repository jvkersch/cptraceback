import platform

from setuptools import setup
from distutils.extension import Extension


PLATFORM = platform.system().lower()
if PLATFORM == 'windows':
    extra_compile_args = ['-Zi', '/EHsc']
    extra_link_args = ['/DEBUG', 'dbghelp.lib']
elif PLATFORM == 'linux':
    print("LINUX")
    extra_compile_args = ['-g']
    extra_link_args = ['-lunwind']
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
