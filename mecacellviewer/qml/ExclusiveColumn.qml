import QtQuick 2.0
import QtQuick.Controls 1.2
Rectangle{
	property string legend;
	property alias column: column 
	signal toggled(var o)
	color: "#25000000"
	width : 160 
	height: childrenRect.height +15 
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
		anchors.top: column.top
		anchors.topMargin:-15
		txt : parent.legend;
	}
	Column {
		id: column 
		anchors.topMargin:20
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.leftMargin: 10 
		//spacing: 5 
	}
}

