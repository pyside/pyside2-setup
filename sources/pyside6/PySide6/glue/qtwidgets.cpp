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

// @snippet qtreewidgetitemiterator-next
if (**%CPPSELF) {
    QTreeWidgetItemIterator *%0 = new QTreeWidgetItemIterator((*%CPPSELF)++);
    %PYARG_0 = %CONVERTTOPYTHON[QTreeWidgetItemIterator *](%0);
}
// @snippet qtreewidgetitemiterator-next

// @snippet qtreewidgetitemiterator-value
QTreeWidgetItem *%0 = %CPPSELF.operator *();
%PYARG_0 = %CONVERTTOPYTHON[QTreeWidgetItem *](%0);
Shiboken::Object::releaseOwnership(%PYARG_0);
// @snippet qtreewidgetitemiterator-value

// @snippet qgraphicsitem
PyObject *userTypeConstant =  PyInt_FromLong(QGraphicsItem::UserType);
PyDict_SetItemString(reinterpret_cast<PyTypeObject *>(Sbk_QGraphicsItem_TypeF())->tp_dict, "UserType", userTypeConstant);
// @snippet qgraphicsitem

// @snippet qgraphicsitem-scene-return-parenting
if (%0) {
    QObject *parent = %0->parent();
    Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QObject *](parent));
    Shiboken::Object::setParent(pyParent, %PYARG_0);
}
// @snippet qgraphicsitem-scene-return-parenting

// @snippet qgraphicsitem-isblockedbymodalpanel
QGraphicsItem *item_ = NULL;
%RETURN_TYPE retval_ = %CPPSELF.%FUNCTION_NAME(&item_);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](retval_));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[QGraphicsItem *](item_));
// @snippet qgraphicsitem-isblockedbymodalpanel

// @snippet qitemeditorfactory-registereditor
Shiboken::Object::releaseOwnership(%PYARG_2);
// @snippet qitemeditorfactory-registereditor

// @snippet qitemeditorfactory-setdefaultfactory
//this function is static we need keep ref to default value, to be able to call python virtual functions
static PyObject *_defaultValue = nullptr;
%CPPSELF.%FUNCTION_NAME(%1);
Py_INCREF(%PYARG_1);
if (_defaultValue)
    Py_DECREF(_defaultValue);

_defaultValue = %PYARG_1;
// @snippet qitemeditorfactory-setdefaultfactory

// @snippet qformlayout-fix-args
int _row;
QFormLayout::ItemRole _role;
%CPPSELF->%FUNCTION_NAME(%ARGUMENT_NAMES, &_row, &_role);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[int](_row));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[QFormLayout::ItemRole](_role));
// @snippet qformlayout-fix-args

// @snippet qfiledialog-return
%RETURN_TYPE retval_ = %CPPSELF.%FUNCTION_NAME(%1, %2, %3, %4, &%5, %6);
%PYARG_0 = PyTuple_New(2);
PyTuple_SET_ITEM(%PYARG_0, 0, %CONVERTTOPYTHON[%RETURN_TYPE](retval_));
PyTuple_SET_ITEM(%PYARG_0, 1, %CONVERTTOPYTHON[%ARG5_TYPE](%5));
// @snippet qfiledialog-return

// @snippet qmenu-glue
inline PyObject *addActionWithPyObject(QMenu *self, const QIcon &icon, const QString &text, PyObject *callback, const QKeySequence &shortcut)
{
    QAction *act = self->addAction(text);

    if (!icon.isNull())
        act->setIcon(icon);

    if (!shortcut.isEmpty())
        act->setShortcut(shortcut);

    self->addAction(act);

    PyObject *pyAct = %CONVERTTOPYTHON[QAction *](act);
    Shiboken::AutoDecRef result(PyObject_CallMethod(pyAct,
                                                    const_cast<char *>("connect"),
                                                    const_cast<char *>("OsO"),
                                                    pyAct,
                                                    SIGNAL(triggered()), callback));
    if (result.isNull()) {
        Py_DECREF(pyAct);
        return nullptr;
    }

    return pyAct;
}
// @snippet qmenu-glue

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
Shiboken::BindingManager &bm = Shiboken::BindingManager::instance();
const auto &actions = %CPPSELF.actions();
for (auto *act : actions) {
    if (auto wrapper = bm.retrieveWrapper(act)) {
        auto pyObj = reinterpret_cast<PyObject *>(wrapper);
        Py_INCREF(pyObj);
        Shiboken::Object::setParent(NULL, pyObj);
        Shiboken::Object::invalidate(pyObj);
        Py_DECREF(pyObj);
    }
}
// @snippet qmenu-clear

