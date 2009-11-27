.. _modifying-arguments:

Modifying Arguments
-------------------

.. _conversion-rule:

conversion-rule
^^^^^^^^^^^^^^^

    The conversion-rule node allows you to write customized code to convert
    the given argument between the target language and C++, and it is a child of the modify-argument node.

    .. code-block:: xml

        <modify-argument ...>
        <conversion-rule class="target | native">
            // the code
        </conversion-rule>
        </modify-argument>

    This node is typically used in combination with the replace-type and
    remove-argument nodes. The given code is used instead of the generator's
    conversion code.

    Writing %N in the code (where N is a number), will insert the name of the
    nth argument. Alternatively, %in and %out which will be replaced with the
    name of the conversion's input and output variable, respectively. Note the
    output variable must be declared explicitly, for example:

    .. code-block:: xml

        <conversion-rule class="native">
        bool %out = (bool) %in;
        </conversion-rule>

    .. note:: You can also use the conversion-rule node to specify :ref:`a conversion code which will be used instead of the generator's conversion code everywhere for a given type <conversion-rule-on-types>`.

remove-argument
^^^^^^^^^^^^^^^

    The remove-argument node removes the given argument from the function's
    signature, and it is a child of the modify-argument node.

    .. code-block:: xml

     <modify-argument>
         <remove-argument />
     </modify-argument>

remove-default-expression
^^^^^^^^^^^^^^^^^^^^^^^^^

    The remove-default-expression node disables the use of the default expression
    for the given argument, and it is a child of the modify-argument node.

    .. code-block:: xml

         <modify-argument...>
             <remove-default-expression />
         </modify-argument>

replace-default-expression
^^^^^^^^^^^^^^^^^^^^^^^^^^

    The replace-default-expression node replaces the specified argument with the
    expression specified by the ``with`` attribute, and it is a child of the
    modify-argument node.

    .. code-block:: xml

         <modify-argument>
             <replace-default-expression with="..." />
         </modify-argument>


replace-type
^^^^^^^^^^^^

    The replace-type node replaces the type of the given argument to the one
    specified by the ``modified-type`` attribute, and it is a child of the
    modify-argument node.

    .. code-block:: xml

         <modify-argument>
             <replace-type modified-type="..." />
         </modify-argument>

    If the new type is a class, the ``modified-type`` attribute must be set to
    the fully qualified name (including name of the package as well as the class
    name).

define-ownership
^^^^^^^^^^^^^^^^

    The define-ownership tag indicates that the function changes the ownership
    rules of the argument object. The ``class`` attribute specifies the class of
    function where to inject the ownership altering code. The ``owner`` attribute
    specifies the new ownership of the object. It accepts the following values:

        * target: the target language will assume full ownership of the object.
                  The native resources will be deleted when the target language
                  object is finalized.
        * c++: The native code assumes full ownership of the object. The target
               language object will not be garbage collected.
        * default: The object will get default ownership, depending on how it
                   was created.

    .. code-block:: xml

        <modify-argument>
              <define-ownership class="target | shell"
                                owner="target | c++ | default" />
         </modify-argument>

replace-value
^^^^^^^^^^^^^

    The ``replace-value`` attribute lets you replace the return statement of a
    function with a fixed string. This attribute can only be used for the
    argument at ``index`` 0, which is always the function's return value.

    .. code-block:: xml

         <modify-argument index="0" replace-value="this"/>


parent
^^^^^^

    The parent node lets you define the argument parent which will
    take ownership of argument and will destroy the C++ child object when the
    parent is destroyed.

    .. code-block:: xml

        <modify-argument index="1">
              <parent index="this" action="add | remove" />
        </modify-argument>

    In the ``index`` argument you must specify the parent argument. The action
    *add* creates a parent link between objects, while *remove* will undo the
    parentage relationship.
