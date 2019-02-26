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
import pkgutil

from shibokensupport.signature import typing
from shibokensupport.signature.typing import TypeVar, Generic

class ellipsis(object):
    def __repr__(self):
        return "..."

ellipsis = ellipsis()
StringList = typing.List[str]
IntList = typing.List[int]
Point = typing.Tuple[float, float]
PointList = typing.List[Point]
IntMatrix = typing.List[IntList]
Variant = typing.Any
ModelIndexList = typing.List[int]
QImageCleanupFunction = typing.Callable

# First time installing our own Pair type into typing.
T = TypeVar('T')
S = TypeVar('S')

class Pair(Generic[T, S]):
    __module__ = "typing"

typing.Pair = Pair


# Building our own Char type, which is much nicer than
# Char = typing.Union[str, int]     # how do I model the limitation to 1 char?

# Copied from the six module:
def with_metaclass(meta, *bases):
    """Create a base class with a metaclass."""
    # This requires a bit of explanation: the basic idea is to make a dummy
    # metaclass for one level of class instantiation that replaces itself with
    # the actual metaclass.
    class metaclass(type):

        def __new__(cls, name, this_bases, d):
            return meta(name, bases, d)

        @classmethod
        def __prepare__(cls, name, this_bases):
            return meta.__prepare__(name, bases)
    return type.__new__(metaclass, 'temporary_class', (), {})

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
        suppress = "support.signature.typing27."
        text = self[len(suppress):] if self.startswith(suppress) else self
        return "{}({})".format(type(self).__name__, text)

    def __call__(self):
        from shibokensupport.signature.mapping import __dict__ as namespace
        text = self if self.endswith(")") else self + "()"
        return eval(text, namespace)

USE_PEP563 = sys.version_info[:2] >= (3, 7)


# Some types are abstract. They just show their name.
class Virtual(_NotCalled):
    pass

# Other types I simply could not find.
class Missing(_NotCalled):
    if not USE_PEP563:
        # The string must be quoted, because the object does not exist.
        def __repr__(self):
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
    _uninitialized = "Shiboken minimal sample other smart".split()
    _prefixes = [""]
    try:
        import PySide2
        _uninitialized += PySide2.__all__ + ["testbinding"]
        _prefixes += ["PySide2."]
    except ImportError:
        pass

    def __init__(self):
        self.sys_module_count = 0
        self.uninitialized = self._uninitialized

    def update(self):
        """
        update is responsible to import all modules from shiboken and PySide
        which are already in sys.modules.
        The purpose is to follow all user imports without introducing new
        ones.
        This function is called by pyside_type_init to adapt imports
        when the number of imported modules has changed.
        """
        if self.sys_module_count == len(sys.modules):
            return
        self.sys_module_count = len(sys.modules)
        g = globals()
        for mod_name in self.uninitialized[:]:
            for prefix in self._prefixes:
                import_name = prefix + mod_name
                if import_name in sys.modules:
                    # check if this is a real module
                    check_module(sys.modules[import_name])
                    # module is real
                    self.uninitialized.remove(mod_name)
                    proc_name = "init_" + mod_name
                    if proc_name in g:
                        # Do the 'import {import_name}' first.
                        # 'top' is PySide2 when we do 'import PySide.QtCore'
                        # or Shiboken if we do 'import Shiboken'.
                        # Convince yourself that these two lines below have the same
                        # global effect as "import Shiboken" or "import PySide2.QtCore".
                        top = __import__(import_name)
                        g[top.__name__] = top
                        # Modules are in place, we can update the type_map.
                        g.update(g[proc_name]())

def check_module(mod):
    # During a build, there exist the modules already as directories,
    # although the '*.so' was not yet created. This causes a problem
    # in Python 3, because it accepts folders as namespace modules
    # without enforcing an '__init__.py'.
    if not getattr(mod, "__file__", None) or os.path.isdir(mod.__file__):
        mod_name = mod.__name__
        raise ImportError("Module '{mod_name}' is at most a namespace!"
                          .format(**locals()))


