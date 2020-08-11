
orcus.xls_xml
=============

.. py:function:: orcus.xls_xml.read

   Read an Excel file from a specified file path and create a
   :py:class:`orcus.Document` instance object.  The file must be saved in the
   SpreadsheetML format.

   :param stream: file object containing byte streams.
   :param bool recalc: optional parameter specifying whether or not to recalculate
       the formula cells on load. Defaults to ``False``.
   :param str error_policy: optional parameter indicating what to do when
       encountering formula cells with invalid formula expressions. The value
       must be either ``fail`` or ``skip``.  Defaults to ``fail``.
   :rtype: :py:class:`orcus.Document`
   :return: document instance object that stores the content of the file.

   Example::

      from orcus import xls_xml

      with open("path/to/file.xls_xml", "rb") as f:
          doc = xls_xml.read(f, recalc=True, error_policy="fail")
