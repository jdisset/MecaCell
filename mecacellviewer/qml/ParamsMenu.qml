import QtQuick 2.0
import QtQuick.Controls 1.2
Item {
	property alias toolMenu: toolMenu
	property alias selectedCellActions: selectedCellActions
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
			id: paramCol
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 5
			anchors.top: parent.top
			anchors.topMargin: 20
			VerticalSpacer {}
			SubTitle {
				txt: "TOOLS"
			}
			Row {
				id: toolMenu
				anchors.horizontalCenter: parent.horizontalCenter
				height: 50
				spacing: 20

					ExclusiveStuff {
					id: toolGroup
					objectsInGroup: [selectTool, moveTool]
				}

					LegendButton {
					radius: 0
					group: toolGroup
					id: selectTool
					selecColor: bitDarker
					label: "\uf0a6"
					legend: "SELECT"
					anchors.verticalCenter: parent.verticalCenter
					onCheck: {
						checked = chk
						if (chk) setCtrl("tool", "select");
					}
				}
				LegendButton {
					checked: true
					id: moveTool
					group: toolGroup
					selecColor: bitDarker
					label: "\uf0b2"
					legend: "MOVE"
					anchors.verticalCenter: parent.verticalCenter
					onCheck: {
						checked = chk
						if (chk) setCtrl("tool", "move");
					}
				}
			}
			Column {
				width: parent.width
				anchors.horizontalCenter: parent.horizontalCenter
				spacing: 5
				id: generalActions
				VerticalSpacer {}
			}
			Column {
				width: parent.width
				anchors.horizontalCenter: parent.horizontalCenter
				spacing: 5
				id: selectedCellActions
				visible: statAvail("selectedCell")
				VerticalSpacer {}
				SubTitle {
					txt: "SELECTED CELL ACTIONS"
				}
			}
		}
	}
}
