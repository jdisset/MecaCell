file(GLOB SRC
	"*.hpp"
	"*.cpp"
	"../mecacell/*.h"
	"../mecacell/*.hpp"
	"../mecacell/*.cpp"
	"../mecacell/utilities/*.h"
	"../mecacell/utilities/*.hpp"
	"../mecacell/utilities/*.cpp"
	"../mecacell/geometry/*.h"
	"../mecacell/geometry/*.hpp"
	"../mecacell/geometry/*.cpp"
	)
add_executable(test ${SRC})