update_mapping = Reloader().update
type_map = {}
namespace = globals()  # our module's __dict__

type_map.update({
    "QList": typing.List,
    "QVector": typing.List,
    "QSet": typing.Set,
    "QPair": Pair,
    })


# The Shiboken Part
def init_Shiboken():
    type_map.update({
        "shiboken2.bool": bool,
        "size_t": int,
        "PyType": type,
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
        "double": float,
        "sample.int": int,
        "Complex": complex,
        "sample.OddBool": bool,
        "sample.bool": bool,
        "sample.PStr": str,
        "OddBool": bool,
        "PStr": str,
        "char": Char,
        "sample.char": Char,
        "sample.Point": Point,
        "sample.ObjectType": object,
        "std.string": str,
        "HANDLE": int,
        "Foo.HANDLE": int,
        "sample.Photon.TemplateBase": Missing("sample.Photon.TemplateBase"),
        "ObjectType.Identifier": Missing("sample.ObjectType.Identifier"),
        "zero(HANDLE)": 0,
        "Null": None,
        "zero(sample.ObjectType)": None,
        "std.size_t": int,
        'Str("<unknown>")': "<unknown>",
        'Str("<unk")': "<unk",
        'Str("nown>")': "nown>",
        "zero(sample.ObjectModel)": None,
        "sample.unsigned char": Char,
        "sample.double": float,
        "zero(sample.bool)": False,
        "PyDate": datetime.date,
        "ZeroIn": 0,
    })
    return locals()


def init_other():
    import numbers
    type_map.update({
        "other.Number": numbers.Number,
        "other.ExtendsNoImplicitConversion": Missing("other.ExtendsNoImplicitConversion"),
    })
    return locals()


def init_smart():
    type_map.update({
        "smart.SharedPtr": Missing("smart.SharedPtr"),  # bad object "SharedPtr<Obj >"
    })
    return locals()

