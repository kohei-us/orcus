
Cell
====

.. py:class:: orcus.Cell

   This class represents a single cell within a :py:class:`.Sheet` object.

   .. py:method:: get_formula_tokens

      :rtype: :py:class:`.FormulaTokens`
      :return: an iterator object for a formula cell.

      Get an iterator object for formula tokens if the cell is a formula cell.
      This method returns ``None`` for a non-formula cell.

   .. py:attribute:: type
      :type: orcus.CellType

      Attribute specifying the type of this cell.

   .. py:attribute:: value

      Attribute containing the value of the cell.

   .. py:attribute:: formula
      :type: str

      Attribute containing the formula string in case of a formula cell.  This
      value will be ``None`` for a non-formula cell.

