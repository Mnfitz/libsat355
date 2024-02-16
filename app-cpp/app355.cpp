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
 // Interface
public:
    SatOrbit(std::size_t inNumThreads = 1);
    virtual ~SatOrbit() = default;

    std::vector<sat355::TLE> ReadFromFile(int argc, char* argv[]);
    std::vector<OrbitalData> CalculateOrbitalData(const std::vector<sat355::TLE>& tleVector);
    void SortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);
    std::vector<std::vector<OrbitalData>> CreateTrains(const std::vector<OrbitalData>& orbitalVector);
    void PrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector);

// Types
private:
    using tle_const_iterator = std::vector<sat355::TLE>::const_iterator;
    using tle_iterator = std::vector<sat355::TLE>::iterator;
    using OrbitalDataVector = std::tuple<std::mutex, std::vector<OrbitalData>>;
    using orbit_iterator = std::vector<OrbitalData>::iterator;
    using orbit_const_iterator = std::vector<OrbitalData>::const_iterator;
    using IteratorPairVector = std::vector<std::tuple<orbit_iterator, orbit_iterator>>;

// Implementation
private:
    virtual std::vector<sat355::TLE> OnReadFromFile(int argc, char* argv[]);
    virtual void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector);
    virtual void OnCalculateOrbitalDataMulti(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataVector& ioDataVector);
    virtual void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);
    virtual void OnSortOrbitalVectorMulti(orbit_iterator& inBegin, orbit_iterator& inEnd);
    virtual void OnSortMergeVector(orbit_iterator& ioBegin, orbit_iterator& ioMid, orbit_iterator& ioEnd);
    virtual std::vector<std::vector<OrbitalData>> OnCreateTrains(const std::vector<OrbitalData>& orbitalVector);
    virtual void OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector);

// Helper
private:
    static bool SortPredicate(const OrbitalData& inLHS, const OrbitalData& inRHS);
    void CalculateOrbitalDataMulti(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector);
    void SortOrbitalVectorMulti(std::vector<OrbitalData>& ioOrbitalVector);

// Data Members
private:
    std::size_t mNumThreads{0};
    OrbitalDataVector mOrbitalVector{};
};

// NVI Interface: Public Non-Virtuals

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
    if (mNumThreads == 1)
    {
        OnCalculateOrbitalData(tleVector, mOrbitalVector);
    }
    else
    {
        CalculateOrbitalDataMulti(tleVector, mOrbitalVector);
    }
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
        SortOrbitalVectorMulti(ioOrbitalVector);
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

// NVI Implementation: Private Virtuals

