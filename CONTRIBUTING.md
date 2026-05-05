# Contributing to Orcus

## Building from Source

The Autotools build is the primary build system and is regularly tested on
Linux. The CMake build exists mainly to support Windows, where Autotools is
not available. It should work on Linux as well, but is not part of the regular
test cycle there - patches to improve CMake portability are welcome.

### Dependencies

Orcus requires the following libraries at build time:

* [Boost](http://boost.org)
* [mdds](http://gitlab.com/mdds/mdds)
* [ixion](http://gitlab.com/ixion/ixion)
* [zlib](http://www.zlib.net/)
* [Apache Arrow](https://arrow.apache.org/) (optional, required for the Apache Parquet import filter)

When building from the `master` branch, we recommend using the latest `mdds`
and `ixion` sources from their respective git repositories. Release tarballs of
those libraries may lag behind the API expected by the current `master`, leading
to build failures or test regressions.

The [orcus/build-env](https://gitlab.com/orcus/build-env) repository provides
scripts to build the full dependency stack from source. It is particularly
recommended for Windows builds, where managing these dependencies manually is
cumbersome. On Linux it is useful when the required versions are not yet
available through the system package manager. Note that it tracks commits
ahead of the latest releases of both `mdds` and `ixion`.

### Autotools

```bash
./autogen.sh
make
make install
```

To enable the Apache Parquet filter, pass `--with-parquet-filter` to
`autogen.sh`. Apache Arrow must be installed and discoverable via `pkg-config`.

```bash
./autogen.sh --with-parquet-filter
```

### CMake

```bash
mkdir build && cd build
cmake .. \
    -DMDDS_INCLUDEDIR=/path/to/mdds/include \
    -DIXION_INCLUDEDIR=/path/to/ixion/include \
    -DIXION_LIBRARYDIR=/path/to/ixion/lib
make
make install
```

Boost and zlib are located automatically via `find_package`. If they are
installed in non-standard locations, set `BOOST_ROOT` or `ZLIB_ROOT`
accordingly.

To enable the Apache Parquet filter, add `-DORCUS_WITH_PARQUET=ON` to the
`cmake` invocation. Apache Arrow must be installed and findable by CMake's
`find_package(Parquet)`.

## Running Tests

Running the test suite after a build is strongly recommended before submitting
any changes.

### Autotools

```bash
make check
```

### CMake

```bash
cd build
ctest
```

## Building Documentation

Orcus uses [Doxygen](https://www.doxygen.nl/), [Sphinx](http://sphinx-doc.org/),
and [Breathe](https://github.com/michaeljones/breathe) to build its
documentation, with [sphinx-rtd-theme](https://sphinx-rtd-theme.readthedocs.io/)
for the page theme. Doxygen extracts API documentation from the C++ sources,
while Sphinx renders the narrative docs and Python binding reference.

Most distros package Doxygen. The Python dependencies can be installed via pip:

```bash
pip install sphinx breathe sphinx-rtd-theme
```

## Reporting Issues

Please report bugs and feature requests on the
[GitLab issue tracker](https://gitlab.com/orcus/orcus/-/issues). When reporting
a bug, include the library version, your platform, and a minimal reproducer if
possible.

## Debugging Test Programs

Orcus uses libtool to manage linking of executables, which means test binaries
in the build tree are wrapper scripts rather than the actual ELF binaries. To
run a test under `gdb`, use `libtool --mode=execute` so the debugger attaches
to the real binary:

```bash
libtool --mode=execute gdb src/parser/parser-test-threaded-sax-token-parser
```
