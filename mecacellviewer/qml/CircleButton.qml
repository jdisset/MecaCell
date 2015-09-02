import QtQuick 2.0
import QtQuick.Controls 1.2

Rectangle {
	property string label;
	property bool checked: false
	property
	var group: menuGroup
	property bool shadow: false;
	property color selecColor: mecaYellow;
	property color notselecColor: "#60FFFFFF";
	signal check(bool chk)
	width: 35
	color: checked && shadow ? "#88000000" : "transparent"
	height: width
	radius: width * 0.5
	id: me

	Rectangle {
		anchors.top: parent.top
		anchors.topMargin: -2
		width: parent.width
		color: checked ? selecColor : "transparent"
		border.width: 1
		border.color: !checked ? notselecColor : "transparent"
		height: width
		radius: width * 0.5

		Text {
			text: label
			color: checked || mouseArea.containsMouse ? "white" : notselecColor
			font.family: fontawesome.name
			font.pointSize: 15
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
		}

		MouseArea {
			id: mouseArea
			acceptedButtons: Qt.LeftButton
			anchors.fill: parent
			hoverEnabled: true
			onClicked: {
				group.toggled(me)
			}
		}
	}
	onCheck: {
		checked = chk
	}
}
