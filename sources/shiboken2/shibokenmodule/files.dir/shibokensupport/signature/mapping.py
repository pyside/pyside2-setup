#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qt for Python.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

from __future__ import print_function, absolute_import

"""
mapping.py

This module has the mapping from the pyside C-modules view of signatures
to the Python representation.

The PySide modules are not loaded in advance, but only after they appear
in sys.modules. This minimizes the loading overhead.
"""

import sys
import struct
import os

from shibokensupport.signature import typing
from shibokensupport.signature.typing import TypeVar, Generic
from shibokensupport.signature.lib.tool import with_metaclass

class ellipsis(object):
    def __repr__(self):
        return "..."

ellipsis = ellipsis()
Point = typing.Tuple[float, float]
Variant = typing.Any
ModelIndexList = typing.List[int]
QImageCleanupFunction = typing.Callable
StringList = typing.List[str]

# unfortunately, typing.Optional[t] expands to typing.Union[t, NoneType]
# Until we can force it to create Optional[t] again, we use this.
NoneType = type(None)

_S = TypeVar("_S")

# Building our own Char type, which is much nicer than
# Char = typing.Union[str, int]     # how do I model the limitation to 1 char?

class _CharMeta(type):
    def __repr__(self):
        return '%s.%s' % (self.__module__, self.__name__)


class Char(with_metaclass(_CharMeta)):
    """
    From http://doc.qt.io/qt-5/qchar.html :

    In Qt, Unicode characters are 16-bit entities without any markup or
    structure. This class represents such an entity. It is lightweight,
    so it can be used everywhere. Most compilers treat it like an
    unsigned short.

    Here, we provide a simple implementation just to avoid long aliases.
    """
    __module__ = "typing"

    def __init__(self, code):
        if isinstance(code, int):
            self.code = code & 0xffff
        else:
            self.code = ord(code)

    def __add__(self, other):
        return chr(self.code) + other

    def __radd__(self, other):
        return other + chr(self.code)

    def __repr__(self):
        return "typing.Char({})".format(self.code)

typing.Char = Char


MultiMap = typing.DefaultDict[str, typing.List[str]]

# ulong_max is only 32 bit on windows.
ulong_max = 2*sys.maxsize+1 if len(struct.pack("L", 1)) != 4 else 0xffffffff
ushort_max = 0xffff

GL_COLOR_BUFFER_BIT = 0x00004000
GL_NEAREST = 0x2600

WId = int

# from 5.9
GL_TEXTURE_2D = 0x0DE1
GL_RGBA = 0x1908


class _NotCalled(str):
    """
    Wrap some text with semantics

    This class is wrapped around text in order to avoid calling it.
    There are three reasons for this:

      - some instances cannot be created since they are abstract,
      - some can only be created after qApp was created,
      - some have an ugly __repr__ with angle brackets in it.

    By using derived classes, good looking instances can be created
    which can be used to generate source code or .pyi files. When the
    real object is needed, the wrapper can simply be called.
    """
    def __repr__(self):
        return "{}({})".format(type(self).__name__, self)

    def __call__(self):
        from shibokensupport.signature.mapping import __dict__ as namespace
        text = self if self.endswith(")") else self + "()"
        return eval(text, namespace)

USE_PEP563 = False
# Note: we cannot know if this feature has been imported.
# Otherwise it would be "sys.version_info[:2] >= (3, 7)".
# We *can* eventually inspect sys.modules and look if
# the calling module has this future statement set,
# but should we do that?


# Some types are abstract. They just show their name.
class Virtual(_NotCalled):
    pass

# Other types I simply could not find.
class Missing(_NotCalled):
    # The string must be quoted, because the object does not exist.
    def __repr__(self):
        if USE_PEP563:
            return _NotCalled.__repr__(self)
        return '{}("{}")'.format(type(self).__name__, self)


class Invalid(_NotCalled):
    pass

# Helper types
class Default(_NotCalled):
    pass


class Instance(_NotCalled):
    pass


