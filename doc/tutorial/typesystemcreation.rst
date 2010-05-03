.. highlight:: xml

.. _gentut-typesystem:

Creating the Typesystem Description
===================================

The typesystem is an specification used when mapping a C++ based library onto a
corresponding Python module. The specification is a handwritten XML document
listing the types that will be available in the generated binding, modifications
to classes and function signatures to better suit the target language,
and listing the components that should be rejected for the binding.

**PySide** uses a typesystem format similar to the one used by **QtJambi** and
**QtScript**, thoroughly described in the page *"The Qt Jambi Type System"*. [#]_

The divergences between **PySide** and QtScript/QtJambi typesystems will be
highlighted whenever they appear. Things to be aware of when writing
a typesystem will be also mentioned.

Describing **libfoo** for Python Audiences
------------------------------------------

All typesystem files start with the root ``typesystem`` tag. The
``package`` attribute carries the name of the package as it will be seen
from Python.

Right after that, all the typesystem files providing information required for
the generation process are included in the same fashion as header files in C.

**foobinding/data/typesystem_foo.xml**
::

    <?xml version="1.0"?>
    <typesystem package="foo">
        <load-typesystem name="typesystem_core.xml" generate="no"/>
        <object-type name="Math"/>
    </typesystem>


The inclusion of other typesystem files is achieved with the
``load-typesystem`` tag. The ``generate`` attribute must be set to ``"no"``
otherwise the generator will try to create more source code for the already
existing bindings included for reference.

The C++ classes derived from **QObject** intended to be exposed in the target
language are described with ``object-type`` tags.


For this example binding just specifying the name of the class does the trick,
since the generator system will automatically catch the methods with arguments
and return value of known types. These types can be described in the same
typesystem file or in the ones referenced with the ``load-typesystem`` tag.

In more complex situations method signatures can be changed or rejected with
other tags that can be checked out in the `typesystem <http://www.pyside.org/docs/apiextractor/typesystem.html>`_
reference.


Other Common Cases and Differences
----------------------------------

What follows now is some common uses of the typesystem capabilities. All of them
can be seen in the Qt4 typesystem files. They are not used for this binding
tutorial example, so if you just want to have things working ASAP, move along.

Templates
~~~~~~~~~

To ease the process of writing custom code for the binding, recurring pieces of
code can be turned generic with the typesystem template mechanism.
They are declared in a way similar to this snippet:

::

    <template name="only_bool*_fix">
        bool ok;
        %RETURN_TYPE retval = self.%FUNCTION_NAME(&ok);
    </template>

And is used as in this example:

::

    <inject-code class="native" position="beginning">
        <insert-template name="only_bool*_fix"/>
    </inject-code>


The ``typesystem_template.xml`` file from the Qt4 bindings can be used as a
good resource for examples of this. Check also the QtJambi documentation on
typesystem templates. [#]_

Non-QObject Derived Classes
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Even in a Qt4 based library it is common to find classes that doesn't
pertain to the QObject hierarchy, these must be declared as ``value-type``:

::

    <value-type name="RectOrSomethingLikeThat"/>


Unused Tags
~~~~~~~~~~~

Some tags defined in the QtScript/QtJambi typesystem has no effect in **PySide**
typesystem, they are:

  + conversion-rule
  + argument-map

Changes to ``"inject-code"`` Tag
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can pass a file name to the **inject-code** tag so the file contents will
be injected in the generated code.

The ``class`` attribute value ``java`` was changed to ``target``, while
``native`` remained the same.

Global Functions
~~~~~~~~~~~~~~~~

The BoostPythonGenerator supports global functions, you can also reject these functions using
the **rejection** tag like is done to reject classes. Just pass an empty string to
the class attribute.

::

    <rejection class="" function-name="qt_noop"/>


.. [#] http://doc.trolltech.com/qtjambi-4.4/html/com/trolltech/qt/qtjambi-typesystem.html
.. [#] http://doc.trolltech.com/qtjambi-4.4/html/com/trolltech/qt/qtjambi-typesystem.html#using-code-templates
