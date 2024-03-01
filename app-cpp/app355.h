#ifndef APP355_H
#define APP355_H
#if (!WIN32)
#define gmtime_s(x, y) (gmtime_r(y, x))
#define _get_timezone(x)
#define _snprintf_s (snprintf)
#endif // WIN32

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
#include <thread>
#include <vector>

// self
#include "libsat355.h"

// All public interface methods in .hpp files should exist in a named namespace
namespace app355 {

#pragma region class OrbitalData
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
#pragma endregion {}

//--------------------------------------------------
#pragma region class SatOrbit
// abstract base class

/// @brief SatOrbit acts as the API for calculating then sorting satellite orbits into groups or, trains"
class SatOrbit
{
public:
    enum class SatOrbitKind
    {
        kDefault = 0,
        kSingle,
        kMulti
    };

 // Interface
public:
    /// @brief Creates a new SatOrbit object 
    /// @param inKind Determines whether multithreading or singlethreading is utilized in computation
    /// @return std::unique_ptr pointing to the newly created SatOrbit object
    static std::unique_ptr<SatOrbit> Make(SatOrbitKind inKind = SatOrbitKind::kDefault);

    /// @brief Scans the inputted text file for satellite TLE data
    /// @param inArgc The number of arguments passed into main()
    /// @param inArgv A string pointing to the location of a text file containing satellite TLE data
    /// @return Vector of all read TLE data
    std::vector<sat355::TLE> ReadFromFile(int inArgc, char* inArgv[]);

    /// @brief Turns the raw TLE data into latitude, longitude, and altitude
    /// @param inTleVector Vector of parsed TLE data
    /// @return Vector of computed orbital data
    std::vector<OrbitalData> CalculateOrbitalData(const std::vector<sat355::TLE>& inTleVector);

    /// @brief Sorts the vector of orbital data by their mean motion
    /// @param ioOrbitalVector Vector of unsorted orbital data
    void SortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);

    /// @brief Satellites in close proximity with a similar orbital path are grouped together, and solo satellites are discarded
    /// @param inOrbitalVector Vector of all sorted orbital data by which the train list is made from
    /// @return Vector of all satellites which can be grouped into trains, where a train is a vector of satellites
    std::vector<std::vector<OrbitalData>> CreateTrains(const std::vector<OrbitalData>& inOrbitalVector);

    /// @brief Prints all satellite data
    /// @param inTrainVector Vector of all satellite trains
    void PrintTrains(const std::vector<std::vector<OrbitalData>>& inTrainVector);

    /// @brief dtor is default, giving access to RO5 methods
    virtual ~SatOrbit() = default;

protected:
    // Hidden because the class is abstract, and thus cannot be constructed on its own
    SatOrbit() = default;

// Types
protected:
    using OrbitalDataVector = std::tuple<std::mutex, std::vector<OrbitalData>>;
    using orbit_iterator = std::vector<OrbitalData>::iterator;

// Implementation
private:
    // SatOrbit
    virtual std::vector<sat355::TLE> OnReadFromFile(int inArgc, char* inArgv[]) = 0;
    virtual void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector) = 0;
    virtual void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector) = 0;
    virtual std::vector<std::vector<OrbitalData>> OnCreateTrains(const std::vector<OrbitalData>& inOrbitalVector) = 0;
    virtual void OnPrintTrains(const std::vector<std::vector<OrbitalData>>& inTrainVector) = 0;
};
#pragma endregion {}
} // namespace app355

// Main is the only function in global namespace
int main(int inArgc, char* inArgv[]);

#endif // APP355_H