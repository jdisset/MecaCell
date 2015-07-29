import QtQuick 2.0
import QtQuick.Controls 1.3

Item {
	width: parent.width
	height: parent.height
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
			id: viewCol
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.left: parent.left
			spacing: 5
			anchors.top: parent.top
			anchors.topMargin: 20
			VerticalSpacer {}
			SubTitle {
				txt: "VISIBLE ELEMENTS"
			}
			Column {
				id: viewMenu
				anchors.left: parent.left
				anchors.leftMargin: 25
				spacing: 10
				CheckableButton {
					checked: true
					id: viewCells
					legend: "Cells"
					onToggled: {
						if (checked) pushUniqueOptionInCtrl("visibleElements", "cells");
						else removeOptionInCtrl("visibleElements", "cells");
					}
				}
				CheckableButton {
					checked: false
					id: viewConnexions
					legend: "Connections"
					onToggled: {
						if (checked) pushUniqueOptionInCtrl("visibleElements", "connections");
						else removeOptionInCtrl("visibleElements", "connections");
					}
				}
				CheckableButton {
					checked: false
					id: viewCenters
					legend: "Centers"
					onToggled: {
						if (checked) pushUniqueOptionInCtrl("visibleElements", "centers");
						else removeOptionInCtrl("visibleElements", "centers");
					}
				}
				CheckableButton {
					checked: false
					id: viewCellGrid
					legend: "Cells grid"
					onToggled: {
						if (checked) pushUniqueOptionInCtrl("visibleElements", "cellGrid");
						else removeOptionInCtrl("visibleElements", "cellGrid");
					}
				}
				CheckableButton {
					checked: false
					id: viewModelGrid
					legend: "Models grid"
					onToggled: {
						if (checked) pushUniqueOptionInCtrl("visibleElements", "modelGrid");
						else removeOptionInCtrl("visibleElements", "modelGrid");
					}
				}
			}
			VerticalSpacer {}
			SubTitle {
				txt: "CELLS COLOR"
			}
			Column {
				id: viewMenu2
				anchors.left: parent.left
				anchors.leftMargin: 25
				spacing: 10
				ExclusiveStuff {
					id: colorMode
					objectsInGroup: [pressureColor, ownColor]
				}
				CheckableButton {
					checked: true
					id: ownColor
					group: colorMode
					legend: "Own color"
					onCheck: {
						if (chk) {
							setCtrl("colorMode", "owncolor");
							checked = true
						} else {
							checked = false
						}
					}
				}
				CheckableButton {
					checked: false
					group: colorMode
					id: pressureColor
					legend: "Pressure"
					onCheck: {
						if (chk) {
							setCtrl("colorMode", "pressure");
							checked = true
						} else {
							checked = false
						}
					}
				}
				VerticalSpacer {}
				SubTitle {
					txt: "GUI OPTIONS"
				}
				Column {
					id: viewMenu3
					anchors.left: parent.left
					anchors.leftMargin: 25
					spacing: 10
					CheckableButton {
						checked: false
						id: takeScreen
						legend: "Screen captures"
						onToggled: {
							if (checked) setCtrl("takeScreen", true);
							else removeCtrl("takeScreen");
						}
					}
				}
			}
		}
	}
}
