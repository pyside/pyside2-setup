
import QtQuick 2.0

ListView {
    width: 300; height: 300
    delegate: Text { text: pysideModelData }
    model: 3
}

