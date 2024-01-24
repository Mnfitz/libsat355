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
    std::vector<OrbitalData> CalculateOrbitalData(const std::vector<sat355::TLE>& tleVector);
    void SortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);
    std::vector<std::vector<OrbitalData>> CreateTrains(const std::vector<OrbitalData>& orbitalVector);
    void PrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector);

private:
    using tle_const_iterator = std::vector<sat355::TLE>::const_iterator;
    using tle_iterator = std::vector<sat355::TLE>::iterator;
    using OrbitalDataVector = std::tuple<std::mutex, std::vector<OrbitalData>>;
    using orbit_iterator = std::vector<OrbitalData>::iterator;
    using orbit_const_iterator = std::vector<OrbitalData>::const_iterator;

private:
    virtual std::vector<sat355::TLE> OnReadFromFile(int argc, char* argv[]);
    //virtual void OnCalculateOrbitalData(, OrbitalDataVector& ioDataVector);
    virtual void OnCalculateOrbitalDataMulti(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataVector& ioDataVector);
    virtual void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);
    virtual void OnSortOrbitalVectorMulti(std::vector<OrbitalData> ioOrbitalVector, std::mutex& ioMutex);
    virtual std::vector<OrbitalData> OnMergeVector(std::vector<std::vector<OrbitalData>>& ioOrbitalVector);
    virtual std::vector<std::vector<OrbitalData>> OnCreateTrains(const std::vector<OrbitalData>& orbitalVector);
    virtual void OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector);

private:
    std::size_t mNumThreads{0};
    OrbitalDataVector mOrbitalVector{};
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

std::vector<OrbitalData> SatOrbit::CalculateOrbitalData(const std::vector<sat355::TLE>& tleVector)
{
    // create a list of threads the size of the number of threads specified
    std::vector<std::thread> threadVector{mNumThreads};
    std::size_t tleVectorSize = tleVector.size();

    // calculate the number of TLEs per thread
    std::size_t tlePerThread = tleVectorSize / mNumThreads;
    auto tleBegin = tleVector.begin();
    auto tleEnd = tleVector.end();

    // loop through the threads
    for (std::size_t i = 0; i < mNumThreads; ++i)
    {
        // calculate the begin and end of the TLEs for the current thread
        tleBegin = tleVector.begin() + (i * tlePerThread);
        tleEnd = tleBegin + tlePerThread;

        // if this is the last thread, add the remaining TLEs
        if (i == mNumThreads - 1)
        {
            tleEnd = tleVector.end();
        }

        // start the thread
        threadVector[i] = std::thread(&SatOrbit::OnCalculateOrbitalDataMulti, this, tleBegin, tleEnd, std::ref(mOrbitalVector));
    }
    // wait for the threads to finish before returning
    std::for_each(threadVector.begin(), threadVector.end(), [](std::thread& inThread)
    {
        inThread.join();
    });

    //OnCalculateOrbitalData(tleVector.begin(), tleVector.end(), mOrbitalVector);
    return std::move(std::get<1>(mOrbitalVector));
}

void SatOrbit::SortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector)
{
    // Multithreaded sort
    // If thread count is 1, use single threaded sort
    if (mNumThreads == 1)
    {
        OnSortOrbitalVector(ioOrbitalVector);
    }
    else
    {
        // create a list of threads the size of the number of threads specified
        std::vector<std::thread> threadVector{mNumThreads};
        std::size_t orbitVectorSize = ioOrbitalVector.size();
        std::mutex mutex;

        // calculate the number of TLEs per thread
        std::size_t orbitPerThread = orbitVectorSize / mNumThreads;
        auto orbitBegin = ioOrbitalVector.begin();
        auto orbitEnd = ioOrbitalVector.end();

        std::vector<std::vector<OrbitalData>> orbitSubVectors;
        // Create a sub-list for each thread
        for (std::size_t i = 0; i < mNumThreads; ++i)
        {
            // calculate the begin and end of the TLEs for the current thread
            orbitBegin = ioOrbitalVector.begin() + (i * orbitPerThread);
            orbitEnd = orbitBegin + orbitPerThread;

            // if this is the last thread, add the remaining TLEs
            if (i == mNumThreads - 1)
            {
                orbitEnd = ioOrbitalVector.end();
            }

            std::vector<OrbitalData> subVector = std::vector<OrbitalData>(orbitBegin, orbitEnd);
            // start the thread
            // Sort each thread's sub-list
            threadVector[i] = std::thread(&SatOrbit::OnSortOrbitalVectorMulti, std::ref(subVector));
            orbitSubVectors.push_back(std::move(subVector));
        }

        // Join the threads together
        // wait for the threads to finish before returning
        std::for_each(threadVector.begin(), threadVector.end(), [](std::thread& inThread)
        {
            inThread.join();
        });


        // Merge the sub-lists together
        ioOrbitalVector = OnMergeVector(orbitSubVectors);
    }
}

std::vector<std::vector<OrbitalData>> SatOrbit::CreateTrains(const std::vector<OrbitalData>& orbitalVector)
{
    return OnCreateTrains(orbitalVector);
}

void SatOrbit::PrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector)
{
    OnPrintTrains(trainVector);
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

    std::vector<OrbitalData> orbitalVector;
    std::string readLine;

    std::string name{};
    std::string line1{};
    std::string line2{};
    
    std::vector<sat355::TLE> tleVector;

    while(!fileStream.eof())
    {
        std::getline(fileStream, name);
        std::getline(fileStream, line1);
        std::getline(fileStream, line2);

       sat355::TLE newTLE{name, line1, line2};
        tleVector.push_back(std::move(newTLE));
    }
    
    return tleVector;
}

