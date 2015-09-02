import QtQuick 2.0
import QtQuick.Controls 1.2
Item {
	id : menuGroup 
	property var objectsInGroup :[]
	signal toggled(var o)
	onToggled:{
		for (var i = 0 ; i < objectsInGroup.length ; ++i){
			if (objectsInGroup[i] != o)
			objectsInGroup[i].check(false)
		}
		o.check(true);
	}
}

