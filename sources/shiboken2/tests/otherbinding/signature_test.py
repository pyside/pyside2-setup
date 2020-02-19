#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
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

'''Test cases for functions signature'''

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from shiboken_paths import init_paths
init_paths()

from other import OtherObjectType
from helper import objectFullname

class SignatureTest(unittest.TestCase):

    # Check if the argument of 'OtherObjectType::enumAsInt(SampleNamespace::SomeClass::PublicScopedEnum value)'
    # has the correct representation
    def testNamespaceFromOtherModule(self):
        argType = OtherObjectType.enumAsInt.__signature__.parameters['value'].annotation
        self.assertEqual(objectFullname(argType), 'sample.SampleNamespace.SomeClass.PublicScopedEnum')

if __name__ == '__main__':
    unittest.main()
