	function fullscreenMode(obj, f) {
		console.log("fullscreen  = " + f);
		obj.leftMenu.visible = !f;
	}

	function getContainer(menu) {
		var container = controlsMenu.generalActions;
		return container;
	}

	function isEmpty(str) {
		return (!str || 0 === str.length);
	}

	function forEach(obj, callback) {
		for (var key in menu) {
			if (validation_messages.hasOwnProperty(key)) {
				var obj = validation_messages[key];
				for (var prop in obj) {
					if (obj.hasOwnProperty(prop)) {
						func(key, obj);
					}
				}
			}
		}
	}

	function createNewComponent(file, container, obj) {
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

	var MenuElement = function(n, t) {
		this.name = n;
		this.type = t;
		this.qmlItem = new Object();
		this.children = new Array();
		this.parent = null;
	}

	var displayMenuContainer = new MenuElement("root", "checkable");

	var elemToggled = function(elem) {
		return function() {
			// on construit l'arborescence qui mène jusqu'à cet élément
			// et on s'arrête si jamais on trouve un élément non checked
			var p = elem.parent;
			var arbo = new Array();
			arbo.push(elem);
			do {
				arbo.push(p);
				p = p.parent;
			} while (p != null);
			if (arbo.length > 0 && arbo[arbo.length - 1].name == "root") {
				//on est bien remonté jusqu'à la racine
				// on rempile dans l'ordre
				var res = new Array();
				for (var i = arbo.length - 2; i >= 0; i--) {
					res.push(arbo[i].name);
				}
				renderer.displayMenuElementToggled(res, elem.qmlItem.checked);
			}
		}
	};

	function createMenuElement(elems, parent) {

		for (var i = 0; i < elems.length; ++i) {
			// we first create our new Elem
			var newElem = new MenuElement(elems[i].name, elems[i].type);
			newElem.parent = parent;
			var toggledFunc = function() {}
			if (elems[i].type === "exclusiveGroup") {
				newElem.qmlItem = createNewComponent("ExclusiveColumn.qml", parent.qmlItem.column, {
					"legend": newElem.name
				});
			} else if (elems[i].type === "checkable") {
				newElem.qmlItem = createNewComponent("CheckableButton.qml", parent.qmlItem.column, {
					"legend": newElem.name,
					"checked": elems[i].checked,
					"group": parent && parent.type === "checkable" ? null : parent.qmlItem,
					"toggledFunc": elemToggled(newElem)
				})
			}
			// we continue to unrol the list of elements
			createMenuElement(elems[i].elems, newElem);
			// we add it to its parent's children
			parent.children.push(newElem);
		}
	}

	function createDisplayMenu(menujs) {
		displayMenuContainer.qmlItem = displayMenu;
		var elems = new Array();
		elems.push(JSON.parse(menujs));
		createMenuElement(elems, displayMenuContainer);
	}


	function addPaintStepComponent(name, category, subgroup, isChecked) {
		if (paintStepsCategories[category] == undefined) {
			paintStepsCategories[category] = createNewComponent("ColumnItem.qml", displayMenu.mainColumn, {
				"legend": category
			});
		}
		var container = paintStepsCategories[category].column;
		if (!isEmpty(subgroup)) {
			//create an exclusivegroup
			if (paintStepsSubgroups[subgroup] == undefined) {
				paintStepsSubgroups[subgroup] = createNewComponent("ExclusiveColumn.qml", container, {
					"legend": subgroup
				})
			}
			container = paintStepsSubgroups[subgroup].column;
		}
		var id = category + name;
		if (paintStepsElem[id] != undefined) {
			paintStepsElem[id].legend = name;
			paintStepsElem[id].checked = isChecked;
		} else {
			var hash = category + subgroup + name;
			if (isChecked) pushUniqueOptionInCtrl("enabledPaintSteps", hash);
			paintStepsElem[id] = createNewComponent("CheckableButton.qml", container, {
				"legend": name,
				"checked": isChecked,
				"group": isEmpty(subgroup) ? null : paintStepsSubgroups[subgroup],
				"toggledFunc": function() {
					if (this.checked) pushUniqueOptionInCtrl("enabledPaintSteps", hash)
					else removeOptionInCtrl("enabledPaintSteps", hash)
				}
			})
		}
	}


	function addButton(id, menu, label, col) {
		if (btnArray[id] != undefined) {
			btnArray[id].text = label;
			btnArray[id].notpressedColor = col;
		} else {
			console.log("Adding button");
			btnArray[id] = createNewComponent("MVButton.qml", getContainer(menu), {
				"name": id,
				"text": label,
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
