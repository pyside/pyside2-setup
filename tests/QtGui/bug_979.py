from PySide.QtWidgets import QDialog
from import_test import PysideImportTest2

class PysideImportTest1(QDialog, PysideImportTest2):
       pass

if __name__ == '__main__':
    quit()

