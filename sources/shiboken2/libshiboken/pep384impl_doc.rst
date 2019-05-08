****************************************
The Transition To The Limited Python API
****************************************


Foreword
========

Python supports a limited API that restricts access to certain structures.
Besides eliminating whole modules and all functions and macros which names
start with an
underscore, the most drastic restriction is the removal of normal type object
declarations.

For details about the eliminated modules and functions, please see the
`PEP 384`_ page for reference.


.. _`PEP 384`: https://www.python.org/dev/peps/pep-0384/



Changed Modules
===============

All changed module's include files are listed with the changed functions here.
As a general rule, it was tried to keep the changes to a minimum diff.
Macros which are not available were changed to functions with the same name
if possible. Completely removed names ``Py{name}`` were re-implemented as ``Pep{name}``.


memoryobject.h
--------------

The buffer protocol was completely removed. We redefined all the structures
and methods, because PySide uses that. This is an exception to the limited API
that we have to check ourselves. The code is extracted in bufferprocs_py37.h .
This is related to the following:


abstract.h
----------

This belongs to the buffer protocol like memoryobject.h .
As replacement for ``Py_buffer`` we defined ``Pep_buffer`` and several other
internal macros.

The version is checked by hand, and the version number must be updated only
if the implementation does not change. Otherwise, we need to write version
dependent code paths.

It is questionable if it is worthwhile to continue using the buffer protocol
or if we should try to get rid of ``Pep_buffer``, completely.


longobject.h
------------

``_PyLong_AsInt`` is not available. We defined a ``_PepLong_AsInt`` function, instead.
Maybe this should be replaced by ``PyLong_AsLong``.


pydebug.h
---------

We have no direct access to ``Py_VerboseFlag`` because debugging is not
supported. We redefined it as macro ``Py_VerboseFlag`` which calls ``Pep_VerboseFlag``.


unicodeobject.h
---------------

The macro ``PyUnicode_GET_SIZE`` was redefined to call into ``PyUnicode_GetSize``
for Python 2, and ``PyUnicode_GetLength`` for Python 3.
Function ``_PyUnicode_AsString`` is unavailable and was replaced by a macro
that calls ``_PepUnicode_AsString``. The implementation was a bit involved,
and it would be better to change the code and replace this function.


bytesobject.h
-------------

The macros ``PyBytes_AS_STRING`` and ``PyBytes_GET_SIZE`` were redefined to call
the according functions.


floatobject.h
-------------

``PyFloat_AS_DOUBLE`` now calls ``PyFloat_AsDouble``.


tupleobject.h
-------------

``PyTuple_GET_ITEM``, ``PyTuple_SET_ITEM`` and ``PyTuple_GET_SIZE`` were redefined as
function calls.


listobject.h
------------

``PyList_GET_ITEM``, ``PyList_SET_ITEM`` and ``PyList_GET_SIZE`` were redefined as
function calls.


methodobject.h
--------------

``PyCFunction_GET_FUNCTION``, ``PyCFunction_GET_SELF`` and ``PyCFunction_GET_FLAGS``
were redefined as function calls.

Direct access to the methoddef structure is not available, and we defined
``PepCFunction_GET_NAMESTR`` as accessor for name strings.


pythonrun.h
-----------

The simple function ``PyRun_String`` is not available. It was re-implemented
in a simplified version for the signature module.


funcobject.h
------------

The definitions of funcobject.h are completely missing, although there
are extra ``#ifdef`` conditional defines inside, too. This suggests that the exclusion
was unintended.

We therefore redefined ``PyFunctionObject`` as an opaque type.

The missing macro ``PyFunction_Check`` was defined, and the macro
``PyFunction_GET_CODE`` calls the according function.

There is no equivalent for function name access, therefore we introduced
``PepFunction_GetName`` either as a function or as a macro.

*TODO: We should fix funcobject.h*


classobject.h
-------------

Classobject is also completely not imported, instead of defining an opaque type.

We defined the missing functions ``PyMethod_New``, ``PyMethod_Function`` and
``PyMethod_Self`` and also redefined ``PyMethod_GET_SELF`` and
``PyMethod_GET_FUNCTION`` as calls to these functions.

*TODO: We should fix classobject.h*


code.h
------

The whole code.c code is gone, although it may make sense to
define some minimum accessibility. This will be clarified on
`Python-Dev`_. We needed access to code objects and defined the missing
PepCode_GET_FLAGS and PepCode_GET_ARGCOUNT either as function or macro.
We further added the missing flags, although few are used:

``CO_OPTIMIZED`` ``CO_NEWLOCALS`` ``CO_VARARGS`` ``CO_VARKEYWORDS`` ``CO_NESTED``
``CO_GENERATOR``

