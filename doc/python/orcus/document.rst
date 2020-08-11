
Document
========

.. py:class:: orcus.Document

   An instance of this class represents a document model.  A document consists
   of multiple sheet objects.

   .. py:attribute:: sheets

      Read-only attribute that stores a tuple of :py:class:`.Sheet` instance
      objects.

   .. py:function:: get_named_expressions

      Get a named expressions iterator.

      :rtype: :obj:`.NamedExpressions`
      :return: named expression object.

