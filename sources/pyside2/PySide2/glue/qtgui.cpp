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

// @snippet qtransform-quadtoquad
QTransform _result;
if (QTransform::quadToQuad(%1, %2, _result)) {
    %PYARG_0 = %CONVERTTOPYTHON[QTransform](_result);
} else {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
}
// @snippet qtransform-quadtoquad

// @snippet qtransform-quadtosquare
QTransform _result;
if (QTransform::quadToSquare(%1, _result)) {
    %PYARG_0 = %CONVERTTOPYTHON[QTransform](_result);
} else {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
}
// @snippet qtransform-quadtosquare

// @snippet qtransform-squaretoquad
QTransform _result;
if (QTransform::squareToQuad(%1, _result)) {
    %PYARG_0 = %CONVERTTOPYTHON[QTransform](_result);
} else {
    Py_INCREF(Py_None);
    %PYARG_0 = Py_None;
}
// @snippet qtransform-squaretoquad

// @snippet qbitmap-fromdata
uchar *buffer = reinterpret_cast<uchar *>(Shiboken::Buffer::getPointer(%PYARG_2));
QBitmap %0 = QBitmap::fromData(%1, buffer, %3);
%PYARG_0 = %CONVERTTOPYTHON[QBitmap](%0);
// @snippet qbitmap-fromdata

// @snippet qtextline-cursortox
%BEGIN_ALLOW_THREADS
%RETURN_TYPE %0 = %CPPSELF->::%TYPE::%FUNCTION_NAME(&%1, %2);
%END_ALLOW_THREADS
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](%0));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[%ARG1_TYPE](%1));
// @snippet qtextline-cursortox

// @snippet qkeysequence-getitem
if (_i < 0 || _i >= %CPPSELF.count()) {
    PyErr_SetString(PyExc_IndexError, "index out of bounds");
    return 0;
}
int item = (*%CPPSELF)[_i];
return %CONVERTTOPYTHON[int](item);
// @snippet qkeysequence-getitem

// @snippet qpicture-data
%PYARG_0 = Shiboken::Buffer::newObject(%CPPSELF.data(), %CPPSELF.size());
// @snippet qpicture-data

// @snippet qtextblock-setuserdata
const QTextDocument *doc = %CPPSELF.document();
if (doc) {
    Shiboken::AutoDecRef pyDocument(%CONVERTTOPYTHON[QTextDocument *](doc));
    Shiboken::Object::setParent(pyDocument, %PYARG_1);
}
// @snippet qtextblock-setuserdata

// @snippet qtextblock-userdata
const QTextDocument *doc = %CPPSELF.document();
if (doc) {
    Shiboken::AutoDecRef pyDocument(%CONVERTTOPYTHON[QTextDocument *](doc));
    Shiboken::Object::setParent(pyDocument, %PYARG_0);
}
// @snippet qtextblock-userdata

// @snippet qopenglshaderprogram_setuniformvalue_float
float value = %2;
%CPPSELF.setUniformValue(%1, value);
// @snippet qopenglshaderprogram_setuniformvalue_float

// @snippet qopenglshaderprogram_setuniformvalue_int
int value = %2;
%CPPSELF.setUniformValue(%1, value);
// @snippet qopenglshaderprogram_setuniformvalue_int

// @snippet qpolygon-reduce
PyObject *points = PyList_New(%CPPSELF.count());
for (int i = 0, i_max = %CPPSELF.count(); i < i_max; ++i){
    int x, y;
    %CPPSELF.point(i, &x, &y);
    QPoint pt = QPoint(x, y);
    PyList_SET_ITEM(points, i, %CONVERTTOPYTHON[QPoint](pt));
}
// @snippet qpolygon-reduce

// @snippet qpolygon-operatorlowerlower
// %FUNCTION_NAME()
*%CPPSELF << %1;
%PYARG_0 = %CONVERTTOPYTHON[QPolygon *](%CPPSELF);
// @snippet qpolygon-operatorlowerlower

// @snippet qpixmap
%0 = new %TYPE(QPixmap::fromImage(%1));
// @snippet qpixmap

// @snippet qimage-constbits
%PYARG_0 = Shiboken::Buffer::newObject(%CPPSELF.%FUNCTION_NAME(), %CPPSELF.byteCount());
// @snippet qimage-constbits

// @snippet qimage-bits
// byteCount() is only available on Qt4.7, so we use bytesPerLine * height
%PYARG_0 = Shiboken::Buffer::newObject(%CPPSELF.%FUNCTION_NAME(), %CPPSELF.bytesPerLine() * %CPPSELF.height(), Shiboken::Buffer::ReadWrite);
// @snippet qimage-bits