std::vector<sat355::TLE> SatOrbit::OnReadFromFile(int argc, char* argv[])
{
    if (argc < 2) 
    {
        std::cout << "Please provide a file path." << std::endl;
        const auto err = std::make_error_code(std::errc::invalid_argument);
        throw std::filesystem::filesystem_error("Path not given", err);
    }

    std::filesystem::path filePath(argv[1]);

    if (!std::filesystem::exists(filePath)) 
    {
        std::cout << "The file " << filePath << " does not exist." << std::endl;
        const auto err = std::make_error_code(std::errc::no_such_file_or_directory);
        throw std::filesystem::filesystem_error("File does not exist", err);
    }

    std::ifstream fileStream(filePath);
    if (!fileStream) 
    {
        std::cout << "Failed to open the file " << filePath << std::endl;
        const auto err = std::make_error_code(std::errc::io_error);
        throw std::filesystem::filesystem_error("File could not be opened", err);
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

void SatOrbit::OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector)
{
    OnCalculateOrbitalDataMulti(inTLEVector.begin(), inTLEVector.end(), ioDataVector);
}

void SatOrbit::OnCalculateOrbitalDataMulti(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataVector& ioDataVector)
{
    const auto size = std::distance(tleBegin, tleEnd);
    std::vector<OrbitalData> orbitalVector{};
    orbitalVector.reserve(size);
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

    auto& [mutex, outputVector] = ioDataVector; // C++17 Structured Binding simplifies tuple unpacking
    // Use mutex to protect access to the list
    {
        std::lock_guard<std::mutex> lock(mutex);
        outputVector.insert(outputVector.end(), orbitalVector.begin(), orbitalVector.end());
    }
}

// Use inplace_merge to merge the list, denoted by the iterators
// It will have a complexity of O(n log n), since backtracking is not required
void SatOrbit::OnSortMergeVector(orbit_iterator& inBegin, orbit_iterator& inMid, orbit_iterator& inEnd)
{
    std::inplace_merge(inBegin, inMid, inEnd, [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

void SatOrbit::OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector)
{
    // Sort by mean motion
    std::sort(ioOrbitalVector.begin(), ioOrbitalVector.end(), [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

void SatOrbit::OnSortOrbitalVectorMulti(orbit_iterator& inBegin, orbit_iterator& inEnd)
{
    // Sort by mean motion
    std::sort(inBegin, inEnd, [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

// Helper function
/*static*/ bool SatOrbit::SortPredicate(const OrbitalData& inLHS, const OrbitalData& inRHS)
{
    return inLHS.GetTLE().GetMeanMotion() < inRHS.GetTLE().GetMeanMotion();
}

void SatOrbit::CalculateOrbitalDataMulti(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector)
{
    // create a list of threads the size of the number of threads specified
    std::vector<std::thread> threadVector{};
    threadVector.reserve(mNumThreads);
    const std::size_t tleVectorSize = inTLEVector.size();

    // calculate the number of TLEs per thread
    const std::size_t tlePerThread = tleVectorSize / mNumThreads;
    
    // loop through the threads
    // (std::prtdiff_t is a signed version of std::size_t)
    for (std::size_t i = 0; i < mNumThreads; ++i)
    {
        // calculate the begin and end of the TLEs for the current thread
        const auto tleBegin = inTLEVector.begin() + (i * tlePerThread);
        // if this is the last thread, add the remaining TLEs
        const auto tleEnd = (i == mNumThreads - 1) ? inTLEVector.end() : tleBegin + tlePerThread;

        // start the thread
        // TRICKY: mnfitz 24jan2024: std::thread usage!!
        // #1 std::ref() to force thread to take a reference& to a value; rather than a copy or pointer
        // #2 Pass 'this' as first parameter since OnCalculateOrbitalDataMulti is an instance method
        threadVector.push_back(std::thread(&SatOrbit::OnCalculateOrbitalDataMulti, this, tleBegin, tleEnd, std::ref(ioDataVector)));
    }
    // wait for the threads to finish before returning
    std::for_each(threadVector.begin(), threadVector.end(), [](std::thread& inThread)
    {
        inThread.join();
    });
}

void SatOrbit::SortOrbitalVectorMulti(std::vector<OrbitalData>& ioOrbitalVector)
{
    const std::size_t orbitVectorSize = ioOrbitalVector.size();

    // calculate the number of TLEs per thread
    const std::size_t orbitPerThread = orbitVectorSize / mNumThreads;
    // Note: no need for mutex as threads do not contend for shared elements during sort
    IteratorPairVector subListVector{};

    // Block scope that has multiple threads quicksort segments of the vector
    {
        // create a list of threads the size of the number of threads specified
        std::vector<std::thread> threadVector{};

        for (std::size_t i = 0; i < mNumThreads; ++i)
        {
            // calculate the begin and end of the TLEs for the current thread
            const auto orbitBegin = ioOrbitalVector.begin() + (i * orbitPerThread);
            // if this is the last thread, add the remaining TLEs, else add orbitPerThread
            const auto orbitEnd = (i >= mNumThreads - 1) ? ioOrbitalVector.end() : orbitBegin + orbitPerThread;
            threadVector.push_back(std::thread(&SatOrbit::OnSortOrbitalVectorMulti, this, orbitBegin, orbitEnd));
            
            auto slice = std::make_tuple(orbitBegin, orbitEnd );
            //IteratorPairVector::value_type slice{ orbitBegin, orbitEnd };
            subListVector.push_back(std::move(slice));
        }
        
        // Join the threads together
        // wait for the threads to finish before returning
        std::for_each(threadVector.begin(), threadVector.end(), [](std::thread& inThread)
        {
            inThread.join();
        });
    }

    // Merge the sorted sublists
    // Use multithreading by merging multilple sublists at a time
    // Join the threads before merging the set of larger sublists
    // Loop until there is only one list of completely sorted OrbitalData
    while (subListVector.size() > 1)
    {
        // Merge sort the sublists 2 at a time
        std::vector<std::thread> threadVector{};
        IteratorPairVector newSubListVector{};

        // This loop will merge the sublists 2 at a time, therefore we start at 1 and increment by 2
        for (std::size_t i = 1; i < subListVector.size(); i += 2)
        {
            // Get first 2 sublists to merge
            auto& [begin, mid] = subListVector[i-1];
            auto& [mid2, end] = subListVector[i];
            // Mids must be sequential in memory
            assert(mid == mid2);

            // Start a thread to merge the 2 sublists
            threadVector.push_back(std::thread(&SatOrbit::OnSortMergeVector, this, begin, mid, end));
            // Create a new sublist element from the 2 merged sublists onto a new list
            newSubListVector.push_back(std::make_tuple(begin, end));
        }
        // Wait for the threads to finish their merge operation
        std::for_each(threadVector.begin(), threadVector.end(), [](std::thread& inThread)
        {
            inThread.join();
        });

        // If the number of sublists is odd, it would be missed in the loop above; add it to the new list
        const bool isOdd = ((subListVector.size() & 1) == 1);
        if (isOdd)
        {
            newSubListVector.push_back(subListVector.back());
        }

        // Update subListVector with the new merged results
        subListVector = std::move(newSubListVector);
    }
}

std::vector<std::vector<OrbitalData>> SatOrbit::OnCreateTrains(const std::vector<OrbitalData>& orbitalVector)
{
    std::vector<std::vector<OrbitalData>> trainVector;
    std::vector<OrbitalData> newTrain;

    double prevMeanMotion = 0;
    double prevInclination = 0;

    std::for_each(orbitalVector.begin(), orbitalVector.end(), [&](auto& data)
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
    });
    if (!newTrain.empty() && newTrain.size() > 2)
    {
        trainVector.push_back(newTrain);
    }

    // Some trains will be in close proximity, therefore we must merge them
    // Merge trains whose satellites' mean motions are within 0.001 degrees of each other
    for (std::size_t i = 0; i < trainVector.size(); ++i)
    {
        for (std::size_t j = i + 1; j < trainVector.size(); ++j)
        {
            double deltaMotion = abs(trainVector[i][0].GetTLE().GetMeanMotion() - trainVector[j][0].GetTLE().GetMeanMotion());
            if (deltaMotion < 0.001)
            {
                trainVector[i].insert(trainVector[i].end(), trainVector[j].begin(), trainVector[j].end());
                trainVector.erase(trainVector.begin() + j);
                --j;
            }
        }
    }
    
    return trainVector;
}

void SatOrbit::OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector)
{
    int trainCount = 0;
    // Print the contents of the train list

    std::for_each(trainVector.begin(), trainVector.end(), [&](auto& train)
    {
        std::cout << "   TRAIN #" << trainCount << std::endl;
        std::cout << "   COUNT: " << train.size() << std::endl;

        std::for_each(train.begin(), train.end(), [](auto& data)
        {
            // Print the name, mean motion, latitude, longitude, and altitude
            std::cout << data.GetTLE().GetName() << ": " << data.GetTLE().GetMeanMotion() << std::endl;
            std::cout << "Lat: " << data.GetLatitude() << std::endl;
            std::cout << "Lon: " << data.GetLongitude() << std::endl;
            std::cout << "Alt: " << data.GetAltitude() << std::endl << std::endl;
        });
        std::cout << std::endl << std::endl;
        trainCount++;
    });
}

class Timer
{
public:
    Timer() = default;
    void Start();
    double Stop();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> mEnd;
    std::chrono::duration<double, std::milli> mElapsedMs;
};

void Timer::Start()
{
    mStart = std::chrono::high_resolution_clock::now();
}

double Timer::Stop()
{
    mEnd = std::chrono::high_resolution_clock::now();
    mElapsedMs = mEnd - mStart;
    return mElapsedMs.count();
}

} // namespace anonymous

int main(int argc, char* argv[])
{
    // measure total time in milliseconds using chrono 
    auto startTotal = std::chrono::high_resolution_clock::now();

    // 4 threads is baseline
    SatOrbit satOrbit{4};

    // meaure time for each section in milliseconds using chrono
    Timer totalTimer{};
    Timer timer{};
    totalTimer.Start();

    timer.Start();
    std::vector<sat355::TLE> tleVector{ satOrbit.ReadFromFile(argc, argv) };
    std::cout << "Read from file: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    std::vector<OrbitalData> orbitalVector{ satOrbit.CalculateOrbitalData(tleVector) };
    std::cout << "Calculate orbital data: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    satOrbit.SortOrbitalVector(orbitalVector);
    std::cout << "Sort orbital list: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    std::vector<std::vector<OrbitalData>> trainVector{ satOrbit.CreateTrains(orbitalVector) };
    std::cout << "Create trains: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    satOrbit.PrintTrains(trainVector);
    std::cout << "Print trains: " << timer.Stop() << " ms" << std::endl;
    
    std::cout << "Total: " << totalTimer.Stop() << " ms" << std::endl;

    return 0;
}