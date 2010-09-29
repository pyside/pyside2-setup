/*
* This file is part of the Shiboken Python Bindings Generator project.
*
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
*
* Contact: PySide team <contact@pyside.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef BASEWRAPPER_P_H
#define BASEWRAPPER_P_H

#include <Python.h>
#include <list>

namespace Shiboken
{
/**
 * Utility function uset to transform PyObject which suppot sequence protocol in a std::list 
 **/
std::list<PyObject*> splitPyObject(PyObject* pyObj);

struct SbkBaseWrapperType;

/**
*   Visitor class used by walkOnClassHierarchy function.
*/
class HierarchyVisitor
{
public:
    HierarchyVisitor() : m_wasFinished(false) {}
    virtual ~HierarchyVisitor() {}
    virtual void visit(SbkBaseWrapperType* node) = 0;
    void finish() { m_wasFinished = true; };
    bool wasFinished() const { return m_wasFinished; }
private:
    bool m_wasFinished;
};

class BaseCountVisitor : public HierarchyVisitor
{
public:
    BaseCountVisitor() : m_count(0) {}

    void visit(SbkBaseWrapperType*)
    {
        m_count++;
    }

    int count() const { return m_count; }
private:
    int m_count;
};

class BaseAccumulatorVisitor : public HierarchyVisitor
{
public:
    BaseAccumulatorVisitor() {}

    void visit(SbkBaseWrapperType* node)
    {
        m_bases.push_back(node);
    }

    std::list<SbkBaseWrapperType*> bases() const { return m_bases; }
private:
    std::list<SbkBaseWrapperType*> m_bases;
};

class GetIndexVisitor : public HierarchyVisitor
{
public:
    GetIndexVisitor(PyTypeObject* desiredType) : m_index(-1), m_desiredType(desiredType) {}
    virtual void visit(SbkBaseWrapperType* node)
    {
        m_index++;
        if (PyType_IsSubtype(reinterpret_cast<PyTypeObject*>(node), m_desiredType))
            finish();
    }
    int index() const { return m_index; }

private:
    int m_index;
    PyTypeObject* m_desiredType;
};

/// \internal Internal function used to walk on classes inheritance trees.
/**
*   Walk on class hierarchy using a DFS algorithm.
*   For each pure Shiboken type found, HiearchyVisitor::visit is called and the algorithm consider
*   all children of this type as visited.
*/
void walkThroughClassHierarchy(PyTypeObject* currentType, HierarchyVisitor* visitor);

inline int getTypeIndexOnHierarchy(PyTypeObject* baseType, PyTypeObject* desiredType)
{
    GetIndexVisitor visitor(desiredType);
    walkThroughClassHierarchy(baseType, &visitor);
    return visitor.index();
}

inline int getNumberOfCppBaseClasses(PyTypeObject* baseType)
{
    BaseCountVisitor visitor;
    walkThroughClassHierarchy(baseType, &visitor);
    return visitor.count();
}

inline std::list<SbkBaseWrapperType*> getCppBaseClasses(PyTypeObject* baseType)
{
    BaseAccumulatorVisitor visitor;
    walkThroughClassHierarchy(baseType, &visitor);
    return visitor.bases();
}

struct SbkBaseWrapper;

/// Linked list of SbkBaseWrapper pointers
typedef std::list<SbkBaseWrapper*> ChildrenList;

/// Struct used to store information about object parent and children.
struct ParentInfo
{
    /// Default ctor.
    ParentInfo() : parent(0), hasWrapperRef(false) {}
    /// Pointer to parent object.
    SbkBaseWrapper* parent;
    /// List of object children.
    ChildrenList children;
    /// has internal ref
    bool hasWrapperRef;
};

/**
*   Decrements the reference counters of every object referred by self.
*   \param self    the wrapper instance that keeps references to other objects.
*/
void clearReferences(SbkBaseWrapper* self);

}

#endif
