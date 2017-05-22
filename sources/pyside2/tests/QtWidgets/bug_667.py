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

import sys

from PySide2.QtCore import QTimer, QPointF
from PySide2.QtWidgets import QApplication, QGraphicsView, QGraphicsScene, QGraphicsEllipseItem

class Ball(QGraphicsEllipseItem):
    def __init__(self, d, parent=None):
        super(Ball, self).__init__(0, 0, d, d, parent)
        self.vel = QPointF(0,0)   #commenting this out prevents the crash

class Foo(QGraphicsView):
    def __init__(self):
        super(Foo, self).__init__(None)
        self.scene = QGraphicsScene(self.rect())
        self.setScene(self.scene)
        self.scene.addItem(Ball(10))


if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = Foo()
    w.show()
    w.raise_()
    QTimer.singleShot(0, w.close)
    sys.exit(app.exec_())