void SatOrbit::OnCalculateOrbitalData(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataVector& ioDataVector)
{
    std::vector<OrbitalData> orbitalVector{};
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
            orbitalVector.push_back(std::move(data));
        }
    });

    auto& [mutex, outputVector] = ioDataVector; // C++17 Structured Binding simplifies tuple manipulation
    // Use mutex to protect access to the list
    {
        std::lock_guard<std::mutex> lock(mutex);
        outputVector.insert(outputVector.end(), orbitalVector.begin(), orbitalVector.end());
    }
}

void SatOrbit::OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector)
{
    // Sort by mean motion
    std::sort(ioOrbitalVector.begin(), ioOrbitalVector.end(), [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return inLHS.GetTLE().GetMeanMotion() < inRHS.GetTLE().GetMeanMotion();
    });
}

void SatOrbit::OnSortOrbitalVectorMulti(std::vector<OrbitalData> ioOrbitalVector, std::mutex& ioMutex)
{
    std::lock_guard<std::mutex> lock(ioMutex);
    // Sort by mean motion
    std::sort(ioOrbitalVector.begin(), ioOrbitalVector.end(), [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return inLHS.GetTLE().GetMeanMotion() < inRHS.GetTLE().GetMeanMotion();
    });
}

// Merge sort after multithreading
std::vector<OrbitalData> SatOrbit::OnMergeVector(std::vector<std::vector<OrbitalData>>& inOrbitalVector)
{
    // Merge the lists together, sorted by mean motion
    // Each individual sub-list has its elements sorted by mean motion, so backtracking is not necessary
    std::vector<OrbitalData> mergedVector;
    while (!inOrbitalVector.empty())
    {
        // Find the smallest mean motion in the list
        auto minVector = inOrbitalVector.begin();
        auto minData = minVector->begin();
        for (auto list = inOrbitalVector.begin(); list != inOrbitalVector.end(); ++list)
        {
            if (list->begin()->GetTLE().GetMeanMotion() < minData->GetTLE().GetMeanMotion())
            {
                minVector = list;
                minData = list->begin();
            }
        }

        // Add the smallest mean motion to the merged list
        mergedVector.push_back(std::move(*minData));
        minVector->erase(minData);

        // Remove the list if it is empty
        if (minVector->empty())
        {
            inOrbitalVector.erase(minVector);
        }
    }
}

std::vector<std::vector<OrbitalData>> SatOrbit::OnCreateTrains(const std::vector<OrbitalData>& orbitalVector)
{
    std::vector<std::vector<OrbitalData>> trainVector;
    std::vector<OrbitalData> newTrain;

    double prevMeanMotion = 0;
    double prevInclination = 0;
    for (auto& data : orbitalVector)
    {
        double deltaMotion = abs(data.GetTLE().GetMeanMotion() - prevMeanMotion);
        double deltaInclination = abs(data.GetTLE().GetInclination() - prevInclination);
        if ((deltaMotion > 0.0001 || deltaInclination > 0.0001) && !newTrain.empty())
        {
            // Sort by longitude
            std::sort(newTrain.begin(), newTrain.end(), [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
            {
                return inLHS.GetLongitude() < inRHS.GetLongitude();
            });

            // Filter out wandering satellites
            if (newTrain.size() > 2)
            {
                trainVector.push_back(std::move(newTrain));
            }
            
            newTrain.clear();
        }
        prevMeanMotion = data.GetTLE().GetMeanMotion();
        prevInclination = data.GetTLE().GetInclination();

        newTrain.push_back(data);
    }
    if (!newTrain.empty() && newTrain.size() > 2)
    {
        trainVector.push_back(newTrain);
    }

    return trainVector;
}

void SatOrbit::OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector)
{
    int trainCount = 0;
    // Print the contents of the train list
    for (auto& train : trainVector)
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
    std::vector<sat355::TLE> tleVector{ satOrbit.ReadFromFile(argc, argv) };
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed1 = end1 - start1;
    std::cout << "Read from file: " << elapsed1.count() << " ms" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    std::vector<OrbitalData> orbitalVector{ satOrbit.CalculateOrbitalData(tleVector) };
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed2 = end2 - start2;
    std::cout << "Calculate orbital data: " << elapsed2.count() << " ms" << std::endl;

    auto start3 = std::chrono::high_resolution_clock::now();
    satOrbit.SortOrbitalVector(orbitalVector);
    auto end3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed3 = end3 - start3;
    std::cout << "Sort orbital list: " << elapsed3.count() << " ms" << std::endl;

    auto start4 = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<OrbitalData>> trainVector{ satOrbit.CreateTrains(orbitalVector) };
    auto end4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed4 = end4 - start4;
    std::cout << "Create trains: " << elapsed4.count() << " ms" << std::endl;

    auto start5 = std::chrono::high_resolution_clock::now();
    satOrbit.PrintTrains(trainVector);
    auto end5 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed5 = end5 - start5;
    std::cout << "Print trains: " << elapsed5.count() << " ms" << std::endl;
    
    auto endTotal = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTotal = endTotal - startTotal;
    std::cout << "Total time: " << elapsedTotal.count() << " ms" << std::endl;

    return 0;
}