*TODO: We should maybe fix code.h*

.. _`Python-Dev`: https://mail.python.org/mailman/listinfo/python-dev

datetime.h
----------

The DateTime module is explicitly not included in the limited API.
We defined all the needed functions but called them via Python instead
of direct call macros. This has a slight performance impact.

The performance could be easily improved by providing an interface
that fetches all attributes at once, instead of going through the object
protocol every time.

The re-defined macros and methods are::

    PyDateTime_GET_YEAR
    PyDateTime_GET_MONTH
    PyDateTime_GET_DAY
    PyDateTime_DATE_GET_HOUR
    PyDateTime_DATE_GET_MINUTE
    PyDateTime_DATE_GET_SECOND
    PyDateTime_DATE_GET_MICROSECOND
    PyDateTime_DATE_GET_FOLD
    PyDateTime_TIME_GET_HOUR
    PyDateTime_TIME_GET_MINUTE
    PyDateTime_TIME_GET_SECOND
    PyDateTime_TIME_GET_MICROSECOND
    PyDateTime_TIME_GET_FOLD

    PyDate_Check
    PyDateTime_Check
    PyTime_Check

    PyDate_FromDate
    PyDateTime_FromDateAndTime
    PyTime_FromTime

*XXX: We should maybe provide an optimized interface to datetime*


object.h
--------

The file object.h contains the ``PyTypeObject`` structure, which is supposed
to be completely opaque. All access to types should be done through
``PyType_GetSlot`` calls. Due to bugs and deficiencies in the limited API
implementation, it was not possible to do that. Instead, we have defined
a simplified structure for ``PyTypeObject`` that has only the fields that
are used in PySide.

We will explain later why and how this was done. Here is the reduced
structure::

    typedef struct _typeobject {
        PyVarObject ob_base;
        const char *tp_name;
        Py_ssize_t tp_basicsize;
        void *X03; // Py_ssize_t tp_itemsize;
        void *X04; // destructor tp_dealloc;
        void *X05; // printfunc tp_print;
        void *X06; // getattrfunc tp_getattr;
        void *X07; // setattrfunc tp_setattr;
        void *X08; // PyAsyncMethods *tp_as_async;
        void *X09; // reprfunc tp_repr;
        void *X10; // PyNumberMethods *tp_as_number;
        void *X11; // PySequenceMethods *tp_as_sequence;
        void *X12; // PyMappingMethods *tp_as_mapping;
        void *X13; // hashfunc tp_hash;
        ternaryfunc tp_call;
        reprfunc tp_str;
        void *X16; // getattrofunc tp_getattro;
        void *X17; // setattrofunc tp_setattro;
        void *X18; // PyBufferProcs *tp_as_buffer;
        void *X19; // unsigned long tp_flags;
        void *X20; // const char *tp_doc;
        traverseproc tp_traverse;
        inquiry tp_clear;
        void *X23; // richcmpfunc tp_richcompare;
        Py_ssize_t tp_weaklistoffset;
        void *X25; // getiterfunc tp_iter;
        void *X26; // iternextfunc tp_iternext;
        struct PyMethodDef *tp_methods;
        void *X28; // struct PyMemberDef *tp_members;
        void *X29; // struct PyGetSetDef *tp_getset;
        struct _typeobject *tp_base;
        PyObject *tp_dict;
        descrgetfunc tp_descr_get;
        void *X33; // descrsetfunc tp_descr_set;
        Py_ssize_t tp_dictoffset;
        initproc tp_init;
        allocfunc tp_alloc;
        newfunc tp_new;
        freefunc tp_free;
        inquiry tp_is_gc; /* For PyObject_IS_GC */
        PyObject *tp_bases;
        PyObject *tp_mro; /* method resolution order */
    } PyTypeObject;

Function ``PyIndex_Check`` had to be defined in an unwanted way due to
a Python issue. See file pep384_issue33738.cpp .

There are extension structures which have been isolated as special macros that
dynamically compute the right offsets of the extended type structures:

*   ``PepType_SOTP`` for ``SbkObjectTypePrivate``
*   ``PepType_SETP`` for ``SbkEnumTypePrivate``
*   ``PepType_PFTP`` for ``PySideQFlagsTypePrivate``
*   ``PepType_SGTP`` for ``_SbkGenericTypePrivate``

How these extension structures are used can best be seen by searching
``PepType_{four}`` in the source.

Due to the new heaptype interface, the names of certain types contain
now the module name in the ``tp_name`` field. To have a compatible way
to access simple type names as C string, ``PepType_GetNameStr`` has been
written that skips over dotted name parts.

