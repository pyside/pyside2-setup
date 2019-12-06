/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "sbkpython.h"
#include "typespec.h"
#include "sbkstaticstrings.h"
#include <structmember.h>

#if PY_MAJOR_VERSION < 3

extern "C"
{

// for some reason python 2.7 needs this on Windows
#ifdef WIN32
static PyGC_Head *_PyGC_generation0;
#endif

// from pymacro.h
#ifndef Py_PYMACRO_H
#define Py_PYMACRO_H

/* Minimum value between x and y */
#define Py_MIN(x, y) (((x) > (y)) ? (y) : (x))

/* Maximum value between x and y */
#define Py_MAX(x, y) (((x) > (y)) ? (x) : (y))

/* Absolute value of the number x */
#define Py_ABS(x) ((x) < 0 ? -(x) : (x))

#define _Py_XSTRINGIFY(x) #x

/* Convert the argument to a string. For example, Py_STRINGIFY(123) is replaced
   with "123" by the preprocessor. Defines are also replaced by their value.
   For example Py_STRINGIFY(__LINE__) is replaced by the line number, not
   by "__LINE__". */
#define Py_STRINGIFY(x) _Py_XSTRINGIFY(x)

/* Get the size of a structure member in bytes */
#define Py_MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

/* Argument must be a char or an int in [-128, 127] or [0, 255]. */
#define Py_CHARMASK(c) ((unsigned char)((c) & 0xff))

/* Assert a build-time dependency, as an expression.

   Your compile will fail if the condition isn't true, or can't be evaluated
   by the compiler. This can be used in an expression: its value is 0.

   Example:

   #define foo_to_char(foo)  \
       ((char *)(foo)        \
        + Py_BUILD_ASSERT_EXPR(offsetof(struct foo, string) == 0))

   Written by Rusty Russell, public domain, http://ccodearchive.net/ */
#define Py_BUILD_ASSERT_EXPR(cond) \
    (sizeof(char [1 - 2*!(cond)]) - 1)

#define Py_BUILD_ASSERT(cond)  do {         \
        (void)Py_BUILD_ASSERT_EXPR(cond);   \
    } while (0)

/* Get the number of elements in a visible array

   This does not work on pointers, or arrays declared as [], or function
   parameters. With correct compiler support, such usage will cause a build
   error (see Py_BUILD_ASSERT_EXPR).

   Written by Rusty Russell, public domain, http://ccodearchive.net/

   Requires at GCC 3.1+ */
// Simplified by "0 &&"
#if 0 && (defined(__GNUC__) && !defined(__STRICT_ANSI__) && \
    (((__GNUC__ == 3) && (__GNU_MINOR__ >= 1)) || (__GNUC__ >= 4)))
/* Two gcc extensions.
   &a[0] degrades to a pointer: a different type from an array */
#define Py_ARRAY_LENGTH(array) \
    (sizeof(array) / sizeof((array)[0]) \
     + Py_BUILD_ASSERT_EXPR(!__builtin_types_compatible_p(typeof(array), \
                                                          typeof(&(array)[0]))))
#else
#define Py_ARRAY_LENGTH(array) \
    (sizeof(array) / sizeof((array)[0]))
#endif


/* Define macros for inline documentation. */
#define PyDoc_VAR(name) static char name[]
#define PyDoc_STRVAR(name,str) PyDoc_VAR(name) = PyDoc_STR(str)
#ifdef WITH_DOC_STRINGS
#define PyDoc_STR(str) str
#else
#define PyDoc_STR(str) ""
#endif

/* Below "a" is a power of 2. */
/* Round down size "n" to be a multiple of "a". */
#define _Py_SIZE_ROUND_DOWN(n, a) ((size_t)(n) & ~(size_t)((a) - 1))
/* Round up size "n" to be a multiple of "a". */
#define _Py_SIZE_ROUND_UP(n, a) (((size_t)(n) + \
        (size_t)((a) - 1)) & ~(size_t)((a) - 1))
