file(GLOB COREHEADERS
	"*.h"
	"*.hpp"
	"geometry/*.h"
	"geometry/*.hpp"
	"utilities/*.h"
	"utilities/*.hpp"
	"plugins/*.h"
	"plugins/*.hpp"
	)
file(GLOB CORESRC
	"*.cpp"
	"geometry/*.cpp"
	"plugins/*.cpp"
	"utilities/*.cpp"
	)
add_library(mecacell SHARED ${COREHEADERS} ${CORESRC})
install (TARGETS mecacell DESTINATION lib)
install (FILES ${COREHEADERS} DESTINATION include/mecacell)
