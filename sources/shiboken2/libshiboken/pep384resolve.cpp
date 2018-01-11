/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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

#if PY_MAJOR_VERSION >= 3

extern "C"
{

/*****************************************************************************
 *
 * Support for pydebug.h
 *
 */
static PyObject *sys_flags = NULL;

int
Py_GetFlag(const char *name)
{
    static int initialized = 0;
    int ret = -1;

    if (!initialized) {
        sys_flags = PySys_GetObject("flags");
        // func gives no error if NULL is returned and does not incref.
        Py_XINCREF(sys_flags);
        initialized = 1;
    }
    if (sys_flags != NULL) {
        PyObject *ob_ret = PyObject_GetAttrString(sys_flags, name);
        if (ob_ret != NULL) {
            long long_ret = PyLong_AsLong(ob_ret);
            Py_DECREF(ob_ret);
            ret = (int) long_ret;
        }
    }
    return ret;
}

int
Py_GetVerboseFlag()
{
    static int initialized = 0;
    static int verbose_flag = -1;

    if (!initialized) {
        verbose_flag = Py_GetFlag("verbose");
        if (verbose_flag != -1)
            initialized = 1;
    }
    return verbose_flag;
}

/*****************************************************************************
 *
 * Support for datetime.h
 *
 */
PyAPI_FUNC(void) _PyObject_Dump(PyObject *);

// init_datetime is called earlier than our module init.
// We use the provided PyDateTime_IMPORT machinery.
datetime_struc *
init_DateTime(void)
{
    static int initialized = 0;

#define GETCHECK(name) \
    ({  PyObject *_V = PyObject_GetAttrString(PyDateTimeAPI->module, name); \
        if (_V == NULL)                                                 \
            Py_FatalError("datetime." name " not found, aborting");     \
        (PyTypeObject *)_V;                                             \
    })

    if (!initialized) {
        PyDateTimeAPI = (datetime_struc *)malloc(sizeof(datetime_struc));
        if (PyDateTimeAPI == NULL)
            Py_FatalError("PyDateTimeAPI malloc error, aborting");
        PyDateTimeAPI->module = PyImport_ImportModule("datetime");
        if (PyDateTimeAPI->module == NULL)
            Py_FatalError("datetime module not found, aborting");
        _PyObject_Dump(PyDateTimeAPI->module);
        PyDateTimeAPI->DateType     = GETCHECK("date");
        PyDateTimeAPI->DateTimeType = GETCHECK("datetime");
        PyDateTimeAPI->TimeType     = GETCHECK("time");
        PyDateTimeAPI->DeltaType    = GETCHECK("timedelta");
        PyDateTimeAPI->TZInfoType   = GETCHECK("tzinfo");
#undef GETCHECK
        initialized = 1;
    }
    return PyDateTimeAPI;
}

int
PyDateTime_Get(PyObject *ob, const char *name)
{
    PyObject *ob_ret;
    int ret = -1;

    ob_ret = PyObject_GetAttrString(ob, name);
    if (ob_ret != NULL) {
        long long_ret = PyLong_AsLong(ob_ret);
        Py_DECREF(ob_ret);
        ret = (int) long_ret;
    }
    return ret;
}

PyObject *
PyDate_FromDate(int year, int month, int day)
{
    return PyObject_CallFunction((PyObject *)PyDateTimeAPI->DateType,
                                 (char *)"(iii)", year, month, day);
}

PyObject *
PyDateTime_FromDateAndTime(int year, int month, int day,
                           int hour, int min, int sec, int usec)
{
    return PyObject_CallFunction((PyObject *)PyDateTimeAPI->DateTimeType,
                                 (char *)"(iiiiiii)", year, month, day,
                                  hour, min, sec, usec);
}

PyObject *
PyTime_FromTime(int hour, int min, int sec, int usec)
{
    return PyObject_CallFunction((PyObject *)PyDateTimeAPI->TimeType,
                                 (char *)"(iiii)", hour, min, sec, usec);
}

/*****************************************************************************
 *
 * Module Initialization
 *
 */

void
PEP384_Init()
{
#ifdef Py_LIMITED_API
    Py_GetVerboseFlag();
#endif
}

} // extern "C"

#endif // PY_MAJOR_VERSION >= 3
