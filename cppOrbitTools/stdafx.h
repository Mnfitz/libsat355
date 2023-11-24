#if WIN32
#include "orbitTools/core/stdafx.h"
#else
#define gmtime_s(x, y) (gmtime_r(y, x))
#define _get_timezone(x)
#define _snprintf_s (snprintf)

#include <stdio.h>
//#include <tchar.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <string>
#include <map>
using namespace std;
#endif