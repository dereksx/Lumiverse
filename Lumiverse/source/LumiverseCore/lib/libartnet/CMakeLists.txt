INCLUDE (CheckIncludeFiles)

include_directories("${CMAKE_CURRENT_LIST_DIR}")

SET (LIBARTNET_HAVE_CONFIG "")

IF (NOT WIN32)
	CHECK_INCLUDE_FILES(endian.h HAVE_ENDIAN_H)

	SET (LIBARTNET_HAVE_CONFIG "#define HAVE_CONFIG_H")

	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
ENDIF (NOT WIN32)

IF (UNIX OR LINUX)
	SET(CMAKE_C_FLAGS "-fPIC")
  MESSAGE("Detected platform Unix: compiling libartnet with -fPIC")
ENDIF (UNIX OR LINUX)
	
add_library(libartnet
	artnet.c
	misc.c
	network.c
	receive.c
	tod.c
	transmit.c
)

FILE(GLOB artnetfiles ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
INSTALL(FILES ${artnetfiles} DESTINATION Debug/include/lib/libartnet CONFIGURATIONS Debug)
INSTALL(FILES ${artnetfiles} DESTINATION Release/include/lib/libartnet CONFIGURATIONS Release)

INSTALL(TARGETS libartnet ARCHIVE DESTINATION Debug/lib CONFIGURATIONS Debug)
INSTALL(TARGETS libartnet ARCHIVE DESTINATION Release/lib CONFIGURATIONS Release)