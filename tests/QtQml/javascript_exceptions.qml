import QtQuick 2.0
import QtQuick.Controls 1.0
import JavaScriptExceptions 1.0

Rectangle {
    JavaScriptExceptions {
        id: obj
    }

    Component.onCompleted: {
        // Method call test
        try {
            obj.methodThrows();
        } catch(e) {
            obj.passTest(1);
        }

        // Property accessor test
        try {
            obj.propertyThrows;
        } catch(e) {
            obj.passTest(2);
        }
    }
}