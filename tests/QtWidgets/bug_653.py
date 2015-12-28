import unittest
from PySide2.QtCore import *
from PySide2.QtWidgets import *

class TestBug653(unittest.TestCase):
    """Crash after calling QWizardPage.wizard()"""
    def testIt(self):
        app = QApplication([])

        wizard = QWizard()
        page = QWizardPage()
        wizard.addPage(page)
        page.wizard() # crash here if the bug still exists due to a circular dependency
        wizard.show()

if __name__ == "__main__":
    unittest.main()