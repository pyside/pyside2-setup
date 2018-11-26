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

// @snippet qtreewidgetitemiterator-next
if (**%CPPSELF) {
    QTreeWidgetItemIterator *%0 = new QTreeWidgetItemIterator((*%CPPSELF)++);
    %PYARG_0 = %CONVERTTOPYTHON[QTreeWidgetItemIterator*](%0);
}
// @snippet qtreewidgetitemiterator-next

// @snippet qtreewidgetitemiterator-value
QTreeWidgetItem *%0 = %CPPSELF.operator*();
%PYARG_0 = %CONVERTTOPYTHON[QTreeWidgetItem*](%0);
Shiboken::Object::releaseOwnership(%PYARG_0);
// @snippet qtreewidgetitemiterator-value

// @snippet qgraphicsitem
PyObject *userTypeConstant =  PyInt_FromLong(QGraphicsItem::UserType);
PyDict_SetItemString(reinterpret_cast<PyTypeObject *>(Sbk_QGraphicsItem_TypeF())->tp_dict, "UserType", userTypeConstant);
// @snippet qgraphicsitem

// @snippet qgraphicsitem-scene-return-parenting
if (%0) {
    QObject *parent = %0->parent();
    Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QObject*](parent));
    Shiboken::Object::setParent(pyParent, %PYARG_0);
}
// @snippet qgraphicsitem-scene-return-parenting

// @snippet qgraphicsitem-isblockedbymodalpanel
QGraphicsItem *item_ = NULL;
%RETURN_TYPE retval_ = %CPPSELF.%FUNCTION_NAME(&item_);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](retval_));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[QGraphicsItem*](item_));
// @snippet qgraphicsitem-isblockedbymodalpanel

// @snippet qitemeditorfactory-registereditor
Shiboken::Object::releaseOwnership(%PYARG_2);
// @snippet qitemeditorfactory-registereditor

// @snippet qitemeditorfactory-setdefaultfactory
//this function is static we need keep ref to default value, to be able to call python virtual functions
static PyObject* _defaultValue = 0;
%CPPSELF.%FUNCTION_NAME(%1);
Py_INCREF(%PYARG_1);
if (_defaultValue)
    Py_DECREF(_defaultValue);

_defaultValue = %PYARG_1;
// @snippet qitemeditorfactory-setdefaultfactory

// @snippet qmenu-addaction-1
%PYARG_0 = addActionWithPyObject(%CPPSELF, QIcon(), %1, %2, %3);
// @snippet qmenu-addaction-1

// @snippet qmenu-addaction-2
%PYARG_0 = addActionWithPyObject(%CPPSELF, %1, %2, %3, %4);
// @snippet qmenu-addaction-2

// @snippet qmenu-addaction-3
%CPPSELF.addAction(%1);
// @snippet qmenu-addaction-3

// @snippet qmenu-clear
Shiboken::BindingManager& bm = Shiboken::BindingManager::instance();
const auto &actions = %CPPSELF.actions();
for (QAction *act : actions) {
    if (auto wrapper = bm.retrieveWrapper(act)) {
        auto pyObj = reinterpret_cast<PyObject *>(wrapper);
        Py_INCREF(pyObj);
        Shiboken::Object::setParent(NULL, pyObj);
        Shiboken::Object::invalidate(pyObj);
        Py_DECREF(pyObj);
    }
}
// @snippet qmenu-clear

// @snippet qmenubar-clear
const auto &actions = %CPPSELF.actions();
for (QAction *act : actions) {
  Shiboken::AutoDecRef pyAct(%CONVERTTOPYTHON[QAction*](act));
  Shiboken::Object::setParent(NULL, pyAct);
  Shiboken::Object::invalidate(pyAct);
}
// @snippet qmenubar-clear

// @snippet qmenubar-addaction-1
%PYARG_0 = addActionWithPyObject(%CPPSELF, %1, %2);
// @snippet qmenubar-addaction-1

// @snippet qmenubar-addaction-2
%CPPSELF.addAction(%1);
// @snippet qmenubar-addaction-2

// @snippet qshortcut-1
%0 = new %TYPE(%1, %2);
// @snippet qshortcut-1

// @snippet qshortcut-2
Shiboken::AutoDecRef result(PyObject_CallMethod(%PYSELF,
    const_cast<char *>("connect"),
    const_cast<char *>("OsO"),
    %PYSELF, SIGNAL(activated()), %PYARG_3)
);
if (!result.isNull())
    Shiboken::Object::setParent(%PYARG_2, %PYSELF);
// @snippet qshortcut-2

