#ifndef APP355_H
#define APP355_H
#if (!WIN32)
#define gmtime_s(x, y) (gmtime_r(y, x))
#define _get_timezone(x)
#define _snprintf_s (snprintf)
#endif // WIN32

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

namespace 
{
#if 0
#pragma region class SatOrbit
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
    using OrbitalDataVector = std::tuple<std::mutex, std::vector<OrbitalData>>;
    using orbit_iterator = std::vector<OrbitalData>::iterator;
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
};
#pragma endregion {}
#else

//--------------------------------------------------

#pragma region class SatOrbit
// abstract base class
class SatOrbit
{
 // Interface
public:
    SatOrbit() = default;
    virtual ~SatOrbit() = default;

    std::vector<sat355::TLE> ReadFromFile(int argc, char* argv[]);
    std::vector<OrbitalData> CalculateOrbitalData(const std::vector<sat355::TLE>& tleVector);
    void SortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);
    std::vector<std::vector<OrbitalData>> CreateTrains(const std::vector<OrbitalData>& orbitalVector);
    void PrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector);

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

//--------------------------------------------------

#pragma region class SatOrbitSingle

class SatOrbitSingle : public SatOrbit
{
public:
    SatOrbitSingle() = default;
    virtual ~SatOrbitSingle() = default;

// Implementation
private:
    // SatOrbit
    std::vector<sat355::TLE> OnReadFromFile(int argc, char* argv[]) override;
    void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector) override;
    void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector) override;
    std::vector<std::vector<OrbitalData>> OnCreateTrains(const std::vector<OrbitalData>& orbitalVector) override;
    void OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector) override;
};
#pragma endregion {}

//--------------------------------------------------

#pragma region class SatOrbitMulti
// Multi thread class: Is-a SatOrbit
class SatOrbitMulti : public SatOrbitSingle
{
// Interface
public:
    SatOrbitMulti(std::size_t inNumThreads = 1);
    ~SatOrbitMulti() override = default;

// Types
private:
    using tle_const_iterator = std::vector<sat355::TLE>::const_iterator;
    using IteratorPairVector = std::vector<std::tuple<orbit_iterator, orbit_iterator>>;

// Implementation
private:
    // SatOrbit
    void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector) override;
    void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector) override;

    // SatOrbitMulti
    virtual void OnCalculateOrbitalDataMulti(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataVector& ioDataVector);
    virtual void OnSortOrbitalVectorMulti(orbit_iterator& inBegin, orbit_iterator& inEnd);
    virtual void OnSortMergeVector(orbit_iterator& ioBegin, orbit_iterator& ioMid, orbit_iterator& ioEnd);

// Data Members
private:
    std::size_t mNumThreads{0};
};
#pragma endregion {}
#endif

//--------------------------------------------------

#pragma region class Timer

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
#pragma endregion {}
} // anonymous namespace

int main(int argc, char* argv[]);

#endif // APP355_H