class Reloader(object):
    """
    Reloder class

    This is a singleton class which provides the update function for the
    shiboken and PySide classes.
    """
    def __init__(self):
        self.sys_module_count = 0

    @staticmethod
    def module_valid(mod):
        if getattr(mod, "__file__", None) and not os.path.isdir(mod.__file__):
            ending = os.path.splitext(mod.__file__)[-1]
            return ending not in (".py", ".pyc", ".pyo", ".pyi")
        return False

    def update(self):
        """
        'update' imports all binary modules which are already in sys.modules.
        The reason is to follow all user imports without introducing new ones.
        This function is called by pyside_type_init to adapt imports
        when the number of imported modules has changed.
        """
        if self.sys_module_count == len(sys.modules):
            return
        self.sys_module_count = len(sys.modules)
        g = globals()
        # PYSIDE-1009: Try to recognize unknown modules in errorhandler.py
        candidates = list(mod_name for mod_name in sys.modules.copy()
                          if self.module_valid(sys.modules[mod_name]))
        for mod_name in candidates:
            # 'top' is PySide2 when we do 'import PySide.QtCore'
            # or Shiboken if we do 'import Shiboken'.
            # Convince yourself that these two lines below have the same
            # global effect as "import Shiboken" or "import PySide2.QtCore".
            top = __import__(mod_name)
            g[top.__name__] = top
            proc_name = "init_" + mod_name.replace(".", "_")
            if proc_name in g:
                # Modules are in place, we can update the type_map.
                g.update(g.pop(proc_name)())


def check_module(mod):
    # During a build, there exist the modules already as directories,
    # although the '*.so' was not yet created. This causes a problem
    # in Python 3, because it accepts folders as namespace modules
    # without enforcing an '__init__.py'.
    if not Reloader.module_valid(mod):
        mod_name = mod.__name__
        raise ImportError("Module '{mod_name}' is not a binary module!"
                          .format(**locals()))

update_mapping = Reloader().update
type_map = {}
namespace = globals()  # our module's __dict__

type_map.update({
    "bool": bool,
    "char": Char,
    "char*": str,
    "char*const": str,
    "double": float,
    "float": float,
    "int": int,
    "List": typing.List,
    "long": int,
    "PyCallable": typing.Callable,
    "PyObject": object,
    "PySequence": typing.Iterable,  # important for numpy
    "PyTypeObject": type,
    "QChar": Char,
    "QHash": typing.Dict,
    "qint16": int,
    "qint32": int,
    "qint64": int,
    "qint8": int,
    "qintptr": int,
    "QList": typing.List,
    "qlonglong": int,
    "QMap": typing.Dict,
    "QPair": typing.Tuple,
    "qptrdiff": int,
    "qreal": float,
    "QSet": typing.Set,
    "QString": str,
    "QStringList": StringList,
    "quint16": int,
    "quint32": int,
    "quint32": int,
    "quint64": int,
    "quint8": int,
    "quintptr": int,
    "qulonglong": int,
    "QVariant": Variant,
    "QVector": typing.List,
    "real": float,
    "short": int,
    "signed char": Char,
    "signed long": int,
    "str": str,
    "true": True,
    "ULONG_MAX": ulong_max,
    "unsigned char": Char,
    "unsigned int": int, # should we define an unsigned type?
    "unsigned long int": int, # 5.6, RHEL 6.6
    "unsigned long long": int,
    "unsigned long": int,
    "unsigned short int": int, # 5.6, RHEL 6.6
    "unsigned short": int,
    "UnsignedShortType": int, # 5.9
    "void": int, # be more specific?
    "WId": WId,
    "zero(bytes)": b"",
    "zero(Char)": 0,
    "zero(float)": 0,
    "zero(int)": 0,
    "zero(object)": None,
    "zero(str)": "",
    "...": "...",
    })


# The Shiboken Part
def init_Shiboken():
    type_map.update({
        "PyType": type,
        "shiboken2.bool": bool,
        "size_t": int,
    })
    return locals()


def init_minimal():
    type_map.update({
        "MinBool": bool,
    })
    return locals()


