import QtQuick 2.0
import QtQuick.Controls 1.2
import "javascript.js" as Logic
Item {
	//property alias selectedCellActions: selectedCellActions
	property alias generalActions: generalActions
	width: parent.width
	height: parent.height
	Title {
		id: titre
		txt: "Controls"
		anchors.top: parent.top
	}
	Rectangle {
		id: controlRect
		anchors.top: titre.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		width: parent.width
		height: childrenRect.height + 40
		color: lightMecaYellow
		Column {
			id:generalActions 
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.left: parent.left
			spacing: 5
			anchors.top: parent.top
			anchors.topMargin: 20
			VerticalSpacer {}
			SubTitle {
				txt: "TOOLS"
			}
		}
	}
}
