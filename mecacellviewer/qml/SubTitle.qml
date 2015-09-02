import QtQuick 2.0
import QtQuick.Controls 1.2
Item {
	property string txt
	anchors.horizontalCenter : parent.horizontalCenter
	height : 20
	width : parent.width
	Text{
		font.family: opensans.name
		font.pointSize: 10
		color : "white"
		text : txt
		anchors.horizontalCenter : parent.horizontalCenter
	}
}

