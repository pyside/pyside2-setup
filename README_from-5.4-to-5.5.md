
Migration from Qt5.4 to Qt5.5
-----------------------------

This is an example session when migrating from Qt5.4.2 to Qt5.5.0 .

2015-09-04

A first build attempt results in the following shiboken warnings on QtCore:

```
Generating class model...                    [WARNING]
    enum 'QSysInfo::WinVersion' does not have a type entry or is not an enum
    enum 'QSysInfo::MacVersion' does not have a type entry or is not an enum
    enum 'QAbstractTransition::TransitionType' does not have a type entry or is not an enum
    Unable to decide type of property: 'TransitionType' in class 'QAbstractTransition'
    

Generating enum model...                     [WARNING]
    enum 'InterfaceType' does not have a type entry or is not an enum
    enum 'Role' does not have a type entry or is not an enum
    enum 'TextBoundaryType' does not have a type entry or is not an enum
    enum 'RelationFlag' does not have a type entry or is not an enum
    

Generating namespace model...                [WARNING]
    enum 'Qt::TabFocusBehavior' does not have a type entry or is not an enum
    enum 'Qt::ItemSelectionOperation' does not have a type entry or is not an enum
    

Resolving typedefs...                        [OK]
Fixing class inheritance...                  [WARNING]
    skipping field 'QSysInfo::WindowsVersion' with unmatched type 'QSysInfo::WinVersion'
    skipping field 'QSysInfo::MacintoshVersion' with unmatched type 'QSysInfo::MacVersion'
    

Detecting inconsistencies in class model...  [OK]
[OK]
    enum 'QAbstractAnimation::DeletionPolicy' is specified in typesystem, but not declared
    enum 'QAbstractAnimation::State' is specified in typesystem, but not declared
    enum 'QLocale::MeasurementSystem' is specified in typesystem, but not declared
    enum 'QLocale::FormatType' is specified in typesystem, but not declared
    enum 'QState::RestorePolicy' is specified in typesystem, but not declared
    


    There's no user provided way (conversion rule, argument removal, custom code, etc) to handle the primitive argument type 'QString *' in function 'QTextStream::readLineInto(QString * line, qint64 maxlen)'.
    

Done, 18 warnings (426 known issues)

```

By systematically looking up the missing types in the search field of the Qt5.5 documentation at
http://doc.qt.io/qt-5/ we apply the following changes to typesystem_core_common.xml:

```
diff --git a/PySide/QtCore/typesystem_core_common.xml b/PySide/QtCore/typesystem_core_common.xml
index 2f39db3..f0a504f 100644
--- a/PySide/QtCore/typesystem_core_common.xml
+++ b/PySide/QtCore/typesystem_core_common.xml
@@ -2935,6 +2935,8 @@
   <object-type name="QSysInfo">
     <enum-type name="Endian"/>
     <enum-type name="Sizes"/>
+    <enum-type name="MacVersion" since="5.5" />
+    <enum-type name="WinVersion" since="5.5" />
   </object-type>
   <object-type name="QTemporaryFile">
     <extra-includes>
@@ -3833,6 +3835,8 @@
   </object-type>
 
   <object-type name="QAbstractTransition" since="4.6">
+    <enum-type name="TransitionType" since="5.5" />
+
     <modify-function signature="QAbstractTransition(QState*)">
       <modify-argument index="1">
         <parent index="this" action="add"/>
```

As a result, this reduces the warnings to this:

```
Generating enum model...                     [WARNING]
    enum 'InterfaceType' does not have a type entry or is not an enum
    enum 'TextBoundaryType' does not have a type entry or is not an enum
    enum 'RelationFlag' does not have a type entry or is not an enum
    enum 'Role' does not have a type entry or is not an enum
    

Generating namespace model...                [WARNING]
    enum 'Qt::TabFocusBehavior' does not have a type entry or is not an enum
    enum 'Qt::ItemSelectionOperation' does not have a type entry or is not an enum
    

Resolving typedefs...                        [OK]
Fixing class inheritance...                  [OK]
Detecting inconsistencies in class model...  [OK]
[OK]
    enum 'QState::RestorePolicy' is specified in typesystem, but not declared
    enum 'QLocale::FormatType' is specified in typesystem, but not declared
    enum 'QAbstractAnimation::DeletionPolicy' is specified in typesystem, but not declared
    enum 'QLocale::MeasurementSystem' is specified in typesystem, but not declared
    enum 'QAbstractAnimation::State' is specified in typesystem, but not declared
    


    There's no user provided way (conversion rule, argument removal, custom code, etc) to handle the primitive argument type 'QString *' in function 'QTextStream::readLineInto(QString * line, qint64 maxlen)'.
    

Done, 12 warnings (422 known issues)

```

The next four enum entries "InterfaceType", "TextBoundaryType", "RelationFlag", "Role" do not
exist in QtCore, and I have no idea how they were found by shiboken. They should be entries for
QAccessible in QtGui and others. I defined them anyway, and the warnings disappeared.

We are now down to 8 warnings:

```
Generating class model...                    [OK]
Generating enum model...                     [OK]
Generating namespace model...                [WARNING]
    enum 'Qt::ItemSelectionOperation' does not have a type entry or is not an enum
    enum 'Qt::TabFocusBehavior' does not have a type entry or is not an enum
    

Resolving typedefs...                        [OK]
Fixing class inheritance...                  [OK]
Detecting inconsistencies in class model...  [OK]
[OK]
    enum 'QLocale::FormatType' is specified in typesystem, but not declared
    enum 'QLocale::MeasurementSystem' is specified in typesystem, but not declared
    enum 'QAbstractAnimation::DeletionPolicy' is specified in typesystem, but not declared
    enum 'QState::RestorePolicy' is specified in typesystem, but not declared
    enum 'QAbstractAnimation::State' is specified in typesystem, but not declared
    


    There's no user provided way (conversion rule, argument removal, custom code, etc) to handle the primitive argument type 'QString *' in function 'QTextStream::readLineInto(QString * line, qint64 maxlen)'.
    

Done, 8 warnings (422 known issues)

```

Removing the next enum complaints

```
Generating class model...                    [OK]
Generating enum model...                     [OK]
Generating namespace model...                [OK]
Resolving typedefs...                        [OK]
Fixing class inheritance...                  [OK]
Detecting inconsistencies in class model...  [OK]
[OK]
    enum 'QLocale::MeasurementSystem' is specified in typesystem, but not declared
    enum 'QState::RestorePolicy' is specified in typesystem, but not declared
    enum 'QLocale::FormatType' is specified in typesystem, but not declared
    enum 'QAbstractAnimation::DeletionPolicy' is specified in typesystem, but not declared
    enum 'QAbstractAnimation::State' is specified in typesystem, but not declared
    


    There's no user provided way (conversion rule, argument removal, custom code, etc) to handle the primitive argument type 'QString *' in function 'QTextStream::readLineInto(QString * line, qint64 maxlen)'.
    

Done, 6 warnings (418 known issues)

```

We now get rid of the last 5 enum complaints:
The enums are moved elsewhere, and I have no idea what to do when something becomes invalid?
What I did was ignoring these errors:

```
  <!-- Qt5.5: No idea how to get rid of the following five enums, which are moved elsewhere since 5.5: -->
  <suppress-warning text="enum 'QLocale::MeasurementSystem' is specified in typesystem, but not declared" />
  <suppress-warning text="enum 'QState::RestorePolicy' is specified in typesystem, but not declared" />
  <suppress-warning text="enum 'QLocale::FormatType' is specified in typesystem, but not declared" />
  <suppress-warning text="enum 'QAbstractAnimation::DeletionPolicy' is specified in typesystem, but not declared" />
  <suppress-warning text="enum 'QAbstractAnimation::State' is specified in typesystem, but not declared" />

</typesystem>
```