def init_sample():
    import datetime
    type_map.update({
        "char": Char,
        "Complex": complex,
        "double": float,
        "Foo.HANDLE": int,
        "HANDLE": int,
        "Null": None,
        "nullptr": None,
        "ObjectType.Identifier": Missing("sample.ObjectType.Identifier"),
        "OddBool": bool,
        "PStr": str,
        "PyDate": datetime.date,
        "sample.bool": bool,
        "sample.char": Char,
        "sample.double": float,
        "sample.int": int,
        "sample.ObjectType": object,
        "sample.OddBool": bool,
        "sample.Photon.TemplateBase": Missing("sample.Photon.TemplateBase"),
        "sample.Point": Point,
        "sample.PStr": str,
        "sample.unsigned char": Char,
        "std.size_t": int,
        "std.string": str,
        "ZeroIn": 0,
        'Str("<unk")': "<unk",
        'Str("<unknown>")': "<unknown>",
        'Str("nown>")': "nown>",
    })
    return locals()


def init_other():
    import numbers
    type_map.update({
        "other.ExtendsNoImplicitConversion": Missing("other.ExtendsNoImplicitConversion"),
        "other.Number": numbers.Number,
    })
    return locals()


def init_smart():
    # This missing type should be defined in module smart. We cannot set it to Missing()
    # because it is a container type. Therefore, we supply a surrogate:
    global SharedPtr
    class SharedPtr(Generic[_S]):
        __module__ = "smart"
    smart.SharedPtr = SharedPtr
    type_map.update({
        "smart.Smart.Integer2": int,
    })
    return locals()

# The PySide Part
def init_PySide2_QtCore():
    from PySide2.QtCore import Qt, QUrl, QDir
    from PySide2.QtCore import QRect, QSize, QPoint, QLocale, QByteArray
    from PySide2.QtCore import QMarginsF # 5.9
    try:
        # seems to be not generated by 5.9 ATM.
        from PySide2.QtCore import Connection
    except ImportError:
        pass
    type_map.update({
        "' '": " ",
        "'%'": "%",
        "'g'": "g",
        "4294967295UL": 4294967295, # 5.6, RHEL 6.6
        "CheckIndexOption.NoOption": Instance(
            "PySide2.QtCore.QAbstractItemModel.CheckIndexOptions.NoOption"), # 5.11
        "false": False,
        "list of QAbstractAnimation": typing.List[PySide2.QtCore.QAbstractAnimation],
        "list of QAbstractState": typing.List[PySide2.QtCore.QAbstractState],
        "long long": int,
        "long": int,
        "NULL": None, # 5.6, MSVC
        "nullptr": None, # 5.9
        "PyByteArray": bytearray,
        "PyBytes": bytes,
        "PyCallable": typing.Callable,
        "PyObject": object,
        "PySequence": typing.Iterable,  # important for numpy
        "PySide2.QtCore.bool": bool,
        "PySide2.QtCore.char": StringList, # A 'char **' is a list of strings.
        "PySide2.QtCore.double": float,
        "PySide2.QtCore.float": float,
        "PySide2.QtCore.int": int,
        "PySide2.QtCore.int32_t": int, # 5.9
        "PySide2.QtCore.int64_t": int, # 5.9
        "PySide2.QtCore.long long": int, # 5.9, MSVC 15
        "PySide2.QtCore.long": int,
        "PySide2.QtCore.QCborStreamReader.StringResult": typing.AnyStr,
        "PySide2.QtCore.QChar": Char,
        "PySide2.QtCore.qint16": int,
        "PySide2.QtCore.qint32": int,
        "PySide2.QtCore.qint64": int,
        "PySide2.QtCore.qint8": int,
        "PySide2.QtCore.qreal": float,
        "PySide2.QtCore.QString": str,
        "PySide2.QtCore.QStringList": StringList,
        "PySide2.QtCore.quint16": int,
        "PySide2.QtCore.quint32": int,
        "PySide2.QtCore.quint64": int,
        "PySide2.QtCore.quint8": int,
        "PySide2.QtCore.QUrl.ComponentFormattingOptions":
            PySide2.QtCore.QUrl.ComponentFormattingOption, # mismatch option/enum, why???
        "PySide2.QtCore.QVariant": Variant,
        "PySide2.QtCore.short": int,
        "PySide2.QtCore.signed char": Char,
        "PySide2.QtCore.uchar": Char,
        "PySide2.QtCore.uint32_t": int, # 5.9
        "PySide2.QtCore.unsigned char": Char, # 5.9
        "PySide2.QtCore.unsigned int": int, # 5.9 Ubuntu
        "PySide2.QtCore.unsigned short": int,
        "PyTypeObject": type,
        "PyUnicode": typing.Text,
        "Q_NULLPTR": None,
        "QChar": Char,
        "QDir.Filters(AllEntries | NoDotAndDotDot)": Instance(
            "QDir.Filters(QDir.AllEntries | QDir.NoDotAndDotDot)"),
        "QDir.SortFlags(Name | IgnoreCase)": Instance(
            "QDir.SortFlags(QDir.Name | QDir.IgnoreCase)"),
        "QGenericArgument((0))": ellipsis, # 5.6, RHEL 6.6. Is that ok?
        "QGenericArgument()": ellipsis,
        "QGenericArgument(0)": ellipsis,
        "QGenericArgument(NULL)": ellipsis, # 5.6, MSVC
        "QGenericArgument(nullptr)": ellipsis, # 5.10
        "QGenericArgument(Q_NULLPTR)": ellipsis,
        "QHash": typing.Dict,
        "QJsonObject": typing.Dict[str, PySide2.QtCore.QJsonValue],
        "QModelIndex()": Invalid("PySide2.QtCore.QModelIndex"), # repr is btw. very wrong, fix it?!
        "QModelIndexList": ModelIndexList,
        "qptrdiff": int,
        "QString": str,
        "QString()": "",
        "QStringList": StringList,
        "QStringList()": [],
        "QStringRef": str,
        "Qt.HANDLE": int, # be more explicit with some consts?
        "quintptr": int,
        "QUrl.FormattingOptions(PrettyDecoded)": Instance(
            "QUrl.FormattingOptions(QUrl.PrettyDecoded)"),
        "QVariant": Variant,
        "QVariant()": Invalid(Variant),
        "QVariant.Type": type, # not so sure here...
        "QVariantMap": typing.Dict[str, Variant],
    })
    try:
        type_map.update({
            "PySide2.QtCore.QMetaObject.Connection": PySide2.QtCore.Connection, # wrong!
        })
    except AttributeError:
        # this does not exist on 5.9 ATM.
        pass
    return locals()


