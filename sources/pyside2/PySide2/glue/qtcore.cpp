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

/*********************************************************************
 * INJECT CODE
 ********************************************************************/

// @snippet include-pyside
#include <pyside.h>
// @snippet include-pyside

// @snippet pystring-check
bool py2kStrCheck(PyObject *obj)
{
#ifdef IS_PY3K
    return false;
#else
    return PyString_Check(obj);
#endif
}
// @snippet pystring-check

// @snippet qvariant-conversion
static const char *QVariant_resolveMetaType(PyTypeObject *type, int *typeId)
{
    if (PyObject_TypeCheck(type, SbkObjectType_TypeF())) {
        SbkObjectType* sbkType = (SbkObjectType*)type;
        const char* typeName = Shiboken::ObjectType::getOriginalName(sbkType);
        if (!typeName)
            return nullptr;
        const bool valueType = '*' != typeName[qstrlen(typeName) - 1];
        // Do not convert user type of value
        if (valueType && Shiboken::ObjectType::isUserType(type))
            return nullptr;
        int obTypeId = QMetaType::type(typeName);
        if (obTypeId) {
            *typeId = obTypeId;
            return typeName;
        }
        // Do not resolve types to value type
        if (valueType)
            return 0;
        // Find in base types. First check tp_bases, and only after check tp_base, because
        // tp_base does not always point to the first base class, but rather to the first
        // that has added any python fields or slots to its object layout.
        // See https://mail.python.org/pipermail/python-list/2009-January/520733.html
        if (type->tp_bases) {
            for (int i = 0, size = PyTuple_GET_SIZE(type->tp_bases); i < size; ++i) {
                const char *derivedName = QVariant_resolveMetaType(reinterpret_cast<PyTypeObject *>(PyTuple_GET_ITEM(
                                          type->tp_bases, i)), typeId);
                if (derivedName)
                    return derivedName;
            }
        }
        else if (type->tp_base) {
            return QVariant_resolveMetaType(type->tp_base, typeId);
        }
    }
    *typeId = 0;
    return 0;
}
static QVariant QVariant_convertToValueList(PyObject *list)
{
    if (PySequence_Size(list) < 0) {
         // clear the error if < 0 which means no length at all
         PyErr_Clear();
         return QVariant();
    }

    Shiboken::AutoDecRef element(PySequence_GetItem(list, 0));
    int typeId;
    const char *typeName = QVariant_resolveMetaType(element.cast<PyTypeObject*>(), &typeId);
    if (typeName) {
        QByteArray listTypeName("QList<");
        listTypeName += typeName;
        listTypeName += '>';
        typeId = QMetaType::type(listTypeName);
        if (typeId > 0) {
            Shiboken::Conversions::SpecificConverter converter(listTypeName);
            if (converter) {
                QVariant var(typeId, nullptr);
                converter.toCpp(list, &var);
                return var;
            }
            qWarning() << "Type converter for :" << listTypeName << "not registered.";
        }
    }
    return QVariant();
}
static bool QVariant_isStringList(PyObject *list)
{
    if (!PySequence_Check(list)) {
        // If it is not a list or a derived list class
        // we assume that will not be a String list neither.
        return false;
    }

    if (PySequence_Size(list) < 0) {
        // clear the error if < 0 which means no length at all
        PyErr_Clear();
        return false;
    }

    Shiboken::AutoDecRef fast(PySequence_Fast(list, "Failed to convert QVariantList"));
    const Py_ssize_t size = PySequence_Fast_GET_SIZE(fast.object());
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *item = PySequence_Fast_GET_ITEM(fast.object(), i);
        if (!%CHECKTYPE[QString](item))
            return false;
    }
    return true;
}
static QVariant QVariant_convertToVariantMap(PyObject *map)
{
    Py_ssize_t pos = 0;
    Shiboken::AutoDecRef keys(PyDict_Keys(map));
    if (!QVariant_isStringList(keys))
        return QVariant();
    PyObject *key;
    PyObject *value;
    QMap<QString,QVariant> ret;
    while (PyDict_Next(map, &pos, &key, &value)) {
        QString cppKey = %CONVERTTOCPP[QString](key);
        QVariant cppValue = %CONVERTTOCPP[QVariant](value);
        ret.insert(cppKey, cppValue);
    }
    return QVariant(ret);
}
static QVariant QVariant_convertToVariantList(PyObject *list)
{
    if (QVariant_isStringList(list)) {
        QList<QString > lst = %CONVERTTOCPP[QList<QString>](list);
        return QVariant(QStringList(lst));
    }
    QVariant valueList = QVariant_convertToValueList(list);
    if (valueList.isValid())
        return valueList;

    if (PySequence_Size(list) < 0) {
        // clear the error if < 0 which means no length at all
        PyErr_Clear();
        return QVariant();
    }

    QList<QVariant> lst;
    Shiboken::AutoDecRef fast(PySequence_Fast(list, "Failed to convert QVariantList"));
    const Py_ssize_t size = PySequence_Fast_GET_SIZE(fast.object());
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *pyItem = PySequence_Fast_GET_ITEM(fast.object(), i);
        QVariant item = %CONVERTTOCPP[QVariant](pyItem);
        lst.append(item);
    }
    return QVariant(lst);
}
// @snippet qvariant-conversion

// @snippet qvariantmap-register
Shiboken::Conversions::registerConverterName(SbkPySide2_QtCoreTypeConverters[SBK_QTCORE_QMAP_QSTRING_QVARIANT_IDX], "QVariantMap");
// @snippet qvariantmap-register

// @snippet qvariantmap-check
static bool QVariantType_isStringList(PyObject *list)
{
    Shiboken::AutoDecRef fast(PySequence_Fast(list, "Failed to convert QVariantList"));
    const Py_ssize_t size = PySequence_Fast_GET_SIZE(fast.object());
    for (Py_ssize_t i=0; i < size; i++) {
        PyObject *item = PySequence_Fast_GET_ITEM(fast.object(), i);
        if (!%CHECKTYPE[QString](item))
            return false;
    }
    return true;
}
static bool QVariantType_checkAllStringKeys(PyObject *dict)
{
    Shiboken::AutoDecRef keys(PyDict_Keys(dict));
    return QVariantType_isStringList(keys);
}
// @snippet qvariantmap-check