Finally, the function ``_PyObject_Dump`` was excluded from the limited API.
This is a useful debugging aid that we always want to have available,
so it is added back, again. Anyway, we did not reimplement it, and so
Windows is not supported.
Therefore, a forgotten debugging call of this functions will break COIN. :-)


Using The New Type API
======================

After converting everything but the object.h file, we were a little
bit shocked: it suddenly was clear that we would have no more
access to type objects, and even more scary that all types which we
use have to be heap types, only!

For PySide with its intense use of heap type extensions in various
flavors, the situation looked quite unsolvable. In the end, it was
nicely solved, but it took almost 3.5 months to get that right.

Before we see how this is done, we will explain the differences
between the APIs and their consequences.


The Interface
-------------

The old type API of Python knows static types and heap types.
Static types are written down as a declaration of a ``PyTypeObject``
structure with all its fields filled in. Here is for example
the definition of the Python type ``object`` (Python 3.6)::

    PyTypeObject PyBaseObject_Type = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "object",                                   /* tp_name */
        sizeof(PyObject),                           /* tp_basicsize */
        0,                                          /* tp_itemsize */
        object_dealloc,                             /* tp_dealloc */
        0,                                          /* tp_print */
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_reserved */
        object_repr,                                /* tp_repr */
        0,                                          /* tp_as_number */
        0,                                          /* tp_as_sequence */
        0,                                          /* tp_as_mapping */
        (hashfunc)_Py_HashPointer,                  /* tp_hash */
        0,                                          /* tp_call */
        object_str,                                 /* tp_str */
        PyObject_GenericGetAttr,                    /* tp_getattro */
        PyObject_GenericSetAttr,                    /* tp_setattro */
        0,                                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
        PyDoc_STR("object()\n--\n\nThe most base type"),  /* tp_doc */
        0,                                          /* tp_traverse */
        0,                                          /* tp_clear */
        object_richcompare,                         /* tp_richcompare */
        0,                                          /* tp_weaklistoffset */
        0,                                          /* tp_iter */
        0,                                          /* tp_iternext */
        object_methods,                             /* tp_methods */
        0,                                          /* tp_members */
        object_getsets,                             /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        object_init,                                /* tp_init */
        PyType_GenericAlloc,                        /* tp_alloc */
        object_new,                                 /* tp_new */
        PyObject_Del,                               /* tp_free */
    };

We can write the same structure in form of a ``PyType_Spec`` structure,
and there is even an incomplete tool *abitype.py* that does this conversion
for us. With a few corrections, the result looks like this::

    static PyType_Slot PyBaseObject_Type_slots[] = {
        {Py_tp_dealloc,     (void *)object_dealloc},
        {Py_tp_repr,        (void *)object_repr},
        {Py_tp_hash,        (void *)_Py_HashPointer},
        {Py_tp_str,         (void *)object_str},
        {Py_tp_getattro,    (void *)PyObject_GenericGetAttr},
        {Py_tp_setattro,    (void *)PyObject_GenericSetAttr},
        {Py_tp_richcompare, (void *)object_richcompare},
        {Py_tp_methods,     (void *)object_methods},
        {Py_tp_getset,      (void *)object_getsets},
        {Py_tp_init,        (void *)object_init},
        {Py_tp_alloc,       (void *)PyType_GenericAlloc},
        {Py_tp_new,         (void *)object_new},
        {Py_tp_free,        (void *)PyObject_Del},
        {0, 0},
    };
    static PyType_Spec PyBaseObject_Type_spec = {
        "object",
        sizeof(PyObject),
        0,
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        PyBaseObject_Type_slots,
    };

This new structure is almost compatible with the old one, but there
are some subtle differences.

* The new types are generated in one step

This seems to be no problem, but it was very much, due to the way the
types were built in PySide. Types were assembled piece by piece, and
finally the ``PyType_Ready`` function was called.

With the new API, ``PyType_Ready`` is called already at the end of
``PyType_FromSpec``, and that meant that the logic of type creation became
completely turned upside down.

* The new types are always heaptypes

With the new type creation functions, it is no longer possible to
create "normal" types. Instead, they all have to be allocated on the
heap and garbage collected. The user should normally not recognize this.
But type creation is more constrained, and you cannot create a subtype
if the ``Py_TPFLAGS_BASETYPE`` is not set. This constraint was already
violated by PySide and needed a quite profound fix.

* The new types always need a module

While this is not a problem per se, the above new type spec will not create
a usable new type, but complain with::

    DeprecationWarning: builtin type object has no __module__ attribute