// @snippet qimage-constscanline
%PYARG_0 = Shiboken::Buffer::newObject(%CPPSELF.%FUNCTION_NAME(%1), %CPPSELF.bytesPerLine());
// @snippet qimage-constscanline

// @snippet qimage-scanline
%PYARG_0 = Shiboken::Buffer::newObject(%CPPSELF.%FUNCTION_NAME(%1), %CPPSELF.bytesPerLine(), Shiboken::Buffer::ReadWrite);
// @snippet qimage-scanline

// @snippet qcolor-setstate
Shiboken::AutoDecRef func(PyObject_GetAttr(%PYSELF, PyTuple_GET_ITEM(%1, 0)));
PyObject *args = PyTuple_GET_ITEM(%1, 1);
%PYARG_0 = PyObject_Call(func, args, NULL);
// @snippet qcolor-setstate

// @snippet qcolor-reduce
switch (%CPPSELF.spec()) {
    case QColor::Rgb:
    {
        qreal r, g, b, a;
        %CPPSELF.getRgbF(&r, &g, &b, &a);
        %PYARG_0 = Py_BuildValue("(ON(s(ffff)))", Py_TYPE(%PYSELF), PyTuple_New(0),
                                 "setRgbF", float(r), float(g), float(b), float(a));
        break;
    }
    case QColor::Hsv:
    {
        qreal h, s, v, a;
        %CPPSELF.getHsvF(&h, &s, &v, &a);
        %PYARG_0 = Py_BuildValue("(ON(s(ffff)))", Py_TYPE(%PYSELF), PyTuple_New(0),
                                 "setHsvF", float(h), float(s), float(v), float(a));
        break;
    }
    case QColor::Cmyk:
    {
        qreal c, m, y, k, a;
        %CPPSELF.getCmykF(&c, &m, &y, &k, &a);
        %PYARG_0 = Py_BuildValue("(ON(s(fffff)))", Py_TYPE(%PYSELF), PyTuple_New(0),
                                 "setCmykF", float(c), float(m), float(y), float(k), float(a));
        break;
    }
    case QColor::Hsl:
    {
        qreal h, s, l, a;
        %CPPSELF.getHslF(&h, &s, &l, &a);
        %PYARG_0 = Py_BuildValue("(ON(s(ffff)))", Py_TYPE(%PYSELF), PyTuple_New(0),
                                 "setHslF", float(h), float(s), float(l), float(a));
        break;
    }
    default:
    {
        %PYARG_0 = Py_BuildValue("(N(O))", PyObject_Type(%PYSELF), Py_None);
    }
}
// @snippet qcolor-reduce

// @snippet qcolor-totuple
switch (%CPPSELF.spec()) {
  case QColor::Rgb:
  {
        int r, g, b, a;
        %CPPSELF.getRgb(&r, &g, &b, &a);
        %PYARG_0 = Py_BuildValue("iiii", r, g, b, a);
        break;
  }
  case QColor::Hsv:
  {
        int h, s, v, a;
        %CPPSELF.getHsv(&h, &s, &v, &a);
        %PYARG_0 = Py_BuildValue("iiii", h, s, v, a);
        break;
  }
  case QColor::Cmyk:
  {
        int c, m, y, k, a;
        %CPPSELF.getCmyk(&c, &m, &y, &k, &a);
        %PYARG_0 = Py_BuildValue("iiiii", c, m, y, k, a);
        break;
  }
  case QColor::Hsl:
  {
        int h, s, l, a;
        %CPPSELF.getHsl(&h, &s, &l, &a);
        %PYARG_0 = Py_BuildValue("iiii", h, s, l, a);
        break;
  }
  default:
  {
        %PYARG_0 = 0;
  }
}
// @snippet qcolor-totuple

// @snippet qcolor
if (%1.type() == QVariant::Color)
    %0 = new %TYPE(%1.value<QColor>());
else
    PyErr_SetString(PyExc_TypeError, "QVariant must be holding a QColor");
// @snippet qcolor

// @snippet qfontmetricsf-boundingrect
int *array = nullptr;
bool errorOccurred = false;

if (numArgs == 5) {
    array = Shiboken::sequenceToIntArray(%PYARG_5, true);
    if (PyErr_Occurred()) {
        delete [] array;
        errorOccurred = true;
    }
}

if (!errorOccurred) {
    %RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(%1, %2, %3, %4, array);

    delete [] array;

    %PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](retval);
}
// @snippet qfontmetricsf-boundingrect

// @snippet qfontmetricsf-size
int *array = nullptr;
bool errorOccurred = false;

if (numArgs == 4) {
    array = Shiboken::sequenceToIntArray(%PYARG_4, true);
    if (PyErr_Occurred()) {
        delete [] array;
        errorOccurred = true;
    }
}

