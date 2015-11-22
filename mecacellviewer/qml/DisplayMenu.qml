import QtQuick 2.0
import QtQuick.Controls 1.2

Item {
	width: parent.width
	height: parent.height
	property alias mainColumn: mainColumn 
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
		height: childrenRect.height + 40
		color: lightMecaBlue
		Column {
			id: mainColumn 
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.left: parent.left
			spacing: 5
			anchors.top: parent.top
			anchors.topMargin: 20
		}
	}
}
