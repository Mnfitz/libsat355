// Self
#include "libsat355.h"
#include "app355.h"

// std
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <tuple>

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

namespace {

class SatOrbit
{
public:
    SatOrbit(std::size_t inNumThreads = 1);
    virtual ~SatOrbit() = default;

    std::vector<sat355::TLE> ReadFromFile(int argc, char* argv[]);
    std::list<OrbitalData> CalculateOrbitalData(const std::vector<sat355::TLE>& tleList);
    void SortOrbitalList(std::list<OrbitalData>& ioOrbitalList);
    std::list<std::list<OrbitalData>> CreateTrains(const std::list<OrbitalData>& orbitalList);
    void PrintTrains(const std::list<std::list<OrbitalData>>& trainList);

private:
    using tle_const_iterator = std::vector<sat355::TLE>::const_iterator;
    using tle_iterator = std::vector<sat355::TLE>::iterator;
    using OrbitalDataList = std::tuple<std::mutex, std::list<OrbitalData>>;

private:
    virtual std::vector<sat355::TLE> OnReadFromFile(int argc, char* argv[]);
    virtual void OnCalculateOrbitalData(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataList& ioDataList);
    virtual void OnSortOrbitalList(std::list<OrbitalData>& ioOrbitalList);
    virtual std::list<std::list<OrbitalData>> OnCreateTrains(const std::list<OrbitalData>& orbitalList);
    virtual void OnPrintTrains(const std::list<std::list<OrbitalData>>& trainList);

private:
    std::size_t mNumThreads{0};
    OrbitalDataList mOrbitalList{};
};

// Public Non-Virtuals
SatOrbit::SatOrbit(std::size_t inNumThreads) : 
    mNumThreads(inNumThreads)
{
    // Do nothing
}

std::vector<sat355::TLE> SatOrbit::ReadFromFile(int argc, char* argv[])
{
    return OnReadFromFile(argc, argv);
}

std::list<OrbitalData> SatOrbit::CalculateOrbitalData(const std::vector<sat355::TLE>& tleList)
{
    // create a list of threads the size of the number of threads specified
    std::vector<std::thread> threadList{mNumThreads};
    std::size_t tleListSize = tleList.size();

    // calculate the number of TLEs per thread
    std::size_t tlePerThread = tleListSize / mNumThreads;
    auto tleBegin = tleList.begin();
    auto tleEnd = tleList.end();

    // loop through the threads
    for (std::size_t i = 0; i < mNumThreads; ++i)
    {
        // calculate the begin and end of the TLEs for the current thread
        tleBegin = tleList.begin() + (i * tlePerThread);
        tleEnd = tleBegin + tlePerThread;

        // if this is the last thread, add the remaining TLEs
        if (i == mNumThreads - 1)
        {
            tleEnd = tleList.end();
        }

        // start the thread
        threadList[i] = std::thread(&SatOrbit::OnCalculateOrbitalData, this, tleBegin, tleEnd, std::ref(mOrbitalList));
    }
    // wait for the threads to finish before returning
    std::for_each(threadList.begin(), threadList.end(), [](std::thread& inThread)
    {
        inThread.join();
    });

    //OnCalculateOrbitalData(tleList.begin(), tleList.end(), mOrbitalList);
    return std::move(std::get<1>(mOrbitalList));
}

void SatOrbit::SortOrbitalList(std::list<OrbitalData>& ioOrbitalList)
{
    OnSortOrbitalList(ioOrbitalList);
}

std::list<std::list<OrbitalData>> SatOrbit::CreateTrains(const std::list<OrbitalData>& orbitalList)
{
    return OnCreateTrains(orbitalList);
}

void SatOrbit::PrintTrains(const std::list<std::list<OrbitalData>>& trainList)
{
    OnPrintTrains(trainList);
}

// Private Virtuals
std::vector<sat355::TLE> SatOrbit::OnReadFromFile(int argc, char* argv[])
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

void SatOrbit::OnCalculateOrbitalData(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataList& ioDataList)
{
    std::list<OrbitalData> orbitalList{};
    std::for_each(tleBegin, tleEnd, [&](const sat355::TLE& inTLE)
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
    });

    auto& [mutex, outputList] = ioDataList; // C++17 Structured Binding simplifies tuple manipulation
    // Use mutex to protect access to the list
    {
        std::lock_guard<std::mutex> lock(mutex);
        outputList.splice(outputList.end(), std::move(orbitalList));
    }
}

void SatOrbit::OnSortOrbitalList(std::list<OrbitalData>& ioOrbitalList)
{
    // Sort by mean motion
    ioOrbitalList.sort([](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return inLHS.GetTLE().GetMeanMotion() < inRHS.GetTLE().GetMeanMotion();
    });
}

std::list<std::list<OrbitalData>> SatOrbit::OnCreateTrains(const std::list<OrbitalData>& orbitalList)
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

void SatOrbit::OnPrintTrains(const std::list<std::list<OrbitalData>>& trainList)
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

} // namespace anonymous

int main(int argc, char* argv[])
{
    // measure total time in milliseconds using chrono 
    auto startTotal = std::chrono::high_resolution_clock::now();

    // 4 threads
    SatOrbit satOrbit{4};

    // meaure time for each section in milliseconds using chrono
    auto start1 = std::chrono::high_resolution_clock::now();
    std::vector<sat355::TLE> tleList{ satOrbit.ReadFromFile(argc, argv) };
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed1 = end1 - start1;
    std::cout << "Read from file: " << elapsed1.count() << " ms" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    std::list<OrbitalData> orbitalList{ satOrbit.CalculateOrbitalData(tleList) };
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed2 = end2 - start2;
    std::cout << "Calculate orbital data: " << elapsed2.count() << " ms" << std::endl;

    auto start3 = std::chrono::high_resolution_clock::now();
    satOrbit.SortOrbitalList(orbitalList);
    auto end3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed3 = end3 - start3;
    std::cout << "Sort orbital list: " << elapsed3.count() << " ms" << std::endl;

    auto start4 = std::chrono::high_resolution_clock::now();
    std::list<std::list<OrbitalData>> trainList{ satOrbit.CreateTrains(orbitalList) };
    auto end4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed4 = end4 - start4;
    std::cout << "Create trains: " << elapsed4.count() << " ms" << std::endl;

    auto start5 = std::chrono::high_resolution_clock::now();
    satOrbit.PrintTrains(trainList);
    auto end5 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed5 = end5 - start5;
    std::cout << "Print trains: " << elapsed5.count() << " ms" << std::endl;
    
    auto endTotal = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTotal = endTotal - startTotal;
    std::cout << "Total time: " << elapsedTotal.count() << " ms" << std::endl;

    return 0;
}