// @snippet qmenubar-glue
inline PyObject *
addActionWithPyObject(QMenuBar *self, const QString &text, PyObject *callback)
{
    QAction *act = self->addAction(text);

    self->addAction(act);

    PyObject *pyAct = %CONVERTTOPYTHON[QAction *](act);
    PyObject *result = PyObject_CallMethod(pyAct,
                                           const_cast<char *>("connect"),
                                           const_cast<char *>("OsO"),
                                           pyAct,
                                           SIGNAL(triggered(bool)), callback);

    if (result == nullptr || result == Py_False) {
        if (result)
            Py_DECREF(result);
        Py_DECREF(pyAct);
        return nullptr;
    }

    return pyAct;
}
// @snippet qmenubar-glue

// @snippet qmenubar-clear
const auto &actions = %CPPSELF.actions();
for (auto *act : actions) {
  Shiboken::AutoDecRef pyAct(%CONVERTTOPYTHON[QAction *](act));
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
    Shiboken::AutoDecRef pyWidget(%CONVERTTOPYTHON[QWidget *](_widget));
    Shiboken::Object::setParent(0, pyWidget);
}
// @snippet qtoolbox-removeitem

// @snippet qlayout-help-functions
void addLayoutOwnership(QLayout *layout, QLayoutItem *item);
void removeLayoutOwnership(QLayout *layout, QWidget *widget);

inline QByteArray retrieveObjectName(PyObject *obj)
{
    Shiboken::AutoDecRef objName(PyObject_Str(obj));
    return Shiboken::String::toCString(objName);
}

inline void addLayoutOwnership(QLayout *layout, QWidget *widget)
{
    //transfer ownership to parent widget
    QWidget *lw = layout->parentWidget();
    QWidget *pw = widget->parentWidget();

   Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QWidget *](widget));

    //Transfer parent to layout widget
    if (pw && lw && pw != lw)
        Shiboken::Object::setParent(0, pyChild);

    if (!lw && !pw) {
        //keep the reference while the layout is orphan
        Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QWidget *](layout));
        Shiboken::Object::keepReference(reinterpret_cast<SbkObject *>(pyParent.object()), retrieveObjectName(pyParent).data(), pyChild, true);
    } else {
        if (!lw)
            lw = pw;
        Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QWidget *](lw));
        Shiboken::Object::setParent(pyParent, pyChild);
    }
}

inline void addLayoutOwnership(QLayout *layout, QLayout *other)
{
    //transfer all children widgets from other to layout parent widget
    QWidget *parent = layout->parentWidget();
    if (!parent) {
        //keep the reference while the layout is orphan
        Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QLayout *](layout));
        Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QLayout *](other));
        Shiboken::Object::keepReference(reinterpret_cast<SbkObject *>(pyParent.object()),
                                        retrieveObjectName(pyParent).data(), pyChild, true);
        return;
    }

    for (int i=0, i_max=other->count(); i < i_max; i++) {
        QLayoutItem *item = other->itemAt(i);
        if (PyErr_Occurred() || !item)
            return;
        addLayoutOwnership(layout, item);
    }

    Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QLayout *](layout));
    Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QLayout *](other));
    Shiboken::Object::setParent(pyParent, pyChild);
}