But there are more problems:

* The new types have unexpected defaults

When fields are empty, you would usually assume that they stay empty.
There are just a few corrections that ``PyType_Ready`` will do to a type.

But there is the following clause in ``PyType_FromSpec`` that can give you
many headaches::

    if (type->tp_dealloc == NULL) {
        /* It's a heap type, so needs the heap types' dealloc.
           subtype_dealloc will call the base type's tp_dealloc, if
           necessary. */
        type->tp_dealloc = subtype_dealloc;
    }

In fact, before the move to the new API, the ``PyType_Ready`` function
filled empty ``tp_dealloc`` fields with ``object_dealloc``. And the code
that has been written with that in mind now becomes pretty wrong if suddenly
``subtype_dealloc`` is used.

The way out was to explicitly provide an ``object_dealloc`` function.
This would then again impose a problem, because ``object_dealloc`` is not
public. Writing our own version is easy, but it again needs access to
type objects. But fortunately, we have broken this rule, already...


* The new types are only partially allocated

The structures used in ``PyType_FromSpec`` are almost all allocated,
only the name field is static. This is no problem for types which are
statically created once. But if you want to parameterize things and
create multiple types with a single slots and spec definition, the name
field that is used for tp_name must be allocated dynamically.
This is misleading, since all the slots already are copies.

* The new types don't support special offsets

The special fields ``tp_weaklistoffset`` and ``tp_dictoffset`` are not supported
by ``PyType_FromSpec``. Unfortunately the documentation does not tell you
if you are allowed to set these fields manually after creating the type or not.
We finally did it and it worked, but we are not sure about correctness.

See basewrapper.cpp function ``SbkObject_TypeF()`` as the only reference to
these fields in PySide. This single reference is absolutely necessary and
very important, since all derived types invisibly inherit these two fields.


Future Versions Of The Limited API
==================================

As we have seen, the current version of the limited API does a bit of
cheating, because it uses parts of the data structure that should be
an opaque type. At the moment, this works fine because the data is
still way more compatible as it could be.

But what if this is changed in the future?

We know that the data structures are stable until Python 3.8 comes out.
Until then, the small bugs and omissions will hopefully all be solved.
Then it will be possible to replace the current small tricks by calls
to ``PyType_GetSlot`` in the way things should be.

At the very moment when the current assumptions about the data structure
are no longer true, we will rewrite the direct attribute access with
calls to ``PyType_GetSlot``. After that, no more changes will be necessary.


Appendix A: The Transition To Simpler Types
===========================================

After all code had been converted to the limited API, there was a
remaining problem with the ``PyHeapTypeObject``.

Why a problem? Well, all the type structures in shiboken use
special extra fields at the end of the heap type object. This
currently enforces extra knowledge at compile time about how large the
heap type object is. In a clean implementation, we would only use
the ``PyTypeObject`` itself and access the fields *behind* the type
by a pointer that is computed at runtime.


Restricted PyTypeObject
-----------------------

Before we are going into details, let us motivate the existence of
the restricted ``PyTypeObject``:

Originally, we wanted to use ``PyTypeObject`` as an opaque type and
restrict ourselves to only use the access function ``PyType_GetSlot``.
This function allows access to all fields which are supported by
the limited API.

But this is a restriction, because we get no access to ``tp_dict``,
which we need to support the signature extension. But we can work
around that.

The real restriction is that ``PyType_GetSlot`` only works for heap
types. This makes the function quite useless, because we have
no access to ``PyType_Type``, which is the most important type ``type``
in Python. We need that for instance to compute the size of
``PyHeapTypeObject`` dynamically.

With much effort, it is possible to clone ``PyType_Type`` as a heap
type. But due to a bug in the Pep 384 support, we need
access to the ``nb_index`` field of a normal type. Cloning does not
help because ``PyNumberMethods`` fields are *not* inherited.

After we realized this dead end, we changed concept and did not
use ``PyType_GetSlot`` at all (except in function ``copyNumberMethods``),
but created a restricted ``PyTypeObject`` with only those fields
defined that are needed in PySide.

Is this breakage of the limited API? I don't think so. A special
function runs on program startup that checks the correct position
of the fields of ``PyTypeObject``, although a change in those fields is
more than unlikely.
The really crucial thing is to no longer use ``PyHeapTypeObject``
explicitly because that *does* change its layout over time.


Diversification
---------------

There were multiple ``Sbk{something}`` structures which all used a "d" field
for their private data. This made it not easy to find the right
fields when switching between objects and types::

    struct LIBSHIBOKEN_API SbkObject
    {
        PyObject_HEAD
        PyObject *ob_dict;
        PyObject *weakreflist;
        SbkObjectPrivate *d;
    };

    struct LIBSHIBOKEN_API SbkObjectType
    {
        PyHeapTypeObject super;
        SbkObjectTypePrivate *d;
    };

