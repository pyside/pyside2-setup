#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
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

'''Unit tests for QMimeDatabase'''

import unittest
import ctypes
import sys

from PySide2.QtCore import QMimeDatabase, QLocale

class QMimeDatabaseTest(unittest.TestCase):
    def testMimeTypeForName(self):
        db = QMimeDatabase()

        s0 = db.mimeTypeForName("application/x-zerosize")
        self.assertTrue(s0.isValid())
        self.assertEqual(s0.name(), "application/x-zerosize")
        if "en" in QLocale().name():
            self.assertEqual(s0.comment(), "empty document")

        s0Again = db.mimeTypeForName("application/x-zerosize")
        self.assertEqual(s0Again.name(), s0.name())

        s1 = db.mimeTypeForName("text/plain")
        self.assertTrue(s1.isValid())
        self.assertEqual(s1.name(), "text/plain")
        # print("Comment is  %s" % s1.comment())

        krita = db.mimeTypeForName("application/x-krita")
        self.assertTrue(krita.isValid())

        # Test <comment> parsing with application/rdf+xml which has the english comment after the other ones
        rdf = db.mimeTypeForName("application/rdf+xml")
        self.assertTrue(rdf.isValid())
        self.assertEqual(rdf.name(), "application/rdf+xml")
        self.assertEqual(rdf.comment(), "RDF file")

        bzip2 = db.mimeTypeForName("application/x-bzip2")
        self.assertTrue(bzip2.isValid())
        self.assertEqual(bzip2.comment(), "Bzip archive")

        defaultMime = db.mimeTypeForName("application/octet-stream")
        self.assertTrue(defaultMime.isValid())
        self.assertEqual(defaultMime.name(), "application/octet-stream")
        self.assertTrue(defaultMime.isDefault())

        doesNotExist = db.mimeTypeForName("foobar/x-doesnot-exist")
        self.assertTrue(not doesNotExist.isValid())

if __name__ == '__main__':
    unittest.main()
