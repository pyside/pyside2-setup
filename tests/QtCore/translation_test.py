#!/usr/bin/python
# -*- coding: utf-8 -*-

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

'''Unit tests to test QTranslator and translation in general.'''

import os
import unittest
import py3kcompat as py3k
from PySide2.QtCore import QObject, QTranslator, QCoreApplication

from helper import UsesQCoreApplication

class TranslationTest(UsesQCoreApplication):
    '''Test case for Qt translation facilities.'''

    def setUp(self):
        super(TranslationTest, self).setUp()
        self.trdir = os.path.join(os.path.dirname(__file__), 'translations')

    def testLatin(self):
        #Set string value to Latin
        translator = QTranslator()
        translator.load(os.path.join(self.trdir, 'trans_latin.qm'))
        self.app.installTranslator(translator)

        obj = QObject()
        obj.setObjectName(obj.tr('Hello World!'))
        self.assertEqual(obj.objectName(), py3k.unicode_('Orbis, te saluto!'))

    def testRussian(self):
        #Set string value to Russian
        translator = QTranslator()
        translator.load(os.path.join(self.trdir, 'trans_russian.qm'))
        self.app.installTranslator(translator)

        obj = QObject()
        obj.setObjectName(obj.tr('Hello World!'))
        self.assertEqual(obj.objectName(), py3k.unicode_('привет мир!'))

    def testUtf8(self):
        translator = QTranslator()
        translator.load(os.path.join(self.trdir, 'trans_russian.qm'))
        self.app.installTranslator(translator)

        obj = QObject()
        obj.setObjectName(obj.tr('Hello World!'))
        self.assertEqual(obj.objectName(), py3k.unicode_('привет мир!'))

    def testTranslateWithNoneDisambiguation(self):
        value = 'String here'
        obj = QCoreApplication.translate('context', value, None)
        self.assertTrue(isinstance(obj, py3k.unicode))
        self.assertEqual(obj, value)

if __name__ == '__main__':
    unittest.main()