We get rid of the function warning from above by looking in the Docs: *readLineInto seems to be a new
function, and it has a target variable, that normally needs extra support by a code snippet in the XML:
```
bool    readLineInto(QString * line, qint64 maxlen = 0)
```

But since we do not need to forcibly support new methods, it is easiest to just remove the method
and leave a comment, as it was done before for different argument types:

```
diff --git a/PySide/QtCore/typesystem_core_common.xml b/PySide/QtCore/typesystem_core_common.xml
index 8cd71f4..63f2342 100644
--- a/PySide/QtCore/typesystem_core_common.xml
+++ b/PySide/QtCore/typesystem_core_common.xml
@@ -3656,6 +3656,8 @@
     <enum-type name="Status"/>
     <!-- Removed because it expect QString to be mutable -->
     <modify-function signature="QTextStream(QString*,QFlags&lt;QIODevice::OpenModeFlag&gt;)" remove="all"/>
+    <!-- Qt5.5: Removed because it expect QString to be mutable -->
+    <modify-function signature="readLineInto(QString*,qint64)" since="5.5" remove="all"/>
     <!-- Removed because we use the non-const version -->
     <modify-function signature="QTextStream(const QByteArray&amp;, QFlags&lt;QIODevice::OpenModeFlag&gt;)" remove="all"/>
 
```

The only remaining problem seems now to be an include file, which is now required for some unclear reason:

```
/Users/tismer/src/pyside-setup2/pyside_build/py3.4-qt5.5.0-64bit-debug/pyside/PySide/QtCore/PySide/QtCore/pyside_qtcore_python.h:56:10: fatal error: 
      'qaccessible.h' file not found
#include <qaccessible.h>
         ^
1 error generated.
```

2015-09-07

This file actually belongs to the QtGui library, and the according include files are not configured
in the cmake file for QtCore (and really should not!).

There must be something referenced by QtCore XML that reaches out into QtGui.
Actually, this looks like a shiboken bug, and we revert the change to define the enums
"InterfaceType", "TextBoundaryType", "RelationFlag", "Role" and ignore them, instead:

After suppressing these four warnings, we are faced with the following compile errors:

```
[ 22%] Building CXX object PySide/QtCore/CMakeFiles/QtCore.dir/PySide/QtCore/qbuffer_wrapper.cpp.o
/Users/tismer/src/pyside-setup2/pyside_build/py3.4-qt5.5.0-64bit-debug/pyside/PySide/QtCore/PySide/QtCore/qabstractanimation_wrapper.cpp:344:81: error: use
      of undeclared identifier 'SBK_QABSTRACTANIMATION_STATE_IDX'
        Shiboken::Conversions::copyToPython(SBK_CONVERTER(SbkPySide_QtCoreTypes[SBK_QABSTRACTANIMATION_STATE_IDX]), &newState),
                                                                                ^
/Users/tismer/src/pyside-setup2/pyside_install/py3.4-qt5.5.0-64bit-debug/include/shiboken/sbkconverter.h:339:68: note: expanded from macro 'SBK_CONVERTER'
#define SBK_CONVERTER(pyType) (*reinterpret_cast<_SbkGenericType*>(pyType)->converter)
                                                                   ^
/Users/tismer/src/pyside-setup2/pyside_build/py3.4-qt5.5.0-64bit-debug/pyside/PySide/QtCore/PySide/QtCore/qanimationgroup_wrapper.cpp:344:81: error: use of
      undeclared identifier 'SBK_QABSTRACTANIMATION_STATE_IDX'
        Shiboken::Conversions::copyToPython(SBK_CONVERTER(SbkPySide_QtCoreTypes[SBK_QABSTRACTANIMATION_STATE_IDX]), &newState),
...                                                                                ^
```

