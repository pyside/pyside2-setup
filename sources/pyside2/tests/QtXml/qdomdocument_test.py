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

from PySide2.QtCore import QByteArray
from PySide2.QtXml import QDomDocument, QDomElement
import py3kcompat as py3k

class QDomDocumentTest(unittest.TestCase):

    def setUp(self):
        self.dom = QDomDocument()

        self.goodXmlData = QByteArray(py3k.b('''
        <typesystem package="PySide2.QtXml">
            <value-type name="QDomDocument"/>
            <value-type name="QDomElement"/>
        </typesystem>
        '''))

        self.badXmlData = QByteArray(py3k.b('''
        <typesystem package="PySide2.QtXml">
            <value-type name="QDomDocument">
        </typesystem>
        '''))

    def tearDown(self):
        del self.dom
        del self.goodXmlData
        del self.badXmlData

    def testQDomDocumentSetContentWithBadXmlData(self):
        '''Sets invalid xml as the QDomDocument contents.'''
        ok, errorStr, errorLine, errorColumn = self.dom.setContent(self.badXmlData, True)
        self.assertFalse(ok)
        self.assertEqual(errorStr, 'tag mismatch')
        self.assertEqual(errorLine, 4)
        self.assertEqual(errorColumn, 21)

    def testQDomDocumentSetContentWithGoodXmlData(self):
        '''Sets valid xml as the QDomDocument contents.'''
        ok, errorStr, errorLine, errorColumn = self.dom.setContent(self.goodXmlData, True)
        self.assertTrue(ok)
        self.assertEqual(errorStr, '')
        self.assertEqual(errorLine, 0)
        self.assertEqual(errorColumn, 0)

    def testQDomDocumentData(self):
        '''Checks the QDomDocument elements for the valid xml contents.'''

        def checkAttribute(element, attribute, value):
            self.assertTrue(isinstance(root, QDomElement))
            self.assertFalse(element.isNull())
            self.assertTrue(element.hasAttribute(attribute))
            self.assertEqual(element.attribute(attribute), value)

        ok, errorStr, errorLine, errorColumn = self.dom.setContent(self.goodXmlData, True)
        root = self.dom.documentElement()
        self.assertEqual(root.tagName(), 'typesystem')
        checkAttribute(root, 'package', 'PySide2.QtXml')

        child = root.firstChildElement('value-type')
        checkAttribute(child, 'name', 'QDomDocument')

        child = child.nextSiblingElement('value-type')
        checkAttribute(child, 'name', 'QDomElement')

if __name__ == '__main__':
    unittest.main()

