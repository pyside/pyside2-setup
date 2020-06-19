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
// @snippet uitools-loadui
/*
 * Based on code provided by:
 *          Antonio Valentino <antonio.valentino at tiscali.it>
 *          Frédéric <frederic.mantegazza at gbiloba.org>
 */

#include <shiboken.h>
#include <QUiLoader>
#include <QFile>
#include <QWidget>

static void createChildrenNameAttributes(PyObject *root, QObject *object)
{
    for (auto *child : object->children()) {
        const QByteArray name = child->objectName().toLocal8Bit();

        if (!name.isEmpty() && !name.startsWith("_") && !name.startsWith("qt_")) {
            Shiboken::AutoDecRef attrName(Py_BuildValue("s", name.constData()));
            if (!PyObject_HasAttr(root, attrName)) {
                Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QObject *](child));
                PyObject_SetAttr(root, attrName, pyChild);
            }
            createChildrenNameAttributes(root, child);
        }
        createChildrenNameAttributes(root, child);
    }
}

static PyObject *QUiLoadedLoadUiFromDevice(QUiLoader *self, QIODevice *dev, QWidget *parent)
{
    QWidget *wdg = self->load(dev, parent);

    if (wdg) {
        PyObject *pyWdg = %CONVERTTOPYTHON[QWidget *](wdg);
        createChildrenNameAttributes(pyWdg, wdg);
        if (parent) {
            Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QWidget *](parent));
            Shiboken::Object::setParent(pyParent, pyWdg);
        }
        return pyWdg;
    }

    if (!PyErr_Occurred())
        PyErr_SetString(PyExc_RuntimeError, "Unable to open/read ui device");
    return nullptr;
}

static PyObject *QUiLoaderLoadUiFromFileName(QUiLoader *self, const QString &uiFile, QWidget *parent)
{
    QFile fd(uiFile);
    return QUiLoadedLoadUiFromDevice(self, &fd, parent);
}
// @snippet uitools-loadui

// @snippet quiloader
Q_IMPORT_PLUGIN(PyCustomWidgets);
// @snippet quiloader

// @snippet quiloader-registercustomwidget
registerCustomWidget(%PYARG_1);
%CPPSELF.addPluginPath(""); // force reload widgets
// @snippet quiloader-registercustomwidget

// @snippet quiloader-load-1
// Avoid calling the original function: %CPPSELF.%FUNCTION_NAME()
%PYARG_0 = QUiLoadedLoadUiFromDevice(%CPPSELF, %1, %2);
// @snippet quiloader-load-1

// @snippet quiloader-load-2
// Avoid calling the original function: %CPPSELF.%FUNCTION_NAME()
%PYARG_0 = QUiLoaderLoadUiFromFileName(%CPPSELF, %1, %2);
// @snippet quiloader-load-2

// @snippet loaduitype
/*
Arguments:
    %PYARG_1 (uifile)
*/
// 1. Generate the Python code from the UI file
#ifdef IS_PY3K
PyObject *strObj = PyUnicode_AsUTF8String(%PYARG_1);
char *arg1 = PyBytes_AsString(strObj);
QByteArray uiFileName(arg1);
Py_DECREF(strObj);
#else
QByteArray uiFileName(PyBytes_AsString(%PYARG_1));
#endif

QFile uiFile(uiFileName);

if (!uiFile.exists()) {
    qCritical().noquote() << "File" << uiFileName << "does not exists";
    Py_RETURN_NONE;
}

if (uiFileName.isEmpty()) {
    qCritical() << "Error converting the UI filename to QByteArray";
    Py_RETURN_NONE;
}

// Use the 'pyside2-uic' wrapper instead of 'uic'
// This approach is better than rely on 'uic' since installing
// the wheels cover this case.
QString uicBin("pyside2-uic");
QStringList uicArgs = {QString::fromUtf8(uiFileName)};

QProcess uicProcess;
uicProcess.start(uicBin, uicArgs);
if (!uicProcess.waitForFinished()) {
    qCritical() << "Cannot run 'pyside2-uic': " << uicProcess.errorString() << " - "
                << "Exit status " << uicProcess.exitStatus()
                << " (" << uicProcess.exitCode() << ")\n"
                << "Check if 'pyside2-uic' is in PATH";
    Py_RETURN_NONE;
}
QByteArray uiFileContent = uicProcess.readAllStandardOutput();
QByteArray errorOutput = uicProcess.readAllStandardError();

