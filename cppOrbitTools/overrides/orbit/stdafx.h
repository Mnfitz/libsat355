//
// stdafx.h
//
#pragma once

#if WIN32
#include "orbitTools/orbit/_stdafx.h"
#else
#define gmtime_s(x, y) (gmtime_r(y, x))
#define _get_timezone(x)
#define _snprintf_s (snprintf)

#include <stdio.h>
//#include <tchar.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "math.h"
#include "time.h"

#include "coreLib.h"

#include <string>
#include <map>
using namespace std;
#endif