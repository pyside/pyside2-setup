
Migration from Qt5.4 to Qt5.5
-----------------------------

This is an example session when migrating from Qt5.4.2 to Qt5.5.0 .

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


