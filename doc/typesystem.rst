The API Extractor Type System
*****************************

The typesystem is used by a binding generator to map a C++ library API onto
a higher level language.

The typesystem specification is a handwritten XML document listing the types
that will be available in the generated target language API; types that are not
declared in the specification will be ignored along with everything depending on
them. In addition, it is possible to manipulate and modify types and functions.
It is even possible to use the typesystem specification to inject arbitrary
code into the source files, such as an extra member function.

The typesystem specification is passed as an argument to the generator.

Below there is a complete reference guide to the various nodes of the typesystem.
For usage examples, take a look at the typesystem files used to generate the Qt
Python bindings. These files can be found in the data directory of the PySide
package.

.. _specifying-types:

Specifying Types
----------------

typesystem
^^^^^^^^^^

    This is the root node containing all the type system information. It can
    have a number of attributes, described below.

    .. code-block:: xml

        <typesystem package="..." default-superclass="...">
        </typesystem>

    The **package** attribute is a string describing the package to be used,
    e.g. "QtCore".
    The *optional* **default-superclass** attribute is the canonical C++ base class
    name of all objects, e.g., "object".

load-typesystem
^^^^^^^^^^^^^^^

    The load-typesystem node specifies which type systems to load when mapping
    multiple libraries to another language or basing one library on another, and
    it is a child of the typesystem node.

    .. code-block:: xml

        <typesystem>
            <load-typesystem name="..." generate="yes | no" />
        </typesystem>

    The **name** attribute is the filename of the typesystem to load, the
    **generate** attribute specifies whether code should be generated or not. The
    later must be specified when basing one library on another, making the generator
    able to understand inheritance hierarchies, primitive mapping, parameter types
    in functions, etc.

    Most libraries will be based on both the QtCore and QtGui modules, in which
    case code generation for these libraries will be disabled.


rejection
^^^^^^^^^

    The rejection node rejects the given class, or the specified function or
    field, and it is a child of the typesystem node.

    .. code-block:: xml

        <typesystem>
            <rejection class="..."
                function-name="..."
                field-name="..." />
        </typesystem>

    The **class** attribute is the C++ class name of the class to reject. Use the
    *optional* **function-name** or **field-name** attributes to reject a particular
    function or field. Note that the **field-name** and **function-name** cannot
    be specified at the same time. To remove all occurrences of a given field or
    function, set the class attribute to \*. You can use an empty class field
    to denote a global function.


primitive-type
^^^^^^^^^^^^^^

    The primitive-type node describes how a primitive type is mapped from C++ to
    the target language, and is a child of the typesystem node. Note that most
    primitives are already specified in the QtCore typesystem.

    .. code-block:: xml

        <typesystem>
            <primitive-type name="..."
                target-name="..."
                preferred-conversion="yes | no" />
        </typesystem>

    The **name** attribute is the name of the primitive in C++, the optimal
    **target-name** attribute is the name of the primitive type in the target
    language. If the later two attributes are not specified their default value
    will be the same as the **name** attribute.

    If the *optional* **preferred-conversion** attribute is set to *no*, it
    indicates that this version of the primitive type is not the preferred C++
    equivalent of the target language type. For example, in Python both "qint64"
    and "long long" become "long" but we should prefer the "qint64" version. For
    this reason we mark "long long" with preferred-conversion="no".

namespace-type
^^^^^^^^^^^^^^

    The namespace-type node maps the given C++ namespace to the target language,
    and it is a child of the typesystem node. Note that within namespaces, the
    generator only supports enums (i.e., no functions or classes).

    .. code-block:: xml

        <typesystem>
            <namespace-type name="..."
                package="..." />
        </typesystem>

    The **name** attribute is the name of the namespace, e.g., "Qt". The **package**
    attribute can be used to override the package of the type system.