// @snippet qt-qabs
double _abs = qAbs(%1);
%PYARG_0 = %CONVERTTOPYTHON[double](_abs);
// @snippet qt-qabs

// @snippet qt-postroutine
namespace PySide {
static QStack<PyObject*> globalPostRoutineFunctions;
void globalPostRoutineCallback()
{
    Shiboken::GilState state;
    foreach (PyObject *callback, globalPostRoutineFunctions) {
        Shiboken::AutoDecRef result(PyObject_CallObject(callback, NULL));
        Py_DECREF(callback);
    }
    globalPostRoutineFunctions.clear();
}
void addPostRoutine(PyObject *callback)
{
    if (PyCallable_Check(callback)) {
        globalPostRoutineFunctions << callback;
        Py_INCREF(callback);
    } else {
        PyErr_SetString(PyExc_TypeError, "qAddPostRoutine: The argument must be a callable object.");
    }
}
} // namespace
// @snippet qt-postroutine

// @snippet qt-addpostroutine
PySide::addPostRoutine(%1);
// @snippet qt-addpostroutine

// @snippet qt-qaddpostroutine
qAddPostRoutine(PySide::globalPostRoutineCallback);
// @snippet qt-qaddpostroutine

// @snippet qt-version
QList<QByteArray> version = QByteArray(qVersion()).split('.');
PyObject *pyQtVersion = PyTuple_New(3);
for (int i = 0; i < 3; ++i)
    PyTuple_SET_ITEM(pyQtVersion, i, PyInt_FromLong(version[i].toInt()));
PyModule_AddObject(module, "__version_info__", pyQtVersion);
PyModule_AddStringConstant(module, "__version__", qVersion());
// @snippet qt-version

// @snippet qobject-connect-1
// %FUNCTION_NAME() - disable generation of function call.
bool %0 = qobjectConnect(%1, %2, %CPPSELF, %3, %4);
%PYARG_0 = %CONVERTTOPYTHON[bool](%0);
// @snippet qobject-connect-1

// @snippet qobject-connect-2
// %FUNCTION_NAME() - disable generation of function call.
bool %0 = qobjectConnect(%1, %2, %3, %4, %5);
%PYARG_0 = %CONVERTTOPYTHON[bool](%0);
// @snippet qobject-connect-2

// @snippet qobject-connect-3
// %FUNCTION_NAME() - disable generation of function call.
bool %0 = qobjectConnect(%1, %2, %3, %4, %5);
%PYARG_0 = %CONVERTTOPYTHON[bool](%0);
// @snippet qobject-connect-3

// @snippet qobject-connect-4
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectConnectCallback(%1, %2, %PYARG_3, %4);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-connect-4

// @snippet qobject-connect-5
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectConnectCallback(%CPPSELF, %1, %PYARG_2, %3);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-connect-5

// @snippet qobject-connect-6
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectConnect(%CPPSELF, %1, %2, %3, %4);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-connect-6

// @snippet qobject-emit
%RETURN_TYPE %0 = PySide::SignalManager::instance().emitSignal(%CPPSELF, %1, %PYARG_2);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-emit

// @snippet qobject-disconnect-1
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectDisconnectCallback(%CPPSELF, %1, %2);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-disconnect-1

// @snippet qobject-disconnect-2
// %FUNCTION_NAME() - disable generation of function call.
%RETURN_TYPE %0 = qobjectDisconnectCallback(%1, %2, %3);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-disconnect-2

// @snippet qfatal
// qFatal doesn't have a stream version, so we do a
// qWarning call followed by a qFatal() call using a
// literal.
qWarning() << %1;
qFatal("[A qFatal() call was made from Python code]");
// @snippet qfatal

// @snippet moduleshutdown
PySide::runCleanupFunctions();
// @snippet moduleshutdown

// @snippet qt-pysideinit
Shiboken::Conversions::registerConverterName(SbkPySide2_QtCoreTypeConverters[SBK_QSTRING_IDX], "unicode");
Shiboken::Conversions::registerConverterName(SbkPySide2_QtCoreTypeConverters[SBK_QSTRING_IDX], "str");
Shiboken::Conversions::registerConverterName(SbkPySide2_QtCoreTypeConverters[SBK_QTCORE_QLIST_QVARIANT_IDX], "QVariantList");

PySide::registerInternalQtConf();
PySide::init(module);
Py_AtExit(QtCoreModuleExit);
// @snippet qt-pysideinit

// @snippet qt-messagehandler
// Define a global variable to handle qInstallMessageHandler callback
static PyObject *qtmsghandler = nullptr;

static void msgHandlerCallback(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    Shiboken::GilState state;
    Shiboken::AutoDecRef arglist(PyTuple_New(3));
    PyTuple_SET_ITEM(arglist, 0, %CONVERTTOPYTHON[QtMsgType](type));
    PyTuple_SET_ITEM(arglist, 1, %CONVERTTOPYTHON[QMessageLogContext &](ctx));
    QByteArray array = msg.toLocal8Bit();
    char *data = array.data();
    PyTuple_SET_ITEM(arglist, 2, %CONVERTTOPYTHON[char *](data));
    Shiboken::AutoDecRef ret(PyObject_CallObject(qtmsghandler, arglist));
}
static void QtCoreModuleExit()
{
    PySide::SignalManager::instance().clear();
}
// @snippet qt-messagehandler

// @snippet qt-installmessagehandler
if (%PYARG_1 == Py_None) {
  qInstallMessageHandler(0);
  %PYARG_0 = qtmsghandler ? qtmsghandler : Py_None;
  qtmsghandler = 0;
} else if (!PyCallable_Check(%PYARG_1)) {
  PyErr_SetString(PyExc_TypeError, "parameter must be callable");
} else {
  %PYARG_0 = qtmsghandler ? qtmsghandler : Py_None;
  Py_INCREF(%PYARG_1);
  qtmsghandler = %PYARG_1;
  qInstallMessageHandler(msgHandlerCallback);
}

if (%PYARG_0 == Py_None)
    Py_INCREF(%PYARG_0);
// @snippet qt-installmessagehandler

