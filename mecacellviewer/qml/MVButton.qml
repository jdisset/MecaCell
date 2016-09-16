import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.3

Button {
	property color notpressedColor: "#00000000"
	property string name : ""

	text: "A button"
	width: parent.width * 0.8
	anchors.left: parent.left
	anchors.leftMargin : parent.width * 0.1
	height: 30
	onClicked: {
		renderer.buttonClick(name);
	}
	style: ButtonStyle {
		label: Text {
			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignHCenter
			//font.family: opensans.name
			font.pointSize: 11
			font.bold: true
			color: "#FFF"
			text: control.text
		}
		background: Rectangle {
			border.width: 1
			border.color: "#90FFFFFF"
			color: control.hovered ? Qt.darker(notpressedColor,1.15) :  (control.pressed ? Qt.darker(notpressedColor,1.3): notpressedColor)
			radius: 5
		}
	}
}
