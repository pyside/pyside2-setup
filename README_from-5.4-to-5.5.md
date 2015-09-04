
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