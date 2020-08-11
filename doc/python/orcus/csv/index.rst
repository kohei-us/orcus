
orcus.csv
=========

.. py:function:: orcus.csv.read

   Read an CSV file from a specified file path and create a :py:class:`orcus.Document`
   instance object.

   :param stream: file object containing byte streams.
   :param bool recalc: optional parameter specifying whether or not to recalculate
       the formula cells on load. Defaults to ``False``.
   :param str error_policy: optional parameter indicating what to do when
       encountering formula cells with invalid formula expressions. The value
       must be either ``fail`` or ``skip``.  Defaults to ``fail``.
   :rtype: :py:class:`orcus.Document`
   :return: document instance object that stores the content of the file.

   Example::

      from orcus import csv

      with open("path/to/file.csv", "rb") as f:
          doc = csv.read(f, recalc=True, error_policy="fail")