This is pretty crazy, and after quite a while of searching, I found out that this is due to the new "Q_ENUM" macro.
I commented the macros away in qabstractanimation.h, and compilation went on much further.

Then, by scanning the sources folder, I found out that certain macros in Qt get special treatment
in shiboken, and now it is clear that I have to augment shiboken, once again.

```
$ grep -rn  Q_ENUM sources/
sources//pyside2/doc/codesnippets/doc/src/snippets/code/src_corelib_kernel_qobject.cpp:290:    Q_ENUMS(Priority)
sources//pyside2/doc/codesnippets/doc/src/snippets/moc/myclass2.h:54:    Q_ENUMS(Priority)
sources//pyside2/doc/pyside.qdocconf.in:159:                          Q_ENUMS \
sources//pyside2/README_from-5.4-to-5.5.md:246:This is pretty crazy, and after quite a while of searching, I found out that this is due to the new "Q_ENUM" macro.
sources//shiboken2/ApiExtractor/abstractmetalang.h:1305:    // Has the enum been declared inside a Q_ENUMS() macro in its enclosing class?
sources//shiboken2/ApiExtractor/parser/lexer.cpp:1338:            token_stream[(int) index++].kind = Token_Q_ENUMS;
sources//shiboken2/ApiExtractor/parser/parser.cpp:425:    case Token_Q_ENUMS:
sources//shiboken2/ApiExtractor/parser/parser.cpp:426:        return parseQ_ENUMS(node);
sources//shiboken2/ApiExtractor/parser/parser.cpp:1781:    } else if (parseQ_ENUMS(node)) {
sources//shiboken2/ApiExtractor/parser/parser.cpp:4002:bool Parser::parseQ_ENUMS(DeclarationAST *&node)
sources//shiboken2/ApiExtractor/parser/parser.cpp:4004:    if (token_stream.lookAhead() != Token_Q_ENUMS)
sources//shiboken2/ApiExtractor/parser/parser.h:167:    bool parseQ_ENUMS(DeclarationAST *&node);
sources//shiboken2/ApiExtractor/parser/tokens.cpp:135:    "Q_ENUMS"
sources//shiboken2/ApiExtractor/parser/tokens.h:136:    Token_Q_ENUMS,
```

Now we first need to change shiboken, and then evaluatet a lot of things, again.

---------

2015-09-17

After a longer break, shiboken was fixed for Qt5.5 .

# QtCore 
has a problem with the new entry in `qmetaobject.h`:

```
    template<typename T> static QMetaEnum fromType() {
        Q_STATIC_ASSERT_X(QtPrivate::IsQEnumHelper<T>::Value,
                          "QMetaEnum::fromType only works with enums declared as Q_ENUM or Q_FLAG");
        const QMetaObject *metaObject = qt_getEnumMetaObject(T());
        const char *name = qt_getEnumName(T());
        return metaObject->enumerator(metaObject->indexOfEnumerator(name));
    }
```
Because the C compiler did not know how to resolve the template, I disabled the function like so:
```
    <!-- Qt5.5: "template<typename T> static QMetaEnum fromType()" is not understood by the compiler.
         We therefore ignore this 5.5 addition for now: -->
    <modify-function signature="fromType()" since="5.5" remove="all" />
```
If somebody finds a better way to support it, please improve this!

Anyway, this change was now enough to make the 5.5 version build!

The remaining warnings are as follows:

