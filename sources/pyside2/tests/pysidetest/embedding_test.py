#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of PySide2.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

import unittest

# This test tests the embedding feature of PySide.
# Normally, embedding is only used when necessary.
# By setting the variable "pyside_uses_embedding",
# we enforce usage of embedding.


class EmbeddingTest(unittest.TestCase):

    # def test_pyside_normal(self):
    #     import sys
    #     self.assertFalse(hasattr(sys, "pyside_uses_embedding"))
    #     import PySide2
    #     # everything has to be imported
    #     self.assertTrue("PySide2.support.signature" in sys.modules)
    #     # there should be a variale in sys, now (no idea if set)
    #     self.assertTrue(hasattr(sys, "pyside_uses_embedding"))

    # Unfortunately, I see no way how to shut things enough down
    # to trigger a second initiatization. Therefore, only one test :-/
    def test_pyside_embedding(self):
        import sys, os
        self.assertFalse(hasattr(sys, "pyside_uses_embedding"))
        sys.pyside_uses_embedding = "anything true"
        import PySide2
        # everything has to be imported
        self.assertTrue("PySide2.support.signature" in sys.modules)
        self.assertEqual(sys.pyside_uses_embedding, True)
        dn = os.path.dirname
        name = os.path.basename(dn(dn(dn(PySide2.support.signature.__file__))))
        self.assertTrue(name.startswith("embedded.") and name.endswith(".zip"))

if __name__ == '__main__':
    unittest.main()
