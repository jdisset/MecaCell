import QtQuick 2.0
Rectangle{
	property int w;
	property real concentration;
	property string name;
	property int type;
	type : 2
	w : 70
	width : w 
	height : 15;
	Rectangle {
		id : full
		color : "steelblue"
		width : concentration*w
		height : parent.height
		anchors.left : parent.left
	}
	Rectangle {
		//id : empty 
		color : "#50111111"
		width : (1.0-concentration)*w
		height : parent.height
		anchors.left : full.right
	}
	Text {
		text: name 
		width : w - 2
		anchors.top: parent.top
		anchors.topMargin : 2
		anchors.left : parent.left
		anchors.leftMargin : 2
		font.family: "Helvetica"
		font.pointSize: 9 
		font.bold: true
		elide: Text.ElideRight
		color: "#BBFFFFFF"
	}
}