/* Round pointer "p" down to the closest "a"-aligned address <= "p". */
#define _Py_ALIGN_DOWN(p, a) ((void *)((uintptr_t)(p) & ~(uintptr_t)((a) - 1)))
/* Round pointer "p" up to the closest "a"-aligned address >= "p". */
#define _Py_ALIGN_UP(p, a) ((void *)(((uintptr_t)(p) + \
        (uintptr_t)((a) - 1)) & ~(uintptr_t)((a) - 1)))
/* Check if pointer "p" is aligned to "a"-bytes boundary. */
#define _Py_IS_ALIGNED(p, a) (!((uintptr_t)(p) & (uintptr_t)((a) - 1)))

#ifdef __GNUC__
#define Py_UNUSED(name) _unused_ ## name __attribute__((unused))
#else
#define Py_UNUSED(name) _unused_ ## name
#endif

#endif /* Py_PYMACRO_H */

// from typeobject.c
static int
extra_ivars(PyTypeObject *type, PyTypeObject *base)
{
    size_t t_size = type->tp_basicsize;
    size_t b_size = base->tp_basicsize;

    assert(t_size >= b_size); /* Else type smaller than base! */
    if (type->tp_itemsize || base->tp_itemsize) {
        /* If itemsize is involved, stricter rules */
        return t_size != b_size ||
            type->tp_itemsize != base->tp_itemsize;
    }
    if (type->tp_weaklistoffset && base->tp_weaklistoffset == 0 &&
        type->tp_weaklistoffset + sizeof(PyObject *) == t_size &&
        type->tp_flags & Py_TPFLAGS_HEAPTYPE)
        t_size -= sizeof(PyObject *);
    if (type->tp_dictoffset && base->tp_dictoffset == 0 &&
        type->tp_dictoffset + sizeof(PyObject *) == t_size &&
        type->tp_flags & Py_TPFLAGS_HEAPTYPE)
        t_size -= sizeof(PyObject *);

    return t_size != b_size;
}

static void
clear_slots(PyTypeObject *type, PyObject *self)
{
    Py_ssize_t i, n;
    PyMemberDef *mp;

    n = Py_SIZE(type);
    mp = PyHeapType_GET_MEMBERS((PyHeapTypeObject *)type);
    for (i = 0; i < n; i++, mp++) {
        if (mp->type == T_OBJECT_EX && !(mp->flags & READONLY)) {
            char *addr = (char *)self + mp->offset;
            PyObject *obj = *(PyObject **)addr;
            if (obj != NULL) {
                *(PyObject **)addr = NULL;
                Py_DECREF(obj);
            }
        }
    }
}