inline void addLayoutOwnership(QLayout *layout, QLayoutItem *item)
{
    if (!item)
        return;

    if (QWidget *w = item->widget()) {
        addLayoutOwnership(layout, w);
    } else {
        if (QLayout *l = item->layout())
            addLayoutOwnership(layout, l);
    }

    Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QLayout *](layout));
    Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QLayoutItem *](item));
    Shiboken::Object::setParent(pyParent, pyChild);
}

static void removeWidgetFromLayout(QLayout *layout, QWidget *widget)
{
    QWidget *parent = widget->parentWidget();

    if (!parent) {
        //remove reference on layout
        Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QWidget *](layout));
        Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QWidget *](widget));
        Shiboken::Object::removeReference(reinterpret_cast<SbkObject *>(pyParent.object()),
                                          retrieveObjectName(pyParent).data(), pyChild);
    } else {
        //give the ownership to parent
        Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QWidget *](parent));
        Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QWidget *](widget));
        Shiboken::Object::setParent(pyParent, pyChild);
    }
}

inline void removeLayoutOwnership(QLayout *layout, QLayoutItem *item)
{
    if (QWidget *w = item->widget()) {
        removeWidgetFromLayout(layout, w);
    } else {
        QLayout *l = item->layout();
        if (l && item != l)
            removeLayoutOwnership(layout, l);
    }

    Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QLayoutItem *](item));
    Shiboken::Object::invalidate(pyChild);
    Shiboken::Object::setParent(0, pyChild);
}

inline void removeLayoutOwnership(QLayout *layout, QWidget *widget)
{
    if (!widget)
        return;

    for (int i=0, i_max=layout->count(); i < i_max; i++) {
        QLayoutItem *item = layout->itemAt(i);
        if (PyErr_Occurred() || !item)
            return;
        if (item->widget() == widget)
            removeLayoutOwnership(layout, item);
    }
}
// @snippet qlayout-help-functions

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
QGraphicsItem *parentItem = %1->parentItem();
Shiboken::AutoDecRef parent(%CONVERTTOPYTHON[QGraphicsItem *](parentItem));
const auto &childItems = %1->childItems();
for (auto *item : childItems)
    Shiboken::Object::setParent(parent, %CONVERTTOPYTHON[QGraphicsItem *](item));
%CPPSELF.%FUNCTION_NAME(%1);
// the arg was destroyed by Qt.
Shiboken::Object::invalidate(%PYARG_1);
// @snippet qgraphicsscene-destroyitemgroup

// @snippet qgraphicsscene-addwidget
%RETURN_TYPE %0 = %CPPSELF.%FUNCTION_NAME(%1, %2);
%PYARG_0 = %CONVERTTOPYTHON[%RETURN_TYPE](%0);
Shiboken::Object::keepReference(reinterpret_cast<SbkObject *>(%PYARG_0), "setWidget(QWidget*)1", %PYARG_1);
// @snippet qgraphicsscene-addwidget

// @snippet qgraphicsscene-clear
const QList<QGraphicsItem *> items = %CPPSELF.items();
Shiboken::BindingManager &bm = Shiboken::BindingManager::instance();
for (auto *item : items) {
    SbkObject *obj = bm.retrieveWrapper(item);
    if (obj) {
        if (reinterpret_cast<PyObject *>(obj)->ob_refcnt > 1) // If the refcnt is 1 the object will vannish anyway.
            Shiboken::Object::invalidate(obj);
        Shiboken::Object::removeParent(obj);
    }
}
%CPPSELF.%FUNCTION_NAME();
// @snippet qgraphicsscene-clear

// @snippet qtreewidget-clear
QTreeWidgetItem *rootItem = %CPPSELF.invisibleRootItem();
Shiboken::BindingManager &bm = Shiboken::BindingManager::instance();

