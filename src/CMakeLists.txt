find_package (ASSIMP REQUIRED)
find_package (OpenVF REQUIRED)

include_directories (${ASSIMP_INCLUDE_DIRS} ${OPENVF_INCLUDE_DIRS})

set (SOURCE_FILES main.cpp Scene.cpp Scene.h Node.cpp Node.h Arguments.cpp Arguments.h)
add_executable (assimp2vf ${SOURCE_FILES})

target_link_libraries (assimp2vf ${ASSIMP_LIBRARIES} ${OPENVF_LIBRARY})

install (TARGETS assimp2vf RUNTIME DESTINATION bin)

add_custom_target (run_install COMMAND ${CMAKE_MAKE_PROGRAM} install DEPENDS assimp2vf)