static void
subtype_dealloc(PyObject *self)
{
    PyTypeObject *type, *base;
    destructor basedealloc;
    PyThreadState *tstate = PyThreadState_GET();

    /* Extract the type; we expect it to be a heap type */
    type = Py_TYPE(self);
    assert(type->tp_flags & Py_TPFLAGS_HEAPTYPE);

    /* Test whether the type has GC exactly once */

    if (!PyType_IS_GC(type)) {
        /* It's really rare to find a dynamic type that doesn't have
           GC; it can only happen when deriving from 'object' and not
           adding any slots or instance variables.  This allows
           certain simplifications: there's no need to call
           clear_slots(), or DECREF the dict, or clear weakrefs. */

        /* Maybe call finalizer; exit early if resurrected */
        if (type->tp_del) {
            type->tp_del(self);
            if (self->ob_refcnt > 0)
                return;
        }

        /* Find the nearest base with a different tp_dealloc */
        base = type;
        while ((basedealloc = base->tp_dealloc) == subtype_dealloc) {
            assert(Py_SIZE(base) == 0);
            base = base->tp_base;
            assert(base);
        }

        /* Extract the type again; tp_del may have changed it */
        type = Py_TYPE(self);

        /* Call the base tp_dealloc() */
        assert(basedealloc);
        basedealloc(self);

        /* Can't reference self beyond this point */
        Py_DECREF(type);

        /* Done */
        return;
    }

    /* We get here only if the type has GC */

    /* UnTrack and re-Track around the trashcan macro, alas */
    /* See explanation at end of function for full disclosure */
    PyObject_GC_UnTrack(self);
    ++_PyTrash_delete_nesting;
    ++ tstate->trash_delete_nesting;
    Py_TRASHCAN_SAFE_BEGIN(self);
    --_PyTrash_delete_nesting;
    -- tstate->trash_delete_nesting;
    /* DO NOT restore GC tracking at this point.  weakref callbacks
     * (if any, and whether directly here or indirectly in something we
     * call) may trigger GC, and if self is tracked at that point, it
     * will look like trash to GC and GC will try to delete self again.
     */

    /* Find the nearest base with a different tp_dealloc */
    base = type;
    while ((basedealloc = base->tp_dealloc) == subtype_dealloc) {
        base = base->tp_base;
        assert(base);
    }

    /* If we added a weaklist, we clear it.      Do this *before* calling
       the finalizer (__del__), clearing slots, or clearing the instance
       dict. */

    if (type->tp_weaklistoffset && !base->tp_weaklistoffset)
        PyObject_ClearWeakRefs(self);

    /* Maybe call finalizer; exit early if resurrected */
    if (type->tp_del) {
        _PyObject_GC_TRACK(self);
        type->tp_del(self);
        if (self->ob_refcnt > 0)
            goto endlabel;              /* resurrected */
        else
            _PyObject_GC_UNTRACK(self);
        /* New weakrefs could be created during the finalizer call.
            If this occurs, clear them out without calling their
            finalizers since they might rely on part of the object
            being finalized that has already been destroyed. */
        if (type->tp_weaklistoffset && !base->tp_weaklistoffset) {
            /* Modeled after GET_WEAKREFS_LISTPTR() */
            PyWeakReference **list = (PyWeakReference **) \
                PyObject_GET_WEAKREFS_LISTPTR(self);
            while (*list)
                _PyWeakref_ClearRef(*list);
        }
    }

    /*  Clear slots up to the nearest base with a different tp_dealloc */
    base = type;
    while (base->tp_dealloc == subtype_dealloc) {
        if (Py_SIZE(base))
            clear_slots(base, self);
        base = base->tp_base;
        assert(base);
    }

    /* If we added a dict, DECREF it */
    if (type->tp_dictoffset && !base->tp_dictoffset) {
        PyObject **dictptr = _PyObject_GetDictPtr(self);
        if (dictptr != NULL) {
            PyObject *dict = *dictptr;
            if (dict != NULL) {
                Py_DECREF(dict);
                *dictptr = NULL;
            }
        }
    }

    /* Extract the type again; tp_del may have changed it */
    type = Py_TYPE(self);

    /* Call the base tp_dealloc(); first retrack self if
     * basedealloc knows about gc.
     */
    if (PyType_IS_GC(base))
        _PyObject_GC_TRACK(self);
    assert(basedealloc);
    basedealloc(self);

    /* Can't reference self beyond this point */
    Py_DECREF(type);

  endlabel:
    ++_PyTrash_delete_nesting;
    ++ tstate->trash_delete_nesting;
    Py_TRASHCAN_SAFE_END(self);
    --_PyTrash_delete_nesting;
    -- tstate->trash_delete_nesting;

    /* Explanation of the weirdness around the trashcan macros:

       Q. What do the trashcan macros do?

       A. Read the comment titled "Trashcan mechanism" in object.h.
          For one, this explains why there must be a call to GC-untrack
          before the trashcan begin macro.      Without understanding the
          trashcan code, the answers to the following questions don't make
          sense.

       Q. Why do we GC-untrack before the trashcan and then immediately
          GC-track again afterward?

       A. In the case that the base class is GC-aware, the base class
          probably GC-untracks the object.      If it does that using the
          UNTRACK macro, this will crash when the object is already
          untracked.  Because we don't know what the base class does, the
          only safe thing is to make sure the object is tracked when we
          call the base class dealloc.  But...  The trashcan begin macro
          requires that the object is *untracked* before it is called.  So
          the dance becomes:

         GC untrack
         trashcan begin
         GC track

       Q. Why did the last question say "immediately GC-track again"?
          It's nowhere near immediately.

       A. Because the code *used* to re-track immediately.      Bad Idea.
          self has a refcount of 0, and if gc ever gets its hands on it
          (which can happen if any weakref callback gets invoked), it
          looks like trash to gc too, and gc also tries to delete self
          then.  But we're already deleting self.  Double deallocation is
          a subtle disaster.

       Q. Why the bizarre (net-zero) manipulation of
          _PyTrash_delete_nesting around the trashcan macros?

       A. Some base classes (e.g. list) also use the trashcan mechanism.
          The following scenario used to be possible:

          - suppose the trashcan level is one below the trashcan limit

          - subtype_dealloc() is called

          - the trashcan limit is not yet reached, so the trashcan level
        is incremented and the code between trashcan begin and end is
        executed

          - this destroys much of the object's contents, including its
        slots and __dict__

          - basedealloc() is called; this is really list_dealloc(), or
        some other type which also uses the trashcan macros

          - the trashcan limit is now reached, so the object is put on the
        trashcan's to-be-deleted-later list

          - basedealloc() returns

          - subtype_dealloc() decrefs the object's type

          - subtype_dealloc() returns

          - later, the trashcan code starts deleting the objects from its
        to-be-deleted-later list

          - subtype_dealloc() is called *AGAIN* for the same object

          - at the very least (if the destroyed slots and __dict__ don't
        cause problems) the object's type gets decref'ed a second
        time, which is *BAD*!!!

          The remedy is to make sure that if the code between trashcan
          begin and end in subtype_dealloc() is called, the code between
          trashcan begin and end in basedealloc() will also be called.
          This is done by decrementing the level after passing into the
          trashcan block, and incrementing it just before leaving the
          block.

          But now it's possible that a chain of objects consisting solely
          of objects whose deallocator is subtype_dealloc() will defeat
          the trashcan mechanism completely: the decremented level means
          that the effective level never reaches the limit.      Therefore, we
          *increment* the level *before* entering the trashcan block, and
          matchingly decrement it after leaving.  This means the trashcan
          code will trigger a little early, but that's no big deal.

       Q. Are there any live examples of code in need of all this
          complexity?

       A. Yes.  See SF bug 668433 for code that crashed (when Python was
          compiled in debug mode) before the trashcan level manipulations
          were added.  For more discussion, see SF patches 581742, 575073
          and bug 574207.
    */
}

