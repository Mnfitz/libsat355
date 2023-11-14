#include <gtest/gtest.h>

#include "../src/libsat355.h"

TEST(libsat355, helloworld)
{
    int result = HelloWorld();
     ASSERT_NEAR(result, 0, 1.0e-11);
}

TEST(libsat355, orbit_to_lla)
{


    const char* in_tle1 = "ISS(ZARYA)";
    const char* in_tle2 = "1 25544U 98067A   23317.47667927  .00014185  00000+0  26255-3 0  9995";
    const char* in_tle3 = "2 25544  51.6432 309.0741 0001011 289.8192 206.7216 15.49283803424949";
    double out_tleage = 0.0;
    double out_latdegs = 0.0;
    double out_londegs = 0.0;
    double out_altkm = 0.0;
    std::time_t inTime = time(nullptr);

    int result = orbit_to_lla(in_tle1, in_tle2, in_tle3, &out_tleage, &out_latdegs, &out_londegs, &out_altkm, inTime);
    std::cout << "out_latdegs: " << out_latdegs << std::endl;
    std::cout << "out_londegs: " << out_londegs << std::endl;
    std::cout << "out_altkm: " << out_altkm << std::endl;
    ASSERT_NEAR(result, 0, 1.0e-11);
    /*
    
    3:26
    Lat: -65.44
    Lon: 34.16
    Alt: 421.31
    */
}