# The PySide Part
def init_QtCore():
    from PySide2.QtCore import Qt, QUrl, QDir
    from PySide2.QtCore import QRect, QSize, QPoint, QLocale, QByteArray
    from PySide2.QtCore import QMarginsF # 5.9
    try:
        # seems to be not generated by 5.9 ATM.
        from PySide2.QtCore import Connection
    except ImportError:
        pass
    type_map.update({
        "str": str,
        "int": int,
        "QString": str,
        "bool": bool,
        "PyObject": object,
        "void": int, # be more specific?
        "char": Char,
        "'%'": "%",
        "' '": " ",
        "false": False,
        "double": float,
        "'g'": "g",
        "long long": int,
        "unsigned int": int, # should we define an unsigned type?
        "Q_NULLPTR": None,
        "long": int,
        "float": float,
        "short": int,
        "unsigned long": int,
        "unsigned long long": int,
        "unsigned short": int,
        "QStringList": StringList,
        "QChar": Char,
        "signed char": Char,
        "QVariant": Variant,
        "QVariant.Type": type, # not so sure here...
        "QStringRef": str,
        "QString()": "",
        "QModelIndexList": ModelIndexList,
        "unsigned char": Char,
        "QJsonObject": typing.Dict[str, PySide2.QtCore.QJsonValue],
        "QStringList()": [],
        "ULONG_MAX": ulong_max,
        "quintptr": int,
        "PyCallable": typing.Callable,
        "PyTypeObject": type,
        "PySequence": typing.Iterable,  # important for numpy
        "qptrdiff": int,
        "true": True,
        "Qt.HANDLE": int, # be more explicit with some consts?
        "list of QAbstractState": typing.List[PySide2.QtCore.QAbstractState],
        "list of QAbstractAnimation": typing.List[PySide2.QtCore.QAbstractAnimation],
        "QVariant()": Invalid(Variant),
        "QMap": typing.Dict,
        "PySide2.QtCore.bool": bool,
        "QHash": typing.Dict,
        "PySide2.QtCore.QChar": Char,
        "PySide2.QtCore.qreal": float,
        "PySide2.QtCore.float": float,
        "PySide2.QtCore.qint16": int,
        "PySide2.QtCore.qint32": int,
        "PySide2.QtCore.qint64": int,
        "PySide2.QtCore.qint8": int,
        "PySide2.QtCore.QString": str,
        "PySide2.QtCore.QStringList": StringList,
        "PySide2.QtCore.QVariant": Variant,
        "PySide2.QtCore.quint16": int,
        "PySide2.QtCore.quint32": int,
        "PySide2.QtCore.quint64": int,
        "PySide2.QtCore.quint8": int,
        "PySide2.QtCore.uchar": Char,
        "PySide2.QtCore.unsigned char": Char, # 5.9
        "PySide2.QtCore.long": int,
        "PySide2.QtCore.QUrl.ComponentFormattingOptions":
            PySide2.QtCore.QUrl.ComponentFormattingOption, # mismatch option/enum, why???
        "QUrl.FormattingOptions(PrettyDecoded)": Instance(
            "QUrl.FormattingOptions(QUrl.PrettyDecoded)"),
        # from 5.9
        "QDir.Filters(AllEntries | NoDotAndDotDot)": Instance(
            "QDir.Filters(QDir.AllEntries | QDir.NoDotAndDotDot)"),
        "NULL": None, # 5.6, MSVC
        "QDir.SortFlags(Name | IgnoreCase)": Instance(
            "QDir.SortFlags(QDir.Name | QDir.IgnoreCase)"),
        "PyBytes": bytes,
        "PyByteArray": bytearray,
        "PyUnicode": typing.Text,
        "signed long": int,
        "PySide2.QtCore.int": int,
        "PySide2.QtCore.char": StringList, # A 'char **' is a list of strings.
        "unsigned long int": int, # 5.6, RHEL 6.6
        "unsigned short int": int, # 5.6, RHEL 6.6
        "4294967295UL": 4294967295, # 5.6, RHEL 6.6
        "PySide2.QtCore.int32_t": int, # 5.9
        "PySide2.QtCore.int64_t": int, # 5.9
        "UnsignedShortType": int, # 5.9
        "nullptr": None, # 5.9
        "uint64_t": int, # 5.9
        "PySide2.QtCore.uint32_t": int, # 5.9
        "PySide2.QtCore.unsigned int": int, # 5.9 Ubuntu
        "PySide2.QtCore.long long": int, # 5.9, MSVC 15
        "QGenericArgument(nullptr)": ellipsis, # 5.10
        "QModelIndex()": Invalid("PySide2.QtCore.QModelIndex"), # repr is btw. very wrong, fix it?!
        "QGenericArgument((0))": ellipsis, # 5.6, RHEL 6.6. Is that ok?
        "QGenericArgument()": ellipsis,
        "QGenericArgument(0)": ellipsis,
        "QGenericArgument(NULL)": ellipsis, # 5.6, MSVC
        "QGenericArgument(Q_NULLPTR)": ellipsis,
        "zero(PySide2.QtCore.QObject)": None,
        "zero(PySide2.QtCore.QThread)": None,
        "zero(quintptr)": 0,
        "zero(str)": "",
        "zero(int)": 0,
        "zero(PySide2.QtCore.QState)": None,
        "zero(PySide2.QtCore.bool)": False,
        "zero(PySide2.QtCore.int)": 0,
        "zero(void)": None,
        "zero(long long)": 0,
        "zero(PySide2.QtCore.QAbstractItemModel)": None,
        "zero(PySide2.QtCore.QJsonParseError)": None,
        "zero(double)": 0.0,
        "zero(PySide2.QtCore.qint64)": 0,
        "zero(PySide2.QtCore.QTextCodec.ConverterState)": None,
        "zero(long long)": 0,
        "zero(QImageCleanupFunction)": None,
        "zero(unsigned int)": 0,
        "zero(PySide2.QtCore.QPoint)": Default("PySide2.QtCore.QPoint"),
        "zero(unsigned char)": 0,
        "zero(PySide2.QtCore.QEvent.Type)": None,
        "CheckIndexOption.NoOption": Instance(
            "PySide2.QtCore.QAbstractItemModel.CheckIndexOptions.NoOption"), # 5.11
        "QVariantMap": typing.Dict[str, Variant],
        "PySide2.QtCore.QCborStreamReader.StringResult": typing.AnyStr,
        "PySide2.QtCore.double": float,
    })
    try:
        type_map.update({
            "PySide2.QtCore.QMetaObject.Connection": PySide2.QtCore.Connection, # wrong!
        })
    except AttributeError:
        # this does not exist on 5.9 ATM.
        pass
    return locals()


