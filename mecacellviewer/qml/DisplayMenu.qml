import QtQuick 2.0
import QtQuick.Controls 1.2

Item {
	width: parent.width
	height: parent.height
	property alias column:column
	property bool checked : true
	Title {
		id: titre
		txt: "View"
		anchors.top: parent.top
	}
	Rectangle {
		id: viewRect
		anchors.top: titre.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		width: parent.width
		height: parent.height
		color: lightMecaBlue
		Column {
			id: column 
			width:parent.width
			anchors.horizontalCenter: parent.horizontalCenter
			height: childrenRect.height +15 
			anchors.left: parent.left
			spacing: 5
			anchors.top: parent.top
			anchors.topMargin: 20
		}
	}
}
