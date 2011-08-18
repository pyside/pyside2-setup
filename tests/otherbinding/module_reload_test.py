import os
import sys
import shutil
import unittest

orig_path = os.path.join(os.path.dirname(__file__))
workdir = os.getcwd()
src = os.path.join(orig_path, 'test_module_template.py')
dst = os.path.join(workdir, 'test_module.py')
shutil.copyfile(src, dst)
sys.path.append(workdir)

class TestModuleReloading(unittest.TestCase):

    def testModuleReloading(self):
        '''Test module reloading with on-the-fly modifications.'''
        import test_module
        for i in range(3):
            oldObject = test_module.obj
            self.assertTrue(oldObject is test_module.obj)
            reload(test_module)
            self.assertFalse(oldObject is test_module.obj)

if __name__ == "__main__":
    unittest.main()
