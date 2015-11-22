import QtQuick 2.0
import QtQuick.Controls 1.2
Item {
	property string legend;
	width : parent.width
	height: childrenRect.height + 20
	property alias column: column 
	VerticalSpacer {}
	SubTitle {
		txt: legend;
	}
	Column {
		id: column 
		anchors.top: parent.top
		anchors.topMargin: 20
		anchors.left: parent.left
		anchors.leftMargin: 25
		spacing: 8 
	}
}

