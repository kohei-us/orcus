
orcus
=====

.. py:function:: orcus.detect_format

   Detects the file format of the stream.

   :param stream: either bytes, or file object containing a byte stream.
   :rtype: :py:class:`orcus.FormatType`
   :return: enum value specifying the detected file format.

   Example::

      import orcus

      with open("path/to/file", "rb") as f:
          fmt = orcus.detect_format(f)


.. toctree::
   :maxdepth: 1

   cell.rst
   cell_type.rst
   document.rst
   format_type.rst
   formula_token.rst
   formula_token_op.rst
   formula_token_type.rst
   formula_tokens.rst
   named_expressions.rst
   sheet.rst
   sheet_rows.rst