// @snippet qline-hash
namespace PySide {
    template<> inline uint hash(const QLine &v) {
        return qHash(qMakePair(qMakePair(v.x1(), v.y1()), qMakePair(v.x2(), v.y2())));
    }
};
// @snippet qline-hash

// @snippet qlinef-intersect
QPointF p;
%RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(%ARGUMENT_NAMES, &p);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](retval));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[QPointF](p));
// @snippet qlinef-intersect

// @snippet qresource-data
const void *d = %CPPSELF.%FUNCTION_NAME();
if (d) {
    %PYARG_0 = Shiboken::Buffer::newObject(d, %CPPSELF.size());
} else {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
}
// @snippet qresource-data

// @snippet qdate-topython
if (!PyDateTimeAPI)
    PySideDateTime_IMPORT;
%PYARG_0 = PyDate_FromDate(%CPPSELF.year(), %CPPSELF.month(), %CPPSELF.day());
// @snippet qdate-topython

// @snippet qdate-getdate
int year, month, day;
%BEGIN_ALLOW_THREADS
%CPPSELF.%FUNCTION_NAME(&year, &month, &day);
%END_ALLOW_THREADS
%PYARG_0 = PyTuple_New(3);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[int](year));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[int](month));
PyTuple_SET_ITEM(%PYARG_0, 2, %CONVERTTOPYTHON[int](day));
// @snippet qdate-getdate

// @snippet qdate-weeknumber
int yearNumber;
%BEGIN_ALLOW_THREADS
int week = %CPPSELF.%FUNCTION_NAME(&yearNumber);
%END_ALLOW_THREADS
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[int](week));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[int](yearNumber));
// @snippet qdate-weeknumber

// @snippet qdatetime-1
QDate date(%1, %2, %3);
QTime time(%4, %5, %6, %7);
%0 = new %TYPE(date, time, Qt::TimeSpec(%8));
// @snippet qdatetime-1

// @snippet qdatetime-2
QDate date(%1, %2, %3);
QTime time(%4, %5, %6);
%0 = new %TYPE(date, time);
// @snippet qdatetime-2

// @snippet qdatetime-topython
QDate date = %CPPSELF.date();
QTime time = %CPPSELF.time();
if (!PyDateTimeAPI) PySideDateTime_IMPORT;
%PYARG_0 = PyDateTime_FromDateAndTime(date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second(), time.msec()*1000);
// @snippet qdatetime-topython

// @snippet qpoint
namespace PySide {
    template<> inline uint hash(const QPoint &v) {
        return qHash(qMakePair(v.x(), v.y()));
    }
};
// @snippet qpoint

// @snippet qrect
namespace PySide {
    template<> inline uint hash(const QRect &v) {
        return qHash(qMakePair(qMakePair(v.x(), v.y()), qMakePair(v.width(), v.height())));
    }
};
// @snippet qrect

// @snippet qsize
namespace PySide {
    template<> inline uint hash(const QSize &v) {
        return qHash(qMakePair(v.width(), v.height()));
    }
};
// @snippet qsize

// @snippet qtime-topython
if (!PyDateTimeAPI)
    PySideDateTime_IMPORT;
%PYARG_0 = PyTime_FromTime(%CPPSELF.hour(), %CPPSELF.minute(), %CPPSELF.second(), %CPPSELF.msec()*1000);
// @snippet qtime-topython

// @snippet qbitarray-len
return %CPPSELF.size();
// @snippet qbitarray-len

// @snippet qbitarray-getitem
if (_i < 0 || _i >= %CPPSELF.size()) {
    PyErr_SetString(PyExc_IndexError, "index out of bounds");
    return 0;
}
bool ret = %CPPSELF.at(_i);
return %CONVERTTOPYTHON[bool](ret);
// @snippet qbitarray-getitem

// @snippet qbitarray-setitem
PyObject *args = Py_BuildValue("(iiO)", _i, 1, _value);
PyObject *result = Sbk_QBitArrayFunc_setBit(self, args);
Py_DECREF(args);
Py_XDECREF(result);
return !result ? -1 : 0;
// @snippet qbitarray-setitem

// @snippet unlock
%CPPSELF.unlock();
// @snippet unlock

// @snippet qabstractitemmodel-createindex
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(%1, %2, %PYARG_3);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qabstractitemmodel-createindex

// @snippet qabstractitemmodel
qRegisterMetaType<QVector<int> >("QVector<int>");
// @snippet qabstractitemmodel

// @snippet qobject-metaobject
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME();
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qobject-metaobject

// @snippet qobject-findchild
QObject *child = _findChildHelper(%CPPSELF, %2, (PyTypeObject*)%PYARG_1);
%PYARG_0 = %CONVERTTOPYTHON[QObject*](child);
// @snippet qobject-findchild

// @snippet qobject-findchildren-1
%PYARG_0 = PyList_New(0);
_findChildrenHelper(%CPPSELF, %2, (PyTypeObject*)%PYARG_1, %PYARG_0);
// @snippet qobject-findchildren-1

// @snippet qobject-findchildren-2
%PYARG_0 = PyList_New(0);
_findChildrenHelper(%CPPSELF, %2, (PyTypeObject*)%PYARG_1, %PYARG_0);
// @snippet qobject-findchildren-2

// @snippet qobject-tr
QString result;
if (QCoreApplication::instance()) {
    PyObject *klass = PyObject_GetAttrString(%PYSELF, "__class__");
    PyObject *cname = PyObject_GetAttrString(klass, "__name__");
    result = QString(QCoreApplication::instance()->translate(Shiboken::String::toCString(cname),
                                                        /*   %1, %2, QCoreApplication::CodecForTr, %3)); */
                                                             %1, %2, %3));

    Py_DECREF(klass);
    Py_DECREF(cname);
} else {
    result = QString(QString::fromLatin1(%1));
}
%PYARG_0 = %CONVERTTOPYTHON[QString](result);
// @snippet qobject-tr

// @snippet qobject-receivers
// Avoid return +1 because SignalManager connect to "destroyed()" signal to control object timelife
int ret = %CPPSELF.%FUNCTION_NAME(%1);
if (ret > 0 && ((strcmp(%1, SIGNAL(destroyed())) == 0) || (strcmp(%1, SIGNAL(destroyed(QObject*))) == 0)))
    ret -= PySide::SignalManager::instance().countConnectionsWith(%CPPSELF);

