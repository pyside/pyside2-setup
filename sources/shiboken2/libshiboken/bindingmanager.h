/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef BINDINGMANAGER_H
#define BINDINGMANAGER_H

#include "sbkpython.h"
#include <set>
#include "shibokenmacros.h"

struct SbkObject;
struct SbkObjectType;

namespace Shiboken
{

struct DestructorEntry;

typedef void (*ObjectVisitor)(SbkObject *, void *);

class LIBSHIBOKEN_API BindingManager
{
public:
    BindingManager(const BindingManager &) = delete;
    BindingManager(BindingManager &&) = delete;
    BindingManager &operator=(const BindingManager &) = delete;
    BindingManager &operator=(BindingManager &&) = delete;

    static BindingManager &instance();

    bool hasWrapper(const void *cptr);

    void registerWrapper(SbkObject *pyObj, void *cptr);
    void releaseWrapper(SbkObject *wrapper);

    void runDeletionInMainThread();
    void addToDeletionInMainThread(const DestructorEntry &);

    SbkObject *retrieveWrapper(const void *cptr);
    PyObject *getOverride(const void *cptr, const char *methodName);

    void addClassInheritance(SbkObjectType *parent, SbkObjectType *child);
    /**
     * \deprecated Use \fn resolveType(void **, SbkObjectType *), this version is broken when used with multiple inheritance
     *             because the \p cptr pointer of the discovered type may be different of the given \p cptr in case
     *             of multiple inheritance
     */
    SBK_DEPRECATED(SbkObjectType *resolveType(void *cptr, SbkObjectType *type));
    /**
     * Try to find the correct type of *cptr knowing that it's at least of type \p type.
     * In case of multiple inheritance this function may change the contents of cptr.
     * \param cptr a pointer to a pointer to the instance of type \p type
     * \param type type of *cptr
     * \warning This function is slow, use it only as last resort.
     */
    SbkObjectType *resolveType(void **cptr, SbkObjectType *type);

    std::set<PyObject *> getAllPyObjects();

    /**
     * Calls the function \p visitor for each object registered on binding manager.
     * \note As various C++ pointers can point to the same PyObject due to multiple inheritance
     *       a PyObject can be called more than one time for each PyObject.
     * \param visitor function called for each object.
     * \param data user data passed as second argument to the visitor function.
     */
    void visitAllPyObjects(ObjectVisitor visitor, void *data);

private:
    ~BindingManager();
    BindingManager();

    struct BindingManagerPrivate;
    BindingManagerPrivate *m_d;
};

} // namespace Shiboken

#endif // BINDINGMANAGER_H

