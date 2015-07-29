import QtQuick 2.0
import QtQuick.Controls 1.3

Rectangle {
	property string legend;
	property bool checked: false
	property
	var group: null
	property color selecColor: "#FFFFFFFF";
	property color notselecColor: "#FFFFFFFF";
	property color notselecIconColor: "#40000000";
	signal toggled()
	signal check(bool chk)
	height: 25
	width: 70
	color: "transparent"
	id: me
	Rectangle {
		anchors.top: parent.top
		height: parent.height
		color: "transparent"
		width: height
		id: lbl
		Text {
			text: checked ? "\uf05d" : "\uf111"
			color: checked ? selecColor : notselecIconColor
			font.family: fontawesome.name
			font.pointSize: 15
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
		}
	}
	Text {
		text: legend
		color: selecColor
		font.family: opensans.name
		font.pointSize: 13
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
			else {
				checked = !checked
				me.toggled()
			}
		}
	}
}
