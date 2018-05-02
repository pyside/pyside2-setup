#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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
from sample import ObjectType
from sample import ObjectView
from sample import ObjectModel



class ObjTest(unittest.TestCase):

    def test_cyclic_dependency_withParent(self):
        """Create 2 objects with a cyclic dependency, so that they can
        only be removed by the garbage collector, and then invoke the
        garbage collector in a different thread.
        """
        import gc

        class CyclicChildObject(ObjectType):
            def __init__(self, parent):
                super(CyclicChildObject, self).__init__(parent)
                self._parent = parent

        class CyclicObject(ObjectType):
            def __init__(self):
                super(CyclicObject, self).__init__()
                CyclicChildObject(self)

        # turn off automatic garbage collection, to be able to trigger it
        # at the 'right' time
        gc.disable()
        alive = lambda :sum(isinstance(o, CyclicObject) for o in gc.get_objects() )

        #
        # first proof that the wizard is only destructed by the garbage
        # collector
        #
        cycle = CyclicObject()
        self.assertTrue(alive())
        del cycle
        self.assertTrue(alive())
        gc.collect()
        self.assertFalse(alive())

    def test_cyclic_dependency_withKeepRef(self):
        """Create 2 objects with a cyclic dependency, so that they can
        only be removed by the garbage collector, and then invoke the
        garbage collector in a different thread.
        """
        import gc

        class CyclicChildObject(ObjectView):
            def __init__(self, model):
                super(CyclicChildObject, self).__init__(None)
                self.setModel(model)

        class CyclicObject(ObjectModel):
            def __init__(self):
                super(CyclicObject, self).__init__()
                self._view = CyclicChildObject(self)

        # turn off automatic garbage collection, to be able to trigger it
        # at the 'right' time
        gc.disable()
        alive = lambda :sum(isinstance(o, CyclicObject) for o in gc.get_objects() )

        #
        # first proof that the wizard is only destructed by the garbage
        # collector
        #
        cycle = CyclicObject()
        self.assertTrue(alive())
        del cycle
        self.assertTrue(alive())
        gc.collect()
        self.assertFalse(alive())

if __name__ == '__main__':
    unittest.main()

