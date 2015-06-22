import QtQuick 2.0

Rectangle {
	property real coef : 0.75
	color : "#40FFFFFF";
	height:0.5;
	width : parent.width * coef;
	anchors.horizontalCenter : parent.horizontalCenter
}