# QtGui
```
[ 22%] Running generator for QtGui...
** WARNING scope not found for function definition:QtPrivate::IsMetaTypePair<T,true>::registerConverter
    definition *ignored*
Generating class model...                    [WARNING]
    enum 'QImageIOHandler::Transformation' does not have a type entry or is not an enum
    enum 'QWheelEvent::DefaultDeltasPerStep' does not have a type entry or is not an enum
    

Generating enum model...                     [OK]
Generating namespace model...                [OK]
Resolving typedefs...                        [OK]
Fixing class inheritance...                  [OK]
Detecting inconsistencies in class model...  [OK]
[OK]

    There's no user provided way (conversion rule, argument removal, custom code, etc) to handle the primitive argument type 'float *' in function 'QQuaternion::getAxisAndAngle(float * x, float * y, float * z, float * angle) const'.
    /Users/tismer/src/pyside-setup2/sources/shiboken2/generator/shiboken/cppgenerator.cpp:4452 FIXME:
    The code tried to qRegisterMetaType the unqualified name 'iterator'. This is currently fixed by a hack(ct) and needs improvement!
    signature 'parent()const' for function modification in 'QSortFilterProxyModel' not found. Possible candidates: parent(QModelIndex)const in QSortFilterProxyModel
    There's no user provided way (conversion rule, argument removal, custom code, etc) to handle the primitive argument type 'float *' in function 'QQuaternion::getAxisAndAngle(QVector3D * axis, float * angle) const'.
    There's no user provided way (conversion rule, argument removal, custom code, etc) to handle the primitive argument type 'float *' in function 'QQuaternion::getEulerAngles(float * pitch, float * yaw, float * roll) const'.
    

Done, 7 warnings (556 known issues)
```
The first enum was a simple addition, after looking it up in http://doc.qt.io/qt-5/qimageiohandler.html :
We simply added the new enum with a 'since' tag.
```
  <object-type name="QImageIOHandler">
    <extra-includes>
      <include file-name="QRect" location="global"/>
    </extra-includes>
    <enum-type name="ImageOption"/>
    <enum-type name="Transformation" flags="Transformations" since="5.5" />
    <modify-function signature="setDevice(QIODevice*)">
      <modify-argument index="1">
        <parent index="this" action="add"/>
      </modify-argument>
    </modify-function>
  </object-type>
```
The `QWheelEvent::DefaultDeltasPerStep` was harder, because it had no entry in the Qt docs. So we had to look
into the source. It turns out that the enum is a nameless one:
```
#ifndef QT_NO_WHEELEVENT
class Q_GUI_EXPORT QWheelEvent : public QInputEvent
{
public:
    enum { DefaultDeltasPerStep = 120 };
    ...
```
We suppress the warning, no idea what else can be done:
```
  <!-- Qt5.5: suppress this nameless enum -->
  <suppress-warning text="enum 'QWheelEvent::DefaultDeltasPerStep' does not have a type entry or is not an enum" />
```
We ignore the `QQuaternion` methods, which have output variables.
It is easy to support them later, but I think nobody cares at the moment.
They are marked for later review.
```
  <value-type name="QQuaternion" since="4.6">
    ...
    <!-- Qt5.5: XXX support the output variables! For now, I just suppressed the new methods. -->
    <modify-function signature="getAxisAndAngle(float *, float *, float *, float *) const" since="5.5" remove="all" />
    <modify-function signature="getAxisAndAngle(QVector3D *, float *) const" since="5.5" remove="all" />
    <modify-function signature="getEulerAngles(float *, float *, float *) const" since="5.5" remove="all" />
  </value-type>
```
The remaining warning is from me, and it shall stay there, until some brave soul fixes that shiboken problem:
```
The code tried to qRegisterMetaType the unqualified name 'iterator'. This is currently fixed by a hack(ct) and needs improvement!
```

The remaining messages in QtGui and QtWidgets are of the form:
```
signature 'parent()const' for function modification in 'QSortFilterProxyModel' not found. ...
```
These entries were useful in Qt5.4 :
```
    <!-- ### This reimplementation of "QObject::parent()" is used in C++ only
         when "using QObject::parent;" is not available. It's useless in Python. -->
    <modify-function signature="parent()const" remove="all"/>
```
but make no more sense in Qt5.5 . Because there is no easy way to specify, when something is _removed_, I simply
removed these entries, which will produce warnings when building for Qt5.4 (ignoring this).

# That's all, Folks




