.. _manipulating-object-and-value-types:

Manipulating Object and Value Types
-----------------------------------

.. _inject-code:

inject-code
^^^^^^^^^^^
    The inject-code node inserts the given code into the generated code for the
    given type or function, and it is a child of the :ref:`object-type`, :ref:`value-type`,
    :ref:`modify-function` and :ref:`add-function` nodes.

    The code can be embedded into XML (be careful to use the correct XML entities
    for characters like '<', '>', '&'):

    .. code-block:: xml

         <value-type>
             <inject-code class="native | target | target-declaration"
                 position="beginning | end" since="...">
                 // the code
             </inject-code>
         </value-type>

    or obtained from an external file:

    .. code-block:: xml

         <value-type>
             <inject-code class="native | target | target-declaration"
                 position="beginning | end" since="..."
                 file="external_source.cpp"
                 snippet="label"/>
         </value-type>


    The ``class`` attribute specifies which module of the generated code that
    will be affected by the code injection
    (see :ref:`codegenerationterminology`). The ``class`` attribute accepts the
    following values:

        * native: The c++ code
        * target: The binding code
        * target-declaration: The code will be injected into the generated header
          file containing the c++ wrapper class definition.
        * file: The file name
        * snippet: The snippet label (optional)

    If the ``position`` attribute is set to *beginning* (the default), the code
    is inserted at the beginning of the function. If it is set to *end*, the code
    is inserted at the end of the function.

    The ``since`` attribute specify the API version where this code was injected.

    If a ``snippet`` label is given, the code between annotations of the form

    .. code-block:: c++

        // @snippet label
        ...
        // @snippet label

    will be extracted.

    For a detailed description, see :ref:`codeinjectionsemantics`.

modify-field
^^^^^^^^^^^^

    The modify-field node allows you to alter the access privileges for a given
    C++ field when mapping it onto the target language, and it is a child of an
    :ref:`object-type` or a :ref:`value-type` node.

    .. code-block:: xml

         <object-type>
             <modify-field name="..."
                 write="true | false"
                 read="true | false" />
         </object-type>

    The ``name`` attribute is the name of the field, the *optional* ``write``
    and ``read`` attributes specify the field's access privileges in the target
    language API (both are set to true by default).
    The ``remove`` attribute is an *optional* attribute, which can mark the field
    to be discarded on generation; it has the same purpose of the deprecated tag
    :ref:`remove`.

.. _modify-function:

modify-function
^^^^^^^^^^^^^^^

    The modify-function node allows you to modify a given C++ function when mapping
    it onto the target language, and it is a child of an :ref:`object-type` or a :ref:`value-type`
    node. Use the :ref:`modify-argument` node to specify which argument the modification
    affects.

    .. code-block:: xml

         <object-type>
             <modify-function signature="..."
                              since="..."
                              remove="all | c++"
                              access="public | private | protected"
                              allow-thread="true | auto | false"
                              exception-handling="off | auto-off | auto-on | on"
                              overload-number="number"
                              rename="..." />
         </object-type>

    The ``signature`` attribute is a normalized C++ signature, excluding return
    values but including potential const declarations.

    The ``since`` attribute specify the API version when this function was modified.

    The ``allow-thread`` attribute specifies whether a function should be wrapped
    into ``Py_BEGIN_ALLOW_THREADS`` and ``Py_END_ALLOW_THREADS``, that is,
    temporarily release the GIL (global interpreter lock). Doing so is required
    for any thread-related  function (wait operations), functions that might call
    a virtual function (potentially reimplemented in Python), and recommended for
    lengthy I/O operations or similar. It has performance costs, though.
    The value ``auto`` means that it will be turned off for functions for which
    it is deemed to be safe, for example, simple getters.
    The attribute defaults to ``false``.

    The ``exception-handling`` attribute specifies whether to generate exception
    handling code (nest the function call into try / catch statements). It accepts
    the following values:

           * no, false: Do not generate exception handling code
           * auto-off: Generate exception handling code for functions
             declaring a non-empty ``throw`` list
           * auto-on: Generate exception handling code unless function
             declares ``noexcept``
           * yes, true: Always generate exception handling code

    The optional ``overload-number`` attribute specifies the position of the
    overload when checking arguments. Typically, when a number of overloads
    exists, as for in example in Qt:

    .. code-block:: c++

        void QPainter::drawLine(QPointF, QPointF);
        void QPainter::drawLine(QPoint, QPoint);

    they will be reordered such that the check for matching arguments for the
    one taking a ``QPoint`` is done first. This is to avoid a potentially
    costly implicit conversion from ``QPoint`` to ``QPointF`` when using the
    2nd overload. There are cases though in which this is not desired;
    most prominently when a class inherits from a container and overloads exist
    for both types as is the case for the ``QPolygon`` class:

    .. code-block:: c++

        class QPolygon : public QList<QPoint> {};

        void QPainter::drawPolygon(QPolygon);
        void QPainter::drawPolygon(QList<QPoint>);

    By default, the overload taking a ``QList`` will be checked first, trying
    to avoid constructing a ``QPolygon`` from ``QList``. The type check for a
    list of points will succeed for a parameter of type ``QPolygon``, too,
    since it inherits ``QList``. This presents a problem since the sequence
    type check is costly due to it checking that each container element is a
    ``QPoint``. It is thus preferable to check for the ``QPolygon`` overload
    first. This is achieved by specifying numbers as follows:

    .. code-block:: xml

        <object-type name="QPainter">
            <modify-function signature="drawPolygon(QPolygon)" overload-number="0"/>
            <modify-function signature="drawPolygon(QList&lt;QPoint&gt;)" overload-number="1"/>
        </object-type>

    Numbers should be given for all overloads; otherwise, the order will be in
    declaration order.

    The ``remove``, ``access`` and ``rename`` attributes are *optional* attributes
    for added convenience; they serve the same purpose as the deprecated tags :ref:`remove`, :ref:`access` and :ref:`rename`.