// @snippet qtoolbox-removeitem
QWidget *_widget = %CPPSELF.widget(%1);
if (_widget) {
    Shiboken::AutoDecRef pyWidget(%CONVERTTOPYTHON[QWidget*](_widget));
    Shiboken::Object::setParent(0, pyWidget);
}
// @snippet qtoolbox-removeitem

// @snippet qlayout-setalignment
%CPPSELF.setAlignment(%1);
// @snippet qlayout-setalignment

// @snippet addownership-0
addLayoutOwnership(%CPPSELF, %0);
// @snippet addownership-0

// @snippet addownership-1
addLayoutOwnership(%CPPSELF, %1);
// @snippet addownership-1

// @snippet addownership-2
addLayoutOwnership(%CPPSELF, %2);
// @snippet addownership-2

// @snippet removeownership-1
removeLayoutOwnership(%CPPSELF, %1);
// @snippet removeownership-1

// @snippet qgridlayout-getitemposition
int a, b, c, d;
%CPPSELF.%FUNCTION_NAME(%1, &a, &b, &c, &d);
%PYARG_0 = PyTuple_New(4);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[int](a));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[int](b));
PyTuple_SET_ITEM(%PYARG_0, 2, %CONVERTTOPYTHON[int](c));
PyTuple_SET_ITEM(%PYARG_0, 3, %CONVERTTOPYTHON[int](d));
// @snippet qgridlayout-getitemposition

// @snippet qgraphicsscene-destroyitemgroup
QGraphicsItem* parentItem = %1->parentItem();
Shiboken::AutoDecRef parent(%CONVERTTOPYTHON[QGraphicsItem*](parentItem));
const auto &childItems = %1->childItems();
for (QGraphicsItem *item : childItems)
    Shiboken::Object::setParent(parent, %CONVERTTOPYTHON[QGraphicsItem*](item));
%BEGIN_ALLOW_THREADS
%CPPSELF.%FUNCTION_NAME(%1);
%END_ALLOW_THREADS
// the arg was destroyed by Qt.
Shiboken::Object::invalidate(%PYARG_1);
// @snippet qgraphicsscene-destroyitemgroup

// @snippet qgraphicsscene-addwidget
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(%1, %2);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
Shiboken::Object::keepReference((SbkObject*)%PYARG_0, "setWidget(QWidget*)1", %PYARG_1);
// @snippet qgraphicsscene-addwidget

// @snippet qgraphicsscene-clear
const QList<QGraphicsItem*> items = %CPPSELF.items();
Shiboken::BindingManager& bm = Shiboken::BindingManager::instance();
for (QGraphicsItem *item : items) {
    SbkObject* obj = bm.retrieveWrapper(item);
    if (obj) {
        if (reinterpret_cast<PyObject*>(obj)->ob_refcnt > 1) // If the refcnt is 1 the object will vannish anyway.
            Shiboken::Object::invalidate(obj);
        Shiboken::Object::removeParent(obj);
    }
}
%CPPSELF.%FUNCTION_NAME();
// @snippet qgraphicsscene-clear

// @snippet qtreewidget-clear
QTreeWidgetItem *rootItem = %CPPSELF.invisibleRootItem();
Shiboken::BindingManager &bm = Shiboken::BindingManager::instance();
for (int i = 0; i < rootItem->childCount(); ++i) {
    QTreeWidgetItem *item = rootItem->child(i);
    SbkObject* wrapper = bm.retrieveWrapper(item);
    if (wrapper)
        Shiboken::Object::setParent(0, reinterpret_cast<PyObject*>(wrapper));
}
// @snippet qtreewidget-clear

// @snippet qtreewidgetitem
// Only call the parent function if this return some value
// the parent can be the TreeWidget
if (%0)
  Shiboken::Object::setParent(%PYARG_0, %PYSELF);
// @snippet qtreewidgetitem

// @snippet qlistwidget-clear
Shiboken::BindingManager &bm = Shiboken::BindingManager::instance();
for (int i = 0, count = %CPPSELF.count(); i < count; ++i) {
    QListWidgetItem *item = %CPPSELF.item(i);
    if (auto wrapper = bm.retrieveWrapper(item)) {
        auto pyObj = reinterpret_cast<PyObject*>(wrapper);
        Py_INCREF(pyObj);
        Shiboken::Object::setParent(NULL, pyObj);
        Shiboken::Object::invalidate(pyObj);
        Py_DECREF(pyObj);
    }
}
%CPPSELF.%FUNCTION_NAME();
// @snippet qlistwidget-clear

// @snippet qwidget-setstyle
Shiboken::Object::keepReference(reinterpret_cast<SbkObject*>(%PYSELF), "__style__",  %PYARG_1);
// @snippet qwidget-setstyle

