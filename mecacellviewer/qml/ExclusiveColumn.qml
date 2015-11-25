import QtQuick 2.0
import QtQuick.Controls 1.2
Rectangle{
	property string legend;
	property alias column: column 
	signal toggled(var o)
	color: "#25000000"
	width : parent.width - 10 
	anchors.left : parent.left
	anchors.leftMargin: 10 
	height : visible? column.height + sbtl.height:0
	onToggled:{
		for (var i = 0; i < column.children.length; i++){
			if (column.children[i] != o){
				column.children[i].checked = false;
				column.children[i].toggled();
			}
		}
		o.checked = true;
		o.toggled();
	}
	SubTitle{
		id : sbtl
		width:parent.width
		anchors.left: parent.left
		anchors.top: parent.top
		txt : parent.legend;
		height : 20
	}
	Column {
		id: column 
		anchors.top : sbtl.bottom
		anchors.left: parent.left
		width:parent.width
		visible: parent.visible
	}
	state: parent.visible ? "enabled" : "disabled"
	states: [
		State { name: "enabled"
		PropertyChanges {
			target: column 
			visible :true 
			height: childrenRect.height
		}
		PropertyChanges {
			visible :true 
		}
	},
	State { name: "disabled"
	PropertyChanges {
		target: column 
		visible : false
		height:0
	}
	PropertyChanges {
		visible : false
	}
}
]
transitions: [
	Transition {
		NumberAnimation { properties: "height" }
	}
]
}

