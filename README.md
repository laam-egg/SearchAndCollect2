# SearchAndCollect2

## Purpose

Training Machine Learning models to classify malware/benign
PE files needs a balanced training dataset, that is,
the number of malware files equal that of benign ones.

However, benign PE files are often not released in datasets
due to copyright restrictions. In most cases, only extracted
and preprocessed features are public.

Therefore, I wrote this program **to collect benign PE files**
from a fresh Windows installation.

Idea and some parts of the source code are based on this
project:

<https://github.com/IOActive/SearchAndCollect>

## But What is New?

Compared to the original project, this has the following
strengths:

- Written entirely in C99, no C++, with minimum heap
    allocations for optimized performance. Uses CMake
    as the build system instead of direct Visual Studio
    tools (like the original project). This way, you
    can even get the source code to compile on aging
    hardware e.g. using CMake + MinGW (just the C
    compiler toolchain, no need for C++), without
    installing the whole bulky Visual Studio.
- Supports [long path names](https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=registry)
    (i.e. paths exceeding 260 characters) and Unicode
    paths, too! It is in fact Unicode-native. No `char`,
    only `wchar_t`.
- Uses only Win API as "external dependencies" (while the
    original uses OpenSSL).
- Includes option to append random bytes to copied files.
    [Read on](#notes-on-output-files) to see the reason why.

It inherits the good points:

- Inspect the DOS header to deduce whether a given file
    is a PE file, instead of relying on file extensions.

Besides, this tool supposedly supports Windows Vista and
above, though I've only tested it on Windows 10 and 11.
If you run into problems compiling this, please let me
know in the Issues tab on GitHub.

## Download

Go to [the project's Release page on GitHub](https://github.com/laam-egg/SearchAndCollect2/releases).

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

# Collects all PE files from C:/input/dir to D:/output/dir
# This tool is aware of Unicode paths!
# If a path contains spaces, wrap it in a pair of double quotes ("")
./SearchAndCollect2.exe C:/input/dir D:/output/dir

# Append random bytes to copied files to change the files'
# signatures
./SearchAndCollect2.exe C:/input/dir D:/output/dir --append-random-bytes

# Prints help text for details on usage!
./SearchAndCollect2.exe
```

## Notes on Output Files

Every copied file is named after the original file's
SHA256 hash, while retaining the original file extension.

If you specify `--append-random-bytes`, only the
content of the files will differ; the file extension
is still retained, and the file name is still named
after the hash of the original file. Since the file
content is now different, its new hash changes
drastically, which might confuse signature-based
malware detectors. So this option would come
in handy when malware detection methods need to
be evaluated in varying situations.

## License

This tool shall be distributed and used under the terms
of the MIT license, as documented below.

    Copyright 2025 Vu Tung Lam
    
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:
    
    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