static PyTypeObject *
solid_base(PyTypeObject *type)
{
    PyTypeObject *base;

    if (type->tp_base)
        base = solid_base(type->tp_base);
    else
        base = &PyBaseObject_Type;
    if (extra_ivars(type, base))
        return type;
    else
        return base;
}

/* Calculate the best base amongst multiple base classes.
   This is the first one that's on the path to the "solid base". */

static PyTypeObject *
best_base(PyObject *bases)
{
    Py_ssize_t i, n;
    PyTypeObject *base, *winner, *candidate, *base_i;
    PyObject *base_proto;

    assert(PyTuple_Check(bases));
    n = PyTuple_GET_SIZE(bases);
    assert(n > 0);
    base = NULL;
    winner = NULL;
    for (i = 0; i < n; i++) {
        base_proto = PyTuple_GET_ITEM(bases, i);
        if (PyClass_Check(base_proto))
            continue;
        if (!PyType_Check(base_proto)) {
            PyErr_SetString(
                PyExc_TypeError,
                "bases must be types");
            return NULL;
        }
        base_i = (PyTypeObject *)base_proto;
        if (base_i->tp_dict == NULL) {
            if (PyType_Ready(base_i) < 0)
                return NULL;
        }
        if (!PyType_HasFeature(base_i, Py_TPFLAGS_BASETYPE)) {
            PyErr_Format(PyExc_TypeError,
                         "type '%.100s' is not an acceptable base type",
                         base_i->tp_name);
            return NULL;
        }
        candidate = solid_base(base_i);
        if (winner == NULL) {
            winner = candidate;
            base = base_i;
        }
        else if (PyType_IsSubtype(winner, candidate))
            ;
        else if (PyType_IsSubtype(candidate, winner)) {
            winner = candidate;
            base = base_i;
        }
        else {
            PyErr_SetString(
                PyExc_TypeError,
                "multiple bases have "
                "instance lay-out conflict");
            return NULL;
        }
    }
    if (base == NULL)
        PyErr_SetString(PyExc_TypeError,
            "a new-style class can't have only classic bases");
    return base;
}

