import QtQuick 2.0

Text {
    text: owner.name + " " + owner.phone 
    Component.onCompleted: { owner.newName = owner.name }
}
