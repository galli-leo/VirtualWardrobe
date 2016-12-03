import os
from setuptools import find_packages
from setuptools import setup

version = '0.1dev'

here = os.path.abspath(os.path.dirname(__file__))
try:
    README = open(os.path.join(here, 'README.md')).read()
    CHANGES = ''
except IOError:
    README = CHANGES = ''

install_requires = [
    'nolearn',
    'Lasagne'
]

tests_require = [
    'mock',
    'pytest',
    'pytest-cov',
    'pytest-pep8',
]

docs_require = [
    'Sphinx',
]

setup(
    name="nolearn_utils",
    version=version,
    description="Utilities for nolearn.lasagne",
    long_description="\n\n".join([README, CHANGES]),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Programming Language :: Python :: 2.7",
    ],
    keywords="",
    author="Felix Lau",
    author_email="felixlaumon@gmail.com",
    url="https://github.com/felixlaumon/nolearn-utils",
    license="MIT",
    packages=find_packages(),
    include_package_data=True,
    zip_safe=False,
    install_requires=install_requires,
    extras_require={
        'testing': tests_require,
        'docs': docs_require,
    },
)
