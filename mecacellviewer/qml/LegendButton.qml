import QtQuick 2.0
import QtQuick.Controls 1.2

Rectangle {
	property string label;
	property string legend;
	property bool checked: false
	property
	var group: null
	property bool shadow: false;
	property color selecColor: mecaYellow;
	property color notselecColor: "#60FFFFFF";
	signal check(bool chk)
	height: 25
	width: 70
	color: checked && shadow ? "#88000000" : "transparent"
	id: me
	Rectangle {
		anchors.top: parent.top
		height: parent.height
		color: checked ? selecColor : "transparent"
		border.color: !checked ? notselecColor : "transparent"
		width: height
		id: lbl
		Text {
			text: label
			color: checked || mouseArea.containsMouse ? "white" : notselecColor
			font.family: fontawesome.name
			font.pointSize: 15
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
		}
	}
	Text {
		text: legend
		color: checked || mouseArea.containsMouse ? "white" : notselecColor
		font.family: opensans.name
		font.pointSize: 9
		anchors.verticalCenter: parent.verticalCenter
		anchors.left: lbl.right
		anchors.leftMargin: 3
	}
	MouseArea {
		id: mouseArea
		acceptedButtons: Qt.LeftButton
		anchors.fill: parent
		hoverEnabled: true
		onClicked: {
			if (group) group.toggled(me)
			else checked = !checked
		}
	}
	onCheck: {
		//checked = chk
	}
}
