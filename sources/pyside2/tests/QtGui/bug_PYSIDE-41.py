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

# TODO:
# move this to QtCore -- QStringListModel is part of QtGui and there is no
# simple model class appropriate for this test in QtCore.

import unittest

from PySide2.QtCore import *
from PySide2.QtGui import *


class TestBugPYSIDE41(unittest.TestCase):

    def testIt(self):

        # list of single-character strings
        strings = list('abcdefghijklmnopqrstuvwxyz')

        model = QStringListModel(strings)

        # Test hashing of both QModelIndex and QPersistentModelIndex
        indexFunctions = []
        indexFunctions.append(model.index)
        indexFunctions.append(lambda i: QPersistentModelIndex(model.index(i)))

        for indexFunction in indexFunctions:

            # If two objects compare equal, their hashes MUST also be equal. (The
            # reverse is not a requirement.)
            for i, _ in enumerate(strings):
                index1 = indexFunction(i)
                index2 = indexFunction(i)
                self.assertEqual(index1, index2)
                self.assertEqual(hash(index1), hash(index2))

            # Adding the full set of indexes to itself is a no-op.
            allIndexes1 = set(indexFunction(i) for i, _ in enumerate(strings))
            allIndexes2 = set(indexFunction(i) for i, _ in enumerate(strings))
            allIndexesCombined = allIndexes1 & allIndexes2
            self.assertEqual(allIndexes1, allIndexesCombined)
            self.assertEqual(allIndexes2, allIndexesCombined)


if __name__ == '__main__':
    unittest.main()
