#!/usr/bin/python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
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

import unittest
from PySide2.QtXml import QXmlSimpleReader

class QXmlSimpleReaderTest(unittest.TestCase):

    def testQXmlSimpleReaderInstatiation(self):
        '''QXmlSimpleReader must be a concrete class not an abstract one.'''
        reader = QXmlSimpleReader()

    def testQXmlSimpleReaderFeatures(self):
        '''Calls the QXmlSimpleReader.features method. The features checked
        (at least the first two) can be found in the QXmlSimpleReader documentation:
        http://qt.nokia.com/doc/4.6/qxmlsimplereader.html#setFeature
        '''
        reader = QXmlSimpleReader()
        hasFeature, ok = reader.feature('http://xml.org/sax/features/namespaces')
        self.assertEqual((hasFeature, ok), (True, True))

        hasFeature, ok = reader.feature('http://xml.org/sax/features/namespace-prefixes')
        self.assertEqual((hasFeature, ok), (False, True))

        hasFeature, ok = reader.feature('foobar')
        self.assertEqual((hasFeature, ok), (False, False))

    def testQXmlSimpleReaderProperty(self):
        '''Tries to get a non existent property.'''
        reader = QXmlSimpleReader()
        prop, ok = reader.property('foo')
        self.assertEqual((prop, ok), (None, False))

if __name__ == '__main__':
    unittest.main()