// PYSIDE-1251:
// Since some objects can be created with a parent and without
// being saved on a local variable (refcount = 1), they will be
// deleted when setting the parent to nullptr, so we change the loop
// to do this from the last child to the first, to avoid the case
// when the child(1) points to the original child(2) in case the
// first one was removed.
for (int i = rootItem->childCount() - 1; i >= 0; --i) {
    QTreeWidgetItem *item = rootItem->child(i);
    if (SbkObject *wrapper = bm.retrieveWrapper(item))
        Shiboken::Object::setParent(nullptr, reinterpret_cast<PyObject *>(wrapper));
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
        auto pyObj = reinterpret_cast<PyObject *>(wrapper);
        Py_INCREF(pyObj);
        Shiboken::Object::setParent(NULL, pyObj);
        Shiboken::Object::invalidate(pyObj);
        Py_DECREF(pyObj);
    }
}
%CPPSELF.%FUNCTION_NAME();
// @snippet qlistwidget-clear

// @snippet qwidget-glue
static QString retrieveObjectName(PyObject *obj)
{
    Shiboken::AutoDecRef objName(PyObject_Str(obj));
    return QString(Shiboken::String::toCString(objName));
}


// Transfer objects ownership from layout to widget
static inline void qwidgetReparentLayout(QWidget *parent, QLayout *layout)
{
    Shiboken::AutoDecRef pyParent(%CONVERTTOPYTHON[QWidget *](parent));

    for (int i=0, i_count = layout->count(); i < i_count; i++) {
        QLayoutItem *item = layout->itemAt(i);
        if (PyErr_Occurred() || !item)
            return;

        if (QWidget *w = item->widget()) {
            QWidget *pw = w->parentWidget();
            if (pw != parent) {
                Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QWidget *](w));
                Shiboken::Object::setParent(pyParent, pyChild);
            }
        } else {
            if (QLayout *l = item->layout())
                qwidgetReparentLayout(parent, l);
        }
    }

    Shiboken::AutoDecRef pyChild(%CONVERTTOPYTHON[QLayout *](layout));
    Shiboken::Object::setParent(pyParent, pyChild);
    //remove previous references
    Shiboken::Object::keepReference(reinterpret_cast<SbkObject *>(pyChild.object()),
                                    qPrintable(retrieveObjectName(pyChild)), Py_None);
}

static inline void qwidgetSetLayout(QWidget *self, QLayout *layout)
{
    if (!layout || self->layout())
        return;

    QObject *oldParent = layout->parent();
    if (oldParent && oldParent != self) {
        if (oldParent->isWidgetType()) {
            // remove old parent policy
            Shiboken::AutoDecRef pyLayout(%CONVERTTOPYTHON[QLayout *](layout));
            Shiboken::Object::setParent(Py_None, pyLayout);
        } else {
            PyErr_Format(PyExc_RuntimeError, "QWidget::setLayout: Attempting to set QLayout \"%s\" on %s \"%s\", when the QLayout already has a parent",
                          qPrintable(layout->objectName()), self->metaObject()->className(), qPrintable(self->objectName()));
            return;
        }
    }

    if (oldParent != self) {
        qwidgetReparentLayout(self, layout);
        if (PyErr_Occurred())
            return;

        self->setLayout(layout);
    }
}
// @snippet qwidget-glue

// @snippet qwidget-setstyle
Shiboken::Object::keepReference(reinterpret_cast<SbkObject *>(%PYSELF), "__style__",  %PYARG_1);
// @snippet qwidget-setstyle

// @snippet qwidget-style
QStyle *myStyle = %CPPSELF->style();
if (myStyle && qApp) {
%PYARG_0 = %CONVERTTOPYTHON[QStyle *](myStyle);
    QStyle *appStyle = qApp->style();
    if (appStyle == myStyle) {
        Shiboken::AutoDecRef pyApp(%CONVERTTOPYTHON[QApplication *](qApp));
        Shiboken::Object::setParent(pyApp, %PYARG_0);
        Shiboken::Object::releaseOwnership(%PYARG_0);
    } else {
        Shiboken::Object::keepReference(reinterpret_cast<SbkObject *>(%PYSELF), "__style__",  %PYARG_0);
    }
}
// @snippet qwidget-style

