// Self
#include "libsat355.h"
#include "app355.h"

// std
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <list>
#include <tuple>
#include <cmath>

class OrbitalData
{
private:
    std::string mName;
    double mLatitude;
    double mLongitude;
    double mAltitude;

public:
    OrbitalData(std::string name, double latitude, double longitude, double altitude)
        : mName(name), mLatitude(latitude), mLongitude(longitude), mAltitude(altitude)
    {
        // Nothing to do here
    }

    OrbitalData() = default;

    ~OrbitalData() = default;

    std::string GetName() const 
    { 
        return mName; 
    }
    void SetName(std::string name) 
    { 
        mName = name; 
    }

    double GetLatitude() const 
    { 
        return mLatitude; 
    }
    void SetLatitude(double latitude) 
    { 
        mLatitude = latitude; 
    }

    double GetLongitude() const 
    { 
        return mLongitude; 
    }
    void SetLongitude(double longitude) 
    { 
        mLongitude = longitude; 
    }

    double GetAltitude() const 
    { 
        return mAltitude; 
    }
    void SetAltitude(double altitude) 
    { 
        mAltitude = altitude; 
    }
};


int main(int argc, char* argv[])
{
    // Read .txt files
    // Get them from commandline argument
    // Parse them, then create a std::vector of TLEs
    // Sort by mean motion
    // For similar MM, see if they are close in distance
    // Create a vector of 'train' objects which are themselves, vectors
    // Compare the trains to see if any endpoints actually connect
    // Coelesce the sub-trains together to create a complete train

    if (argc < 2) 
    {
        std::cout << "Please provide a file path." << std::endl;
        return 1;
    }

    std::filesystem::path filePath(argv[1]);

    if (!std::filesystem::exists(filePath)) 
    {
        std::cout << "The file " << filePath << " does not exist." << std::endl;
        return 1;
    }

    std::ifstream fileStream(filePath);
    if (!fileStream) 
    {
        std::cout << "Failed to open the file " << filePath << std::endl;
        return 1;
    }

    std::list<std::tuple<double, OrbitalData>> orbitMap;
    std::string readLine;

    std::string name = "";
    std::string line1 = "";
    std::string line2 = "";
    double meanMotion = 0.0;

    while(!fileStream.eof())
    {
        std::getline(fileStream, name);
        std::getline(fileStream, line1);
        std::getline(fileStream, line2);

        std::size_t pos = line2.find_last_of(" ");
        meanMotion = std::stod(line2.substr(pos + 1));

        double out_tleage = 0.0;
        double out_latdegs = 0.0;
        double out_londegs = 0.0;
        double out_altkm = 0.0;

        // Update TLE list with web address
        // https://celestrak.org/NORAD/elements/gp.php?NAME=Starlink&FORMAT=TLE
        // get current time as a long long in seconds
        long long testTime = time(NULL);

        int result = orbit_to_lla(testTime, name.c_str(), line1.c_str(), line2.c_str(), &out_tleage, &out_latdegs, &out_londegs, &out_altkm);
        if (result == 0)
        {
            OrbitalData data(name, out_latdegs, out_londegs, out_altkm);
            orbitMap.push_back(std::make_pair(meanMotion, data));
        }
    }
    // Sort by mean motion
    orbitMap.sort([](const std::tuple<double, OrbitalData>& a, const std::tuple<double, OrbitalData>& b) -> bool
    {
        return std::get<0>(a) < std::get<0>(b);
    });

    // Print out the sorted list
    // remember the previous mean motion
    double prevMeanMotion = 0;
    int count = 0;
    for (auto& data : orbitMap)
    {
        std::cout << std::get<1>(data).GetName() << ": " << std::get<0>(data) << std::endl;
        std::cout << "Lat: " << std::get<1>(data).GetLatitude() << std::endl;
        std::cout << "Lon: " << std::get<1>(data).GetLongitude() << std::endl;
        std::cout << "Alt: " << std::get<1>(data).GetAltitude() << std::endl; 

        // A gap exists
        
        double diff = abs(std::get<0>(data) - prevMeanMotion);
        if (diff > 10)
        {
            std::cout << "-----GAP-----" << std::endl;
            std::cout << "Count: " << count << std::endl;
            count = 0;
        }
        prevMeanMotion = std::get<0>(data);
        count++;
    }

    return 0;
}