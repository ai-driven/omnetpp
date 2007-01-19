//==========================================================================
//  ENVDEFS.H - part of
//                     OMNeT++/OMNEST
//             Discrete System Simulation in C++
//
//  General defines for the Envir library
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __ENVDEFS_H
#define __ENVDEFS_H

#include "simkerneldefs.h"  // for OPP_DLLIMPORT, OPP_DLLEXPORT

// OPP_DLLIMPORT/EXPORT are empty if non-Windows, non-dll, etc.
#ifdef BUILDING_ENVIR
#  define ENVIR_API  OPP_DLLEXPORT
#else
#  define ENVIR_API  OPP_DLLIMPORT
#endif


#endif
