file(GLOB SRC
	"*.hpp"
	"*.cpp"
	"../mecacell/*.h"
	"../mecacell/*.hpp"
	"../mecacell/*.cpp"
	)
add_executable(test ${SRC})
