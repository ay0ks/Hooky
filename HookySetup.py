import setuptools
from setuptools import Extension

setuptools.setup(
    name="hooky",
    ext_modules=[
        Extension("_hooky", ["Hooky.c"], extra_compile_args=["-g", "-O2"], language="c"),
    ],
)
