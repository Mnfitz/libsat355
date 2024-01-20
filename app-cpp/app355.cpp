// Self
#include "libsat355.h"
#include "app355.h"

// std
#include <algorithm>
#include <chrono>
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
    sat355::TLE mTLE;
    double mLatitude;
    double mLongitude;
    double mAltitude;
    std::string mName;
    double mMeanMotion;

public:
    OrbitalData(sat355::TLE inTLE, double latitude, double longitude, double altitude)
        : mTLE{std::move(inTLE)}, mLatitude(latitude), mLongitude(longitude), mAltitude(altitude)
    {
        mName = mTLE.GetName();
        mMeanMotion = mTLE.GetMeanMotion();
    }

    OrbitalData() = delete;

    ~OrbitalData() = default;

    const sat355::TLE& GetTLE() const 
    { 
        return mTLE; 
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

// Section 1: Read from file
std::vector<sat355::TLE> ReadFromFile(int argc, char* argv[])
{
    if (argc < 2) 
    {
        std::cout << "Please provide a file path." << std::endl;
        //return 1;
    }

    std::filesystem::path filePath(argv[1]);

    if (!std::filesystem::exists(filePath)) 
    {
        std::cout << "The file " << filePath << " does not exist." << std::endl;
        //return 1;
    }

    std::ifstream fileStream(filePath);
    if (!fileStream) 
    {
        std::cout << "Failed to open the file " << filePath << std::endl;
        //return 1;
    }

    std::list<OrbitalData> orbitalList;
    std::string readLine;

    std::string name{};
    std::string line1{};
    std::string line2{};
    
    std::vector<sat355::TLE> tleList;

    while(!fileStream.eof())
    {
        std::getline(fileStream, name);
        std::getline(fileStream, line1);
        std::getline(fileStream, line2);

       sat355::TLE newTLE{name, line1, line2};
        tleList.push_back(std::move(newTLE));
    }
    
    return tleList;
}

// Section 2: Calculate orbital data
std::list<OrbitalData> CalculateOrbitalData(std::vector<sat355::TLE> tleList)
{
    std::list<OrbitalData> orbitalList;

    for(auto& inTLE : tleList)
    {
        double out_tleage = 0.0;
        double out_latdegs = 0.0;
        double out_londegs = 0.0;
        double out_altkm = 0.0;

        // Update TLE list with web address
        // https://celestrak.org/NORAD/elements/gp.php?NAME=Starlink&FORMAT=TLE
        // get current time as a long long in seconds
        
        //long long testTime = time(nullptr);
        long long testTime = 1705781559; // Time TLEs stored in StarlinkTLE.txt were recorded

        int result = orbit_to_lla(testTime, inTLE.GetName().data(), inTLE.GetLine1().data(), inTLE.GetLine2().data(), &out_tleage, &out_latdegs, &out_londegs, &out_altkm);
        if (result == 0)
        {
            // Warning! Cannot use tle anymore as it has been moved!
            OrbitalData data(inTLE, out_latdegs, out_londegs, out_altkm);
            orbitalList.push_back(std::move(data));
        }
    }

    // Sort by mean motion
    orbitalList.sort([](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return inLHS.GetTLE().GetMeanMotion() < inRHS.GetTLE().GetMeanMotion();
    });
    return orbitalList;
}

// Section 3: Create Trains
std::list<std::list<OrbitalData>> CreateTrains(std::list<OrbitalData> orbitalList)
{
    std::list<std::list<OrbitalData>> trainList;
    std::list<OrbitalData> newTrain;

    double prevMeanMotion = 0;
    double prevInclination = 0;
    for (auto& data : orbitalList)
    {
        double deltaMotion = abs(data.GetTLE().GetMeanMotion() - prevMeanMotion);
        double deltaInclination = abs(data.GetTLE().GetInclination() - prevInclination);
        if ((deltaMotion > 0.0001 || deltaInclination > 0.0001) && !newTrain.empty())
        {
            // Sort by longitude
            newTrain.sort([](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
            {
                return inLHS.GetLongitude() < inRHS.GetLongitude();
            });

            // Filter out wandering satellites
            if (newTrain.size() > 2)
            {
                trainList.push_back(std::move(newTrain));
            }
            
            newTrain.clear();
        }
        prevMeanMotion = data.GetTLE().GetMeanMotion();
        prevInclination = data.GetTLE().GetInclination();

        newTrain.push_back(data);
    }
    if (!newTrain.empty() && newTrain.size() > 2)
    {
        trainList.push_back(newTrain);
    }

    return trainList;
}

// Section 4: Print Trains
void PrintTrains(std::list<std::list<OrbitalData>> trainList)
{
    int trainCount = 0;
    // Print the contents of the train list
    for (auto& train : trainList)
    {
        std::cout << "   TRAIN #" << trainCount << std::endl;
        std::cout << "   COUNT: " << train.size() << std::endl;
    
        for (auto& data : train)
        {
            // Print the name, mean motion, latitude, longitude, and altitude
            std::cout << data.GetTLE().GetName() << ": " << data.GetTLE().GetMeanMotion() << std::endl;
            std::cout << "Lat: " << data.GetLatitude() << std::endl;
            std::cout << "Lon: " << data.GetLongitude() << std::endl;
            std::cout << "Alt: " << data.GetAltitude() << std::endl << std::endl;
        }
        std::cout << std::endl << std::endl;
        trainCount++;
    }
}

int main(int argc, char* argv[])
{
    // measure total time in milliseconds using chrono 
    auto startTotal = std::chrono::high_resolution_clock::now();

    // meaure time for each section in milliseconds using chrono
    auto start1 = std::chrono::high_resolution_clock::now();
    std::vector<sat355::TLE> tleList{ ReadFromFile(argc, argv) };
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed1 = end1 - start1;
    std::cout << "Read from file: " << elapsed1.count() << " ms" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    std::list<OrbitalData> orbitalList{ CalculateOrbitalData(tleList) };
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed2 = end2 - start2;
    std::cout << "Calculate orbital data: " << elapsed2.count() << " ms" << std::endl;

    auto start3 = std::chrono::high_resolution_clock::now();
    std::list<std::list<OrbitalData>> trainList{ CreateTrains(orbitalList) };
    auto end3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed3 = end3 - start3;
    std::cout << "Create trains: " << elapsed3.count() << " ms" << std::endl;

    auto start4 = std::chrono::high_resolution_clock::now();
    PrintTrains(trainList);
    auto end4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed4 = end4 - start4;
    std::cout << "Print trains: " << elapsed4.count() << " ms" << std::endl;
    
    auto endTotal = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTotal = endTotal - startTotal;
    std::cout << "Total time: " << elapsedTotal.count() << " ms" << std::endl;

    return 0;
}