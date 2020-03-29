
TODO: Organize this page with more info.


# Linux

## Building from source code

Orcus uses autoconf and automake as its build system.  As such, building it
from sources should be familiar to those who are used to these tools.

In short, run the following command:

```bash
./autogen.sh
make
make install
```

at the root directory after either cloning from the repository or unpacking
the source package.

### Build dependencies

Orcus has build-time dependency on the following libraries:

* [boost](http://boost.org)
* [mdds](http://gitlab.com/mdds/mdds)
* [ixion](http://gitlab.com/ixion/ixion)
* [zlib](http://www.zlib.net/)

Note that when you are building from the master branch of the git repository,
we recommend that you also use the latest mdds source code from its git
repository for the build as well as the latest ixion library built from its
git repository, else you may encounter build issues or test failures.

### Building documentation

Orcus uses a combination of [Doxygen](http://www.stack.nl/~dimitri/doxygen/),
[Sphinx](http://sphinx-doc.org/) and [Breathe](https://github.com/michaeljones/breathe)
to build its documentation.  It also use [Sphinx Bootstrap Theme](https://ryan-roemer.github.io/sphinx-bootstrap-theme/)
for the page layout and theme.

Most distros package Doxygen, and Sphinx, Breathe and Sphinx Bootstrap Theme
can be installed via pip.

## Debugging test programs

Orcus uses libtool to manage linking of executables.  When you have a test
failure and you wish to debug it by running it under, say, `gdb`, you need
to use `libtool` to wrap the libtool wrapper script that wraps the actual
executable binary.  For instance, the following command executes the test
program named `parser-test-threaded-sax-token-parser` under `gdb` with the
help of `libtool`:

```bash
libtool --mode=execute gdb src/parser/parser-test-threaded-sax-token-parser
```

