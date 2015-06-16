file(GLOB VIEWSRC 
	"*.h"
	"*.hpp"
	"*.cpp"
	)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Quick)
find_package(Qt5OpenGL)
qt5_add_resources(RESOURCES res.qrc)

add_executable(mecacellView ${VIEWSRC} ${RESOURCES})
qt5_use_modules(mecacellView Quick Core Gui OpenGL)
target_link_libraries(mecacellView MecaCell)
