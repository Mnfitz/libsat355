// self
#include "libsat355.h"

// std

#include <iostream>

#if (!WIN32)
#define gmtime_s(x, y) (gmtime_r(y, x))
#define _get_timezone(x)
#define _snprintf_s (snprintf)
#endif // WIN32

// orbittools
// *NOTE: Large portions of this was stolen from orbittools demo
// "coreLib.h" includes basic types from the core library,
// such as cSite, cJulian, etc. The header file also contains a
// "using namespace" statement for Zeptomoby::OrbitTools.
// C:\Users\mn-fi\Projects\git\libsat355\cppOrbitTools\orbitTools\core\coreLib.h
#include "coreLib.h"
// "orbitLib.h" includes basic types from the orbit library,
// including cOrbit.
#include "orbitLib.h"

// IOS Core Foundation: Date::init(timeIntervalSinceReferenceDate: TimeInterval)
const double EPOCH_JAN1_00H_2001 = 2451910.5; // Jan  1.0 2001 = Jan  1 2001 00h UTC

// TRICKY: extern "C"- Make functions callable from SwiftUI.
// Force orbit_to_lla() to be "C" rather than "C++" function.
// Needed because SwiftUI binding header can only call into "C".

extern "C" {

int HelloWorld()
{
	std::cout << "Hello World!" << std::endl;

	return kOK;
}

// orbit_to_lla:
// Calculate satellite Lat/Lon/Alt for time "now" using
// input TLE-format orbital data
int orbit_to_lla(	long long 	in_time,	// time in seconds since 1970
					const char* in_tle1,	// TLE (Sat Name)
					const char* in_tle2,	// TLE line 1
					const char* in_tle3,	// TLE line 2

					double* out_tleage,		// age of TLE in secs since: Jan 1, 2001 00h UTC
					double* out_latdegs,	// latitude in degs
					double* out_londegs,	// longitude in degs
					double* out_altkm)		// altitude in km
{
	try
	{
		// Create a TLE object using the data above
		cTle tleSGP4(in_tle1, in_tle2, in_tle3);

		// Create a satellite object from the TLE object
		cSatellite satSGP4(tleSGP4);

		// Get the Julian Date for GMT "now"
		// time '0' is 1970-01-01 00:00:00 UTC, AKA the Unix epoch
		time_t epoch = 0; 
		//Create a tm struct from epoch
    	struct tm epoch_tm{};

		(void) gmtime_s(&epoch_tm, &epoch);

		// Add in_time seconds to epoch_tm to get inputted time in time_t format
		epoch_tm.tm_sec += in_time;
		time_t now = mktime(&epoch_tm);

		//const std::time_t now = std::time(nullptr);
		cJulian jdNow(now);

		// Get Earth-Centered-Interial position of satellite for time: now
		cEciTime eciSGP4 = satSGP4.PositionEci(jdNow);
		// Convert the ECI to geocentric coordinates
		cGeo geo(eciSGP4, eciSGP4.Date());

		const double altkm = geo.AltitudeKm();
		// Latitude correctly indicates S)outh using negative values
		const double latdeg = geo.LatitudeDeg();
		// Longitude indicates W)est using positives values > 180.0
		double londeg = geo.LongitudeDeg();
		// Convert W)est into negative values for googlemaps compatibility
		if (londeg > 180.0)
		{
			londeg -= 360.0;
		}

		int epochYear = (int) tleSGP4.GetField(cTle::FLD_EPOCHYEAR);
		double epochDay = tleSGP4.GetField(cTle::FLD_EPOCHDAY);
		if (epochYear < 57)
		{
			epochYear += 2000;
		}
		else
		{
			epochYear += 1900;
		}

		cJulian jdEpoch(epochYear, epochDay);
		const double tleage = (jdEpoch.Date() - EPOCH_JAN1_00H_2001) * SEC_PER_DAY;

		// Return calculated values
		*out_tleage = tleage;
		*out_latdegs = latdeg;
		*out_londegs = londeg;
		*out_altkm = altkm;

		return kOK;
	}
    catch (...)
    {
        // Some unknown excption was thrown
        std::cerr << "Unexpected exception encountered.\n";

		return kInternalError;
    }
}

// orbit2lla:
// Calculate satellite Lat/Lon/Alt for time "now" using
// input TLE-format orbital data
int orbit_to_lla2( 	long long   in_time,	// time in seconds since 1970
					const char* in_tle1,	// TLE (Sat Name)
					const char* in_tle2,	// TLE line 1
					const char* in_tle3,	// TLE line 2
					
					double in_gpslat,	// my GPS latitude in degs 
					double in_gpslon,	// my GPS longitude in degs
					double in_gpsalt,	// my GPS altitude in km
					double* out_tleage,	// age of TLE in secs since: Jan 1, 2001 00h UTC
					double* out_latdegs,	// latitude in degs
					double* out_londegs,	// longitude in degs
					double* out_altkm,	// altitude in km
					double* out_azdegs,	// look angle azimuth in degs
					double* out_eledegs)	// look angle elevation in degs
{
	try
	{
		// Create a TLE object using the data above
		cTle tleSGP4(in_tle1, in_tle2, in_tle3);

		// Create a satellite object from the TLE object
		cSatellite satSGP4(tleSGP4);

		// Get the Julian Date for GMT "now"
		// Get the Julian Date for GMT "now"
		// time '0' is 1970-01-01 00:00:00 UTC, AKA the Unix epoch
		time_t epoch = 0; 
		//Create a tm struct from epoch
    	struct tm epoch_tm{};

		(void) gmtime_s(&epoch_tm, &epoch);
		
		// Add in_time seconds to epoch_tm to get inputted time in time_t format
		epoch_tm.tm_sec += in_time;
		time_t now = mktime(&epoch_tm);
		cJulian jdNow(now);

		// Get Earth-Centered-Interial position of satellite for time: now
		cEciTime eciSGP4 = satSGP4.PositionEci(jdNow);
		// Convert the ECI to geocentric coordinates
		cGeo geo(eciSGP4, eciSGP4.Date());

		const double altkm = geo.AltitudeKm();
		// Latitude correctly indicates S)outh using negative values
		const double latdeg = geo.LatitudeDeg();
		// Longitude indicates W)est using positives values > 180.0
		double londeg = geo.LongitudeDeg();
		// Convert W)est into negative values for googlemaps compatibility
		if (londeg > 180.0)
		{
			londeg -= 360.0;
		}

		// Now create a site object. Site objects represent a location on the 
		// surface of the earth. Here we arbitrarily select a point using
		// provided GPS coords
		cSite siteGPS(in_gpslat, in_gpslon, in_gpsalt);

		// Now get the "look angle" from the site to the satellite. 
		// Note that the ECI object "eciSGP4" contains a time associated
		// with the coordinates it contains; this is the time at which
		// the look angle is valid.
		cTopo topoGPS = siteGPS.GetLookAngle(eciSGP4);

		const double azdeg = topoGPS.AzimuthDeg();
		const double eledeg = topoGPS.ElevationDeg();

		int epochYear = (int)tleSGP4.GetField(cTle::FLD_EPOCHYEAR);
		double epochDay = tleSGP4.GetField(cTle::FLD_EPOCHDAY);
		if (epochYear < 57)
		{
			epochYear += 2000;
		}
		else
		{
			epochYear += 1900;
		}

		cJulian jdEpoch(epochYear, epochDay);
		const double tleage = (jdEpoch.Date() - EPOCH_JAN1_00H_2001) * SEC_PER_DAY;

		// Return calculated values
		*out_tleage = tleage;
		*out_latdegs = latdeg;
		*out_londegs = londeg;
		*out_altkm = altkm;

		*out_azdegs = azdeg;
		*out_eledegs = eledeg;

		return kOK;
	}
    catch (...)
    {
        // Some unknown excption was thrown
        std::cerr << "Unexpected exception encountered.\n";

		return kInternalError;
    }
}

} // extern "C"