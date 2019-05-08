# Resource object code (Python 3)
# Created by: object code
# Created by: The Resource Compiler for Qt version 5.14.0
# WARNING! All changes made in this file will be lost!

from PySide2 import QtCore

qt_resource_data = b"\
\x00\x00\x06{\
T\
EMPLATE = app\x0aLA\
NGUAGE = C++\x0aTAR\
GET         = as\
sistant\x0a\x0aCONFIG \
       += qt war\
n_on\x0aQT         \
   += xml networ\
k\x0a\x0aPROJECTNAME  \
      = Assistan\
t\x0aDESTDIR       \
     = ../../bin\
\x0a\x0aFORMS += findd\
ialog.ui \x5c\x0a     \
   helpdialog.ui\
 \x5c\x0a        mainw\
indow.ui \x5c\x0a     \
   settingsdialo\
g.ui \x5c\x0a        t\
abbedbrowser.ui \
\x5c\x0a        topicc\
hooser.ui\x0a\x0aSOURC\
ES += main.cpp \x5c\
\x0a        helpwin\
dow.cpp \x5c\x0a      \
  topicchooser.c\
pp \x5c\x0a        doc\
uparser.cpp \x5c\x0a  \
      settingsdi\
alog.cpp \x5c\x0a     \
   index.cpp \x5c\x0a \
       profile.c\
pp \x5c\x0a        con\
fig.cpp \x5c\x0a      \
  finddialog.cpp\
 \x5c\x0a        helpd\
ialog.cpp \x5c\x0a    \
    mainwindow.c\
pp \x5c\x0a        tab\
bedbrowser.cpp\x0a\x0a\
HEADERS        +\
= helpwindow.h \x5c\
\x0a        topicch\
ooser.h \x5c\x0a      \
  docuparser.h \x5c\
\x0a        setting\
sdialog.h \x5c\x0a    \
    index.h \x5c\x0a  \
      profile.h \
\x5c\x0a        finddi\
alog.h \x5c\x0a       \
 helpdialog.h \x5c\x0a\
        mainwind\
ow.h \x5c\x0a        t\
abbedbrowser.h \x5c\
\x0a        config.\
h\x0a\x0aRESOURCES += \
assistant.qrc\x0a\x0aD\
EFINES += QT_KEY\
WORDS\x0a#DEFINES +\
=  QT_PALMTOPCEN\
TER_DOCS\x0a!networ\
k:DEFINES       \
 += QT_INTERNAL_\
NETWORK\x0aelse:QT \
+= network\x0a!xml:\
 DEFINES        \
        += QT_IN\
TERNAL_XML\x0aelse:\
QT += xml\x0ainclud\
e( ../../src/qt_\
professional.pri\
 )\x0a\x0awin32 {\x0a    \
LIBS += -lshell3\
2\x0a    RC_FILE = \
assistant.rc\x0a}\x0a\x0a\
macos {\x0a    ICON\
 = assistant.icn\
s\x0a    TARGET = a\
ssistant\x0a#    QM\
AKE_INFO_PLIST =\
 Info_mac.plist\x0a\
}\x0a\x0a#target.path \
= $$[QT_INSTALL_\
BINS]\x0a#INSTALLS \
+= target\x0a\x0a#assi\
stanttranslation\
s.files = *.qm\x0a#\
assistanttransla\
tions.path = $$[\
QT_INSTALL_TRANS\
LATIONS]\x0a#INSTAL\
LS += assistantt\
ranslations\x0a\x0aTRA\
NSLATIONS       \
 = assistant_de.\
ts \x5c\x0a           \
       assistant\
_fr.ts\x0a\x0a\x0aunix:!c\
ontains(QT_CONFI\
G, zlib):LIBS +=\
 -lz\x0a\x0a\x0atarget.pa\
th=$$[QT_INSTALL\
_BINS]\x0aINSTALLS \
+= target\x0a\
"

qt_resource_name = b"\
\x00\x08\
\x0e\x84\x7fC\
\x00e\
\x00x\x00a\x00m\x00p\x00l\x00e\x00s\
\x00\x07\
\x0c\xe8G\xe5\
\x00e\
\x00x\x00a\x00m\x00p\x00l\x00e\
"

qt_resource_struct = b"\
\x00\x00\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x01\
\x00\x00\x00\x00\x00\x00\x00\x00\
\x00\x00\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x02\
\x00\x00\x00\x00\x00\x00\x00\x00\
\x00\x00\x00\x16\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\
\x00\x00\x01e\xaf\x16\xd2\xa1\
"

def qInitResources():
    QtCore.qRegisterResourceData(0x03, qt_resource_struct, qt_resource_name, qt_resource_data)

def qCleanupResources():
    QtCore.qUnregisterResourceData(0x03, qt_resource_struct, qt_resource_name, qt_resource_data)

qInitResources()