def init_PySide2_QtGui():
    from PySide2.QtGui import QPageLayout, QPageSize # 5.12 macOS
    type_map.update({
        "0.0f": 0.0,
        "1.0f": 1.0,
        "GL_COLOR_BUFFER_BIT": GL_COLOR_BUFFER_BIT,
        "GL_NEAREST": GL_NEAREST,
        "int32_t": int,
        "PySide2.QtCore.uint8_t": int, # macOS 5.9
        "PySide2.QtGui.QGenericMatrix": Missing("PySide2.QtGui.QGenericMatrix"),
        "PySide2.QtGui.QPlatformSurface": int, # a handle
        "QList< QTouchEvent.TouchPoint >()": [], # XXX improve?
        "QPixmap()": Default("PySide2.QtGui.QPixmap"), # can't create without qApp
        "QVector< QTextLayout.FormatRange >()": [], # do we need more structure?
        "uint32_t": int,
        "uint8_t": int,
        "USHRT_MAX": ushort_max,
        "WId": WId,
    })
    return locals()


def init_PySide2_QtWidgets():
    from PySide2.QtWidgets import QWidget, QMessageBox, QStyleOption, QStyleHintReturn, QStyleOptionComplex
    from PySide2.QtWidgets import QGraphicsItem, QStyleOptionGraphicsItem # 5.9
    type_map.update({
        "QMessageBox.StandardButtons(Yes | No)": Instance(
            "QMessageBox.StandardButtons(QMessageBox.Yes | QMessageBox.No)"),
        "QVector< int >()": [],
        "QWidget.RenderFlags(DrawWindowBackground | DrawChildren)": Instance(
            "QWidget.RenderFlags(QWidget.DrawWindowBackground | QWidget.DrawChildren)"),
        "SH_Default": QStyleHintReturn.SH_Default,
        "SO_Complex": QStyleOptionComplex.SO_Complex,
        "SO_Default": QStyleOption.SO_Default,
        "static_cast<Qt.MatchFlags>(Qt.MatchExactly|Qt.MatchCaseSensitive)": Instance(
            "Qt.MatchFlags(Qt.MatchExactly | Qt.MatchCaseSensitive)"),
        "Type": PySide2.QtWidgets.QListWidgetItem.Type,
    })
    return locals()