.. _add-function:

add-function
^^^^^^^^^^^^

    The add-function node allows you to add a given function onto the target language,
    and it is a child of an :ref:`object-type` or :ref:`value-type` nodes if the
    function is supposed to be a method, or :ref:`namespace` and :ref:`typesystem` if
    the function is supposed to be a function inside a namespace or a global function.

    Typically when adding a function some code must be injected to provide the function
    logic. This can be done using the :ref:`inject-code` node.

    .. code-block:: xml

         <object-type>
             <add-function signature="..." return-type="..." access="public | protected" static="yes | no" since="..."/>
         </object-type>

    The ``return-type`` attribute defaults to *void*, the ``access`` to *public* and the ``static`` one to *no*.

    The ``since`` attribute specify the API version when this function was added.

    Within the signature, names for the function parameters can be specified by
    enclosing them within the delimiter *@*:

    .. code-block::

        void foo(int @parameter1@,float)

.. _declare-function:

declare-function
^^^^^^^^^^^^^^^^

    The declare-function node allows you to declare a function present in the
    type.

    .. code-block:: xml

         <container-type>
             <declare-function signature="..." return-type="..." since="..."/>
         </container-type>

    The ``return-type`` attribute defaults to *void*.

    The ``since`` attribute specifies the API version when this function was
    added.

    This is useful to make functions known to shiboken which its code parser
    does not detect. For example, in Qt 6, the ``append()`` function of the
    ``QList<T>`` container takes an argument of ``parameter_type`` which is
    specialized to ``T`` for simple types and ``const T &`` for complex types
    by some template expression which the code parser cannot resolve.
    In that case, the function can be declared with a simple signature:

    .. code-block:: xml

         <container-type name="QList">
             <declare-function signature="append(T)"/>
         </container-type>

    This tells shiboken a public function of that signature exists and
    bindings will be created in specializations of ``QList``.

.. _conversion-rule-on-types:

conversion-rule
^^^^^^^^^^^^^^^

    The conversion-rule node allows you to write customized code to convert the given argument between the target
    language and C++, and is a child of the :ref:`value-type`, :ref:`object-type`, :ref:`primitive-type` and
    :ref:`container-type` nodes.

    The code pointed by the file attribute is very tied to the generator using APIExtractor, so it don't follow any
    rules, but the generator rules..

    .. code-block:: xml

        <value-type name="Foo">
            <convertion-rule file="my_converter_implementation.h" since="..."/>
        </value-type>

    The ``since`` attribute specify the API version when this conversion rule became valid.

    .. note:: You can also use the conversion-rule node to specify :ref:`how the conversion of a single function argument should be done in a function <conversion-rule>`.

    The ``file`` and ``snippet`` attributes are also supported (see :ref:`inject-code` nodes).


property
^^^^^^^^

    The ``property`` element allows you to specify properties consisting of
    a type and getter and setter functions.

    It may appear as a child of a complex type such as ``object-type`` or
    ``value-type``.

    If the PySide2 extension is not present, code will be generated using the
    ``PyGetSetDef`` struct, similar to what is generated for fields.

    If the PySide2 extension is present, those properties complement the
    properties obtained from the ``Q_PROPERTY`` macro in Qt-based code.
    The properties will be handled in ``libpyside`` unless code generation
    is forced.

    .. code-block:: xml

        <property name="..." type="..." get="..." set="..." " generate-getsetdef="yes | no" since="..."/>

    The ``name`` attribute specifies the name of the property, the ``type``
    attribute specifies the C++ type and the ``get`` attribute specifies the
    name of the accessor function.

    The optional ``set`` attribute specifies name of the setter function.

    The optional ``generate-getsetdef`` attribute specifies whether to generate
    code for if the PySide2 extension is present (indicating this property is not
    handled by libpyside). It defaults to *no*.

    The optional ``since`` attribute specifies the API version when this
    property appears.

    For a typical C++ class, like:

    .. code-block:: c++

        class Test {
        public:
            int getValue() const;
            void setValue();
        };

    ``value`` can then be specified to be a property:

    .. code-block:: xml

        <value-type name="Test">
            <property name="value" type="int" get="getValue" set="setValue"/>

    With that, a more pythonic style can be used:

    .. code-block:: python

        test = Test()
        test.value = 42

    For Qt classes (with the PySide2 extension present), additional setters
    and getters that do not appear as ``Q_PROPERTY``, can be specified to
    be properties:

    .. code-block:: xml

        <object-type name="QMainWindow">
            <property name="centralWidget" type="QWidget *" get="centralWidget" set="setCentralWidget"/>

    in addition to the normal properties of ``QMainWindow`` defined for
    Qt Designer usage.

    .. note:: In the *Qt* coding style, the property name typically conflicts
        with the getter name. It is recommended to exclude the getter from the
        wrapper generation using the ``remove`` function modification.