static const short slotoffsets[] = {
    -1, /* invalid slot */
/* Generated by typeslots.py */
0,
0,
offsetof(PyHeapTypeObject, as_mapping.mp_ass_subscript),
offsetof(PyHeapTypeObject, as_mapping.mp_length),
offsetof(PyHeapTypeObject, as_mapping.mp_subscript),
offsetof(PyHeapTypeObject, as_number.nb_absolute),
offsetof(PyHeapTypeObject, as_number.nb_add),
offsetof(PyHeapTypeObject, as_number.nb_and),
offsetof(PyHeapTypeObject, as_number.nb_nonzero),
offsetof(PyHeapTypeObject, as_number.nb_divmod),
offsetof(PyHeapTypeObject, as_number.nb_float),
offsetof(PyHeapTypeObject, as_number.nb_floor_divide),
offsetof(PyHeapTypeObject, as_number.nb_index),
offsetof(PyHeapTypeObject, as_number.nb_inplace_add),
offsetof(PyHeapTypeObject, as_number.nb_inplace_and),
offsetof(PyHeapTypeObject, as_number.nb_inplace_floor_divide),
offsetof(PyHeapTypeObject, as_number.nb_inplace_lshift),
offsetof(PyHeapTypeObject, as_number.nb_inplace_multiply),
offsetof(PyHeapTypeObject, as_number.nb_inplace_or),
offsetof(PyHeapTypeObject, as_number.nb_inplace_power),
offsetof(PyHeapTypeObject, as_number.nb_inplace_remainder),
offsetof(PyHeapTypeObject, as_number.nb_inplace_rshift),
offsetof(PyHeapTypeObject, as_number.nb_inplace_subtract),
offsetof(PyHeapTypeObject, as_number.nb_inplace_true_divide),
offsetof(PyHeapTypeObject, as_number.nb_inplace_xor),
offsetof(PyHeapTypeObject, as_number.nb_int),
offsetof(PyHeapTypeObject, as_number.nb_invert),
offsetof(PyHeapTypeObject, as_number.nb_lshift),
offsetof(PyHeapTypeObject, as_number.nb_multiply),
offsetof(PyHeapTypeObject, as_number.nb_negative),
offsetof(PyHeapTypeObject, as_number.nb_or),
offsetof(PyHeapTypeObject, as_number.nb_positive),
offsetof(PyHeapTypeObject, as_number.nb_power),
offsetof(PyHeapTypeObject, as_number.nb_remainder),
offsetof(PyHeapTypeObject, as_number.nb_rshift),
offsetof(PyHeapTypeObject, as_number.nb_subtract),
offsetof(PyHeapTypeObject, as_number.nb_true_divide),
offsetof(PyHeapTypeObject, as_number.nb_xor),
offsetof(PyHeapTypeObject, as_sequence.sq_ass_item),
offsetof(PyHeapTypeObject, as_sequence.sq_concat),
offsetof(PyHeapTypeObject, as_sequence.sq_contains),
offsetof(PyHeapTypeObject, as_sequence.sq_inplace_concat),
offsetof(PyHeapTypeObject, as_sequence.sq_inplace_repeat),
offsetof(PyHeapTypeObject, as_sequence.sq_item),
offsetof(PyHeapTypeObject, as_sequence.sq_length),
offsetof(PyHeapTypeObject, as_sequence.sq_repeat),
offsetof(PyHeapTypeObject, ht_type.tp_alloc),
offsetof(PyHeapTypeObject, ht_type.tp_base),
offsetof(PyHeapTypeObject, ht_type.tp_bases),
offsetof(PyHeapTypeObject, ht_type.tp_call),
offsetof(PyHeapTypeObject, ht_type.tp_clear),
offsetof(PyHeapTypeObject, ht_type.tp_dealloc),
offsetof(PyHeapTypeObject, ht_type.tp_del),
offsetof(PyHeapTypeObject, ht_type.tp_descr_get),
offsetof(PyHeapTypeObject, ht_type.tp_descr_set),
offsetof(PyHeapTypeObject, ht_type.tp_doc),
offsetof(PyHeapTypeObject, ht_type.tp_getattr),
offsetof(PyHeapTypeObject, ht_type.tp_getattro),
offsetof(PyHeapTypeObject, ht_type.tp_hash),
offsetof(PyHeapTypeObject, ht_type.tp_init),
offsetof(PyHeapTypeObject, ht_type.tp_is_gc),
offsetof(PyHeapTypeObject, ht_type.tp_iter),
offsetof(PyHeapTypeObject, ht_type.tp_iternext),
offsetof(PyHeapTypeObject, ht_type.tp_methods),
offsetof(PyHeapTypeObject, ht_type.tp_new),
offsetof(PyHeapTypeObject, ht_type.tp_repr),
offsetof(PyHeapTypeObject, ht_type.tp_richcompare),
offsetof(PyHeapTypeObject, ht_type.tp_setattr),
offsetof(PyHeapTypeObject, ht_type.tp_setattro),
offsetof(PyHeapTypeObject, ht_type.tp_str),
offsetof(PyHeapTypeObject, ht_type.tp_traverse),
offsetof(PyHeapTypeObject, ht_type.tp_members),
offsetof(PyHeapTypeObject, ht_type.tp_getset),
offsetof(PyHeapTypeObject, ht_type.tp_free),
offsetof(PyHeapTypeObject, as_number.nb_long),
offsetof(PyHeapTypeObject, as_number.nb_divide),
offsetof(PyHeapTypeObject, as_sequence.sq_slice),
};

