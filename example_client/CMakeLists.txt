# This file is part of the MinVR3 cmake build system.  
# See the main ../CMakeLists.txt file for details.

project(force-server-example-client)


# Source:
set (SOURCEFILES
  main.cpp
)
set (HEADERFILES
)



# Define the target
add_executable(${PROJECT_NAME} ${HEADERFILES} ${SOURCEFILES})


# Add external dependency on libMinVR3:
find_package(MinVR3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC MinVR3::MinVR3)


# Installation:
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${INSTALL_BIN_DEST}
        COMPONENT Tests)


# For better organization when using an IDE with folder structures:
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Tests")
source_group("Header Files" FILES ${HEADERFILES})
