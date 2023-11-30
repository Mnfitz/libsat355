//
// orbitLib.h
//
// Copyright (c) 2014 Michael F. Henry
// Version 06/2014
//
#if WIN32
#include "orbitTools/orbit/_orbitLib.h"
#else
#include <stdio.h>
//#include <tchar.h>

#include <assert.h>

using namespace std;

#include "cOrbit.h"
#include "cSatellite.h"

using namespace Zeptomoby::OrbitTools;
#endif