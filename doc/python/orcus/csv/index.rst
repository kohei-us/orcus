
orcus.csv
=========

.. py:function:: orcus.csv.read(stream, split=False, delimiters=',', text_qualifier='"')

   Read a CSV file and create a :py:class:`orcus.Document` instance object.

   :param stream: String value, or file object containing a string stream.
   :param bool split: If ``True``, split data into multiple sheets when the number of rows
      exceeds the sheet limit. Defaults to ``False``.
   :param str delimiters: Delimiter characters to use. Defaults to ``','``.
   :param str text_qualifier: A single character used as the text qualifier. Defaults to ``'"'``.
   :rtype: :py:class:`orcus.Document`
   :return: Document instance object that stores the content of the file.

   Example::

      from orcus import csv

      with open("path/to/file.csv", "r") as f:
          doc = csv.read(f)
