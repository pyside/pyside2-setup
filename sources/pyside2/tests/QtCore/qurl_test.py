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

'''Test suite for QtCore.QUrl'''

import unittest

from PySide2.QtCore import QUrl
from PySide2.QtCore import QUrlQuery

class QUrlBasicConstructor(unittest.TestCase):
    '''Tests the basic constructors'''

    def testBasic(self):
        #Default constructor for QUrl
        url = QUrl()
        self.assertEqual(url.toString(), "")

    def testSetAttributes(self):
        #Construct QUrl by set* methods
        url = QUrl()

        url.setScheme('ftp')
        self.assertEqual(url.toString(), 'ftp:')

        url.setHost('www.google.com')
        self.assertEqual(url.toString(), 'ftp://www.google.com')

        url.setPort(8080)
        self.assertEqual(url.toString(), 'ftp://www.google.com:8080')

        url.setPath('/mail/view')
        self.assertEqual(url.toString(),
                        'ftp://www.google.com:8080/mail/view')

        url.setUserName('john')
        self.assertEqual(url.toString(),
                        'ftp://john@www.google.com:8080/mail/view')

        url.setPassword('abc123')
        self.assertEqual(url.toString(),
                        'ftp://john:abc123@www.google.com:8080/mail/view')

class QueryItemsTest(unittest.TestCase):
    '''Test query item management'''

    def testQueryItems(self):
        url = QUrl('http://www.google.com/search?q=python&hl=en')
        valid_data = [(('q'), ('python')), (('hl'), ('en'))]

        self.assertEqual(sorted(QUrlQuery(url.query()).queryItems()), sorted(valid_data))

    def testEncodedQueryItems(self):
        url = QUrl('http://www.google.com/search?q=python&hl=en')
        valid_data = [(('q'), ('python')), (('hl'), ('en'))]

        self.assertEqual(sorted(QUrlQuery(url.query()).queryItems()), sorted(valid_data))

    def testSetQueryItems(self):
        urla = QUrl('http://www.google.com/search?q=python&hl=en')
        urlb = QUrl('http://www.google.com/search')

        urlb.setQuery(urla.query())

        self.assertEqual(urla, urlb)

    def testAddQueryItem(self):
        url = QUrlQuery()
        valid_data = [('hl', 'en'), ('user', 'konqui')]

        url.addQueryItem(*valid_data[0])
        self.assertEqual(url.queryItems()[0], valid_data[0])

        url.addQueryItem(*valid_data[1])
        self.assertEqual(sorted(url.queryItems()), sorted(valid_data))

    def testAllQueryItemsValues(self):
        url = QUrlQuery()
        key = 'key'
        valid_data = ['data', 'valid', 'test']

        for i, data in enumerate(valid_data):
            url.addQueryItem(key, data)
            self.assertEqual(url.allQueryItemValues(key),
                          list(valid_data[:i+1]))

    def testPath(self):
        url = QUrl("http://qt-project.org/images/ban/pgs_front.jpg")
        self.assertEqual(url.path(), "/images/ban/pgs_front.jpg")

# PYSIDE-345: No bindings for QUrlQuery
class QueryItemsTest(unittest.TestCase):
    '''Test query item management'''

    def testQueryItems(self):
        url = QUrl('http://www.google.com/search?q=python&hl=en')
        valid_data = [(('q'), ('python')), (('hl'), ('en'))]

        self.assertEqual(sorted(QUrlQuery(url.query()).queryItems()), sorted(valid_data))

    def testEncodedQueryItems(self):
        url = QUrl('http://www.google.com/search?q=python&hl=en')
        valid_data = [(('q'), ('python')), (('hl'), ('en'))]

        self.assertEqual(sorted(QUrlQuery(url.query()).queryItems()), sorted(valid_data))

    def testSetQueryItems(self):
        urla = QUrl('http://www.google.com/search?q=python&hl=en')
        urlb = QUrl('http://www.google.com/search')

        urlb.setQuery(urla.query())

        self.assertEqual(urla, urlb)

    def testAddQueryItem(self):
        url = QUrlQuery()
        valid_data = [('hl', 'en'), ('user', 'konqui')]

        url.addQueryItem(*valid_data[0])
        self.assertEqual(url.queryItems()[0], valid_data[0])

        url.addQueryItem(*valid_data[1])
        self.assertEqual(sorted(url.queryItems()), sorted(valid_data))

    def testAllQueryItemsValues(self):
        url = QUrlQuery()
        key = 'key'
        valid_data = ['data', 'valid', 'test']

        for i, data in enumerate(valid_data):
            url.addQueryItem(key, data)
            self.assertEqual(url.allQueryItemValues(key),
                             list(valid_data[:i+1]))

if __name__ == '__main__':
    unittest.main()
