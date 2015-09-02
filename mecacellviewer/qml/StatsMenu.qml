import QtQuick 2.0
import QtQuick.Controls 1.2
Item {
	width: parent.width
	height: parent.height
	Title {
		id: titre
		txt: "Stats & Graphs"
		anchors.top: parent.top
	}
	Rectangle {
		id: statsRect
		anchors.top: titre.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		width: parent.width
		height: childrenRect.height + 40
		color: lightMecaRed
		Column {
			id: statsCol
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 5
			anchors.top: parent.top
			anchors.topMargin: 20
			VerticalSpacer {}
			Column {
				width: parent.width
				anchors.horizontalCenter: parent.horizontalCenter
				spacing: 5
				height: 50
				id: statsInfos
				SubTitle {
					txt: "STATS & INFOS"
				}
			}
		}
	}
}
