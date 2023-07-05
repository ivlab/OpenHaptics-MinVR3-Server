
#ifndef FORCE_SERVER_OPENHAPTICS_H
#define FORCE_SERVER_OPENHAPTICS_H

// OpenHaptics and OpenGL are tightly integrated.  This includes OpenGL plus four OpenHaptics libraries: hd, hl, hdu, hlu.

#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

// OpenGL includes
#if defined(WIN32) || defined(linux)
# include <GL/glut.h>
#elif defined(__APPLE__)
# include <GLUT/glut.h>
#endif

// HD and HL are closed source.  The library and headers are installed with the OpenHaptics library
#include <HD/hd.h>
#include <HL/hl.h>

// HDU and HLU are utility libraries.  The source and pre-built binaries are distributed with the OpenHaptics Library.  The binaries are quite old and may need to be rebuilt with a newer version of Visual Studio, e.g., in July 2023 -- using VS2019 worked fine.
#include <HDU/hdu.h>
#include <HDU/hdu.h>
#include <HDU/hduError.h>
#include <HDU/hduGenericMatrix.h>
#include <HDU/hduHapticDevice.h>
#include <HDU/hduLine.h>
#include <HDU/hduLineSegment.h>
#include <HDU/hduMath.h>
#include <HDU/hduMatrix.h>
#include <HDU/hduPlane.h>
#include <HDU/hduQuaternion.h>
#include <HDU/hduRecord.h>
#include <HDU/hduVector.h>

#include <HLU/hlu.h>

#endif