def init_PySide2_QtSql():
    from PySide2.QtSql import QSqlDatabase
    type_map.update({
        "QLatin1String(defaultConnection)": QSqlDatabase.defaultConnection,
        "QVariant.Invalid": Invalid("Variant"), # not sure what I should create, here...
    })
    return locals()


def init_PySide2_QtNetwork():
    type_map.update({
        "QMultiMap": MultiMap,
    })
    return locals()


def init_PySide2_QtXmlPatterns():
    from PySide2.QtXmlPatterns import QXmlName
    type_map.update({
        "QXmlName.NamespaceCode": Missing("PySide2.QtXmlPatterns.QXmlName.NamespaceCode"),
        "QXmlName.PrefixCode": Missing("PySide2.QtXmlPatterns.QXmlName.PrefixCode"),
    })
    return locals()


def init_PySide2_QtMultimedia():
    import PySide2.QtMultimediaWidgets
    # Check if foreign import is valid. See mapping.py in shiboken2.
    check_module(PySide2.QtMultimediaWidgets)
    type_map.update({
        "QGraphicsVideoItem": PySide2.QtMultimediaWidgets.QGraphicsVideoItem,
        "QVideoWidget": PySide2.QtMultimediaWidgets.QVideoWidget,
    })
    return locals()


def init_PySide2_QtOpenGL():
    type_map.update({
        "GLbitfield": int,
        "GLenum": int,
        "GLfloat": float, # 5.6, MSVC 15
        "GLint": int,
        "GLuint": int,
        "PySide2.QtOpenGL.GLint": int,
        "PySide2.QtOpenGL.GLuint": int,
    })
    return locals()


def init_PySide2_QtQml():
    type_map.update({
        "PySide2.QtQml.bool volatile": bool,
        "QJSValueList()": [],
        "QVariantHash()": typing.Dict[str, Variant],  # XXX sorted?
    })
    return locals()


def init_PySide2_QtQuick():
    type_map.update({
        "PySide2.QtCore.uint": int,
        "PySide2.QtQuick.QSharedPointer": int,
        "T": int,
    })
    return locals()


def init_PySide2_QtScript():
    type_map.update({
        "QScriptValueList()": [],
    })
    return locals()


def init_PySide2_QtTest():
    type_map.update({
        "PySide2.QtTest.QTouchEventSequence": PySide2.QtTest.QTest.QTouchEventSequence,
    })
    return locals()

# from 5.6, MSVC
def init_PySide2_QtWinExtras():
    type_map.update({
        "QList< QWinJumpListItem* >()": [],
    })
    return locals()

# from 5.12, macOS
def init_PySide2_QtDataVisualization():
    from PySide2.QtDataVisualization import QtDataVisualization
    QtDataVisualization.QBarDataRow = typing.List[QtDataVisualization.QBarDataItem]
    QtDataVisualization.QBarDataArray = typing.List[QtDataVisualization.QBarDataRow]
    QtDataVisualization.QSurfaceDataRow = typing.List[QtDataVisualization.QSurfaceDataItem]
    QtDataVisualization.QSurfaceDataArray = typing.List[QtDataVisualization.QSurfaceDataRow]
    type_map.update({
        "100.0f": 100.0,
    })
    return locals()


def init_testbinding():
    type_map.update({
        "testbinding.PySideCPP2.TestObjectWithoutNamespace": testbinding.TestObjectWithoutNamespace,
    })
    return locals()

# end of file
