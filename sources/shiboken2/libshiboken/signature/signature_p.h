/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef SIGNATURE_IMPL_H
#define SIGNATURE_IMPL_H

#include "signature.h"

extern "C" {

// signature_globals.cpp

typedef struct safe_globals_struc {
    // init part 1: get arg_dict
    PyObject *helper_module;
    PyObject *arg_dict;
    PyObject *map_dict;
    PyObject *value_dict;       // for writing signatures
    PyObject *feature_dict;     // registry for PySide.support.__feature__
    // init part 2: run module
    PyObject *pyside_type_init_func;
    PyObject *create_signature_func;
    PyObject *seterror_argument_func;
    PyObject *make_helptext_func;
    PyObject *finish_import_func;
} safe_globals_struc, *safe_globals;

extern safe_globals pyside_globals;
extern PyMethodDef signature_methods[];

void init_module_1(void);
void init_module_2(void);

// signature.cpp

PyObject *GetTypeKey(PyObject *ob);

PyObject *GetSignature_Function(PyObject *, PyObject *);
PyObject *GetSignature_TypeMod(PyObject *, PyObject *);
PyObject *GetSignature_Wrapper(PyObject *, PyObject *);

PyObject *get_signature_intern(PyObject *ob, PyObject *modifier);
PyObject *PySide_BuildSignatureProps(PyObject *class_mod);
PyObject *GetClassOrModOf(PyObject *ob);

// signature_extend.cpp

PyObject *pyside_cf_get___signature__(PyObject *func, PyObject *modifier);
PyObject *pyside_sm_get___signature__(PyObject *sm, PyObject *modifier);
PyObject *pyside_md_get___signature__(PyObject *ob_md, PyObject *modifier);
PyObject *pyside_wd_get___signature__(PyObject *ob, PyObject *modifier);
PyObject *pyside_tp_get___signature__(PyObject *obtype_mod, PyObject *modifier);

int PySide_PatchTypes(void);
PyObject *pyside_tp_get___doc__(PyObject *tp);

// signature_helper.cpp

PyObject *_get_qualname(PyObject *ob);
int add_more_getsets(PyTypeObject *type, PyGetSetDef *gsp, PyObject **doc_descr);
PyObject *name_key_to_func(PyObject *ob);
PyObject *_get_class_of_cf(PyObject *ob_cf);
PyObject *_get_class_of_sm(PyObject *ob_sm);
PyObject *_get_class_of_descr(PyObject *ob);
PyObject *_address_to_stringlist(PyObject *numkey);
int _finish_nested_classes(PyObject *dict);

} // extern "C"

#endif // SIGNATURE_IMPL_H
