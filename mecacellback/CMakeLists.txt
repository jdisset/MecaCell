file(GLOB COREHEADERS 
	"*.h"
	"*.hpp"
	)
file(GLOB CORESRC
	"*.cpp"
	)

add_library(mecacell SHARED ${CORESRC} ${COREHEADERS})
install (TARGETS mecacell DESTINATION lib)
install (FILES ${COREHEADERS} DESTINATION include/mecacell)
