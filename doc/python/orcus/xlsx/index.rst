
orcus.xlsx
==========

.. py:function:: orcus.xlsx.read

   Read an Excel file from a specified file path and create a
   :py:class:`orcus.Document` instance object.  The file must be of Excel 2007
   XML format.

   :param stream: file object containing byte streams.
   :param bool recalc: optional parameter specifying whether or not to recalculate
       the formula cells on load. Defaults to ``False``.
   :param str error_policy: optional parameter indicating what to do when
       encountering formula cells with invalid formula expressions. The value
       must be either ``fail`` or ``skip``.  Defaults to ``fail``.
   :rtype: :py:class:`orcus.Document`
   :return: document instance object that stores the content of the file.

   Example::

      from orcus import xlsx

      with open("path/to/file.xlsx", "rb") as f:
          doc = xlsx.read(f, recalc=True, error_policy="fail")
