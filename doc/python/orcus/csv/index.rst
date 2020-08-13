
orcus.csv
=========

.. py:function:: orcus.csv.read

   Read an CSV file from a specified file path and create a :py:class:`orcus.Document`
   instance object.

   :param stream: either string value, or file object containing a string stream.
   :rtype: :py:class:`orcus.Document`
   :return: document instance object that stores the content of the file.

   Example::

      from orcus import csv

      with open("path/to/file.csv", "r") as f:
          doc = csv.read(f)
