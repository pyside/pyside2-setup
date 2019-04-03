****************
Object ownership
****************

One of the main things a binding developer should have in mind is
how the C++ instances lives will cope with Python's reference count.
The last thing you want is to crash a program due to a segfault
when your C++ instance was deleted and the
wrapper object tries to access the invalid memory there.

In this section we'll show how |project| deals with object ownership
and parentship, taking advantage of the information provided by the
APIExtractor.

Ownership basics
================

As any python binding, |project|-based bindings uses reference counting
to handle the life of the wrapper object (the Python object that contains the
C++ object, do not confuse with the *wrapped* C++ object).
When a reference count reaches zero, the wrapper is deleted by Python garbage
collector and tries to delete the wrapped instance, but sometimes the wrapped
C++ object is already deleted, or maybe the C++ object should not be freed after
the Python wrapper go out of scope and die, because C++ is already taking care of
the wrapped instance.

In order to handle this, you should tell the
generator whether the instance's ownership belongs to the binding or
to the C++ Library. When belonging to the binding, we are sure that the C++ object
won't be deleted by C++ code and we can call the C++ destructor when the refcount
reaches 0. Otherwise, instances owned by C++ code can be destroyed arbitrarily,
without notifying the Python wrapper of its destruction.

Invalidating objects
====================

To prevent segfaults and double frees, the wrapper objects are invalidated.
An invalidated can't be passed as argument or have an attribute or method accessed.
Trying to do this will raise RuntimeError.

The following situations can invalidate an object:

C++ taking ownership
--------------------

    When an object is passed to a function or method that takes ownership of it, the wrapper
    is invalidated as we can't be sure of when the object is destroyed, unless it has a
    :ref:`virtual destructor <ownership-virt-method>` or the transfer is due to the special case
    of :ref:`parent ownership <ownership-parent>`.

    Besides being passed as argument, the called object can have its ownership changed, like
    the `setParent` method in Qt's `QObject`.

Invalidate after use
--------------------

    Objects marked with *invalidate-after-use* in the type system description always are
    virtual method arguments provided by a C++ originated call. They should be
    invalidated right after the Python function returns.

.. _ownership-virt-method:

Objects with virtual methods
----------------------------

    A little bit of implementation details:
    virtual methods are supported by creating a C++ class, the **shell**, that inherits
    from the class with virtual methods, the native one, and override those methods to check if
    any derived class in Python also override it.

    If the class has a virtual destructor (and C++ classes with virtual methods should have), this
    C++ instance invalidates the wrapper only when the overridden destructor is called.

    One exception to this rule is when the object is created in C++, like in a
    factory method. This way the wrapped object is a C++ instance of the native
    class, not the shell one, and we cannot know when it is destroyed.

.. _ownership-parent:

Parent-child relationship
=========================

One special type of ownership is the parent-child relationship.
Being a child of an object means that when the object's parent dies,
the C++ instance also dies, so the Python references will be invalidated.
Qt's QObject system, for example, implements this behavior, but this is valid
for any C++ library with similar behavior.

.. _ownership-parent-heuristics:

Parentship heuristics
---------------------

    As the parent-child relationship is very common, |project| tries to automatically
    infer what methods falls into the parent-child scheme, adding the extra
    directives related to ownership.

    This heuristic will be triggered when generating code for a method and:

    * The function is a constructor.
    * The argument name is `parent`.
    * The argument type is a pointer to an object.

    When triggered, the heuristic will set the argument named "parent"
    as the parent of the object being created by the constructor.

    The main focus of this process was to remove a lot of hand written code from
    type system when binding Qt libraries. For Qt, this heuristic works in all cases,
    but be aware that it might not when binding your own libraries.

    To activate this heuristic, use the :ref:`--enable-parent-ctor-heuristic <parent-heuristic>`
    command line switch.

.. _return-value-heuristics:

Return value heuristics
-----------------------

    When enabled, object returned as pointer in C++ will become child of the object on which the method
    was called.

    To activate this heuristic, use the :ref:`--enable-return-value-heuristic <return-heuristic>`

Common pitfalls
===============

Not saving unowned objects references
-------------------------------------

    Sometimes when you pass an instance as argument to a method and the receiving
    instance will need that object to live indefinitely, but will not take ownership
    of the argument instance. In this case, you should hold a reference to the argument
    instance.

    For example, let's say that you have a renderer class that will use a source class
    in a setSource method but will not take ownership of it. The following code is wrong,
    because when `render` is called the `Source` object created during the call to `setSource`
    is already destroyed.

    .. code-block:: python

       renderer.setModel(Source())
       renderer.render()

    To solve this, you should hold a reference to the source object, like in

    .. code-block:: python

       source = Source()
       renderer.setSource(source)
       renderer.render()


Ownership Management in  the Typesystem
=======================================

Ownership transfer from C++ to target
-------------------------------------

    When an object currently owned by C++ has its ownership transferred
    back to the target language, the binding can know for sure when the object will be deleted and
    tie the C++ instance existence to the wrapper, calling the C++ destructor normally when the
    wrapper is deleted.

    .. code-block:: xml

        <modify-argument index="1">
            <define-ownership class="target" owner="target" />
        </modify-argument>

Ownership transfer from target to C++
-------------------------------------

    In the opposite direction, when an object ownership is transferred from the target language
    to C++, the native code takes full control of the object life and you don't
    know when that object will be deleted, rendering the wrapper object invalid,
    unless you're wrapping an object with a virtual destructor,
    so you can override it and be notified of its destruction.

    By default it's safer to just render the wrapper
    object invalid and raise some error if the user tries to access
    one of this objects members or pass it as argument to some function, to avoid unpleasant segfaults.
    Also you should avoid calling the C++ destructor when deleting the wrapper.

    .. code-block:: xml

        <modify-argument index="1">
            <define-ownership class="target" owner="c++" />
        </modify-argument>


Parent-child relationship
-------------------------

One special type of relationship is the parent-child. When an object is called
the parent of another object (the child), the former is in charge of deleting its
child when deleted and the target language can trust that the child will be alive
as long as the parent is, unless some other method can take the C++ ownership away from the parent.

One of the main uses of this scheme is Qt's object system, with ownership among QObject-derived
classes, creating "trees" of instances.

    .. code-block:: xml

        <modify-argument index="this">
            <parent index="1" action="add">
        </modify-argument>

In this example, the instance with the method that is being invoked (indicated by 'index="this"' on
modify-argument) will be marked as a child
of the first argument using the `parent` tag. To remove ownership, just use "remove" in the action attribute. **Removing
parentship also transfers the ownership back to python.**

Invalidation after use
----------------------

Sometimes an object is created as a virtual method call argument and destroyed after the
call returned. In this case, you should use the ``invalidate-after-use`` attribute in the
``modify-argument`` tag to mark the wrapper as invalid right after the virtual method returns.

    .. code-block:: xml

        <modify-argument index="2" invalidate-after-use="yes"/>

In this example the second argument will be invalidated after this method call.

.. [#] See *Object Trees and Object Ownership* http://doc.qt.io/qt-5/objecttrees.html
