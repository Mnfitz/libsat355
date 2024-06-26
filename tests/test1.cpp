#include <gtest/gtest.h>
#include <ctime>

#include "libsat355.h"

TEST(libsat355, TLE)
{
    const char* in_tle1 = "ISS(ZARYA)";
    const char* in_tle2 = "1 25544U 98067A   23320.50172660  .00012336  00000+0  22877-3 0  9990";
    const char* in_tle3 = "2 25544  51.6432 294.0998 0000823 293.3188 166.8114 15.49366195425413";
    sat355::TLE tle(in_tle1, in_tle2, in_tle3);
    ASSERT_EQ(tle.GetName(), in_tle1);
}

TEST(libsat355, TLE_GetMeanMotion)
{
    const char* in_tle1 = "ISS(ZARYA)";
    const char* in_tle2 = "1 25544U 98067A   23320.50172660  .00012336  00000+0  22877-3 0  9990";
    const char* in_tle3 = "2 25544  51.6432 294.0998 0000823 293.3188 166.8114 15.49366195425413";
    sat355::TLE tle(in_tle1, in_tle2, in_tle3);
    ASSERT_EQ(tle.GetMeanMotion(), 15.49366195425413);
}

TEST(libsat355, TLE_GetInclination)
{
    const char* in_tle1 = "ISS(ZARYA)";
    const char* in_tle2 = "1 25544U 98067A   23320.50172660  .00012336  00000+0  22877-3 0  9990";
    const char* in_tle3 = "2 25544  51.6432 294.0998 0000823 293.3188 166.8114 15.49366195425413";
    sat355::TLE tle(in_tle1, in_tle2, in_tle3);
    ASSERT_EQ(tle.GetInclination(), 51.6432);
}

TEST(libsat355, orbit_to_lla)
{
    std::time_t epoch = 0; 
    std::tm epoch_tm = *std::gmtime(&epoch);
    epoch = std::mktime(&epoch_tm);

// Test for current time, or on 11:00 AM PST on 11/16/2023
#if 1
    // Current Time
    std::time_t test = std::time(0);
    std::tm test_tm = *std::localtime(&test);
    test = std::mktime(&test_tm);
#else
    // 11:00 AM PST on 11/16/2023
    std::time_t test = std::time(0);
    std::tm test_tm = localtime_s( &test);
    test_tm.tm_year = 2023 - 1900;
    test_tm.tm_mon = 11 - 1;
    test_tm.tm_mday = 16;
    test_tm.tm_hour = 11;
    test_tm.tm_min = 0;
    test_tm.tm_sec = 0;

    test = std::mktime(&test_tm);

    // ISS (ZARYA) Coords
    // Lat: -51.0321
    // Lon: -124.568
    // Alt: 435.506

#endif

    long long seconds = static_cast<long long>(std::difftime(test, epoch));

    /*
    ISS (ZARYA)             
    1 25544U 98067A   23319.53465485  .00014606  00000+0  26950-3 0  9999
    2 25544  51.6431 298.8848 0000798 288.3336 174.2333 15.49347168425266
    */

    const char* in_tle1 = "ISS(ZARYA)";
    const char* in_tle2 = "1 25544U 98067A   23320.50172660  .00012336  00000+0  22877-3 0  9990";
    const char* in_tle3 = "2 25544  51.6432 294.0998 0000823 293.3188 166.8114 15.49366195425413";
    double out_tleage = 0.0;
    double out_latdegs = 0.0;
    double out_londegs = 0.0;
    double out_altkm = 0.0;
    //std::time_t inTime = std::time(nullptr);

    int result = orbit_to_lla(seconds, in_tle1, in_tle2, in_tle3, &out_tleage, &out_latdegs, &out_londegs, &out_altkm);
    std::cout << "out_latdegs: " << out_latdegs << std::endl;
    std::cout << "out_londegs: " << out_londegs << std::endl;
    std::cout << "out_altkm: " << out_altkm << std::endl;
    std::cout << "seconds: " << seconds << std::endl;
    ASSERT_NEAR(result, 0, 1.0e-11);
    /*
    
    3:26
    Lat: -65.44
    Lon: 34.16
    Alt: 421.31
    */
}