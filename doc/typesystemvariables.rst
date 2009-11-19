*********************
Type System Variables
*********************

User written code can be placed in arbitrary places using the
:doc:`inject-code <codeinjectionsemantics>` tag. To ease the binding developer
work, the injected code can make use of special variables that will be replaced
by the correct values. This also shields the developer from some |project|
implementation specifics.


Variables
=========

**%0**

  Replaced by the name of the return variable of the Python method/function wrapper.


**%#**

  Replaced by the name of a C++ argument in the position indicated by ``#``.
  The argument counting starts with ``%1``, since ``%0`` represents the return
  variable name. If the number indicates a variable that was removed in the
  type system description, but there is a default value for it, this value will
  be used. Consider this example:

      .. code-block:: c++

          void argRemoval(int a0, int a1 = 123);


      .. code-block:: xml

            <modify-function signature="argRemoval(int, int)">
                <modify-argument index="2">
                    <remove-argument/>
                </modify-argument>
            </modify-function>

  The ``%1`` will be replaced by the C++ argument name, and ``%2`` will get the
  value ``123``.


**%ARGUMENT_NAMES**

  Replaced by a comma separated list with the names of all C++ arguments that
  were not removed on the type system description for the method/function. If
  the removed argument has a default value (original or provided in the type
  system), this value will be inserted in the argument list.

  Take the following method and related type system description as an example:

      .. code-block:: c++

          void argRemoval(int a0, Point a1 = Point(1, 2), bool a2 = true, Point a3 = Point(3, 4), int a4 = 56);


      .. code-block:: xml

            <modify-function signature="argRemoval(int, Point, bool, Point, int)">
                <modify-argument index="2">
                    <remove-argument/>
                    <replace-default-expression with="Point(6, 9)"/>
                </modify-argument>
                <modify-argument index="4">
                    <remove-argument/>
                </modify-argument>
            </modify-function>

  As seen on the XML description, the function's ``a1`` and ``a3`` arguments
  were removed. If any ``inject-code`` for this function uses ``%ARGUMENT_NAMES``
  the resulting list will be the equivalent of using individual argument type
  system variables this way:

      .. code-block:: c++

            %1, Point(6, 9), %3, Point(3, 4), %5


**%CONVERTTOPYTHON[CPPTYPE]**

  Replaced by a |project| conversion call that converts a C++ variable of the
  type indicated by ``CPPTYPE`` to the proper Python object.


**%CPPSELF**

  Replaced by the wrapped C++ object instance that owns the method in which the
  code with this variable was inserted.


**%FUNCTION_NAME**

  Replaced by the name of a function or method.


**%PYARG_#**

  Similar to ``%#``, but is replaced by the Python arguments (PyObjects)
  received by the Python wrapper method.


**%PYSELF**

  Replaced by the Python wrapper variable (a PyObject) representing the instance
  bounded to the Python wrapper method which receives the custom code.


**%PYTHONTYPEOBJECT**

  Replaced by the Python type object for the context in which it is inserted:
  method or class modification.


**%RETURN_TYPE**

  Replaced by the type returned by a function or method.


**%TYPE**

  Replaced by the name of the class to which a function belongs. Should be used
  in code injected to methods.


Example
=======

Just to illustrate the usage of the variables described in the previous
sections, below is an excerpt from the type system description of a |project|
test. It changes a method that received ``argc/argv`` arguments into something
that expects a Python sequence instead.

    .. code-block:: xml

        <modify-function signature="overloadedMethod(int, char**)">
            <modify-argument index="1">
                <replace-type modified-type="PySequence" />
            </modify-argument>
            <modify-argument index="2">
                <remove-argument />
            </modify-argument>
            <inject-code class="target" position="beginning">
                int argc;
                char** argv;
                if (!PySequence_to_argc_argv(%PYARG_1, &amp;argc, &amp;argv)) {
                    PyErr_SetString(PyExc_TypeError, "error");
                    return 0;
                }
                %RETURN_TYPE foo = %CPPSELF.%FUNCTION_NAME(argc, argv);
                %0 = %CONVERTTOPYTHON[%RETURN_TYPE](foo);

                for (int i = 0; i &lt; argc; ++i)
                    delete[] argv[i];
                delete[] argv;
            </inject-code>
        </modify-function>

