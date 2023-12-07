#ifndef LIBSAT355_H
#define LIBSAT355_H

#include <stdio.h>
#include <time.h>

#include <cassert>
#include <string_view>
#include <stdexcept>
#include <string>
#include <string_view>

#include "dllmain.h"

// TRICKY: Make sure only C++ compiles this.
// When compiled from C or SwiftUI, it is excluded
#ifdef __cplusplus
DLL_EXPORT int HelloWorld2(int val1, double val2);
extern "C" {
#endif

enum ErrorCode
{
    kOK = 0,
    kInvalidTLE,
    kInvalidTime,
    kInternalError
};

DLL_EXPORT int HelloWorld();

// orbit_to_lla:
// Calculate satellite Lat/Lon/Alt for time "now" using
// input TLE-format orbital data
DLL_EXPORT int  orbit_to_lla(	
                    long long   in_time,	// time in seconds since 1970
					const char* in_tle1,	// TLE (Sat Name)
					const char* in_tle2,	// TLE line 1
					const char* in_tle3,	// TLE line 2

					double* out_tleage,		// age of TLE in secs since: Jan 1, 2001 00h UTC
					double* out_latdegs,	// latitude in degs
					double* out_londegs,	// longitude in degs
					double* out_altkm);		// altitude in km

// orbit_to_lla2:
// Calculate satellite Lat/Lon/Alt plus look-angles
// for time "now" using input TLE-format orbital data
DLL_EXPORT int orbit_to_lla2(	
					long long   in_time,	// time in seconds since 1970
                    const char* in_tle1,	// TLE (Sat Name)
					const char* in_tle2,	// TLE line 1
					const char* in_tle3,	// TLE line 2
					double in_gpslat,		// my GPS latitude in degs 
					double in_gpslon,		// my GPS longitude in degs
					double in_gpsalt,		// my GPS altitude in km
					double* out_tleage,		// age of TLE in secs since: Jan 1, 2001 00h UTC
					double* out_latdegs,	// latitude in degs
					double* out_londegs,	// longitude in degs
					double* out_altkm,		// altitude in km
					double* out_azdegs,		// look angle azimuth in degs
					double* out_eledegs);	// look angle elevation in degs

// TLE helper functions
struct TLE;
typedef struct TLE TLE;

int TLE_Make(TLE** outTLE);

int TLE_Delete(TLE* inTLE);

int TLE_GetName(const TLE* inTLE, const char* outName[]);
int TLE_SetName(TLE* ioTLE, const char inName[]);

int TLE_GetLine1(const TLE* inTLE, const char* outLine1[]);
int TLE_SetLine1(TLE* ioTLE, const char inLine1[]);

int TLE_GetLine2(const TLE* inTLE, const char* outLine2[]);
int TLE_SetLine2(TLE* ioTLE, const char inLine2[]);

#ifdef __cplusplus
} // extern "C"

namespace sat355 {

class exception : public std::runtime_error
{
public:
	explicit exception(const std::string& what_arg) : std::runtime_error(what_arg) {}
	explicit exception(const char* what_arg) : std::runtime_error(what_arg) {}
};

class TLE
{
public:
	TLE()
	{
		const int errCode = TLE_Make(&mTLE);
		if (errCode != kOK)
		{
			throw exception("TLE_Make failed");
		}
	}

	TLE(const std::string& inName, const std::string& inLine1, const std::string& inLine2) : 
		TLE{} // Delegate to default constructor
	{
		SetName(inName);
		SetLine1(inLine1);
		SetLine2(inLine2);
	}

	~TLE()
	{
		const int errCode = TLE_Delete(mTLE);
		if (errCode != kOK)
		{
			// C++ exceptions should not be thrown from destructors
			// In release builds, we just ignore any exceptions
			assert(!"TLE_Delete failed");
		}
	}

	std::string_view GetName() const
	{
		const char* name = nullptr;
		int errCode = TLE_GetName(mTLE, &name);
		if (errCode != kOK)
		{
			throw exception("GetName failed");
		}
		return std::string_view(name);
	}
	void SetName(const std::string& inName)
	{
		const char* name = inName.c_str();
		int errCode = TLE_SetName(mTLE, name);
		if (errCode != kOK)
		{
			throw exception("SetName failed");
		}
	}

	std::string_view GetLine1() const
	{
		const char* line1 = nullptr;
		int errCode = TLE_GetLine1(mTLE, &line1);
		if (errCode != kOK)
		{
			throw exception("GetLine1 failed");
		}
		return std::string_view(line1);
	}
	void SetLine1(const std::string& inLine1)
	{
		const char* line1 = inLine1.c_str();
		int errCode = TLE_SetLine1(mTLE, line1);
		if (errCode != kOK)
		{
			throw exception("SetLine1 failed");
		}
	}

	std::string_view GetLine2() const
	{
		const char* line2 = nullptr;
		int errCode = TLE_GetLine2(mTLE, &line2);
		if (errCode != kOK)
		{
			throw exception("GetLine2 failed");
		}
		return std::string_view(line2);
	}
	void SetLine2(const std::string& inLine2)
	{
		const char* line2 = inLine2.c_str();
		int errCode = TLE_SetLine2(mTLE, line2);
		if (errCode != kOK)
		{
			throw exception("SetLine2 failed");
		}
	}

private:
	::TLE* mTLE{nullptr};
};

} // namespace sat355

#endif 

#endif // LIBSAT355_H