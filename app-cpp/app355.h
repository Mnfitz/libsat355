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
    SatOrbit() = default;
    virtual ~SatOrbit() = default;
    std::vector<sat355::TLE> ReadFromFile(int argc, char* argv[]);
    std::vector<OrbitalData> CalculateOrbitalData(const std::vector<sat355::TLE>& tleVector);
    void SortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);
    std::vector<std::vector<OrbitalData>> CreateTrains(const std::vector<OrbitalData>& orbitalVector);
    void PrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector);
    static std::unique_ptr<SatOrbit> Make(SatOrbitKind inKind = SatOrbitKind::kDefault);

// Types
protected:
    using OrbitalDataVector = std::tuple<std::mutex, std::vector<OrbitalData>>;
    using orbit_iterator = std::vector<OrbitalData>::iterator;

// Implementation
private:
    // SatOrbit
    virtual std::vector<sat355::TLE> OnReadFromFile(int argc, char* argv[]) = 0;
    virtual void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector) = 0;
    virtual void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector) = 0;
    virtual std::vector<std::vector<OrbitalData>> OnCreateTrains(const std::vector<OrbitalData>& orbitalVector) = 0;
    virtual void OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector) = 0;
};
#pragma endregion {}

int main(int argc, char* argv[]);

#endif // APP355_H