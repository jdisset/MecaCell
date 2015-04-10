file(GLOB CORESRC 
	"*.h"
	"*.hpp"
	"*.cpp"
	)
add_library(MecaCell ${CORESRC})