The first step was to rename the SbkObjectTypePrivate part from "d" to
"sotp". It was chosen to be short but easy to remember as abbreviation
of "SbkObjectTypePrivate", leading to::

    struct LIBSHIBOKEN_API SbkObjectType
    {
        PyHeapTypeObject super;
        SbkObjectTypePrivate *sotp;
    };

After renaming, it was easier to do the following transformations.


Abstraction
-----------

After renaming the type extension pointers to ``sotp``, I replaced
them by function-like macros which did the special access *behind*
the types, instead of those explicit fields. For instance, the
expression::

    type->sotp->converter

became::

    PepType_SOTP(type)->converter

The macro expansion can be seen here::

    #define PepHeapType_SIZE \
        (reinterpret_cast<PyTypeObject *>(&PyType_Type)->tp_basicsize)

    #define _genericTypeExtender(etype) \
        (reinterpret_cast<char *>(etype) + PepHeapType_SIZE)

    #define PepType_SOTP(etype) \
        (*reinterpret_cast<SbkObjectTypePrivate **>(_genericTypeExtender(etype)))

This looks complicated, but in the end there is only a single new
indirection via ``PyType_Type``, which happens at runtime. This is the
key to fulfil what Pep 384 wants to achieve: *No more version-dependent fields*.


Simplification
--------------

After all type extension fields were replaced by macro calls, we
could remove the following version dependent re-definition of ``PyHeapTypeObject``
::

    typedef struct _pyheaptypeobject {
        union {
            PyTypeObject ht_type;
            void *opaque[PY_HEAPTYPE_SIZE];
        };
    } PyHeapTypeObject;

, and the version dependent structure::

    struct LIBSHIBOKEN_API SbkObjectType
    {
        PyHeapTypeObject super;
        SbkObjectTypePrivate *sotp;
    };

could be replaced by the simplified::

    struct LIBSHIBOKEN_API SbkObjectType
    {
        PyTypeObject type;
    };

which is no longer version-dependent.
Note that we tried to replace the above struct directly by ``PyTypeObject``,
but that was too much. The distinction between ``SbkObjectType`` and
``PyTypeObject`` is still needed.


Appendix B: Verification Of PyTypeObject
========================================

We have introduced a limited PyTypeObject in the same place
as the original PyTypeObject, and now we need to prove that
we are allowed to do so.

When using the limited API as intended, then types are completely
opaque, and access is only through ``PyType_FromSpec`` and (from
version 3.5 upwards) through ``PyType_GetSlot``.

Python then uses all the slot definitions in the type description
and produces a regular heap type object.


Unused Information
------------------

We know many things about types that are not explicitly said,
but they are inherently clear:

(a) The basic structure of a type is always the same, regardless
    if it is a static type or a heap type.

(b) types are evolving very slowly, and a field is never replaced
    by another field with different semantics.

Inherent rule (a) gives us the following information: If we calculate
the offsets of the basic fields, then this info is also usable for non-heap
types.

The validation checks if rule (b) is still valid.


How it Works
------------

The basic idea of the validation is to produce a new type using
``PyType_FromSpec`` and to see where in the type structure these fields
show up. So we build a ``PyType_Slot`` structure with all the fields we
are using and make sure that these values are all unique in the
type.

Most fields are not interrogated by ``PyType_FromSpec``, and so we
simply used some numeric value. Some fields are interpreted, like
``tp_members``. This field must really be a ``PyMemberDef``. And there are
``tp_base`` and ``tp_bases`` which have to be type objects and lists
thereof. It was easiest to not produce these fields from scratch
but use them from the ``type`` object ``PyType_Type``.

Then one would think to write a function that searches the known
values in the opaque type structure.

But we can do better and use optimistically the observation (b):
We simply use the restricted ``PyTypeObject`` structure and assume that
every field lands exactly where we are awaiting it.

And that is the whole proof: If we find all the disjoint values at
the places where we expect them, then verification is done.


About ``tp_dict``
-----------------

One word about the ``tp_dict`` field: This field is a bit special in
the proof, since it does not appear in the spec and cannot easily
be checked by ``type.__dict__`` because that creates a *dictproxy*
object. So how do we prove that is really the right dict?

We have to create that ``PyMethodDef`` structure anyway, and instead of
leaving it empty, we insert a dummy function. Then we ask the
``tp_dict`` field if it has the awaited object in it, and that's it!

#EOT