enum-type
^^^^^^^^^

    The enum-type node maps the given enum from C++ to the target language,
    and it is a child of the typesystem node. Use the reject-enum-value to
    reject values.

    .. code-block:: xml

        <typesystem>
            <enum-type name="..."
                flags="yes | no"
                lower-bound="..."
                upper-bound="..."
                force-integer="yes | no"
                extensible="yes | no" />
        </typesystem>

    The **name** attribute is the fully qualified C++ name of the enum
    (e.g.,"Qt::FillRule"). If the *optional* **flags** attribute is set to *yes*
    (the default is *no*), the generator will expect an existing QFlags<T> for the
    given enum type. The **lower-bound** and **upper-bound** attributes are used
    to specify runtime bounds checking for the enum value. The value must be a
    compilable target language statement, such as "QGradient.Spread.PadSpread"
    (taking again Python as an example). If the **force-integer** attribute is
    set to *yes* (the default is *no*), the generated target language code will
    use the target language integers instead of enums. And finally, the
    **extensible** attribute specifies whether the given enum can be extended
    with user values (the default is *no*).


value-type
^^^^^^^^^^

    The value-type node indicates that the given C++ type is mapped onto the target
    language as a value type. This means that it is an object passed by value on C++,
    i.e. it is stored in the function call stack. It is a child of the typesystem node.

    .. code-block:: xml

        <typesystem>
            <value-type  name="..."
             copyable="yes | no"
             hash-function="..." />
        </typesystem>

    The **name** attribute is the fully qualified C++ class name, such as
    "QMatrix" or "QPainterPath::Element". The **copyable** attribute is used to
    force or not specify if this type is copyable. The *optional* **hash-function**
    attribute informs the function name of a hash function for the type.


object-type
^^^^^^^^^^^

    The object-type node indicates that the given C++ type is mapped onto the target
    language as an object type. This means that it is an object passed by pointer on
    C++ and it is stored on the heap. It is a child of the typesystem node.

    .. code-block:: xml

        <typesystem>
            <object-type name="..."
             copyable="yes | no"
             hash-function="..." />
        </typesystem>

    The **name** attribute is the fully qualified C++ class name. If there is no
    C++ base class, the default-superclass attribute can be used to specify a 
    superclass for the given type, in the generated target language API. The 
    **copyable** and **hash-function** attributes are the same as described for 
    :ref:`value-type`.


interface-type
^^^^^^^^^^^^^^

    The interface-type node indicates that the given class is replaced by an
    interface pattern when mapping from C++ to the target language. Using the
    interface-type node implicitly makes the given type an object type.

    .. code-block:: xml

        <typesystem>
            <interface-type name="..."
                package ="..."
                default-superclass ="..." />
        </typesystem>

    The **name** attribute is the fully qualified C++ class name. The *optional*
    **package** attribute can be used to override the package of the type system.
    If there is no C++ base class, the *optional* **default-superclass** attribute
    can be used to specify a superclass in the generated target language API, for
    the given class.

suppress-warning
^^^^^^^^^^^^^^^^

    The generator will generate several warnings which may be irrelevant to the
    user. The suppress-warning node suppresses the specified warning, and it is
    a child of the typesystem node.

    .. code-block:: xml

        <typesystem>
            <suppress-warning text="..." />
        </typesystem>

    The **text* attribute is the warning text to suppress, and may contain the *
    wildcard (use "" to escape regular expression matching if the warning contain
    a regular "*").


