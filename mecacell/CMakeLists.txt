file(GLOB COREHEADERS
	"*.h"
	"*.hpp"
	)
file(GLOB GEOMETRYHEADERS
	"geometry/*.h"
	"geometry/*.hpp"
	)
file(GLOB UTILITIESHEADERS
	"utilities/*.h"
	"utilities/*.hpp"
	)
file(GLOB PLUGINSHEADERS
	"plugins/*.h"
	"plugins/*.hpp"
	)
file(GLOB EXTERNALHEADERS
	"utilities/external/*.h"
	"utilities/external/*.hpp"
	)
file(GLOB SRC
	"*.cpp"
	"geometry/*.cpp"
	"plugins/*.cpp"
	"utilities/*.cpp"
	)

add_library(mecacell SHARED ${COREHEADERS} ${GEOMETRYHEADERS} ${PLUGINSHEADERS} ${UTILITIESHEADERS} ${EXTERNALHEADERS} ${SRC})
install (TARGETS mecacell DESTINATION lib)
install (FILES ${COREHEADERS} DESTINATION include/mecacell)
install (FILES ${GEOMETRYHEADERS} DESTINATION include/mecacell/geometry)
install (FILES ${PLUGINSHEADERS} DESTINATION include/mecacell/plugins)
install (FILES ${UTILITIESHEADERS} DESTINATION include/mecacell/utilities)
install (FILES ${EXTERNALHEADERS} DESTINATION include/mecacell/utilities/external)
