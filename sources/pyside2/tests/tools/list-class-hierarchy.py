#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of PySide2.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################


# This is a small script printing out Qt binding class hierarchies
# for comparison purposes.
#
# Usage:
#
# ./list-class-hierarchy.py PySide2 > pyside2.list
# ./list-class-hierarchy.py PyQt5 > pyqt5.list
#
# meld pyside.list pyqt5.list

from __future__ import print_function

import sys
import pdb
from inspect import isclass

ignore = ["staticMetaObject",
          "pyqtConfigure",
          "registerUserData",
          "thread",
         ]

def recurse_into(el,obj):
    #s = el.split('.')[-1]
    #pdb.set_trace()
    symbols = []
    for item in sorted(dir(obj)):
        if item[0]=='_':
            continue
        mel = el + '.' + item
        try:
            mobj = eval(mel)
        except Exception:
            continue

        if item in ignore:
            continue
        else:
            symbols.append(mel)

        if isclass(mobj):
            symbols += recurse_into(mel,mobj)

    return symbols

if __name__=='__main__':
    modules = [ 'QtCore',
                'QtGui',
                'QtHelp',
               #'QtMultimedia',
                'QtNetwork',
               #'QtOpenGL',
                'QtScript',
                'QtScriptTools',
                'QtSql',
                'QtSvg',
                'QtTest',
               #'QtUiTools',
                'QtWebKit',
                'QtXml',
                'QtXmlPatterns' ]

    libraries = ["PySide2", "PyQt5"]
    librarySymbols = {}
    for l in libraries:
        dictionary = []
        if l =="PyQt5":
            import sip
            sip.setapi('QDate',2)
            sip.setapi('QDateTime',2)
            sip.setapi('QString',2)
            sip.setapi('QTextStream',2)
            sip.setapi('QTime',2)
            sip.setapi('QUrl',2)
            sip.setapi('QVariant',2)

        for m in modules:
            exec("from %s import %s" % (l,m), globals(), locals())
            dictionary += recurse_into(m, eval(m))
        librarySymbols[l] = dictionary

    print("PyQt5: ", len(librarySymbols["PyQt5"]), " PySide2: ", len(librarySymbols["PySide2"]))

    for symbol in librarySymbols["PyQt5"]:
        if not (symbol in librarySymbols["PySide2"]):
            print("Symbol not found in PySide2:", symbol)