%PYARG_0 = %CONVERTTOPYTHON[int](ret);
// @snippet qobject-receivers

// @snippet qregexp-replace
%1.replace(*%CPPSELF, %2);
%PYARG_0 = %CONVERTTOPYTHON[QString](%1);
// @snippet qregexp-replace

// @snippet qbytearray-operatorplus-1
QByteArray ba = QByteArray(PyBytes_AS_STRING(%PYARG_1), PyBytes_GET_SIZE(%PYARG_1)) + *%CPPSELF;
%PYARG_0 = %CONVERTTOPYTHON[QByteArray](ba);
// @snippet qbytearray-operatorplus-1

// @snippet qbytearray-operatorplus-2
QByteArray ba = QByteArray(PyByteArray_AsString(%PYARG_1), PyByteArray_Size(%PYARG_1)) + *%CPPSELF;
%PYARG_0 = %CONVERTTOPYTHON[QByteArray](ba);
// @snippet qbytearray-operatorplus-2

// @snippet qbytearray-operatorplus-3
QByteArray ba = *%CPPSELF + QByteArray(PyByteArray_AsString(%PYARG_1), PyByteArray_Size(%PYARG_1));
%PYARG_0 = %CONVERTTOPYTHON[QByteArray](ba);
// @snippet qbytearray-operatorplus-3

// @snippet qbytearray-operatorplusequal
*%CPPSELF += QByteArray(PyByteArray_AsString(%PYARG_1), PyByteArray_Size(%PYARG_1));
// @snippet qbytearray-operatorplusequal

// @snippet qbytearray-operatorequalequal
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF == ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorequalequal

// @snippet qbytearray-operatornotequal
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF != ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatornotequal

// @snippet qbytearray-operatorgreater
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF > ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorgreater

// @snippet qbytearray-operatorgreaterequal
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF >= ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorgreaterequal

// @snippet qbytearray-operatorlower
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF < ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorlower

// @snippet qbytearray-operatorlowerequal
if (PyUnicode_CheckExact(%PYARG_1)) {
    Shiboken::AutoDecRef data(PyUnicode_AsASCIIString(%PYARG_1));
    QByteArray ba = QByteArray(PyBytes_AsString(data.object()), PyBytes_GET_SIZE(data.object()));
    bool cppResult = %CPPSELF <= ba;
    %PYARG_0 = %CONVERTTOPYTHON[bool](cppResult);
}
// @snippet qbytearray-operatorlowerequal

// @snippet qbytearray-repr
PyObject *aux = PyBytes_FromStringAndSize(%CPPSELF.constData(), %CPPSELF.size());
if (aux == NULL) {
    return NULL;
}
QByteArray b(Py_TYPE(%PYSELF)->tp_name);
#ifdef IS_PY3K
    %PYARG_0 = PyUnicode_FromFormat("%s(%R)", b.constData(), aux);
#else
    aux = PyObject_Repr(aux);
    b += '(';
    b += QByteArray(PyBytes_AS_STRING(aux), PyBytes_GET_SIZE(aux));
    b += ')';
    %PYARG_0 = Shiboken::String::fromStringAndSize(b.constData(), b.size());
#endif
Py_DECREF(aux);
// @snippet qbytearray-repr

// @snippet qbytearray-1
if (PyBytes_Check(%PYARG_1)) {
    %0 = new QByteArray(PyBytes_AsString(%PYARG_1), PyBytes_GET_SIZE(%PYARG_1));
} else if (Shiboken::String::check(%PYARG_1)) {
    %0 = new QByteArray(Shiboken::String::toCString(%PYARG_1), Shiboken::String::len(%PYARG_1));
}
// @snippet qbytearray-1

// @snippet qbytearray-2
%0 = new QByteArray(PyByteArray_AsString(%PYARG_1), PyByteArray_Size(%PYARG_1));
// @snippet qbytearray-2

// @snippet qbytearray-3
%0 = new QByteArray(PyBytes_AS_STRING(%PYARG_1), PyBytes_GET_SIZE(%PYARG_1));
// @snippet qbytearray-3

// @snippet qbytearray-py3
#if PY_VERSION_HEX < 0x03000000
    Shiboken::SbkType<QByteArray>()->tp_as_buffer = &SbkQByteArrayBufferProc;
    Shiboken::SbkType<QByteArray>()->tp_flags |= Py_TPFLAGS_HAVE_GETCHARBUFFER;
#endif
// @snippet qbytearray-py3

// @snippet qbytearray-data
%PYARG_0 = PyBytes_FromStringAndSize(%CPPSELF.%FUNCTION_NAME(), %CPPSELF.size());
// @snippet qbytearray-data

// @snippet qbytearray-fromrawdata
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(PyBytes_AsString(%PYARG_1), PyBytes_GET_SIZE(%PYARG_1));
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qbytearray-fromrawdata

// @snippet qbytearray-str
PyObject *aux = PyBytes_FromStringAndSize(%CPPSELF.constData(), %CPPSELF.size());
if (aux == NULL) {
    return NULL;
}
#ifdef IS_PY3K
    %PYARG_0 = PyObject_Repr(aux);
    Py_DECREF(aux);
#else
    %PYARG_0 = aux;
#endif
// @snippet qbytearray-str

// @snippet qbytearray-len
return %CPPSELF.count();
// @snippet qbytearray-len

// @snippet qbytearray-getitem
if (_i < 0 || _i >= %CPPSELF.size()) {
    PyErr_SetString(PyExc_IndexError, "index out of bounds");
    return 0;
} else {
    char res[2];
    res[0] = %CPPSELF.at(_i);
    res[1] = 0;
    return PyBytes_FromStringAndSize(res, 1);
}
// @snippet qbytearray-getitem

// @snippet qbytearray-setitem
%CPPSELF.remove(_i, 1);
PyObject *args = Py_BuildValue("(nO)", _i, _value);
PyObject *result = Sbk_QByteArrayFunc_insert(self, args);
Py_DECREF(args);
Py_XDECREF(result);
return !result ? -1 : 0;
// @snippet qbytearray-setitem

// @snippet qfiledevice-unmap
uchar *ptr = reinterpret_cast<uchar*>(Shiboken::Buffer::getPointer(%PYARG_1));
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(ptr);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qfiledevice-unmap

