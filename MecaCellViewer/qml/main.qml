import SceneGraphRendering 1.0 
import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.3

Item{
	width  : 1200
	height : 750
	id : main

	Renderer {
		objectName     : "renderer"
		id             : renderer
		anchors.fill   : parent
		focus          : true
	}

	Text {
		id             : infos 
		anchors.bottom : parent.bottom
		anchors.right: parent.right
		anchors.bottomMargin: 10;
		anchors.rightMargin: 10;
		property real fps : 0;
		property real u : 0;
		property real nbc : 0;
		text:  nbc + " cells | " + u + " updates | " + fps.toFixed(1) + " fps"
		color: "grey"
		font.pointSize : 10 
	}
	Timer {
		interval: 1000/60; running: true; repeat: true
		onTriggered: {
			glview.callUpdate();
		}
	}
	Timer {
		interval: 100; running: true; repeat: true
		onTriggered: {
		}
	}
}
