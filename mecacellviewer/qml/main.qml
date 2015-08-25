import SceneGraphRendering 1.0
import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.3
Item {
	property
	var guictrl: renderer.getGuiCtrl();
	width: 1200
	height: 800
	id: main
	Component.onCompleted: {
		guictrl["visibleElements"] = new Array();
		guictrl["visibleElements"].push("cells");
		renderer.setGuiCtrl(guictrl);
	}

	function addButton(menu, label) {
		var container = controlsMenu.selectedCellActions;
		if (menu == "SELECTEDCELL_MENU") {
			container = controlsMenu.selectedCellActions;
		}
		var component;
		var finishCreation = function() {
			if (component.status == Component.Ready) {
				var btn = component.createObject(container, {
					"text": label,
					"menu": menu
				});
			} else if (component.status == Component.Error) {
				console.log("Error loading component:", component.errorString());
			}
		};
		component = Qt.createComponent("MVButton.qml");
		if (component.status == Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function removeCtrl(k) {
		if (guictrl[k] != undefined) {
			delete guictrl[k];
			renderer.setGuiCtrl(guictrl);
		}
	}

	function setCtrl(k, v) {
		guictrl[k] = v;
		renderer.setGuiCtrl(guictrl);
	}

	function removeOptionInCtrl(k, v) {
		if (guictrl[k] != undefined) {
			for (var i = 0; i < guictrl[k].length; ++i) {
				if (guictrl[k][i] == v) {
					guictrl[k].splice(i, 1);
					break;
				}
			}
		}
		renderer.setGuiCtrl(guictrl);
	}

	function pushUniqueOptionInCtrl(k, v) {
		var exists = false;
		if (guictrl[k] != undefined) {
			for (var i = 0; i < guictrl[k].length; ++i) {
				if (guictrl[k][i] == v) {
					exists = true;
				}
			}
		} else {
			guictrl[k] = new Array();
		}
		if (!exists) {
			guictrl[k].push(v);
			renderer.setGuiCtrl(guictrl);
		}
	}

	function statAvail(k) {
		return renderer.stats[k] ? true : false;
	}

	function getStat(k) {
		return renderer.stats[k] ? renderer.stats[k] : 0;
	}

	property color mecaYellow: "#A4DED48A"
	property color lightMecaYellow: "#30DED48A"
	property color mecaBlue: "#A42DB2D6"
	property color lightMecaBlue: "#302DB2D6"
	property color mecaRed: "#B4E3343A"
	property color bitDarker: "#50000000"
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
		objectName: "bglol"
		color: background
			//color:"transparent" 
		width: 200
		Rectangle {
			id: logo
			width: parent.width
			height: 75
			color: bitDarker
			Image {
				anchors.top: parent.top
				anchors.left: parent.left
				anchors.leftMargin: 60
				anchors.topMargin: 15
				source: "images/logo.png"
				fillMode: Image.PreserveAspectFit
				horizontalAlignment: Image.AlignLeft
				width: 80
			}
		}
		Row {
			id: menuChooser
			anchors.top: logo.bottom
			anchors.topMargin: 10
			height: 50
			spacing: 38
			anchors.horizontalCenter: parent.horizontalCenter

				ExclusiveStuff {
				id: menuGroup
				objectsInGroup: [displayButton, paramsButton, statsButton]
			}

				CircleButton {
				group: menuGroup
				id: displayButton
				selecColor: mecaBlue
				label: "\uf108"
				anchors.verticalCenter: parent.verticalCenter
			}
			CircleButton {
				id: paramsButton
				checked: true
				group: menuGroup
				selecColor: mecaYellow
				label: "\uf1de"
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

		ParamsMenu {
			id: controlsMenu
			width: parent.width
			anchors.top: menuChooser.bottom
			anchors.bottom: player.top
			visible: paramsButton.checked
		}
		DisplayMenu {
			id: displayLoader
			width: parent.width
			anchors.top: menuChooser.bottom
			anchors.bottom: player.top
			visible: displayButton.checked
		}
		Rectangle {
			id: player
			anchors.bottom: mainStats.top
			anchors.bottomMargin: 15
			height: 35
			width: parent.width
			color: bitDarker
			property int fontSize: 19
			Row {
				height: parent.height
				spacing: 38
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
					property bool playing: true;
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
							setCtrl("playing", parent.playing);
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
			anchors.bottomMargin: 5
			anchors.leftMargin: 7
			Row {
				height: parent.height
				width: parent.width - parent.anchors.leftMargin
				spacing: 10
				ValueWatcher {
					label: "FPS"
					value: getStat("fps").toFixed(0)
				}
				HorizontalSpacer {
					color: Qt.lighter(mainStats.color, 1.5)
				}
				ValueWatcher {
					label: "CELLS"
					value: getStat("nbCells")
				}
				HorizontalSpacer {
					color: Qt.lighter(mainStats.color, 1.5)
				}
				ValueWatcher {
					label: "UPDATE"
					value: getStat("nbUpdates")
				}
			}
		}
	}

	Renderer {
		objectName: "renderer"
		id: renderer
		anchors.left: leftMenu.right
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		anchors.fill: parent
		focus: true
		z: -1
	}

}