if (!errorOccurred) {
    %RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(%1, %2, %3, array);

    delete [] array;

    %PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](retval);
}
// @snippet qfontmetricsf-size

// @snippet qfontmetrics-boundingrect-1
int *array = nullptr;
bool errorOccurred = false;

if (numArgs == 8) {
    array = Shiboken::sequenceToIntArray(%PYARG_8, true);
    if (PyErr_Occurred()) {
        delete [] array;
        errorOccurred = true;
    }
}

if (!errorOccurred) {
    %RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(%1, %2, %3, %4, %5, %6, %7, array);

    delete [] array;

    %PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](retval);
}
// @snippet qfontmetrics-boundingrect-1

// @snippet qfontmetrics-boundingrect-2
int *array = nullptr;
bool errorOccurred = false;

if (numArgs == 5) {
    array = Shiboken::sequenceToIntArray(%PYARG_5, true);
    if (PyErr_Occurred()) {
        delete [] array;
        errorOccurred = true;
    }
}

if (!errorOccurred) {
    %RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(%1, %2, %3, %4, array);

    delete [] array;

    %PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](retval);
}
// @snippet qfontmetrics-boundingrect-2

// @snippet qfontmetrics-size
int *array = nullptr;
bool errorOccurred = false;

if (numArgs == 4) {
    array = Shiboken::sequenceToIntArray(%PYARG_4, true);
    if (PyErr_Occurred()) {
        delete [] array;
        errorOccurred = true;
    }
}

if (!errorOccurred) {
    %RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(%1, %2, %3, array);

    delete [] array;

    %PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](retval);
}
// @snippet qfontmetrics-size

// @snippet qpixmapcache-find
QPixmap p;
if (%CPPSELF.%FUNCTION_NAME(%1, &p)) {
    %PYARG_0 = %CONVERTTOPYTHON[QPixmap](p);
} else {
    %PYARG_0 = Py_None;
    Py_INCREF(%PYARG_0);
}
// @snippet qpixmapcache-find

// @snippet qstandarditem-setchild-1
// Clear parent from the old child
QStandardItem *_i = %CPPSELF->child(%1, %2);
if (_i) {
    PyObject *_pyI = %CONVERTTOPYTHON[QStandardItem *](_i);
    Shiboken::Object::setParent(nullptr, _pyI);
}
// @snippet qstandarditem-setchild-1

// @snippet qstandarditem-setchild-2
// Clear parent from the old child
QStandardItem *_i = %CPPSELF->child(%1);
if (_i) {
    PyObject *_pyI = %CONVERTTOPYTHON[QStandardItem *](_i);
    Shiboken::Object::setParent(nullptr, _pyI);
}
// @snippet qstandarditem-setchild-2

// @snippet qkeyevent-operatornotequal
bool ret = !(&%CPPSELF == %1);
%PYARG_0 = %CONVERTTOPYTHON[bool](ret);
// @snippet qkeyevent-operatornotequal

// @snippet qstandarditemmodel-setitem-1
// Clear parent from the old child
QStandardItem *_i = %CPPSELF->item(%1, %2);
if (_i) {
    PyObject *_pyI = %CONVERTTOPYTHON[QStandardItem *](_i);
    Shiboken::Object::setParent(nullptr, _pyI);
}
// @snippet qstandarditemmodel-setitem-1

// @snippet qstandarditemmodel-setitem-2
// Clear parent from the old child
QStandardItem *_i = %CPPSELF->item(%1);
if (_i) {
    PyObject *_pyI = %CONVERTTOPYTHON[QStandardItem *](_i);
    Shiboken::Object::setParent(nullptr, _pyI);
}
// @snippet qstandarditemmodel-setitem-2

// @snippet qstandarditemmodel-setverticalheaderitem
// Clear parent from the old child
QStandardItem *_i = %CPPSELF->verticalHeaderItem(%1);
if (_i) {
    PyObject *_pyI = %CONVERTTOPYTHON[QStandardItem *](_i);
    Shiboken::Object::setParent(nullptr, _pyI);
}
// @snippet qstandarditemmodel-setverticalheaderitem

// @snippet qstandarditemmodel-clear
Shiboken::BindingManager &bm = Shiboken::BindingManager::instance();
SbkObject *pyRoot = bm.retrieveWrapper(%CPPSELF.invisibleRootItem());
if (pyRoot) {
  Shiboken::Object::destroy(pyRoot, %CPPSELF.invisibleRootItem());
}

for (int r=0, r_max = %CPPSELF.rowCount(); r < r_max; r++) {
  QList<QStandardItem *> ri = %CPPSELF.takeRow(0);

  PyObject *pyResult = %CONVERTTOPYTHON[QList<QStandardItem * >](ri);
  Shiboken::Object::setParent(Py_None, pyResult);
  Py_XDECREF(pyResult);
}
// @snippet qstandarditemmodel-clear

