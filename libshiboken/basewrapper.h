/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation. Please
 * review the following information to ensure the GNU Lesser General
 * Public License version 2.1 requirements will be met:
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * As a special exception to the GNU Lesser General Public License
 * version 2.1, the object code form of a "work that uses the Library"
 * may incorporate material from a header file that is part of the
 * Library.  You may distribute such object code under terms of your
 * choice, provided that the incorporated material (i) does not exceed
 * more than 5% of the total size of the Library; and (ii) is limited to
 * numerical parameters, data structure layouts, accessors, macros,
 * inline functions and templates.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef BASEWRAPPER_H
#define BASEWRAPPER_H

#include <Python.h>
#include "python25compat.h"
#include "bindingmanager.h"
#include <list>
#include <map>
#include <string>

namespace Shiboken
{

/**
 * This mapping associates a method and argument of an wrapper object with the wrapper of
 * said argument when it needs the binding to help manage its reference counting.
 */
typedef std::map<std::string, std::list<PyObject*> > RefCountMap;

extern "C"
{
/// Function signature for the multiple inheritance information initializers that should be provided by classes with multiple inheritance.
typedef int* (*MultipleInheritanceInitFunction)(const void*);
struct SbkBaseWrapperType;

/**
*   Special cast function is used to correctly cast an object when it's
*   part of a multiple inheritance hierarchy.
*   The implementation of this function is auto generated by the generator and you don't need to care about it.
*/
typedef void* (*SpecialCastFunction)(void*, SbkBaseWrapperType*);
typedef void* (*ObjectCopierFunction)(const void*);
typedef SbkBaseWrapperType* (*TypeDiscoveryFunc)(void*, SbkBaseWrapperType*);

typedef void* (*ExtendedToCppFunc)(PyObject*);
typedef bool (*ExtendedIsConvertibleFunc)(PyObject*);

// Used in userdata dealloc function
typedef void (*DeleteUserDataFunc)(void*);

extern LIBSHIBOKEN_API PyTypeObject SbkBaseWrapperType_Type;
extern LIBSHIBOKEN_API SbkBaseWrapperType SbkBaseWrapper_Type;

/// PyTypeObject extended with C++ multiple inheritance information.
struct LIBSHIBOKEN_API SbkBaseWrapperType
{
    PyHeapTypeObject super;
    int* mi_offsets;
    MultipleInheritanceInitFunction mi_init;
    /// Special cast function, null if this class doesn't have multiple inheritance.
    SpecialCastFunction mi_specialcast;
    TypeDiscoveryFunc type_discovery;
    ObjectCopierFunction obj_copier;
    /// Extended "isConvertible" function to be used when a conversion operator is defined in another module.
    ExtendedIsConvertibleFunc ext_isconvertible;
    /// Extended "toCpp" function to be used when a conversion operator is defined in another module.
    ExtendedToCppFunc ext_tocpp;
    /// Pointer to a function responsible for deletetion of the C++ instance calling the proper destructor.
    void (*cpp_dtor)(void*);
    /// True if this type holds two or more C++ instances, e.g.: a Python class which inherits from two C++ classes.
    int is_multicpp:1;
    /// True if this type was definied by the user.
    int is_user_type:1;
    /// C++ name
    const char* original_name;
    /// Type user data
    void *user_data;
    DeleteUserDataFunc d_func;
};

struct ParentInfo;

/// Base Python object for all the wrapped C++ classes.
struct LIBSHIBOKEN_API SbkBaseWrapper
{
    PyObject_HEAD
    /// Pointer to the C++ class.
    void** cptr;
    /// Instance dictionary.
    PyObject* ob_dict;
    /// True when Python is responsible for freeing the used memory.
    unsigned int hasOwnership : 1;
    /// Is true when the C++ class of the wrapped object has a virtual destructor AND was created by Python.
    unsigned int containsCppWrapper : 1;
    /// Marked as false when the object is lost to C++ and the binding can not know if it was deleted or not.
    unsigned int validCppObject : 1;
    /// Information about the object parents and children, can be null.
    ParentInfo* parentInfo;
    /// List of weak references
    PyObject* weakreflist;
    /// Manage reference counting of objects that are referred but not owned.
    RefCountMap* referredObjects;
};

} // extern "C"

/**
*   Init shiboken library.
*/
LIBSHIBOKEN_API void initShiboken();

/**
*   Set the parent of \p child to \p parent.
*   When an object dies, all their children, granchildren, etc, are tagged as invalid.
*   \param parent the parent object, if null, the child will have no parents.
*   \param child the child.
*/
LIBSHIBOKEN_API void setParent(PyObject* parent, PyObject* child);

/**
*   Remove this child from their parent, if any.
*   \param child the child.
*/
LIBSHIBOKEN_API void removeParent(SbkBaseWrapper* child);

/**
* \internal This is an internal function called by SbkBaseWrapper_Dealloc, it's exported just for techinical reasons.
* \note Do not call this function inside your bindings.
*/
LIBSHIBOKEN_API void destroyParentInfo(SbkBaseWrapper* obj, bool removeFromParent = true);

/**
*   Returns true if the object is an instance of a type created by the Shiboken generator.
*/
inline bool isShibokenType(PyObject*& pyObj)
{
    return pyObj->ob_type->ob_type == &SbkBaseWrapperType_Type;
}

/**
*   Returns true if this object is an instance of an user defined type derived from an Shiboken type.
*/
inline bool isUserType(PyObject*& pyObj)
{
    return isShibokenType(pyObj) && reinterpret_cast<SbkBaseWrapperType*>(pyObj->ob_type)->is_user_type;
}

/**
*   Get the C++ pointer of type \p desiredType from a Python object.
*/
LIBSHIBOKEN_API void* getCppPointer(PyObject* wrapper, PyTypeObject* desiredType);

/**
*   Set the C++ pointer of type \p desiredType of a Python object.
*/
LIBSHIBOKEN_API bool setCppPointer(SbkBaseWrapper* wrapper, PyTypeObject* desiredType, void* cptr);

/**
 *   Get/Set Userdata in type class
 */
LIBSHIBOKEN_API void setTypeUserData(SbkBaseWrapper* wrapper, void* user_data, DeleteUserDataFunc d_func);
LIBSHIBOKEN_API void* getTypeUserData(SbkBaseWrapper* wrapper);

/**
*   Returns true if the constructor of \p ctorType can be called for a instance of type \p myType.
*   \note This function set a python error when returning false.
*/
LIBSHIBOKEN_API bool canCallConstructor(PyTypeObject* myType, PyTypeObject* ctorType);

/**
 * Shiboken_TypeCheck macro performs a type check using the values registered with SbkType<>() template.
 */
#define Shiboken_TypeCheck(pyobj, type) (PyObject_TypeCheck(pyobj, SbkType<type>()))

#define SbkBaseWrapper_Check(op) PyObject_TypeCheck(op, (PyTypeObject*)&Shiboken::SbkBaseWrapper_Type)
#define SbkBaseWrapper_CheckExact(op) ((op)->ob_type == &Shiboken::SbkBaseWrapper_Type)

#define SbkBaseWrapper_instanceDict(pyobj)           (((Shiboken::SbkBaseWrapper*)pyobj)->ob_dict)
#define SbkBaseWrapper_setInstanceDict(pyobj,d)      (((Shiboken::SbkBaseWrapper*)pyobj)->ob_dict = d)
#define SbkBaseWrapper_hasOwnership(pyobj)           (((Shiboken::SbkBaseWrapper*)pyobj)->hasOwnership)
#define SbkBaseWrapper_setOwnership(pyobj,o)         (((Shiboken::SbkBaseWrapper*)pyobj)->hasOwnership = o)
#define SbkBaseWrapper_hasParentInfo(pyobj)          (((Shiboken::SbkBaseWrapper*)pyobj)->parentInfo)
#define SbkBaseWrapper_containsCppWrapper(pyobj)     (((Shiboken::SbkBaseWrapper*)pyobj)->containsCppWrapper)
#define SbkBaseWrapper_setContainsCppWrapper(pyobj,o)(((Shiboken::SbkBaseWrapper*)pyobj)->containsCppWrapper = o)
#define SbkBaseWrapper_validCppObject(pyobj)         (((Shiboken::SbkBaseWrapper*)pyobj)->validCppObject)
#define SbkBaseWrapper_setValidCppObject(pyobj,v)    (((Shiboken::SbkBaseWrapper*)pyobj)->validCppObject = v)

LIBSHIBOKEN_API PyObject*
SbkBaseWrapper_New(SbkBaseWrapperType* instanceType,
                   void* cptr,
                   bool hasOwnership = true,
                   bool isExactType = false,
                   const char* typeName = 0);

LIBSHIBOKEN_API PyObject*
SbkBaseWrapper_TpNew(PyTypeObject* subtype, PyObject*, PyObject*);

/**
 *   Increments the reference count of the referred Python object.
 *   A previous Python object in the same position identified by the 'key' parameter
 *   will have its reference counter decremented automatically when replaced.
 *   All the kept references should be decremented when the Python wrapper indicated by
 *   'self' dies.
 *   No checking is done for any of the passed arguments, since it is meant to be used
 *   by generated code it is supposed that the generator is correct.
 *   \param self            the wrapper instance that keeps references to other objects.
 *   \param key             a key that identifies the C++ method signature and argument where the referredObject came from.
 *   \parem referredObject  the object whose reference is used by the self object.
 */
LIBSHIBOKEN_API void keepReference(SbkBaseWrapper* self, const char* key, PyObject* referredObject, bool append=false);

/// Returns true and sets a Python RuntimeError if the Python wrapper is not marked as valid.
LIBSHIBOKEN_API bool cppObjectIsInvalid(PyObject* wrapper);

/// Dealloc the python object \p pyObj and the C++ object represented by it.
LIBSHIBOKEN_API void deallocWrapper(PyObject* pyObj);

/// Delete the class T allocated on \p cptr.
template<typename T>
void callCppDestructor(void* cptr)
{
    delete reinterpret_cast<T*>(cptr);
}

LIBSHIBOKEN_API void deallocWrapperWithPrivateDtor(PyObject* self);
LIBSHIBOKEN_API bool importModule(const char* moduleName, PyTypeObject*** cppApiPtr);
LIBSHIBOKEN_API void setErrorAboutWrongArguments(PyObject* args, const char* funcName, const char** cppOverloads);

} // namespace Shiboken

#endif // BASEWRAPPER_H