if (!errorOutput.isEmpty()) {
    qCritical().noquote() << errorOutput;
    Py_RETURN_NONE;
}

// 2. Obtain the 'classname' and the Qt base class.
QByteArray className;
QByteArray baseClassName;

// Problem
// The generated Python file doesn't have the Qt Base class information.

// Solution
// Use the XML file
if (!uiFile.open(QIODevice::ReadOnly))
    Py_RETURN_NONE;

// This will look for the first <widget> tag, e.g.:
//      <widget class="QWidget" name="ThemeWidgetForm">
// and then extract the information from "class", and "name",
// to get the baseClassName and className respectively
QXmlStreamReader reader(&uiFile);
while (!reader.atEnd() && baseClassName.isEmpty() && className.isEmpty()) {
    auto token = reader.readNext();
    if (token == QXmlStreamReader::StartElement && reader.name() == "widget") {
        baseClassName = reader.attributes().value(QLatin1String("class")).toUtf8();
        className = reader.attributes().value(QLatin1String("name")).toUtf8();
    }
}

uiFile.close();

if (className.isEmpty() || baseClassName.isEmpty() || reader.hasError()) {
    qCritical() << "An error occurred when parsing the UI file while looking for the class info "
                << reader.errorString();
    Py_RETURN_NONE;
}

QByteArray pyClassName("Ui_"+className);

PyObject *module = PyImport_ImportModule("__main__");
PyObject *loc = PyModule_GetDict(module);

// 3. exec() the code so the class exists in the context: exec(uiFileContent)
// The context of PyRun_SimpleString is __main__.
// 'Py_file_input' is the equivalent to using exec(), since it will execute
// the code, without returning anything.
Shiboken::AutoDecRef codeUi(Py_CompileString(uiFileContent.constData(), "<stdin>", Py_file_input));
if (codeUi.isNull()) {
    qCritical() << "Error while compiling the generated Python file";
    Py_RETURN_NONE;
}
PyObject *uiObj = nullptr;
#ifdef IS_PY3K
uiObj = PyEval_EvalCode(codeUi, loc, loc);
#else
uiObj = PyEval_EvalCode(reinterpret_cast<PyCodeObject *>(codeUi.object()), loc, loc);
#endif

if (uiObj == nullptr) {
    qCritical() << "Error while running exec() on the generated code";
    Py_RETURN_NONE;
}

// 4. eval() the name of the class on a variable to return
// 'Py_eval_input' is the equivalent to using eval(), since it will just
// evaluate an expression.
Shiboken::AutoDecRef codeClass(Py_CompileString(pyClassName.constData(),"<stdin>", Py_eval_input));
if (codeClass.isNull()) {
    qCritical() << "Error while compiling the Python class";
    Py_RETURN_NONE;
}

Shiboken::AutoDecRef codeBaseClass(Py_CompileString(baseClassName.constData(), "<stdin>", Py_eval_input));
if (codeBaseClass.isNull()) {
    qCritical() << "Error while compiling the base class";
    Py_RETURN_NONE;
}

#ifdef IS_PY3K
PyObject *classObj = PyEval_EvalCode(codeClass, loc, loc);
PyObject *baseClassObj = PyEval_EvalCode(codeBaseClass, loc, loc);
#else
PyObject *classObj = PyEval_EvalCode(reinterpret_cast<PyCodeObject *>(codeClass.object()), loc, loc);
PyObject *baseClassObj = PyEval_EvalCode(reinterpret_cast<PyCodeObject *>(codeBaseClass.object()), loc, loc);
#endif

%PYARG_0  = PyTuple_New(2);
if (%PYARG_0 == nullptr) {
    qCritical() << "Error while creating the return Tuple";
    Py_RETURN_NONE;
}
PyTuple_SET_ITEM(%PYARG_0, 0, classObj);
PyTuple_SET_ITEM(%PYARG_0, 1, baseClassObj);
// @snippet loaduitype
