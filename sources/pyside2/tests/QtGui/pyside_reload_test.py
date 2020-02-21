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

import os
import shutil
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "util"))

import py3kcompat as py3k

orig_path = os.path.join(os.path.dirname(__file__))
workdir = os.getcwd()
src = os.path.normpath(os.path.join(orig_path, '..', 'QtWidgets', 'test_module_template.py'))
dst = os.path.join(workdir, 'test_module.py')
shutil.copyfile(src, dst)
sys.path.append(workdir)

def reload_module(moduleName):
    if py3k.IS_PY3K:
        import importlib
        importlib.reload(moduleName)
    else:
        reload(moduleName)

def increment_module_value():
    modfile = open(dst, 'a')
    modfile.write('Sentinel.value += 1' + os.linesep)
    modfile.flush()
    modfile.close()
    if not sys.dont_write_bytecode:
        if py3k.IS_PY3K:
            import importlib.util
            cacheFile = importlib.util.cache_from_source(dst)
        else:
            cacheFile = dst + 'c'
        os.remove(cacheFile)

class TestModuleReloading(unittest.TestCase):

    def testModuleReloading(self):
        '''Test module reloading with on-the-fly modifications.'''

        import test_module
        self.assertEqual(test_module.Sentinel.value, 10)

        increment_module_value()
        reload_module(sys.modules['test_module'])
        self.assertEqual(test_module.Sentinel.value, 11)

        reload_module(sys.modules['test_module'])
        self.assertEqual(test_module.Sentinel.value, 11)

        increment_module_value()
        reload_module(sys.modules['test_module'])
        self.assertEqual(test_module.Sentinel.value, 12)

if __name__ == "__main__":
    unittest.main()


