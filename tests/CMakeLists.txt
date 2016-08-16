file(GLOB SRC
	"*.hpp"
	"*.cpp"
	"../mecacell/*.h"
	"../mecacell/*.hpp"
	"../mecacell/*.cpp"
	"../mecacell/geometry/*.h"
	"../mecacell/geometry/*.hpp"
	"../mecacell/geometry/*.cpp"
	"../mecacell/utilities/*.h"
	"../mecacell/utilities/*.hpp"
	"../mecacell/utilities/*.cpp"
	"../mecacell/plugins/*.h"
	"../mecacell/plugins/*.hpp"
	"../mecacell/plugins/*.cpp"
	)
add_executable(test ${SRC})
