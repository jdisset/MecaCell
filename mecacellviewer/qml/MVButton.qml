import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.3

Button {
	property color pressedColor: "#333"
	property color notpressedColor: "#00000000"
	property color hoveredColor: "#20000000"
	property string menu: ""
	property string name : ""

	text: "A button"
	width: parent.width
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
			color: control.hovered ? hoveredColor : (control.pressed ? pressedColor : notpressedColor)
			radius: 5
		}
	}
}