// @snippet qapplication-init
static void QApplicationConstructor(PyObject *self, PyObject *pyargv, QApplicationWrapper **cptr)
{
    static int argc;
    static char **argv;
    PyObject *stringlist = PyTuple_GET_ITEM(pyargv, 0);
    if (Shiboken::listToArgcArgv(stringlist, &argc, &argv, "PySideApp")) {
        *cptr = new QApplicationWrapper(argc, argv, 0);
        Shiboken::Object::releaseOwnership(reinterpret_cast<SbkObject *>(self));
        PySide::registerCleanupFunction(&PySide::destroyQCoreApplication);
    }
}
// @snippet qapplication-init

// @snippet qapplication-setStyle
if (qApp) {
    Shiboken::AutoDecRef pyApp(%CONVERTTOPYTHON[QApplication *](qApp));
    Shiboken::Object::setParent(pyApp, %PYARG_1);
    Shiboken::Object::releaseOwnership(%PYARG_1);
}
// @snippet qapplication-setStyle

// @snippet qwidget-setlayout
qwidgetSetLayout(%CPPSELF, %1);
// %FUNCTION_NAME() - disable generation of function call.
// @snippet qwidget-setlayout

// @snippet qtabwidget-removetab
QWidget *tab = %CPPSELF.widget(%1);
if (tab) {
    Shiboken::AutoDecRef pyWidget(%CONVERTTOPYTHON[QWidget *](tab));
    %CPPSELF.%FUNCTION_NAME(%1);
}
// @snippet qtabwidget-removetab

// @snippet qtabwidget-clear
Shiboken::BindingManager &bm = Shiboken::BindingManager::instance();
for (int i = 0, count = %CPPSELF.count(); i < count; ++i) {
    QWidget *widget = %CPPSELF.widget(i);
    if (bm.hasWrapper(widget)) {
        Shiboken::AutoDecRef pyWidget(%CONVERTTOPYTHON[QWidget *](widget));
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
%PYARG_0 = %CONVERTTOPYTHON[QAction *](action);
Shiboken::AutoDecRef result(PyObject_CallMethod(%PYARG_0,
    const_cast<char *>("connect"),
    const_cast<char *>("OsO"),
    %PYARG_0, SIGNAL(triggered()), %PYARG_3)
);
// @snippet qtoolbar-addaction-1

// @snippet qtoolbar-addaction-2
QAction *action = %CPPSELF.addAction(%1);
%PYARG_0 = %CONVERTTOPYTHON[QAction *](action);
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
QList<PyObject *> lst;
Shiboken::BindingManager &bm = Shiboken::BindingManager::instance();
const auto &toolButtonChildren = %CPPSELF.findChildren<QToolButton *>();
for (auto *child : toolButtonChildren) {
    if (bm.hasWrapper(child)) {
        PyObject *pyChild = %CONVERTTOPYTHON[QToolButton *](child);
        Shiboken::Object::setParent(0, pyChild);
        lst << pyChild;
    }
}

//Remove actions
const auto &actions = %CPPSELF.actions();
for (auto *act : actions) {
    Shiboken::AutoDecRef pyAct(%CONVERTTOPYTHON[QAction *](act));
    Shiboken::Object::setParent(NULL, pyAct);
    Shiboken::Object::invalidate(pyAct);
}

%CPPSELF.clear();
for (auto *obj : lst) {
    Shiboken::Object::invalidate(reinterpret_cast<SbkObject *>(obj));
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
QWidget *_old = %CPPSELF.widget();
if (_old)
   Shiboken::Object::setParent(nullptr, %CONVERTTOPYTHON[QWidget *](_old));
%CPPSELF.%FUNCTION_NAME(%1);
Shiboken::Object::setParent(%PYSELF, %PYARG_1);
// @snippet qgraphicsproxywidget-setwidget

/*********************************************************************
 * CONVERSIONS
 ********************************************************************/

/*********************************************************************
 * NATIVE TO TARGET CONVERSIONS
 ********************************************************************/
