
Spreadsheet Document
====================

.. py:module:: orcus

.. py:class:: Document

   An instance of this class represents a document model.  A document consists
   of multiple sheet objects.

   .. py:attribute:: sheets

      Read-only attribute that stores a tuple of :py:class:`.Sheet` instance
      objects.

   .. py:function:: get_named_expressions

      Get a named expressions iterator.

      Returns (:obj:`.NamedExpressions`):
          Named expression object.


.. py:class:: Sheet

   An instance of this class represents a single sheet inside a document.

   .. py:function:: get_rows

      This function returns a row iterator object that allows you to iterate
      through rows in the data region.

      :rtype: :py:class:`.SheetRows`
      :return: row iterator object.

      Example::

         rows = sheet.get_rows()

         for row in rows:
             print(row)  # tuple of cell values

   .. py:function:: get_named_expressions

      Get a named expressions iterator.

      Returns (:obj:`.NamedExpressions`):
          Named expression object.

   .. py:function:: write

      Write sheet content to specified file object.

      Args:
          file:
              writable object to write the sheet content to.
          format (:obj:`.FormatType`):
              format of the output. Note that it currently only supports a
              subset of the formats provided by the :obj:`.FormatType` type.

   .. py:attribute:: name

      Read-only attribute that stores the name of the sheet.

   .. py:attribute:: sheet_size

      Read-only dictionary object that stores the column and row sizes of the
      sheet with the **column** and **row** keys, respectively.

   .. py:attribute:: data_size

      Read-only dictionary object that stores the column and row sizes of the
      data region of the sheet with the **column** and **row** keys, respectively.
      The data region is the smallest possible range that includes all non-empty
      cells in the sheet.  The top-left corner of the data region is always at
      the top-left corner of the sheet.


.. py:class:: SheetRows

   Iterator for rows within a sheet.


.. py:class:: NamedExpressions

   Iterator for named expressions.

   .. py:attribute:: names
      :type: set

      A set of strings representing the names of the named expressions.