// @snippet qclipboard-text
%BEGIN_ALLOW_THREADS
%RETURN_TYPE retval_ = %CPPSELF.%FUNCTION_NAME(%1, %2);
%END_ALLOW_THREADS
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](retval_));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[%ARG1_TYPE](%1));
// @snippet qclipboard-text

// @snippet qpainter-drawpolygon
%BEGIN_ALLOW_THREADS
%CPPSELF.%FUNCTION_NAME(%1.data(), %1.size(), %2);
%END_ALLOW_THREADS
// @snippet qpainter-drawpolygon

// @snippet qmatrix-map-point
QPoint p(%CPPSELF.%FUNCTION_NAME(%1));
%PYARG_0 = %CONVERTTOPYTHON[QPoint](p);
// @snippet qmatrix-map-point

// @snippet qmatrix4x4
if (PySequence_Size(%PYARG_1) == 16) {
    float values[16];
    for (int i=0; i < 16; ++i) {
        PyObject *pv = PySequence_Fast_GET_ITEM(%PYARG_1, i);
        values[i] = PyFloat_AsDouble(pv);
    }

    %0 = new %TYPE(values[0], values[1], values[2], values[3],
                   values[4], values[5], values[6], values[7],
                   values[8], values[9], values[10], values[11],
                   values[12], values[13], values[14], values[15]);
}
// @snippet qmatrix4x4

// @snippet qmatrix4x4-copydatato
float values[16];
%CPPSELF.%FUNCTION_NAME(values);
%PYARG_0 = PyTuple_New(16);
for (int i = 0; i < 16; ++i) {
  PyObject *v = PyFloat_FromDouble(values[i]);
  PyTuple_SET_ITEM(%PYARG_0, i, v);
}
// @snippet qmatrix4x4-copydatato

// @snippet qmatrix4x4-mgetitem
if (PySequence_Check(_key)) {
    Shiboken::AutoDecRef key(PySequence_Fast(_key, "Invalid matrix index."));
    if (PySequence_Fast_GET_SIZE(key.object()) == 2) {
        PyObject *posx = PySequence_Fast_GET_ITEM(key.object(), 0);
        PyObject *posy = PySequence_Fast_GET_ITEM(key.object(), 1);
        Py_ssize_t x = PyInt_AsSsize_t(posx);
        Py_ssize_t y = PyInt_AsSsize_t(posy);
        float ret = (*%CPPSELF)(x,y);
        return %CONVERTTOPYTHON[float](ret);
    }
}
PyErr_SetString(PyExc_IndexError, "Invalid matrix index.");
return 0;
// @snippet qmatrix4x4-mgetitem

// @snippet qguiapplication-init
static void QGuiApplicationConstructor(PyObject *self, PyObject *pyargv, QGuiApplicationWrapper **cptr)
{
    static int argc;
    static char **argv;
    PyObject *stringlist = PyTuple_GET_ITEM(pyargv, 0);
    if (Shiboken::listToArgcArgv(stringlist, &argc, &argv, "PySideApp")) {
        *cptr = new QGuiApplicationWrapper(argc, argv, 0);
        Shiboken::Object::releaseOwnership(reinterpret_cast<SbkObject *>(self));
        PySide::registerCleanupFunction(&PySide::destroyQCoreApplication);
    }
}
// @snippet qguiapplication-init

// @snippet qguiapplication-1
QGuiApplicationConstructor(%PYSELF, args, &%0);
// @snippet qguiapplication-1

// @snippet qguiapplication-2
PyObject *empty = PyTuple_New(2);
if (!PyTuple_SetItem(empty, 0, PyList_New(0))) {
    QGuiApplicationConstructor(%PYSELF, empty, &%0);
}
// @snippet qguiapplication-2

// @snippet qscreen-grabWindow
WId id = %1;
%RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(id, %2, %3, %4, %5);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](retval);
// @snippet qscreen-grabWindow

// @snippet qwindow-fromWinId
WId id = %1;
%RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(id);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](retval);
// @snippet qwindow-fromWinId

/*********************************************************************
 * CONVERSIONS
 ********************************************************************/

// @snippet conversion-pylong
%out = reinterpret_cast<%OUTTYPE>(PyLong_AsVoidPtr(%in));
// @snippet conversion-pylong

/*********************************************************************
 * NATIVE TO TARGET CONVERSIONS
 ********************************************************************/

// @snippet return-pylong-voidptr
return PyLong_FromVoidPtr(reinterpret_cast<void *>(%in));
// @snippet return-pylong-voidptr
