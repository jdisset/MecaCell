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

	function addButton(id, menu, label, col) {
		if (btnArray[id] != undefined) {
			// update, not creation
			btnArray[id].text = label;
			btnArray[id].notpressedColor = col;
		} else {
			var container = getContainer(menu);
			var component;
			var finishCreation = function() {
				if (component.status == Component.Ready) {
					var btn = component.createObject(container, {
						"name": id,
						"text": label,
						"menu": menu,
						"notpressedColor": col
					});
					btnArray[id] = btn;
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
