import unittest

from helper import adjust_filename, UsesQApplication

from PySide2.QtCore import Slot, Property, Signal, QObject, QUrl
from PySide2.QtQml import QJSEngine, qmlRegisterType
from PySide2.QtQuick import QQuickView

test_error_message = "This is an error."

method_test_string = """
(function (obj) {
    obj.methodThrows();
})
"""

property_test_string = """
(function (obj) {
    obj.propertyThrows;
})
"""

test_1 = False
test_2 = False

class TestClass(QObject):
    @Slot()
    def methodThrows(self):
        raise TypeError(test_error_message)

    @Property(str)
    def propertyThrows(self):
        raise TypeError(test_error_message)

    @Slot(int)
    def passTest(self, test):
        global test_1, test_2

        if test == 1:
            test_1 = True
        else:
            test_2 = True

class JavaScriptExceptionsTest(UsesQApplication):
    def test_jsengine(self):
        engine = QJSEngine()
        test_object = TestClass()
        test_value = engine.newQObject(test_object)

        result_1 = engine.evaluate(method_test_string).call([test_value])

        self.assertTrue(result_1.isError())
        self.assertEqual(result_1.property('message').toString(), test_error_message)
        self.assertEqual(result_1.property('name').toString(), 'TypeError')

        result_2 = engine.evaluate(property_test_string).call([test_value])

        self.assertTrue(result_2.isError())
        self.assertEqual(result_2.property('message').toString(), test_error_message)
        self.assertEqual(result_2.property('name').toString(), 'TypeError')

    def test_qml_type(self):
        qmlRegisterType(TestClass, 'JavaScriptExceptions', 1, 0, 'JavaScriptExceptions');

        view = QQuickView()
        qml_url = QUrl.fromLocalFile(adjust_filename('javascript_exceptions.qml', __file__))

        view.setSource(qml_url)

        self.assertTrue(test_1)
        self.assertTrue(test_2)


if __name__ == '__main__':
    unittest.main()
