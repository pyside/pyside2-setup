#!/usr/bin/python
# -*- coding: utf-8 -*-
import unittest

from helper import TimedQApplication

from PySide2.QtQuick import QQuickView, QQuickItem

class TestBug915(TimedQApplication):
    def testReturnPolicy(self):
        view = QQuickView()

        item1 = QQuickItem()
        item1.setObjectName("Item1")
        # TODO: This existed in QDeclarativeView but not in QQuickView.
        # Have to rewrite this to the QQuickView equivalent
        view.scene().addItem(item1)
        self.assertEqual(item1.objectName(), "Item1") # check if the item still valid

        item2 = QQuickItem()
        item2.setObjectName("Item2")
        item1.scene().addItem(item2)
        item1 = None
        self.assertEqual(item2.objectName(), "Item2") # check if the item still valid

        view = None

if __name__ == '__main__':
    unittest.main()