// @snippet qfiledevice-map
%PYARG_0 = Shiboken::Buffer::newObject(%CPPSELF.%FUNCTION_NAME(%1, %2, %3), %2, Shiboken::Buffer::ReadWrite);
// @snippet qfiledevice-map

// @snippet qiodevice-readdata
QByteArray ba(1 + int(%2), char(0));
%CPPSELF.%FUNCTION_NAME(ba.data(), int(%2));
%PYARG_0 = Shiboken::String::fromCString(ba.constData());
// @snippet qiodevice-readdata

// @snippet qcryptographichash-adddata
%CPPSELF.%FUNCTION_NAME(Shiboken::String::toCString(%PYARG_1), Shiboken::String::len(%PYARG_1));
// @snippet qcryptographichash-adddata

// @snippet qsocketnotifier
Shiboken::AutoDecRef socket(%PYARG_1);
if (!socket.isNull()) {
    // We use qintptr as PyLong, but we check for int
    // since it is currently an alias to be Python2 compatible.
    // Internally, ints are qlonglongs.
    if (%CHECKTYPE[int](socket)) {
        int cppSocket = %CONVERTTOCPP[int](socket);
        qintptr socket = (qintptr)cppSocket;
        %0 = new %TYPE(socket, %2, %3);
    } else {
        PyErr_SetString(PyExc_TypeError,
            "QSocketNotifier: first argument (socket) must be an int.");
    }
}
// @snippet qsocketnotifier

// @snippet qtranslator-load
Py_ssize_t size;
uchar *ptr = reinterpret_cast<uchar*>(Shiboken::Buffer::getPointer(%PYARG_1, &size));
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(const_cast<const uchar*>(ptr), size);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qtranslator-load

// @snippet qtimer-singleshot-1
// %FUNCTION_NAME() - disable generation of c++ function call
(void) %2; // remove warning about unused variable
Shiboken::AutoDecRef emptyTuple(PyTuple_New(0));
PyObject *pyTimer = reinterpret_cast<PyTypeObject *>(Shiboken::SbkType<QTimer>())->tp_new(Shiboken::SbkType<QTimer>(), emptyTuple, 0);
reinterpret_cast<PyTypeObject *>(Shiboken::SbkType<QTimer>())->tp_init(pyTimer, emptyTuple, 0);

QTimer* timer = %CONVERTTOCPP[QTimer*](pyTimer);
Shiboken::AutoDecRef result(
    PyObject_CallMethod(pyTimer,
                        const_cast<char*>("connect"),
                        const_cast<char*>("OsOs"),
                        pyTimer,
                        SIGNAL(timeout()),
                        %PYARG_2,
                        %3)
);
Shiboken::Object::releaseOwnership((SbkObject*)pyTimer);
Py_XDECREF(pyTimer);
timer->setSingleShot(true);
timer->connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
timer->start(%1);
// @snippet qtimer-singleshot-1

// @snippet qtimer-singleshot-2
// %FUNCTION_NAME() - disable generation of c++ function call
Shiboken::AutoDecRef emptyTuple(PyTuple_New(0));
PyObject *pyTimer = reinterpret_cast<PyTypeObject *>(Shiboken::SbkType<QTimer>())->tp_new(Shiboken::SbkType<QTimer>(), emptyTuple, 0);
reinterpret_cast<PyTypeObject *>(Shiboken::SbkType<QTimer>())->tp_init(pyTimer, emptyTuple, 0);
QTimer* timer = %CONVERTTOCPP[QTimer*](pyTimer);
timer->setSingleShot(true);

if (PyObject_TypeCheck(%2, PySideSignalInstanceTypeF())) {
    PySideSignalInstance *signalInstance = reinterpret_cast<PySideSignalInstance*>(%2);
    Shiboken::AutoDecRef signalSignature(Shiboken::String::fromFormat("2%s", PySide::Signal::getSignature(signalInstance)));
    Shiboken::AutoDecRef result(
        PyObject_CallMethod(pyTimer,
                            const_cast<char*>("connect"),
                            const_cast<char*>("OsOO"),
                            pyTimer,
                            SIGNAL(timeout()),
                            PySide::Signal::getObject(signalInstance),
                            signalSignature.object())
    );
} else {
    Shiboken::AutoDecRef result(
        PyObject_CallMethod(pyTimer,
                            const_cast<char*>("connect"),
                            const_cast<char*>("OsO"),
                            pyTimer,
                            SIGNAL(timeout()),
                            %PYARG_2)
    );
}

timer->connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()), Qt::DirectConnection);
Shiboken::Object::releaseOwnership((SbkObject*)pyTimer);
Py_XDECREF(pyTimer);
timer->start(%1);
// @snippet qtimer-singleshot-2

// @snippet qprocess-startdetached
qint64 pid;
%RETURN_TYPE retval = %TYPE::%FUNCTION_NAME(%1, %2, %3, &pid);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](retval));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[qint64](pid));
// @snippet qprocess-startdetached

// @snippet qprocess-pid
long result;
#ifdef WIN32
    _PROCESS_INFORMATION *procInfo = %CPPSELF.%FUNCTION_NAME();
    result = procInfo ? procInfo->dwProcessId : 0;
#else
    result = %CPPSELF.%FUNCTION_NAME();
#endif
%PYARG_0 = %CONVERTTOPYTHON[long](result);
// @snippet qprocess-pid

// @snippet qcoreapplication-1
QCoreApplicationConstructor(%PYSELF, args, &%0);
// @snippet qcoreapplication-1

// @snippet qcoreapplication-2
PyObject *empty = PyTuple_New(2);
if (!PyTuple_SetItem(empty, 0, PyList_New(0))) {
    QCoreApplicationConstructor(%PYSELF, empty, &%0);
}
// @snippet qcoreapplication-2

// @snippet qcoreapplication-instance
QCoreApplication *app = QCoreApplication::instance();
PyObject *pyApp = Py_None;
if (app) {
    pyApp = reinterpret_cast<PyObject*>(
        Shiboken::BindingManager::instance().retrieveWrapper(app));
    if (!pyApp)
        pyApp = %CONVERTTOPYTHON[QCoreApplication*](app);
        // this will keep app live after python exit (extra ref)
}
// PYSIDE-571: make sure that we return the singleton "None"
if (pyApp == Py_None)
    Py_DECREF(MakeSingletonQAppWrapper(0)); // here qApp and instance() diverge
