	function fullscreenMode(obj, f) {
		console.log("fullscreen  = " + f);
		obj.leftMenu.visible = !f;
	}

	function getContainer(obj, menu) {
		var container = obj.controlsMenu.selectedCellActions;
		if (menu == "SELECTEDCELL_MENU") {
			container = obj.controlsMenu.selectedCellActions;
		}
		if (menu == "GENERALACTIONS_MENU") {
			container = obj.controlsMenu.generalActions;
		}
		return container;
	}

	function addButton(gui, id, menu, label, col) {
		if (gui.btnArray[id] != undefined) {
			// update, not creation
			gui.btnArray[id].text = label;
			gui.btnArray[id].notpressedColor = col;
		} else {
			var container = getContainer(gui, menu);
			var component;
			var finishCreation = function() {
				if (component.status == Component.Ready) {
					var btn = component.createObject(container, {
						"name": id,
						"text": label,
						"menu": menu,
						"notpressedColor": col
					});
					gui.btnArray[id] = btn;
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
	}

	function removeCtrl(gui, k) {
		if (gui.guictrl[k] != undefined) {
			delete gui.guictrl[k];
			gui.renderer.setGuiCtrl(guictrl);
		}
	}

	function setCtrl(gui, k, v) {
		gui.guictrl[k] = v;
		gui.renderer.setGuiCtrl(guictrl);
	}

	function removeOptionInCtrl(gui, k, v) {
		if (gui.guictrl[k] != undefined) {
			for (var i = 0; i < gui.guictrl[k].length; ++i) {
				if (gui.guictrl[k][i] == v) {
					gui.guictrl[k].splice(i, 1);
					break;
				}
			}
		}
		gui.renderer.setGuiCtrl(guictrl);
	}

	function pushUniqueOptionInCtrl(gui, k, v) {
		var exists = false;
		if (gui.guictrl[k] != undefined) {
			for (var i = 0; i < gui.guictrl[k].length; ++i) {
				if (gui.guictrl[k][i] == v) {
					exists = true;
				}
			}
		} else {
			gui.guictrl[k] = new Array();
		}
		if (!exists) {
			gui.guictrl[k].push(v);
			gui.renderer.setGuiCtrl(guictrl);
		}
	}

	function statAvail(gui, k) {
		return gui.renderer.stats[k] ? true : false;
	}

	function getStat(gui, k) {
		return gui.renderer.stats[k] ? gui.renderer.stats[k] : 0;
	}
