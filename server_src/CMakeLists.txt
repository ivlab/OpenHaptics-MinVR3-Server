# This file is part of the MinVR3 cmake build system.  
# See the main ../CMakeLists.txt file for details.


# Source:
set (SOURCEFILES
  server_src/main.cpp
)
set (HEADERFILES
)



# Define the target
add_executable(${PROJECT_NAME} ${HEADERFILES} ${SOURCEFILES})


# Add dependency on libMinVR3:
find_package(MinVR3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC MinVR3::MinVR3)


# Installation:
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${INSTALL_BIN_DEST}
        COMPONENT Server)


# For better organization when using an IDE with folder structures:
source_group("Header Files" FILES ${HEADERFILES})
