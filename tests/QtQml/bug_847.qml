
import QtQuick 2.0

Rectangle {
    width: 500
    height: 500
    color: 'red'

    property variant pythonObject: undefined

    Text {
        anchors.centerIn: parent
        text: 'click me'
        color: 'white'
    }

    Timer {
        interval: 100; running: true;
        onTriggered: {
            if (pythonObject != undefined) {
                pythonObject.blubb(42, 84)
            }
        }
    }
}