template
^^^^^^^^

    The template node registers a template that can be used to avoid duplicate
    code when extending the generated code, and it is a child of the typesystem
    node.

    .. code-block:: xml

        <typesystem>
            <template name="my_template">
                // the code
            </template>
        </typesystem>

    Use the insert-template node to insert the template code (identified by the
    template's **name** attribute) into the generated code base.



.. _value-type-requirements:

Value Type Requirements
------------------------

custom-constructor
^^^^^^^^^^^^^^^^^^

    In some target languages, such as Python, value types are required to have a
    copy constructor. If a C++ class without a copy constructor is mapped onto
    the target language as a value type, it is possible to provide a custom
    constructor using the custom-constructor node which is a child of the
    value-type node.

    .. code-block:: xml

        <value-type>
            <custom-constructor
                 name="..."
                 param-name="...">
                    // code for custom constructor
            </custom-constructor>
        </value-type>

    The custom constructor's signature becomes:

        T \*name(T \*param-name);

    If not specified the **name** of the constructor becomes
    <lowercase type name>_create() and the **param-name** becomes copy.

    **name** and **param-name** attributes are *optional*.

custom-destructor
^^^^^^^^^^^^^^^^^

    When a custom constructor is provided using the custom-constructor node, it is
    most likely required to clean up the allocated memory. For that reason, it is
    also possible to provide a custom destructor using the custom-destructor node
    which is a child of the value-type node.

    .. code-block:: xml

        <value-type>
            <custom-destructor
                 name="..."
                 param-name="...">
                    // code for custom destructor
            </custom-destructor>
        </value-type>

    The custom destructor must have the following signature:

        T \*name(T \*param-name);

    If not specified the **name** of the destructor becomes
    <lowercase type name>_delete() and the **param-name** becomes copy.

    **name** and **param-name** attributes are *optional*.

insert-template
^^^^^^^^^^^^^^^

    Documented in the :ref:`using-code-templates`



.. _manipulating-object-and-value-types:

Manipulating Object and Value Types
-----------------------------------

inject-code
^^^^^^^^^^^
    The inject-code node inserts the given code into the generated code for the
    given type or function, and it is a child of the object-type, value-type and
    modify-function nodes.

    .. code-block:: xml

         <value-type>
             <inject-code class="native | target | target-declaration"
                 position="beginning | end">
                 // the code
             </inject-code>
         </value-type>

    The **class** attribute specifies which module of the generated code that
    will be affected by the code injection. The **class** attribute accepts the
    following values:

        * native: The c++ code
        * target: The binding code
        * target-declaration: The code will be injected into the generated header
          file containing the c++ wrapper class definition.

    If the **position** attribute is set to *beginning* (the default), the code
    is inserted at the beginning of the function. If it is set to *end*, the code
    is inserted at the end of the function.

modify-field
^^^^^^^^^^^^

    The modify-field node allows you to alter the access privileges for a given
    C++ field when mapping it onto the target language, and it is a child of an
    object-type or a value-type node.

    .. code-block:: xml

         <object-type>
             <modify-field name="..."
                 write="true | false"
                 read="true | false" />
         </object-type>

    The **name** attribute is the name of the field, the *optional* **write**
    and **read** attributes specify the field's access privileges in the target
    language API (both are set to true by default).

modify-function
^^^^^^^^^^^^^^^

    The modify-function node allows you to modify a given C++ function when mapping
    it onto the target language, and it is a child of an object-type or a value-type
    node. Use the modify-argument node to specify which argument the modification
    affects.

    .. code-block:: xml

         <object-type>
             <modify-function signature="..."
                              remove="all | c++"
                              access="public | private | protected"
                              rename="..." >
         </object-type>

    The **signature** attribute is a normalized C++ signature, excluding return
    values but including potential const declarations.

    The **remove**, **access** and **rename** attributes are *optional* attributes
    for added convenience; they serve the same purpose as the tags remove, access
    and rename.


include
^^^^^^^

    Documented in the :ref:`manipulating-namespace-and-interface-types`


extra-includes
^^^^^^^^^^^^^^

    Documented in the :ref:`manipulating-namespace-and-interface-types`


insert-template
^^^^^^^^^^^^^^^

    Documented in the :ref:`using-code-templates`


.. _manipulating-namespace-and-interface-types:

Manipulating Namespace and Interface Types
------------------------------------------

extra-includes
^^^^^^^^^^^^^^

    The extra-includes node contains declarations of additional include files,
    and it can be a child of the interface-type, namespace-type, value-type and
    object-type nodes.

    The generator automatically tries to read the global header for each type but
    sometimes it is required to include extra files in the generated C++ code to
    make sure that the code compiles. These files must be listed using include
    nodes witin the extra-include node:

    .. code-block:: xml

         <value-type>
             <extra-includes>
                 <include file-name="..." location="global | local"/>
             </extra-includes>
         </value-type>

    The **file-name** attribute is the file to include, such as "QStringList".
    The **location** attribute is where the file is located: *global* means that
    the file is located in $INCLUDEPATH and will be included using #include <...>,
    *local* means that the file is in a local directory and will be included
    using #include "...".


include
^^^^^^^

    The include node specifies the name and location of a file that must be
    included, and it is a child of the interface-type, namespace-type, value-type,
    object-type or extra-includes nodes

    The generator automatically tries to read the global header for each type. Use
    the include node to override this behavior, providing an alternative file. The
    include node can also be used to specify extra include files.

    .. code-block:: xml

         <value-type>
             <include file-name="..."
                 location="global | local"/>
         </value-type>
    The **file-name** attribute is the file to include, such as "QStringList".
    The **location** attribute is where the file is located: *global* means that
    the file is located in $INCLUDEPATH and will be included using #include <...>,
    *local* means that the file is in a local directory and will be included
    using #include "...".


.. _manipulating-enum-types:

Manipulating Enum Types
-----------------------

reject-enum-value
^^^^^^^^^^^^^^^^^

    The reject-enum-value node rejects the enum value specified by the **name**
    attribute, and it is a child of the enum-type node.

    .. code-block:: xml

         <enum-type>
             <reject-enum-value name="..."/>
         </enum-type>

    This node is used when a C++ enum implementation has several identical numeric
    values, some of which are typically obsolete.
    
    **WARNING:** If the enum has some duplicated value don't forget to remove one
                 of them.


.. _modifying-functions:

Modifying Functions
-------------------

modify-argument
^^^^^^^^^^^^^^^

    The modify-argument node specifies which of the given function's arguments the
    modification affects, and is a child of the modify-function node. Use the
    remove-argument, replace-default-expression, remove-default-expression,
    replace-type, reference-count and define-ownership nodes to specify the details
    of the modification.

    .. code-block:: xml

         <modify-function>
             <modify-argument index="return | this | 1 ..." >
                 // modifications
             </modify-argument>
         </modify-function>

    Set the **index** attribute to "1" for the first argument, "2" for the second
    one and so on. Alternatively, set it to "return" or "this" if you want to
    modify the function's return value or the object the function is called upon,
    respectively.


remove
^^^^^^

    The remove node removes the given method from the generated target language
    API, and it is a child of the modify-function node.

    .. code-block:: xml

         <modify-function>
             <remove class="all" />
         </modify-function>


access
^^^^^^

    The access node changes the access privileges of the given function in the
    generated target language API, and it is a child of the modify-function node.

    .. code-block:: xml

        <modify-function>
            <access modifier="public | protected | private"/>
        </modify-function>


rename
^^^^^^

    The rename node changes the name of the given function in the generated target
    language API, and it is a child of the modify-function node.

    .. code-block:: xml

        <modify-function>
            <rename to="..." />
        </modify-function>

    The **to** attribute is the new name of the function.


inject-code
^^^^^^^^^^^

    Documented in the :ref:`manipulating-object-and-value-types`


argument-map
^^^^^^^^^^^^

    The argument-map node maps a C++ argument name to the argument name used in
    the generated target language API, and is a child of the inject-code node
    when the later is a child of a modify-function node.

    .. code-block:: xml

        <modify-function>
          <inject-code>
             <argument-map index="numeric value"
                 meta-name="string value">
          </inject-code>
        </modify-function>

    The **index** attribute is an index, starting at 1, indicating the argument
    position to which this argument mapping applies. The **meta-name** attribute
    is the name used within the code injection to adress the argument at the
    given position.


.. _modifying-arguments:

Modifying Arguments
-------------------

remove-argument
^^^^^^^^^^^^^^^

    The remove-argument node removes the given argument from the function's
    signature, and it is a child of the modify-argument node.

    .. code-block:: xml

     <modify-argument>
         <remove-argument />
     </modify-argument>

    Typically, when removing an argument, some conversion rules must be specified,
    e.g., when converting from the target language to C++. This can be done using
    the :ref:`conversion-rule` node.


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
    expression specified by the **with** attribute, and it is a child of the
    modify-argument node.

    .. code-block:: xml

         <modify-argument>
             <replace-default-expression with="..." />
         </modify-argument>


replace-type
^^^^^^^^^^^^

    The replace-type node replaces the type of the given argument to the one
    specified by the **modified-type** attribute, and it is a child of the
    modify-argument node.

    .. code-block:: xml

         <modify-argument>
             <replace-type modified-type="..." />
         </modify-argument>

    If the new type is a class, the **modified-type** attribute must be set to
    the fully qualified name (including name of the package as well as the class
    name).

    Typically when changing the type of an argument some conversion rules must be
    specified. This can be done using the :ref:`conversion-rule` node.


define-ownership
^^^^^^^^^^^^^^^^

    The define-ownership tag indicates that the function changes the ownership
    rules of the argument object. The **class** attribute specifies the class of
    function where to inject the ownership altering code. The **owner** attribute
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


insert-template
^^^^^^^^^^^^^^^

    Documented in the :ref:`using-code-templates`


replace-value
^^^^^^^^^^^^^

    The **replace-value** attribute lets you replace the return statement of a
    function with a fixed string. This attribute can only be used for the
    argument at **index** 0, which is always the function's return value.

    .. code-block:: xml

         <modify-argument index="0" replace-value="this"/>


parent
^^^^^^

    The **parent** attribute lets you define argument parent which will
    take ownership of argument and will destroy the C++ child object when the
    parent is destroyed.

    .. code-block:: xml

        <modify-argument>
              <parent index="this" action="add | remove" />
         </modify-argument>

    In the **index** argument you must specify the parent argument. The action
    *add* creates a parent link between objects, while *remove* will undo the
    parentage relationship.


.. _using-code-templates:

Using Code Templates
--------------------

insert-template
^^^^^^^^^^^^^^^

    The insert-template node includes the code template identified by the name
    attribute, and it can be a child of the inject-code, conversion-rule, template,
    custom-constructor and custom-destructor nodes.

    .. code-block:: xml

         <inject-code class="target" position="beginning">
             <insert-template name="my_template" />
         </inject-code>

    Use the replace node to modify the template code.


replace
^^^^^^^

    The replace node allows you to modify template code before inserting it into
    the generated code, and it can be a child of the insert-template node.

    .. code-block:: xml

        <insert-template name="my_template">
           <replace from="..." to="..." />
        </insert-template>

    This node will replace the attribute **from** with the value pointed by
    **to**.


Manipulating Documentation
--------------------------

inject-documentation
^^^^^^^^^^^^^^^^^^^^

    The inject-documentation node inserts the documentation into the generated
    documentation. This node is a child of the object-type, value-type and
    modify-function nodes.

    .. code-block:: xml

         <value-type>
             <inject-documentation mode="append | prepend | replace" format="native | target" >
                 // the documentation
             </inject-code>
         </value-type>

    The **mode** attribute default value is *replace*.

    The **format** attribute specifies when the documentation injection will
    occur and it accepts the following values:

        * native: Before XML<->Backend transformation occur, so the injected code *must* be a valid XML.
        * target: Before XML<->Backend transformation occur, so the injected code *must* be a valid backend format.

    At the moment the only supported backend is Sphinx.

modify-documentation
^^^^^^^^^^^^^^^^^^^^

    The modify-documentation node allows you to change the auto-generated 
    documentation. API Extractor transforms XML's from qdoc3 (the Qt documentation
    tool) into .rst files to be processed later using Sphinx. So you can modify
    the XML before the transformation occur.

    .. code-block:: xml

        <modify-documentation xpath="...">
            <!-- new documentation -->
        </modify-documentation>

    The **xpath** attribute is the XPath to the node that you want to modify.
