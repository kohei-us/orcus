
Filters
=======

This section presents the API's from the ``liborcus`` part of this library,
which contains the import filters that process various file formats containing
spreadsheet document contents or contents that can be loaded into spreadsheet
documents.  It consists of the filter classes that parse the file streams and
put their contents into the document store via a set of pre-defined interfaces,
and these interfaces themselves.

This module does not contain the document store itself, which is provided by
the ``liborcus-spreadsheet-model`` module.  Alternatively, the user can
provide their own document store implementation wrapped inside a factory that
provides all required interfaces.

.. toctree::
   :maxdepth: 1

   import-filter/index.rst
   interface/index.rst
   types/index.rst
   utils.rst
   config.rst
