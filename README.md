# SearchAndCollect2

## Purpose

Training Machine Learning models to classify malware/benign
PE files need a balanced training dataset, that is,
number of malware files equal that of benign ones.

However, benign files are often not released in dataset due
to copyright restrictions.

Therefore, I wrote this program **to collect benign PE files**
from a fresh OS installation.

Idea and some parts of the source code are based on this
project:

<https://github.com/IOActive/SearchAndCollect>\

## But What is New?

Compared to the original project, this has the following
strengths:

- Written entirely in C, no C++, with minimum heap
    allocations for optimized performance.
- Use only Win API as "external dependencies" (while the
    original uses OpenSSL).
- Supposedly support Windows Vista and above,
    though I've only tested it on Windows 10 and 11.
- Use CMake as the build system.

It inherits the good points:

- Inspect the DOS header to deduce whether a given file
    is a PE file, instead of relying on file extensions.

## Building

Using CMake as usual.

```sh
cd $PROJECT_ROOT
mkdir build
cd build
cmake ..
cmake --build .
```

## Running

```sh
cd $PROJECT_ROOT
cd build
# Prints help
./SearchAndCollect2.exe
# Collects all PE files from C:/input/dir to D:/output/dir
./SearchAndCollect2.exe C:/input/dir D:/output/dir
```
