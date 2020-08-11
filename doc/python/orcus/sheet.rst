
Sheet
=====

.. py:class:: orcus.Sheet

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

      :rtype: :obj:`.NamedExpressions`
      :return: named expression object.

   .. py:function:: write

      Write sheet content to specified file object.

      :param file: writable object to write the sheet content to.
      :param format: format of the output. Note that it currently
          only supports a subset of the formats provided by the :obj:`.FormatType`
          type.
      :type format: :obj:`.FormatType`

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
