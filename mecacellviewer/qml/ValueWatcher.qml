import QtQuick 2.0
Item {
	property string label;
	property string value;
	property color labelColor: "#EEEEEE"
	property color valueColor: "#DDDDDD"
	width: (parent.width - 4 * parent.spacing) / 3
	height: parent.height
	Item {
		id: titleRect
		anchors.top: parent.top
		width: parent.width
		height: parent.height / 2
		Text {
			text: label
			color: labelColor
			font.family: oswald.name
			font.pointSize: 10
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.bottom: parent.bottom
		}
	}
	Item {
		anchors.top: titleRect.bottom
		width: parent.width
		height: parent.height / 2
		Text {
			text: value
			color: valueColor
			font.pointSize: 10
			font.family: opensans.name
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.top: parent.top
		}
	}
}
