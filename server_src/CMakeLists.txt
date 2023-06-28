# This file is part of the MinVR3 cmake build system.  
# See the main ../CMakeLists.txt file for details.


# Source:
set (SOURCEFILES
  server_src/ambient_friction.cpp
  server_src/ambient_viscous.cpp
  server_src/event_mgr.cpp
  server_src/force_effect.h
  server_src/graphics.cpp
  server_src/line_constraint.cpp
  server_src/point_constraint.cpp
  server_src/surface_constraint.cpp
  server_src/touch_surface.cpp
  server_src/main.cpp
  server_src/phantom.cpp
)

set (HEADERFILES
  server_src/ambient_friction.h
  server_src/ambient_viscous.h
  server_src/event_mgr.h
  server_src/force_effect.h
  server_src/force_messages.h
  server_src/graphics.h
  server_src/graphics_headers.h
  server_src/line_constraint.h
  server_src/point_constraint.h
  server_src/surface_constraint.h
  server_src/touch_surface.h
  server_src/phantom.h
)



# Define the target
add_executable(${PROJECT_NAME} ${HEADERFILES} ${SOURCEFILES})


# Add dependency on libMinVR3:
find_package(MinVR3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC MinVR3::MinVR3)

# Add dependency on OpenHaptics
find_package(OpenHaptics REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENHAPTICS_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENHAPTICS_INCLUDE_DIRS})



# Installation:
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${INSTALL_BIN_DEST}
        COMPONENT Server)


# For better organization when using an IDE with folder structures:
source_group("Header Files" FILES ${HEADERFILES})
