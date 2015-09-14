import QtQuick 2.0
import QtQuick.Controls 1.2
Item {
	property string txt
	anchors.horizontalCenter : parent.horizontalCenter
	height : 35
	Text{
		font.family: oswald.name
		font.pointSize: 15
		color : "white"
		text : txt
		anchors.horizontalCenter : parent.horizontalCenter
		height : parent.height
	}
}
