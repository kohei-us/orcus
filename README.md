Orcus - library for processing spreadsheet documents.
=====================================================

Orcus is a library that provides a collection of standalone file processing
filters.  It is focused primarily on providing filters for spreadsheet documents.

The library currently includes the following import filters:

* Microsoft Excel 2007 XML
* Microsoft Excel 2003 XML
* Open Document Spreadsheet
* Plain Text
* Gnumeric XML
* Generic XML
* Apache Parquet (via Apache Arrow library)

The library also includes low-level parsers for the following:

* CSV
* CSS
* XML
* JSON
* YAML (experimental)

These parsers are all implemented as C++ templates and require a handler class
passed as a template argument so that the handler class receives various
callbacks from the parser as the file is being parsed.

## API Documentation

* [Official API documentation](https://orcus.readthedocs.io/en/latest/) for
  general users of the library.

## Pages

* [Old packages](OLD-DOWNLOADS.md)
* [For contributors](CONTRIBUTING.md)