%PYARG_0 = pyApp;
Py_XINCREF(%PYARG_0);
// @snippet qcoreapplication-instance

// @snippet qdatastream-readrawdata
QByteArray data;
data.resize(%2);
int result = %CPPSELF.%FUNCTION_NAME(data.data(), data.size());
if (result == -1) {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
} else {
    %PYARG_0 = PyBytes_FromStringAndSize(data.data(), result);
}
// @snippet qdatastream-readrawdata

// @snippet qdatastream-writerawdata
int r = %CPPSELF.%FUNCTION_NAME(%1, Shiboken::String::len(%PYARG_1));
%PYARG_0 = %CONVERTTOPYTHON[int](r);
// @snippet qdatastream-writerawdata

// @snippet releaseownership
Shiboken::Object::releaseOwnership(%PYARG_0);
// @snippet releaseownership

// @snippet qanimationgroup-clear
for (int counter = 0, count = %CPPSELF.animationCount(); counter < count; ++counter ) {
    QAbstractAnimation *animation = %CPPSELF.animationAt(counter);
    PyObject *obj = %CONVERTTOPYTHON[QAbstractAnimation*](animation);
    Shiboken::Object::setParent(nullptr, obj);
    Py_DECREF(obj);
}
%CPPSELF.clear();
// @snippet qanimationgroup-clear

// @snippet qeasingcurve
PySideEasingCurveFunctor::init();
// @snippet qeasingcurve

// @snippet qeasingcurve-setcustomtype
QEasingCurve::EasingFunction func = PySideEasingCurveFunctor::createCustomFuntion(%PYSELF, %PYARG_1);
if (func)
    %CPPSELF.%FUNCTION_NAME(func);
// @snippet qeasingcurve-setcustomtype

// @snippet qeasingcurve-customtype
//%FUNCTION_NAME()
%PYARG_0 = PySideEasingCurveFunctor::callable(%PYSELF);
// @snippet qeasingcurve-customtype

// @snippet qsignaltransition
if (PyObject_TypeCheck(%1, PySideSignalInstanceTypeF())) {
    PyObject *dataSource = PySide::Signal::getObject((PySideSignalInstance*)%PYARG_1);
    Shiboken::AutoDecRef obType(PyObject_Type(dataSource));
    QObject* sender = %CONVERTTOCPP[QObject*](dataSource);
    if (sender) {
        const char*dataSignature = PySide::Signal::getSignature((PySideSignalInstance*)%PYARG_1);
        QByteArray signature(dataSignature); // Append SIGNAL flag (2)
        signature.prepend('2');
        %0 = new QSignalTransitionWrapper(sender, signature, %2);
    }
}
// @snippet qsignaltransition

// @snippet qstate-addtransition-1
QString signalName(%2);
if (PySide::SignalManager::registerMetaMethod(%1, signalName.mid(1).toLatin1().data(), QMetaMethod::Signal)) {
    QSignalTransition *%0 = %CPPSELF->addTransition(%1, %2, %3);
    %PYARG_0 = %CONVERTTOPYTHON[QSignalTransition*](%0);
} else {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
}
// @snippet qstate-addtransition-1

// @snippet qstate-addtransition-2
// Obviously the label used by the following goto is a very awkward solution,
// since it refers to a name very tied to the generator implementation.
// Check bug #362 for more information on this
// http://bugs.openbossa.org/show_bug.cgi?id=362
if (!PyObject_TypeCheck(%1, PySideSignalInstanceTypeF()))
    goto Sbk_%TYPEFunc_%FUNCTION_NAME_TypeError;
PySideSignalInstance *signalInstance = reinterpret_cast<PySideSignalInstance*>(%1);
QObject* sender = %CONVERTTOCPP[QObject*](PySide::Signal::getObject(signalInstance));
QSignalTransition *%0 = %CPPSELF->%FUNCTION_NAME(sender, PySide::Signal::getSignature(signalInstance),%2);
%PYARG_0 = %CONVERTTOPYTHON[QSignalTransition*](%0);
// @snippet qstate-addtransition-2

// @snippet qstatemachine-configuration
%PYARG_0 = PySet_New(0);
foreach (QAbstractState *abs_state, %CPPSELF.configuration()) {
        Shiboken::AutoDecRef obj(%CONVERTTOPYTHON[QAbstractState*](abs_state));
        Shiboken::Object::setParent(self, obj);
        PySet_Add(%PYARG_0, obj);
}
// @snippet qstatemachine-configuration

// @snippet qstatemachine-defaultanimations
%PYARG_0 = PyList_New(0);
foreach (QAbstractAnimation *abs_anim, %CPPSELF.defaultAnimations()) {
        Shiboken::AutoDecRef obj(%CONVERTTOPYTHON[QAbstractAnimation*](abs_anim));
        Shiboken::Object::setParent(self, obj);
        PyList_Append(%PYARG_0, obj);
}
// @snippet qstatemachine-defaultanimations

// @snippet qt-signal
%PYARG_0 = Shiboken::String::fromFormat("2%s",QMetaObject::normalizedSignature(%1).constData());
// @snippet qt-signal

// @snippet qt-slot
%PYARG_0 = Shiboken::String::fromFormat("1%s",QMetaObject::normalizedSignature(%1).constData());
// @snippet qt-slot

// @snippet qt-registerresourcedata
QT_BEGIN_NAMESPACE
extern bool
qRegisterResourceData(int,
                      const unsigned char *,
                      const unsigned char *,
                      const unsigned char *);

extern bool
qUnregisterResourceData(int,
                        const unsigned char *,
                        const unsigned char *,
                        const unsigned char *);
QT_END_NAMESPACE
// @snippet qt-registerresourcedata

// @snippet qt-qregisterresourcedata
%RETURN_TYPE %0 = %FUNCTION_NAME(%1, reinterpret_cast<uchar*>(PyBytes_AS_STRING(%PYARG_2)),
                                     reinterpret_cast<uchar*>(PyBytes_AS_STRING(%PYARG_3)),
                                     reinterpret_cast<uchar*>(PyBytes_AS_STRING(%PYARG_4)));
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qt-qregisterresourcedata

