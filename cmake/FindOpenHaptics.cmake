
# HD Includes
find_path(HD_INCLUDE_DIR 
    HD/hd.h
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/include 
	C:/OpenHaptics/Developer/3.4.0/include
)

# HL Includes
find_path(HL_INCLUDE_DIR 
    HL/hl.h
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/include 
	C:/OpenHaptics/Developer/3.4.0/include
)

# HD Utilities Includes
find_path(HDU_INCLUDE_DIR 
    HDU/hdu.h
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/utilities/include 
	C:/OpenHaptics/Developer/3.4.0/utilities/include
)

# HL Utilities Includes
find_path(HLU_INCLUDE_DIR 
    HLU/hlu.h
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/utilities/include 
	C:/OpenHaptics/Developer/3.4.0/utilities/include
)


# HD Libraries
find_library(HD_OPT_LIBRARIES 
  NAMES 
    hd
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/lib/x64/Release
	C:/OpenHaptics/Developer/3.4.0/lib/x64/Release
)

find_library(HD_DEBUG_LIBRARIES 
  NAMES 
    hd
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/lib/x64/Debug
	C:/OpenHaptics/Developer/3.4.0/lib/x64/Debug
)

# HL Libraries
find_library(HL_OPT_LIBRARIES 
  NAMES 
    hl
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/lib/x64/Release
	C:/OpenHaptics/Developer/3.4.0/lib/x64/Release
)

find_library(HL_DEBUG_LIBRARIES 
  NAMES 
    hl
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/lib/x64/Debug
	C:/OpenHaptics/Developer/3.4.0/lib/x64/Debug
)

# HD Utilities Libraries
find_library(HDU_OPT_LIBRARIES 
  NAMES 
    hdu
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/utilities/lib/x64/Release
	C:/OpenHaptics/Developer/3.4.0/utilities/lib/x64/Release
)

find_library(HDU_DEBUG_LIBRARIES 
  NAMES 
    hdu
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/utilities/lib/x64/Debug
	C:/OpenHaptics/Developer/3.4.0/utilities/lib/x64/Debug
)

# HL Utilities Libraries
find_library(HLU_OPT_LIBRARIES 
  NAMES 
    hlu
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/utilities/lib/x64/Release
	C:/OpenHaptics/Developer/3.4.0/utilities/lib/x64/Release
)

find_library(HLU_DEBUG_LIBRARIES 
  NAMES 
    hlu
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/utilities/lib/x64/Debug
	C:/OpenHaptics/Developer/3.4.0/utilities/lib/x64/Debug
)

# GLUT Libraries
find_library(GLUT_OPT_LIBRARIES 
  NAMES 
    glut32
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/utilities/lib/x64/Release
	C:/OpenHaptics/Developer/3.4.0/utilities/lib/x64/Release
)

find_library(GLUT_DEBUG_LIBRARIES 
  NAMES 
    glut32
  HINTS 
    $ENV{OPENHAPTICS_ROOT}/utilities/lib/x64/Debug
	C:/OpenHaptics/Developer/3.4.0/utilities/lib/x64/Debug
)


# Combined set of includes for all of OpenHaptics
unset(OPENHAPTICS_INCLUDE_DIRS)
set(OPENHAPTICS_INCLUDE_DIRS
    ${HD_INCLUDE_DIR}
    ${HL_INCLUDE_DIR}
	${HDU_INCLUDE_DIR}
	${HLU_INCLUDE_DIR}
)


# Combined set of libraries for all of OpenHaptics
unset(OPENHAPTICS_LIBRARIES)
if (HD_OPT_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES optimized ${HD_OPT_LIBRARIES})
endif()
if (HD_DEBUG_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES debug ${HD_DEBUG_LIBRARIES})
endif()

if (HL_OPT_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES optimized ${HL_OPT_LIBRARIES})
endif()
if (HL_DEBUG_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES debug ${HL_DEBUG_LIBRARIES})
endif()

if (HDU_OPT_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES optimized ${HDU_OPT_LIBRARIES})
endif()
if (HDU_DEBUG_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES debug ${HDU_DEBUG_LIBRARIES})
endif()

if (HLU_OPT_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES optimized ${HLU_OPT_LIBRARIES})
endif()
if (HLU_DEBUG_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES debug ${HLU_DEBUG_LIBRARIES})
endif()

if (GLUT_OPT_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES optimized ${GLUT_OPT_LIBRARIES})
endif()
if (GLUT_DEBUG_LIBRARIES)
  list(APPEND OPENHAPTICS_LIBRARIES debug ${GLUT_DEBUG_LIBRARIES})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    OpenHaptics
    DEFAULT_MSG
    OPENHAPTICS_INCLUDE_DIRS
    OPENHAPTICS_LIBRARIES
)

mark_as_advanced(
	HD_INCLUDE_DIR
	HD_DEBUG_LIBRARIES
	HD_OPT_LIBRARIES
	HL_INCLUDE_DIR
	HL_DEBUG_LIBRARIES
	HL_OPT_LIBRARIES
	HDU_INCLUDE_DIR
	HDU_DEBUG_LIBRARIES
	HDU_OPT_LIBRARIES
	HLU_INCLUDE_DIR
	HLU_DEBUG_LIBRARIES
	HLU_OPT_LIBRARIES
    OPENHAPTICS_INCLUDE_DIR 
    OPENHAPTICS_INCLUDE_DIRS 
    OPENHAPTICS_OPT_LIBRARIES 
    OPENHAPTICS_DEBUG_LIBRARIES 
    OPENHAPTICS_LIBRARIES
)