PyObject *
PyType_FromSpecWithBases(PyType_Spec *spec, PyObject *bases)
{
    auto res = reinterpret_cast<PyHeapTypeObject *>(PyType_GenericAlloc(&PyType_Type, 0));
    PyTypeObject *type, *base;
    PyObject *modname;
    auto res_start = reinterpret_cast<char *>(res);
    PyType_Slot *slot;

    /* Set the type name and qualname */
    auto s = const_cast<char *>(strrchr(spec->name, '.')); // C++11
    if (s == NULL)
        s = const_cast<char *>(spec->name);
    else
        s++;

    if (res == NULL)
        return NULL;
    type = &res->ht_type;
    /* The flags must be initialized early, before the GC traverses us */
    type->tp_flags = spec->flags | Py_TPFLAGS_HEAPTYPE;
    // was PyUnicode_FromString in Python 3
    res->ht_name = PyString_FromString(s);
    if (!res->ht_name)
        goto fail;
    // no ht_qualname in Python 2
    // res->ht_qualname = res->ht_name;
    // Py_INCREF(res->ht_qualname);
    type->tp_name = spec->name;
    if (!type->tp_name)
        goto fail;

    /* Adjust for empty tuple bases */
    if (!bases) {
        base = &PyBaseObject_Type;
        /* See whether Py_tp_base(s) was specified */
        for (slot = spec->slots; slot->slot; slot++) {
            if (slot->slot == Py_tp_base)
                base = (PyTypeObject *)slot->pfunc; // C++11
            else if (slot->slot == Py_tp_bases) {
                bases = (PyObject *)slot->pfunc; // C++11
                Py_INCREF(bases);
            }
        }
        if (!bases)
            bases = PyTuple_Pack(1, base);
        if (!bases)
            goto fail;
    }
    else
        Py_INCREF(bases);

    /* Calculate best base, and check that all bases are type objects */
    base = best_base(bases);
    if (base == NULL) {
        goto fail;
    }
    if (!PyType_HasFeature(base, Py_TPFLAGS_BASETYPE)) {
        PyErr_Format(PyExc_TypeError,
                     "type '%.100s' is not an acceptable base type",
                     base->tp_name);
        goto fail;
    }

    /* Initialize essential fields */
    // no async in Python 2
    // type->tp_as_async = &res->as_async;
    type->tp_as_number = &res->as_number;
    type->tp_as_sequence = &res->as_sequence;
    type->tp_as_mapping = &res->as_mapping;
    type->tp_as_buffer = &res->as_buffer;
    /* Set tp_base and tp_bases */
    type->tp_bases = bases;
    bases = NULL;
    Py_INCREF(base);
    type->tp_base = base;

    type->tp_basicsize = spec->basicsize;
    type->tp_itemsize = spec->itemsize;

    for (slot = spec->slots; slot->slot; slot++) {
        if (slot->slot < 0
            || (size_t)slot->slot >= Py_ARRAY_LENGTH(slotoffsets)) {
            PyErr_SetString(PyExc_RuntimeError, "invalid slot offset");
            goto fail;
        }
        if (slot->slot == Py_tp_base || slot->slot == Py_tp_bases)
            /* Processed above */
            continue;
        *reinterpret_cast<void **>(res_start + slotoffsets[slot->slot]) = slot->pfunc;

        /* need to make a copy of the docstring slot, which usually
           points to a static string literal */
        if (slot->slot == Py_tp_doc) {
            // No signature in Python 2
            // const char *old_doc = _PyType_DocWithoutSignature(type->tp_name, slot->pfunc);
            const char *old_doc = (const char *)slot->pfunc;
            size_t len = strlen(old_doc)+1;
            char *tp_doc = (char *)PyObject_MALLOC(len); // C++11
            if (tp_doc == NULL) {
                PyErr_NoMemory();
                goto fail;
            }
            memcpy(tp_doc, old_doc, len);
            type->tp_doc = tp_doc;
        }
    }
    if (type->tp_dealloc == NULL) {
        /* It's a heap type, so needs the heap types' dealloc.
           subtype_dealloc will call the base type's tp_dealloc, if
           necessary. */
        type->tp_dealloc = subtype_dealloc;
    }

    if (PyType_Ready(type) < 0)
        goto fail;

    // no ht_cached_keys in Python 2
    // if (type->tp_dictoffset) {
    //     res->ht_cached_keys = _PyDict_NewKeysForClass();
    // }

    /* Set type.__module__ */
    s = (char *)strrchr(spec->name, '.'); // c++11
    if (s != NULL) {
        int err;
        // was PyUnicode_FromStringAndSize in Python 3
        modname = PyString_FromStringAndSize(
                spec->name, (Py_ssize_t)(s - spec->name));
        if (modname == NULL) {
            goto fail;
        }
        // no PyId_ things in Python 2
        // err = _PyDict_SetItemId(type->tp_dict, &PyId___module__, modname);
        err = PyDict_SetItem(type->tp_dict, Shiboken::PyMagicName::module(), modname);
        Py_DECREF(modname);
        if (err != 0)
            goto fail;
    } else {
        // no PyErr_WarnFormat in Python 2
        // if (PyErr_WarnFormat(PyExc_DeprecationWarning, 1,
        //         "builtin type %.200s has no __module__ attribute",
        //         spec->name))
        char msg[250];
        sprintf(msg, "builtin type %.200s has no __module__ attribute", spec->name);
        if (PyErr_WarnEx(PyExc_DeprecationWarning, msg, 1))
            goto fail;
    }

    return reinterpret_cast<PyObject *>(res);

 fail:
    Py_DECREF(res);
    return NULL;
}

PyObject *
PyType_FromSpec(PyType_Spec *spec)
{
    return PyType_FromSpecWithBases(spec, NULL);
}

void *
PyType_GetSlot(PyTypeObject *type, int slot)
{
    if (!PyType_HasFeature(type, Py_TPFLAGS_HEAPTYPE) || slot < 0) {
        PyErr_BadInternalCall();
        return NULL;
    }
    if ((size_t)slot >= Py_ARRAY_LENGTH(slotoffsets)) {
        /* Extension module requesting slot from a future version */
        return NULL;
    }
    return  *reinterpret_cast<void **>(reinterpret_cast<char *>(type) + slotoffsets[slot]);
}

} // extern "C"
#endif // PY_MAJOR_VERSION < 3
