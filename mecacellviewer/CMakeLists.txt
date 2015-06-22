file(GLOB VIEWHEADERS 
	"*.h"
	"*.hpp"
	)
file(GLOB PRIMHEADERS 
	"primitives/*.h"
	"primitives/*.hpp"
	)
file(GLOB VIEWSRC 
	"*.cpp"
	)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Quick)
find_package(Qt5OpenGL)
add_library(mecacellviewer SHARED ${VIEWSRC} ${VIEWHEADERS} ${PRIMHEADERS})
qt5_add_resources(RESOURCES resourcesLibMecacellViewer.qrc)
qt5_use_modules(mecacellviewer Quick Core Gui OpenGL)

install (TARGETS mecacellviewer DESTINATION lib)
install (FILES ${VIEWHEADERS} DESTINATION include/mecacell/viewer)
install (FILES ${PRIMHEADERS} DESTINATION include/mecacell/viewer/primitives)
message(${RESOURCES})