// @snippet qwidget-style
QStyle* myStyle = %CPPSELF->style();
if (myStyle && qApp) {
%PYARG_0 = %CONVERTTOPYTHON[QStyle*](myStyle);
    QStyle *appStyle = qApp->style();
    if (appStyle == myStyle) {
        Shiboken::AutoDecRef pyApp(%CONVERTTOPYTHON[QApplication*](qApp));
        Shiboken::Object::setParent(pyApp, %PYARG_0);
        Shiboken::Object::releaseOwnership(%PYARG_0);
    } else {
        Shiboken::Object::keepReference(reinterpret_cast<SbkObject*>(%PYSELF), "__style__",  %PYARG_0);
    }
}
// @snippet qwidget-style

// @snippet qwidget-setlayout
qwidgetSetLayout(%CPPSELF, %1);
// %FUNCTION_NAME() - disable generation of function call.
// @snippet qwidget-setlayout

// @snippet qtabwidget-removetab
QWidget* tab = %CPPSELF.widget(%1);
if (tab) {
    Shiboken::AutoDecRef pyWidget(%CONVERTTOPYTHON[QWidget*](tab));
    %CPPSELF.%FUNCTION_NAME(%1);
}
// @snippet qtabwidget-removetab

// @snippet qtabwidget-clear
Shiboken::BindingManager& bm = Shiboken::BindingManager::instance();
for (int i = 0, count = %CPPSELF.count(); i < count; ++i) {
    QWidget* widget = %CPPSELF.widget(i);
    if (bm.hasWrapper(widget)) {
        Shiboken::AutoDecRef pyWidget(%CONVERTTOPYTHON[QWidget*](widget));
        Shiboken::Object::releaseOwnership(pyWidget);
    }
}
%CPPSELF.%FUNCTION_NAME();
// @snippet qtabwidget-clear

// @snippet qlineedit-addaction
%CPPSELF.addAction(%1);
// @snippet qlineedit-addaction

// @snippet qtoolbar-addaction-1
QAction *action = %CPPSELF.addAction(%1, %2);
%PYARG_0 = %CONVERTTOPYTHON[QAction*](action);
Shiboken::AutoDecRef result(PyObject_CallMethod(%PYARG_0,
    const_cast<char *>("connect"),
    const_cast<char *>("OsO"),
    %PYARG_0, SIGNAL(triggered()), %PYARG_3)
);
// @snippet qtoolbar-addaction-1

// @snippet qtoolbar-addaction-2
QAction *action = %CPPSELF.addAction(%1);
%PYARG_0 = %CONVERTTOPYTHON[QAction*](action);
Shiboken::AutoDecRef result(PyObject_CallMethod(%PYARG_0,
    const_cast<char *>("connect"),
    const_cast<char *>("OsO"),
    %PYARG_0, SIGNAL(triggered()), %PYARG_2)
);
// @snippet qtoolbar-addaction-2

// @snippet qtoolbar-addaction-3
%CPPSELF.addAction(%1);
// @snippet qtoolbar-addaction-3

// @snippet qtoolbar-clear
QList<PyObject* > lst;
Shiboken::BindingManager& bm = Shiboken::BindingManager::instance();
const auto &toolButtonChildren = %CPPSELF.findChildren<QToolButton*>();
for (QToolButton *child : toolButtonChildren) {
    if (bm.hasWrapper(child)) {
        PyObject* pyChild = %CONVERTTOPYTHON[QToolButton*](child);
        Shiboken::Object::setParent(0, pyChild);
        lst << pyChild;
    }
}

//Remove actions
const auto &actions = %CPPSELF.actions();
for (QAction *act : actions) {
    Shiboken::AutoDecRef pyAct(%CONVERTTOPYTHON[QAction*](act));
    Shiboken::Object::setParent(NULL, pyAct);
    Shiboken::Object::invalidate(pyAct);
}

%CPPSELF.clear();
for (PyObject *obj : lst) {
    Shiboken::Object::invalidate(reinterpret_cast<SbkObject* >(obj));
    Py_XDECREF(obj);
}
// @snippet qtoolbar-clear

// @snippet qapplication-1
QApplicationConstructor(%PYSELF, args, &%0);
// @snippet qapplication-1

// @snippet qapplication-2
PyObject *empty = PyTuple_New(2);
if (!PyTuple_SetItem(empty, 0, PyList_New(0)))
    QApplicationConstructor(%PYSELF, empty, &%0);
// @snippet qapplication-2

// @snippet qgraphicsproxywidget-setwidget
QWidget* _old = %CPPSELF.widget();
if (_old)
   Shiboken::Object::setParent(nullptr, %CONVERTTOPYTHON[QWidget*](_old));
%CPPSELF.%FUNCTION_NAME(%1);
Shiboken::Object::setParent(%PYSELF, %PYARG_1);
// @snippet qgraphicsproxywidget-setwidget
