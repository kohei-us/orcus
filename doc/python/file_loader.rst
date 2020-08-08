
File Loader
===========

.. py:module:: orcus.xlsx

.. py:function:: read_file

   Read an Excel file from a specified file path and create a
   :py:class:`orcus.Document` instance object.  The file must be of Excel 2007
   XML format.

   :param filepath: file path.
   :rtype: :py:class:`orcus.Document`
   :return: document instance object that stores the content of the file.

   Example::

      from orcus import xlsx

      doc = xlsx.read_file("/path/to/file.xlsx")
