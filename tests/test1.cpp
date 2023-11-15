#include <gtest/gtest.h>
#include <time.h>

#include "../src/libsat355.h"

TEST(libsat355, helloworld)
{
    int result = HelloWorld();
     ASSERT_NEAR(result, 0, 1.0e-11);
}

TEST(libsat355, orbit_to_lla)
{
    time_t now = time(0);
    time_t epoch = 0; 
    //Create a tm struct from epoch
    struct tm now_tm = *localtime(&now);
    struct tm epoch_tm = *gmtime(&epoch);

    now = mktime(&now_tm);
    epoch = mktime(&epoch_tm);

    double seconds = difftime(now, epoch);

    /*
    ISS (ZARYA)             
    1 25544U 98067A   23319.53465485  .00014606  00000+0  26950-3 0  9999
    2 25544  51.6431 298.8848 0000798 288.3336 174.2333 15.49347168425266
    */

    const char* in_tle1 = "ISS(ZARYA)";
    const char* in_tle2 = "1 25544U 98067A   23319.53465485  .00014606  00000+0  26950-3 0  9999";
    const char* in_tle3 = "2 25544  51.6431 298.8848 0000798 288.3336 174.2333 15.49347168425266";
    double out_tleage = 0.0;
    double out_latdegs = 0.0;
    double out_londegs = 0.0;
    double out_altkm = 0.0;
    //std::time_t inTime = time(nullptr);

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