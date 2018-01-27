Orcus - library for processing spreadsheet documents.
=====================================================

Orcus is a library that provides a collection of standalone file processing
filters.  It is currently focused on providing filters for spreadsheet
documents, but filters for other productivity application types (such as
wordprocessor and presentation) are in consideration.

The library currently includes the following import filters:

* Microsoft Excel 2007 XML
* Microsoft Excel 2003 XML
* Open Document Spreadsheet
* Plain Text
* Gnumeric XML
* Generic XML

The library also includes low-level parsers for the following:

* CSV
* CSS
* XML
* JSON
* YAML

These parsers are all implemented as C++ templates and require a handler class
passed as a template argument so that the handler class receives various
callbacks from the parser as the file is being parsed.

## API Documentation

* [Official API documentation](http://kohei.us/files/orcus/doc/)
* [Doxygen code documentation](http://kohei.us/files/orcus/doxygen/)

## Download source packages

| Version | API Version | Release Date | Download | Checksum | File Size (bytes) |
|---------|-------------|--------------|----------|----------|-------------------|
| 0.13.2 | 0.13 | 2018-01-26 | [liborcus-0.13.2.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.13.2.tar.xz) | sha256sum: 9eb69cdd636bdaea22c9073c9f480ebaae05ef62bd483d68c8d4c05af927ea43 | 1816884 |
|        |      |            | [liborcus-0.13.2.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.13.2.tar.gz) | sha256sum: 0f8e85dc163881c358bd175227a970e3c70443109e2bb0d8d422439c31641df2 | 2326515 |
| 0.13.1 | 0.13 | 2017-11-15 | [liborcus-0.13.1.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.13.1.tar.xz) | sha256sum: bce854ae699468305c08f97225d49d2013888f96d5f57baf535ea759ce04a9e6 | 1814552 |
|        |      |            | [liborcus-0.13.1.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.13.1.tar.gz) | sha256sum: d7041ef455bb78db66b4ba7876af1b3d0fa377b9444e3ef72ceaccd2e8400937 | 2323107 |
| 0.13.0 | 0.13 | 2017-08-15 | [liborcus-0.13.0.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.13.0.tar.xz) | sha256sum: 08c779722471d49f38de30dad538dbf3ae1c26eb9aeb7f5eb5ca64516513e6d7 | 1812468 |
|        |      |            | [liborcus-0.13.0.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.13.0.tar.gz) | sha256sum: 1b03e1970aca31ecceae2d6412c4ead23d727c7c655efc26cf49d4ed83ba36e2 | 2309677 |
| 0.12.1 | 0.12 | 2016-09-18 | [liborcus-0.12.1.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.12.1.tar.xz) | sha256sum: d1b936c66944d23e1b2582d0e7129e44670052510d03f19fef644e9814ae2b9c | 1673880 |
|        |      |            | [liborcus-0.12.1.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.12.1.tar.gz) | sha256sum: 676b1fedd721f64489650f5e76d7f98b750439914d87cae505b8163d08447908 | 2117890 |
| 0.12.0 | 0.12 | 2016-08-21 | [liborcus-0.12.0.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.12.0.tar.xz) | sha256sum: a0b904c4c501a4428cacf1178b2a0c4c8dc89fcade8d0310f4826a32495750df | 1672940 |
|        |      |            | [liborcus-0.12.0.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.12.0.tar.gz) | sha256sum: 0c3ea6cee3700aeab20878d7d4d8a03b5fb594b81cbb2bb1ff6b603847fd6568 | 2116641 |
| 0.11.2 | 0.11 | 2016-05-11 | [liborcus-0.11.2.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.11.2.tar.xz) | sha256sum: 6911dc65a4d8276e42d32764e9ebee920b325cf59cfa7070fc72744d43b259ad | 1627540 |
|        |      |            | [liborcus-0.11.2.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.11.2.tar.gz) | sha256sum: 10afc617fd7600fa02bd4467d2e3c7bd058f84e4d672d558e1db90e82dafd256 | 2034926 |
| 0.11.1 | 0.11 | 2016-03-11 | [liborcus-0.11.1.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.11.1.tar.xz) | sha256sum: 02a6251a9b732c2c555c1be083411a2d0383dfd2ebc79f60ac282875b1ada49b | 1627332 |
| | | | [liborcus-0.11.1.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.11.1.tar.gz) | sha256sum: cbfa4cf777b3b5201661e68a4b3df9f6ed721d077f0a284a7b43b3b450b6c2f1 | 2034831 |
| 0.11.0 | 0.11 | 2016-03-03 | [liborcus-0.11.0.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.11.0.tar.xz) | sha256sum: bc4eefe44b3237004df7edb8b98cf74ac4e18f117b901e08c2b435ec339b0f97 | 1624972 |
| | | | [liborcus-0.11.0.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.11.0.tar.gz) | sha256sum: 7af7b9993801dd0b1ccdf734f51c1617975f92bcb9396c4de49ed88444e5b357 | 2032540 |
| 0.9.2 | | 2015-06-18 | [liborcus-0.9.2.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.9.2.tar.xz) | md5sum: 3ff918cc988cb325e12d8bbc7f8c3deb<br/>sha1sum: 4c55f1bdc65490e8e79bbf7d069a64381eb1d1bc | - |
| | | | [liborcus-0.9.2.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.9.2.tar.gz) | md5sum: e6efcbe50a5fd4d50d513c9a7a4139b0<br/>sha1sum: f44c7ec7bd0c54f506efe3cb600bca4cd1401638 | - |
| 0.9.1 | | 2015-05-17 | [liborcus-0.9.1.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.9.1.tar.xz) | md5sum: 88d24d9d8c5cc9014c1e842a4f612921<br/>sha1sum: 2115ccccad88c528bc9d3ac5d0cc287f80f03529 | - |
| | | | [liborcus-0.9.1.tar.gz](http://kohei.us/files/orcus/src/liborcus-0.9.1.tar.gz) | md5sum: 18814358772ed7bb476e04b0384af082<br/>sha1sum: c9755028ef50c518c5a17fb78ddf9e85519999c0 | - |
| 0.9.0 | | 2015-04-18 | [liborcus-0.9.0.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.9.0.tar.xz) | md5sum: 3f64f27b9fd59e55ca4c5ba95cd32da0<br/>sha1sum: 72d1bdb7fbaec9adce36ed728e08e16b951388e3 | - |
| 0.7.1 | | 2015-02-24 | [liborcus-0.7.1.tar.xz](http://kohei.us/files/orcus/src/liborcus-0.7.1.tar.xz) | md5sum: 644145470758d3ea4dd1d63582e82f8e<br/>sha1sum: 4480f519c6724ee66d76072df32e9b12d55505da | - |
| 0.7.0 | | 2013-12-14 | [liborcus-0.7.0.tar.bz2](http://kohei.us/files/orcus/src/liborcus-0.7.0.tar.bz2) | md5sum: 7681383be6ce489d84c1c74f4e7f9643<br/>sha1sum: c33e1eb55144fef1070cb0bf35a2c12198fcaa71 | - |
| 0.5.2 | | 2013-05-21 | [liborcus-0.5.2.tar.bz2](http://kohei.us/files/orcus/src/liborcus-0.5.2.tar.bz2) | md5sum: c0bd33e0ff17f469032062e2ee60ecb0<br/>sha1sum: f2638bf0b0e1715c49f1a8d356bb88a21de31dad | - |
| 0.5.1 | | 2013-04-13 | [liborcus-0.5.1.tar.bz2](http://kohei.us/files/orcus/src/liborcus-0.5.1.tar.bz2) | md5sum: ea2acaf140ae40a87a952caa75184f4d<br/>sha1sum: 9303d513e4b63a1d6e4bce7cfeb13635e568b466 | - |
| 0.5.0 | | 2013-04-11 | [liborcus-0.5.0.tar.bz2](http://kohei.us/files/orcus/src/liborcus-0.5.0.tar.bz2) | md5sum: 8a43b3de758dcd529b16ac96b46069fb<br/>sha1sum: ad76bed79b123e331f0b6dced6e9085a81b92449 | - |
| 0.3.0 | | 2012-11-28 | [liborcus_0.3.0.tar.bz2](http://kohei.us/files/orcus/src/liborcus_0.3.0.tar.bz2) | md5sum: 8755aac23317494a9028569374dc87b2<br/>sha1sum: 73b8fae832453fd517015f5dfae36448658af1a9 | - |
| 0.1.0 | | 2012-09-07 | [liborcus_0.1.0.tar.bz2](http://kohei.us/files/orcus/src/liborcus_0.1.0.tar.bz2) | md5sum: 46d9f4cf8b145c21ce1056e116d2ce71<br/>sha1sum: 7c961dd8f0bdd7ed039f305d6419be3cbdcc6cc6 | - |

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

