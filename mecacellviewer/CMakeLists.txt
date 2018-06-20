set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Quick)
find_package(Qt5OpenGL)
find_package(Qt5QuickWidgets)
qt5_add_resources(RESOURCES resourcesLibMecacellViewer.qrc)

file(GLOB VIEWHEADERS
	"*.h"
	"*.hpp"
	)
file(GLOB PRIMHEADERS
	"primitives/*.h"
	"primitives/*.hpp"
	)
file(GLOB PLUGINSHEADERS
	"plugins/*.h"
	"plugins/*.hpp"
	)
file(GLOB MANAGERSHEADERS
	"managers/*.h"
	"managers/*.hpp"
	)
file(GLOB RENDERABLESHEADERS
	"renderables/*.h"
	"renderables/*.hpp"
	)
file(GLOB UTILITIESHEADERS
	"utilities/*.h"
	"utilities/*.hpp"
	)
file(GLOB MENUHEADERS
	"menu/*.h"
	"menu/*.hpp"
	)
file(GLOB VIEWSRC
	"*.cpp"
	"utilities/*.cpp"
	)
set(VIEWSRC
	${VIEWSRC}
	${RESOURCES}
	)
add_library(mecacellviewer SHARED ${VIEWSRC} ${VIEWHEADERS} ${PRIMHEADERS} ${PLUGINSHEADERS} ${MANAGERSHEADERS} ${RENDERABLESHEADERS} ${UTILITIESHEADERS} ${MENUHEADERS})
target_link_libraries(mecacellviewer Qt5::Quick Qt5::Core Qt5::Gui OpenGL)

install (TARGETS mecacellviewer DESTINATION lib)
install (FILES ${VIEWHEADERS} DESTINATION include/mecacell/viewer)
install (FILES ${PRIMHEADERS} DESTINATION include/mecacell/viewer/primitives)
install (FILES ${PLUGINSHEADERS} DESTINATION include/mecacell/viewer/plugins)
install (FILES ${MANAGERSHEADERS} DESTINATION include/mecacell/viewer/managers)
install (FILES ${UTILITIESHEADERS} DESTINATION include/mecacell/viewer/utilities)
install (FILES ${RENDERABLESHEADERS} DESTINATION include/mecacell/viewer/renderables)
install (FILES ${MENUHEADERS} DESTINATION include/mecacell/viewer/menu)
