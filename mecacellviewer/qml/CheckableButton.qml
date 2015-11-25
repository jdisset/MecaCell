import QtQuick 2.0
import QtQuick.Controls 1.2

Rectangle {
	property string legend;
	property bool checked: false
	property
	var group: null
	property color selecColor: "#FFFFFFFF";
	property color notselecColor: "#FFFFFFFF";
	property color notselecIconColor: "#40000000";
	property alias column:column
	property var toggledFunc: null
	signal toggled()
	//color: "transparent"
	color: "#25000000"


	id: me
	width : parent.width - anchors.leftMargin 
	anchors.left: parent.left
	anchors.leftMargin: 10 
	anchors.topMargin: 5
	height : visible ?column.height + lbl.height + anchors.topMargin : 0
	visible: parent.visible
	onToggled: {toggledFunc();}
	Rectangle {
		anchors.left : parent.left
		anchors.top: parent.top
		height: 20
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
		id : txtbox
		height : lbl.height
		width: 130
		text: legend
		color: selecColor
		font.family: opensans.name
		font.pointSize: 13
		anchors.left: lbl.right
		anchors.leftMargin: 3
		anchors.top: lbl.top
	}
	MouseArea {
		id: mouseArea
		acceptedButtons: Qt.LeftButton
		anchors.fill:lbl 
		hoverEnabled: true
		onClicked: {
			if (group) group.toggled(me)
			else {
				checked = !checked
				me.toggled()
			}
		}
	}
	state: checked && parent.visible ? "enabled" : "disabled"
	Column {
		width :parent.width  
		id: column 
		anchors.top: txtbox.bottom
		anchors.left: parent.left
	}
	states: [
		State { name: "enabled"
		PropertyChanges {
			target: column 
			visible :true 
			height: childrenRect.height
		}
	},
	State { name: "disabled"
	PropertyChanges {
		target: column 
		visible : false
		height:0
	}
}
]
transitions: [
	Transition {
		NumberAnimation { properties: "height,visible" }
	}
]
}
