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


	function createNewComponent(file, container, obj) {
		console.log("asked to create obj " + obj);
		console.log(" in container " + container);
		var component;
		var elem;
		var finishCreation = function() {
			if (component.status == Component.Ready) {
				elem = component.createObject(container, obj);
			} else if (component.status == Component.Error) {
				console.log("Error loading component:", component.errorString());
			}
		};
		component = Qt.createComponent(file);
		if (component.status == Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
		return elem;
	}


	function addPaintStepComponent(name, category, isChecked) {
		if (paintStepsCategories[category] == undefined) {
			paintStepsCategories[category] = createNewComponent("ColumnItem.qml", displayMenu.mainColumn, {
				"legend": category
			});
		}
		var id = category + name;
		if (paintStepsElem[id] != undefined) {
			paintStepsElem[id].legend = name;
			paintStepsElem[id].checked = isChecked;
		} else {
			paintStepsElem[id] = createNewComponent("CheckableButton.qml", paintStepsCategories[category].column, {
				"legend": name,
				"checked": isChecked,
				"onToggled": function() {
					if (elem.checked) pushUniqueOptionInCtrl("visibleElements", elem.legend)
					else removeOptionInCtrl("visibleElements", elem.legend)
				}
			})
		}
	}

	function addButton(id, menu, label, col) {
		if (btnArray[id] != undefined) {
			btnArray[id].text = label;
			btnArray[id].notpressedColor = col;
		} else {
			btnArray[id] = createNewComponent("MVButton.qml", getContainer(menu), {
				"name": id,
				"text": label,
				"menu": menu,
				"notpressedColor": col
			});
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
