from helper import adjust_filename, UsesQApplication

from PySide2.QtGui import QGuiApplication
from PySide2.QtQuick import QQuickView

app = QGuiApplication([])
view = QQuickView(adjust_filename('bug_995.qml', __file__))
view.show()
view.resize(200, 200)
# TODO: is there QQuick alternative to tis?
item = view.itemAt(100, 100)

# it CAN NOT crash here
print(item)

