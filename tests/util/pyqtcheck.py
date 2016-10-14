#!/usr/bin/env python

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

import sys
from optparse import OptionParser

import PyQt4
from PyQt4 import QtCore, QtGui, QtNetwork

modules = [QtCore, QtGui, QtNetwork]

def search(klass, method=None):
    for module in modules:
        try:
            klass_obj = getattr(module, klass)
            print "%s *found* on module %s" % (klass, module.__name__)
        except AttributeError:
            print "%s not found on module %s" % (klass, module.__name__)
            continue

        if method is None:
            continue

        try:
            meth_obj = getattr(klass_obj, method)

            meth_name = ".".join([klass, method])
            klass_name = ".".join([module.__name__, klass])
            print "\"%s\" *found* on class %s" % (meth_name, klass_name)
        except AttributeError:
            print "\"%s\" not found on class %s" % (method, klass)


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]

    try:
        klass, method = argv[0].split(".")
    except:
        klass = argv[0]
        method = None

    search(klass, method)

if __name__ == '__main__':
    main()