def init_QtGui():
    from PySide2.QtGui import QPageLayout, QPageSize # 5.12 macOS
    type_map.update({
        "QVector< QTextLayout.FormatRange >()": [], # do we need more structure?
        "USHRT_MAX": ushort_max,
        "0.0f": 0.0,
        "1.0f": 1.0,
        "uint32_t": int,
        "uint8_t": int,
        "int32_t": int,
        "GL_COLOR_BUFFER_BIT": GL_COLOR_BUFFER_BIT,
        "GL_NEAREST": GL_NEAREST,
        "WId": WId,
        "PySide2.QtGui.QPlatformSurface": int, # a handle
        "QList< QTouchEvent.TouchPoint >()": [], # XXX improve?
        "QPixmap()": Default("PySide2.QtGui.QPixmap"), # can't create without qApp
        "PySide2.QtCore.uint8_t": int, # macOS 5.9
        "zero(uint32_t)": 0,
        "zero(PySide2.QtGui.QWindow)": None,
        "zero(PySide2.QtGui.QOpenGLContext)": None,
        "zero(PySide2.QtGui.QRegion)": None,
        "zero(PySide2.QtGui.QPaintDevice)": None,
        "zero(PySide2.QtGui.QTextLayout.FormatRange)": None,
        "zero(PySide2.QtGui.QTouchDevice)": None,
        "zero(PySide2.QtGui.QScreen)": None,
        "PySide2.QtGui.QGenericMatrix": Missing("PySide2.QtGui.QGenericMatrix"),
    })
    return locals()


def init_QtWidgets():
    from PySide2.QtWidgets import QWidget, QMessageBox, QStyleOption, QStyleHintReturn, QStyleOptionComplex
    from PySide2.QtWidgets import QGraphicsItem, QStyleOptionGraphicsItem # 5.9
    type_map.update({
        "QMessageBox.StandardButtons(Yes | No)": Instance(
            "QMessageBox.StandardButtons(QMessageBox.Yes | QMessageBox.No)"),
        "QWidget.RenderFlags(DrawWindowBackground | DrawChildren)": Instance(
            "QWidget.RenderFlags(QWidget.DrawWindowBackground | QWidget.DrawChildren)"),
        "static_cast<Qt.MatchFlags>(Qt.MatchExactly|Qt.MatchCaseSensitive)": Instance(
            "Qt.MatchFlags(Qt.MatchExactly | Qt.MatchCaseSensitive)"),
        "QVector< int >()": [],
        "WId": WId,
        # from 5.9
        "Type": PySide2.QtWidgets.QListWidgetItem.Type,
        "SO_Default": QStyleOption.SO_Default,
        "SH_Default": QStyleHintReturn.SH_Default,
        "SO_Complex": QStyleOptionComplex.SO_Complex,
        "zero(PySide2.QtWidgets.QWidget)": None,
        "zero(PySide2.QtWidgets.QGraphicsItem)": None,
        "zero(PySide2.QtCore.QEvent)": None,
        "zero(PySide2.QtWidgets.QStyleOption)": None,
        "zero(PySide2.QtWidgets.QStyleHintReturn)": None,
        "zero(PySide2.QtWidgets.QGraphicsLayoutItem)": None,
        "zero(PySide2.QtWidgets.QListWidget)": None,
        "zero(PySide2.QtGui.QKeySequence)": None,
        "zero(PySide2.QtWidgets.QAction)": None,
        "zero(PySide2.QtWidgets.QUndoCommand)": None,
        "zero(WId)": 0,
    })
    return locals()