// @snippet qt-qunregisterresourcedata
%RETURN_TYPE %0 = %FUNCTION_NAME(%1, reinterpret_cast<uchar*>(PyBytes_AS_STRING(%PYARG_2)),
                                     reinterpret_cast<uchar*>(PyBytes_AS_STRING(%PYARG_3)),
                                     reinterpret_cast<uchar*>(PyBytes_AS_STRING(%PYARG_4)));
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qt-qunregisterresourcedata

// @snippet use-stream-for-format-security
// Uses the stream version for security reasons
// see gcc man page at -Wformat-security
%FUNCTION_NAME() << %1;
// @snippet use-stream-for-format-security

// @snippet qresource-registerResource
 auto ptr = reinterpret_cast<uchar*>(Shiboken::Buffer::getPointer(%PYARG_1));
 %RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(const_cast<const uchar*>(ptr), %2);
 %PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
// @snippet qresource-registerResource

// @snippet qstring-return
%PYARG_0 = %CONVERTTOPYTHON[QString](%1);
// @snippet qstring-return

// @snippet stream-write-method
(*%CPPSELF) << %1;
// @snippet stream-write-method

// @snippet stream-read-method
%RETURN_TYPE _cpp_result;
(*%CPPSELF) >> _cpp_result;
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](_cpp_result);
// @snippet stream-read-method

/*********************************************************************
 * CONVERSIONS
 ********************************************************************/

// @snippet conversion-pybool
%out = %OUTTYPE(%in == Py_True);
// @snippet conversion-pybool

// @snippet conversion-pylong
%out = %OUTTYPE(PyLong_AsLong(%in));
// @snippet conversion-pylong

// @snippet conversion-pylong-unsigned
%out = %OUTTYPE(PyLong_AsUnsignedLong(%in));
// @snippet conversion-pylong-unsigned

// @snippet conversion-pyunicode
#ifndef Py_LIMITED_API
Py_UNICODE* unicode = PyUnicode_AS_UNICODE(%in);
# if defined(Py_UNICODE_WIDE)
// cast as Py_UNICODE can be a different type
%out = QString::fromUcs4((const uint*)unicode);
# else
%out = QString::fromUtf16((const ushort*)unicode, PyUnicode_GET_SIZE(%in));
# endif
#else
wchar_t *temp = PyUnicode_AsWideCharString(%in, NULL);
%out = QString::fromWCharArray(temp);
PyMem_Free(temp);
#endif
// @snippet conversion-pyunicode

// @snippet conversion-pystring
#ifndef IS_PY3K
const char* str = %CONVERTTOCPP[const char*](%in);
%out = %OUTTYPE(str);
#endif
// @snippet conversion-pystring

// @snippet conversion-pynone
%out = %OUTTYPE();
// @snippet conversion-pynone

// @snippet conversion-pystring-char
char c = %CONVERTTOCPP[char](%in);
%out = %OUTTYPE(c);
// @snippet conversion-pystring-char

// @snippet conversion-pyint
int i = %CONVERTTOCPP[int](%in);
%out = %OUTTYPE(i);
// @snippet conversion-pyint

// @snippet conversion-qlonglong
qlonglong in = %CONVERTTOCPP[qlonglong](%in);
%out = %OUTTYPE(in);
// @snippet conversion-qlonglong

// @snippet conversion-qstring
QString in = %CONVERTTOCPP[QString](%in);
%out = %OUTTYPE(in);
// @snippet conversion-qstring

// @snippet conversion-qbytearray
QByteArray in = %CONVERTTOCPP[QByteArray](%in);
%out = %OUTTYPE(in);
// @snippet conversion-qbytearray

// @snippet conversion-pyfloat
double in = %CONVERTTOCPP[double](%in);
%out = %OUTTYPE(in);
// @snippet conversion-pyfloat

// @snippet conversion-sbkobject
// a class supported by QVariant?
int typeCode;
const char *typeName = QVariant_resolveMetaType(Py_TYPE(%in), &typeCode);
if (!typeCode || !typeName)
    return;
QVariant var(typeCode, (void*)0);
Shiboken::Conversions::SpecificConverter converter(typeName);
converter.toCpp(pyIn, var.data());
%out = var;
// @snippet conversion-sbkobject

// @snippet conversion-pydict
QVariant ret = QVariant_convertToVariantMap(%in);
%out = ret.isValid() ? ret : QVariant::fromValue(PySide::PyObjectWrapper(%in));
// @snippet conversion-pydict

// @snippet conversion-pylist
QVariant ret = QVariant_convertToVariantList(%in);
%out = ret.isValid() ? ret : QVariant::fromValue(PySide::PyObjectWrapper(%in));
// @snippet conversion-pylist

// @snippet conversion-pyobject
// Is a shiboken type not known by Qt
%out = QVariant::fromValue(PySide::PyObjectWrapper(%in));
// @snippet conversion-pyobject

// @snippet conversion-qvariant-invalid
%out = QVariant::Invalid;
// @snippet conversion-qvariant-invalid

// @snippet conversion-qvariant-pytypeobject
const char *typeName;
if (Shiboken::String::checkType(reinterpret_cast<PyTypeObject *>(%in)))
    typeName = "QString";
else if (%in == reinterpret_cast<PyObject*>(&PyFloat_Type))
    typeName = "double"; // float is a UserType in QVariant.
else if (%in == reinterpret_cast<PyObject*>(&PyLong_Type))
    typeName = "int";    // long is a UserType in QVariant.
else if (Py_TYPE(%in) == SbkObjectType_TypeF())
    typeName = Shiboken::ObjectType::getOriginalName((SbkObjectType*)%in);
else
    typeName = reinterpret_cast<PyTypeObject *>(%in)->tp_name;
%out = QVariant::nameToType(typeName);
// @snippet conversion-qvariant-pytypeobject

// @snippet conversion-qvariant-pystring
%out = QVariant::nameToType(Shiboken::String::toCString(%in));
// @snippet conversion-qvariant-pystring

// @snippet conversion-qvariant-pydict
%out = QVariant::nameToType("QVariantMap");
// @snippet conversion-qvariant-pydict

