from PySide2.QtCore import *
from PySide2.QtWidgets import *

app = QApplication([])

scene = QGraphicsScene()

# don't segfault due to lack of keepReferenceCall
textEdit = scene.addWidget(QTextEdit())

layout = QGraphicsLinearLayout()
layout.addItem(textEdit)

view = QGraphicsView(scene)
view.show()