def init_QtSql():
    from PySide2.QtSql import QSqlDatabase
    type_map.update({
        "QLatin1String(defaultConnection)": QSqlDatabase.defaultConnection,
        "QVariant.Invalid": Invalid("Variant"), # not sure what I should create, here...
    })
    return locals()


def init_QtNetwork():
    type_map.update({
        "QMultiMap": MultiMap,
        "zero(unsigned short)": 0,
        "zero(PySide2.QtCore.QIODevice)": None,
        "zero(QList)": [],
    })
    return locals()


def init_QtXmlPatterns():
    from PySide2.QtXmlPatterns import QXmlName
    type_map.update({
        "QXmlName.PrefixCode": Missing("PySide2.QtXmlPatterns.QXmlName.PrefixCode"),
        "QXmlName.NamespaceCode": Missing("PySide2.QtXmlPatterns.QXmlName.NamespaceCode")
    })
    return locals()


def init_QtMultimedia():
    import PySide2.QtMultimediaWidgets
    # Check if foreign import is valid. See mapping.py in shiboken2.
    check_module(PySide2.QtMultimediaWidgets)
    type_map.update({
        "QGraphicsVideoItem": PySide2.QtMultimediaWidgets.QGraphicsVideoItem,
        "QVideoWidget": PySide2.QtMultimediaWidgets.QVideoWidget,
    })
    return locals()


def init_QtOpenGL():
    type_map.update({
        "GLuint": int,
        "GLenum": int,
        "GLint": int,
        "GLbitfield": int,
        "PySide2.QtOpenGL.GLint": int,
        "PySide2.QtOpenGL.GLuint": int,
        "GLfloat": float, # 5.6, MSVC 15
        "zero(PySide2.QtOpenGL.QGLContext)": None,
        "zero(GLenum)": 0,
        "zero(PySide2.QtOpenGL.QGLWidget)": None,
    })
    return locals()


def init_QtQml():
    type_map.update({
        "QJSValueList()": [],
        "PySide2.QtQml.bool volatile": bool,
        # from 5.9
        "QVariantHash()": typing.Dict[str, Variant],  # XXX sorted?
        "zero(PySide2.QtQml.QQmlContext)": None,
        "zero(PySide2.QtQml.QQmlEngine)": None,
    })
    return locals()


def init_QtQuick():
    type_map.update({
        "PySide2.QtQuick.QSharedPointer": int,
        "PySide2.QtCore.uint": int,
        "T": int,
        "zero(PySide2.QtQuick.QQuickItem)": None,
        "zero(GLuint)": 0,
    })
    return locals()


def init_QtScript():
    type_map.update({
        "QScriptValueList()": [],
    })
    return locals()


def init_QtTest():
    type_map.update({
        "PySide2.QtTest.QTouchEventSequence": PySide2.QtTest.QTest.QTouchEventSequence,
    })
    return locals()

# from 5.9
def init_QtWebEngineWidgets():
    type_map.update({
        "zero(PySide2.QtWebEngineWidgets.QWebEnginePage.FindFlags)": 0,
    })
    return locals()

# from 5.6, MSVC
def init_QtWinExtras():
    type_map.update({
        "QList< QWinJumpListItem* >()": [],
    })
    return locals()

# from 5.12, macOS
def init_QtDataVisualization():
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
