import QtQuick 2.0
import QtQuick.Controls 1.2
LegendButton {
	id: moveTool
	group: toolGroup
	selecColor: bitDarker
	label: checked ? "\uf205" : "\u204"
	legend: "MOVE"
	anchors.verticalCenter: parent.verticalCenter
	onCheck: {
		checked = chk
		if (chk) setCtrl("tool", "move");
	}
}
