import SceneGraphRendering 1.0
import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.2
import "javascript.js" as Logic
import QtQuick.Window 2.1

Window {
	property var guictrl: renderer.getGuiCtrl();
	property var btnArray: new Object();
	property var paintStepsCategories: new Object();
	property var paintStepsSubgroups: new Object();
	property var paintStepsElem: new Object();
	width: 1920
	height:1080 
	id: main
	function createDisplayMenu(m){Logic.createDisplayMenu(m);} 
	function addPaintStepComponent(a,b,c,d){Logic.addPaintStepComponent(a,b,c,d);}
	function addButton(a,b,c,d){Logic.addButton(a,b,c,d);}
	Renderer {
		objectName: "renderer"
		id: renderer
		anchors.fill:parent
		//anchors.left: leftMenu.right
		//anchors.top: parent.top
		//anchors.bottom: parent.bottom
		//anchors.right: parent.right
		focus: true
		//z: -1
	}

	property color mecaYellow: "#F2DC83"
	property color lightMecaYellow: "#30F5ECA6"
	property color mecaBlue: "#2DB2D6"
	property color lightMecaBlue: "#302DB2D6"
	property color mecaRed: "#D33035"
	property color lightMecaRed: "#30D33035"
	property color bitDarker: "#20000000"
	property color darker: "#60000000"
	property color background: "#80000000"
	FontLoader {
		id: fontawesome;source: "fonts/fontawesome.ttf"
	}
	FontLoader {
		id: oswald;source: "fonts/Oswald-Regular.ttf"
	}
	FontLoader {
		id: opensans;source: "fonts/OpenSans-Light.ttf"
	}

	Rectangle {
		id: leftMenu
		anchors.left: parent.left
		anchors.top: parent.top
		height: parent.height
		color: background
		//color:"transparent" 
		width: 200
		Rectangle {
			id: logo
			width: parent.width
			height: 75
			color: darker
			Image {
				anchors.top: parent.top
				anchors.left: parent.left
				anchors.leftMargin: 57
				anchors.topMargin: 13
				source: "images/logo.png"
				fillMode: Image.PreserveAspectFit
				horizontalAlignment: Image.AlignLeft
				width: 85
			}
		}
		Rectangle {
			id: menuChooser
			anchors.top: logo.bottom
			height: 50
			width: parent.width
			color: "#25000000"
			Row {
				anchors.top: parent.top
				anchors.topMargin: 10
				spacing: 38
				anchors.horizontalCenter: parent.horizontalCenter

				ExclusiveStuff {
					id: menuGroup
					objectsInGroup: [displayButton, controlsButton, statsButton]
				}

				CircleButton {
					group: menuGroup
					id: displayButton
					selecColor: mecaBlue
					label: "\uf108"
					anchors.verticalCenter: parent.verticalCenter
				}
				CircleButton {
					id: controlsButton
					checked: true
					group: menuGroup
					selecColor: mecaYellow
					label: "\uf00a"
					anchors.verticalCenter: parent.verticalCenter
				}
				CircleButton {
					id: statsButton
					group: menuGroup
					selecColor: mecaRed
					label: "\uf080"
					anchors.verticalCenter: parent.verticalCenter
				}
			}
		}
		ParamsMenu {
			id: controlsMenu
			width: parent.width
			anchors.top: menuChooser.bottom
			anchors.bottom: player.top
			visible: controlsButton.checked
			anchors.topMargin: 10
		}
		DisplayMenu {
			id: displayMenu
			width: parent.width
			anchors.top: menuChooser.bottom
			anchors.bottom: player.top
			visible: displayButton.checked
			anchors.topMargin: 10
		}
		StatsMenu {
			id: statsMenu
			width: parent.width
			anchors.top: menuChooser.bottom
			anchors.bottom: player.top
			visible: statsButton.checked
			anchors.topMargin: 10
		}
		Rectangle {
			id: player
			anchors.bottom: mainStats.top
			anchors.bottomMargin: 5
			height: 42
			width: parent.width
			color: bitDarker
			property int fontSize: 16
			Row {
				height: parent.height
				spacing: 50
				anchors.horizontalCenter: parent.horizontalCenter
				Text {
					id: resetButton
					text: "\uf01e"
					font.family: fontawesome.name
					font.pointSize: player.fontSize
					anchors.verticalCenter: parent.verticalCenter
					color: ma_reset.containsMouse ? mecaRed : "white"
					MouseArea {
						id: ma_reset
						acceptedButtons: Qt.LeftButton
						anchors.fill: parent
						hoverEnabled: true
						onClicked: {}
					}
				}
				Text {
					id: playButton
					property bool playing: false;
					text: playing ? "\uf04c" : "\uf04b"
					color: ma_play.containsMouse ? mecaYellow : "white"
					font.family: fontawesome.name
					font.pointSize: player.fontSize
					anchors.verticalCenter: parent.verticalCenter
					MouseArea {
						id: ma_play
						acceptedButtons: Qt.LeftButton
						anchors.fill: parent
						hoverEnabled: true
						onClicked: {
							parent.playing = !parent.playing
							Logic.setCtrl(main,"playing", parent.playing);
							glview.setWorldUpdate(parent.playing);
						}
					}
				}
				Text {
					id: stepButton
					text: "\uf051"
					color: ma_step.containsMouse ? mecaBlue : "white"
					font.family: fontawesome.name
					font.pointSize: player.fontSize
					anchors.verticalCenter: parent.verticalCenter
					MouseArea {
						id: ma_step
						acceptedButtons: Qt.LeftButton
						anchors.fill: parent
						hoverEnabled: true
						onClicked: {
							glview.step();
						}
					}
				}
			}
		}
		Rectangle {
			id: mainStats
			height: 35
			width: parent.width
			color: "transparent"
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 3
			Row {
				height: parent.height
				width: parent.width - parent.anchors.leftMargin
				spacing: 10
				ValueWatcher {
					label: "FPS"
					value: Logic.getStat("fps").toFixed(0)
				}
				RowSpacer {
					color: "#20FFFFFF"
					coef: 1
				}
				ValueWatcher {
					label: "CELLS"
					value: Logic.getStat("nbCells")
				}
				RowSpacer {
					color: "#20FFFFFF"
					coef: 1
				}
				ValueWatcher {
					label: "UPDATE"
					value: Logic.getStat("nbUpdates")
				}
			}
		}
	}
}