// @snippet conversion-qvariant-pysequence
%out = QVariantType_isStringList(%in) ? QVariant::StringList : QVariant::List;
// @snippet conversion-qvariant-pysequence

// @snippet conversion-qjsonobject-pydict
QVariant dict = QVariant_convertToVariantMap(%in);
QJsonValue val = QJsonValue::fromVariant(dict);
%out = val.toObject();
// @snippet conversion-qjsonobject-pydict

// @snippet conversion-qpair-pysequence
%out.first = %CONVERTTOCPP[%OUTTYPE_0](PySequence_Fast_GET_ITEM(%in, 0));
%out.second = %CONVERTTOCPP[%OUTTYPE_1](PySequence_Fast_GET_ITEM(%in, 1));
// @snippet conversion-qpair-pysequence

// @snippet conversion-qdate-pydate
int day = PyDateTime_GET_DAY(%in);
int month = PyDateTime_GET_MONTH(%in);
int year = PyDateTime_GET_YEAR(%in);
%out = %OUTTYPE(year, month, day);
// @snippet conversion-qdate-pydate

// @snippet conversion-qdatetime-pydatetime
int day = PyDateTime_GET_DAY(%in);
int month = PyDateTime_GET_MONTH(%in);
int year = PyDateTime_GET_YEAR(%in);
int hour = PyDateTime_DATE_GET_HOUR(%in);
int min = PyDateTime_DATE_GET_MINUTE(%in);
int sec = PyDateTime_DATE_GET_SECOND(%in);
int usec = PyDateTime_DATE_GET_MICROSECOND(%in);
%out = %OUTTYPE(QDate(year, month, day), QTime(hour, min, sec, usec/1000));
// @snippet conversion-qdatetime-pydatetime

// @snippet conversion-qtime-pytime
int hour = PyDateTime_TIME_GET_HOUR(%in);
int min = PyDateTime_TIME_GET_MINUTE(%in);
int sec = PyDateTime_TIME_GET_SECOND(%in);
int usec = PyDateTime_TIME_GET_MICROSECOND(%in);
%out = %OUTTYPE(hour, min, sec, usec/1000);
// @snippet conversion-qtime-pytime

// @snippet conversion-qbytearray-pybytes
#ifdef IS_PY3K
%out = %OUTTYPE(PyBytes_AS_STRING(%in), PyBytes_GET_SIZE(%in));
#else
%out = %OUTTYPE(Shiboken::String::toCString(%in), Shiboken::String::len(%in));
#endif
// @snippet conversion-qbytearray-pybytes

// @snippet conversion-qbytearray-pybytearray
%out = %OUTTYPE(PyByteArray_AsString(%in), PyByteArray_Size(%in));
// @snippet conversion-qbytearray-pybytearray

// @snippet conversion-qbytearray-pystring
%out = %OUTTYPE(Shiboken::String::toCString(%in), Shiboken::String::len(%in));
// @snippet conversion-qbytearray-pystring

/*********************************************************************
 * NATIVE TO TARGET CONVERSIONS
 ********************************************************************/

// @snippet return-pybool
return PyBool_FromLong((bool)%in);
// @snippet return-pybool

// @snippet return-pylong
return PyLong_FromLong(%in);
// @snippet return-pylong

// @snippet return-pylong-unsigned
return PyLong_FromUnsignedLong(%in);
// @snippet return-pylong-unsigned

// @snippet return-pyunicode
QByteArray ba = %in.toUtf8();
return PyUnicode_FromStringAndSize(ba.constData(), ba.size());
// @snippet return-pyunicode

// @snippet return-pyunicode-qstringref
 const int N = %in.length();
 wchar_t *str = new wchar_t[N];
 %in.toString().toWCharArray(str);
 PyObject *%out = PyUnicode_FromWideChar(str, N);
 delete[] str;
 return %out;
// @snippet return-pyunicode-qstringref

// @snippet return-pyunicode-qchar
wchar_t c = (wchar_t)%in.unicode();
return PyUnicode_FromWideChar(&c, 1);
// @snippet return-pyunicode-qchar

// @snippet return-qvariant
if (!%in.isValid())
    Py_RETURN_NONE;

if (qstrcmp(%in.typeName(), "QVariantList") == 0) {
    QList<QVariant> var = %in.value<QVariantList>();
    return %CONVERTTOPYTHON[QList<QVariant>](var);
}

if (qstrcmp(%in.typeName(), "QStringList") == 0) {
    QStringList var = %in.value<QStringList>();
    return %CONVERTTOPYTHON[QList<QString>](var);
}

if (qstrcmp(%in.typeName(), "QVariantMap") == 0) {
    QMap<QString, QVariant> var = %in.value<QVariantMap>();
    return %CONVERTTOPYTHON[QMap<QString, QVariant>](var);
}

Shiboken::Conversions::SpecificConverter converter(cppInRef.typeName());
if (converter) {
   void *ptr = cppInRef.data();
   return converter.toPython(ptr);
}
PyErr_Format(PyExc_RuntimeError, "Can't find converter for '%s'.", %in.typeName());
return 0;
// @snippet return-qvariant

// @snippet return-qvariant-type
const char *typeName = QVariant::typeToName(%in);
PyObject *%out;
PyTypeObject *pyType = nullptr;
if (typeName)
    pyType = Shiboken::Conversions::getPythonTypeObject(typeName);
%out = pyType ? (reinterpret_cast<PyObject*>(pyType)) : Py_None;
Py_INCREF(%out);
return %out;
// @snippet return-qvariant-type

// @snippet return-qjsonobject
// The QVariantMap returned by QJsonObject seems to cause a segfault, so
// using QJsonObject.toVariantMap() won't work.
// Wrapping it in a QJsonValue first allows it to work
QJsonValue val(%in);
QVariant ret = val.toVariant();

return %CONVERTTOPYTHON[QVariant](ret);
// @snippet return-qjsonobject

// @snippet return-qpair
PyObject *%out = PyTuple_New(2);
PyTuple_SET_ITEM(%out, 0, %CONVERTTOPYTHON[%INTYPE_0](%in.first));
PyTuple_SET_ITEM(%out, 1, %CONVERTTOPYTHON[%INTYPE_1](%in.second));
return %out;
// @snippet return-qpair
