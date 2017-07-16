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
add_executable(mecacell_test ${SRC})
target_link_libraries(mecacell_test pthread)
