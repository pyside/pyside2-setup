.. _modifying-functions:

Modifying Functions
-------------------

.. _modify-argument:

modify-argument
^^^^^^^^^^^^^^^

    The modify-argument node specifies which of the given function's arguments the
    modification affects, and is a child of the modify-function node. Use the
    remove-argument, replace-default-expression, remove-default-expression,
    replace-type, reference-count and define-ownership nodes to specify the details
    of the modification.

    .. code-block:: xml

         <modify-function>
             <modify-argument index="return | this | 1 ..." rename="...">
                 // modifications
             </modify-argument>
         </modify-function>

    Set the ``index`` attribute to "1" for the first argument, "2" for the second
    one and so on. Alternatively, set it to "return" or "this" if you want to
    modify the function's return value or the object the function is called upon,
    respectively.

    The optional ``rename`` attribute is used to rename a argument and use this
    new name in the generated code.
