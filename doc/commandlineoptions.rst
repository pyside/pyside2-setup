Command line options
********************

Usage
-----

::

   shiboken [options]


Options
-------

``--disable-verbose-error-messages``
    Disable verbose error messages. Turn the CPython code hard to debug but saves a few kilobytes
    in the generated binding.

.. _parent-heuristic:

``--enable-parent-ctor-heuristic``
    This flag enable an useful heuristic which can save a lot of work related to object ownership when
    writing the typesystem.
    For more info, check :ref:`ownership-parent-heuristics`.

.. _pyside-extensions:

``--enable-pyside-extensions``
    Enable pyside extensions like support for signal/slots. Use this if you are creating a binding based
    on PySide.

.. _return-heuristic:

``--enable-return-value-heuristic``
    Enable heuristics to detect parent relationship on return values.
    For more info, check :ref:`return-value-heuristics`.

