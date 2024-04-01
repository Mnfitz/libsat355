// Self
#include "app355.h"

// Anonymous namespace should only exist in .cpp
namespace /*anonymous*/ {
//----------------------------------------
#pragma region SatOrbitSingle

class SatOrbitSingle : public app355::SatOrbit
{
public:
    SatOrbitSingle() = default;
    virtual ~SatOrbitSingle() = default;
    static std::unique_ptr<SatOrbitSingle> Make();

// Helper
protected:
    static bool SortPredicate(const app355::OrbitalData& inLHS, const app355::OrbitalData& inRHS);

// Implementation
private:
    // SatOrbit
    std::vector<sat355::TLE> OnReadFromFile(int inArgc, char* inArgv[]) override;
    void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector) override;
    void OnSortOrbitalVector(std::vector<app355::OrbitalData>& ioOrbitalVector) override;
    std::vector<std::vector<app355::OrbitalData>> OnCreateTrains(const std::vector<app355::OrbitalData>& inOrbitalVector) override;
    void OnPrintTrains(const std::vector<std::vector<app355::OrbitalData>>& inTrainVector) override;
};

std::unique_ptr<SatOrbitSingle> SatOrbitSingle::Make()
{
    auto satOrbit = std::make_unique<SatOrbitSingle>();
    return satOrbit;
}

// Single Threaded
void SatOrbitSingle::OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector)
{   
    auto tleBegin = inTLEVector.begin();
    auto tleEnd = inTLEVector.end();
    const auto size = static_cast<std::size_t>(std::distance(tleBegin, tleEnd));
    std::vector<app355::OrbitalData> orbitalVector{};
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
        
        long long testTime = time(nullptr);
        //long long testTime = 1705781559; // Time TLEs stored in StarlinkTLE.txt were recorded

        int result = orbit_to_lla(testTime, inTLE.GetName().data(), inTLE.GetLine1().data(), inTLE.GetLine2().data(), &out_tleage, &out_latdegs, &out_londegs, &out_altkm);
        if (result == 0)
        {
            // Warning! Cannot use tle anymore as it has been moved!
            app355::OrbitalData data(inTLE, out_latdegs, out_londegs, out_altkm);
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

void SatOrbitSingle::OnSortOrbitalVector(std::vector<app355::OrbitalData>& ioOrbitalVector)
{
    std::sort(ioOrbitalVector.begin(), ioOrbitalVector.end(), [](const app355::OrbitalData& inLHS, const app355::OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

// Thread Independent
std::vector<sat355::TLE> SatOrbitSingle::OnReadFromFile(int inArgc, char* inArgv[])
{
    if (inArgc < 2) 
    {
        std::cout << "Please provide a file path." << std::endl;
        const auto err = std::make_error_code(std::errc::invalid_argument);
        throw std::filesystem::filesystem_error("Path not given", err);
    }

    std::filesystem::path filePath(inArgv[1]);

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

    std::vector<app355::OrbitalData> orbitalVector;
    std::string readLine;

    std::string name{};
    std::string line1{};
    std::string line2{};
    
    std::vector<sat355::TLE> tleVector;

    while (!fileStream.eof())
    {
        std::getline(fileStream, name);
        std::getline(fileStream, line1);
        std::getline(fileStream, line2);

       sat355::TLE newTLE{name, line1, line2};
        tleVector.push_back(std::move(newTLE));
    }
    
    return tleVector;
}

std::vector<std::vector<app355::OrbitalData>> SatOrbitSingle::OnCreateTrains(const std::vector<app355::OrbitalData>& inOrbitalVector)
{
    std::vector<std::vector<app355::OrbitalData>> trainVector;
    std::vector<app355::OrbitalData> newTrain;

    double prevMeanMotion = 0;
    double prevInclination = 0;

    std::for_each(inOrbitalVector.begin(), inOrbitalVector.end(), [&](auto& data)
    {
        double deltaMotion = abs(data.GetTLE().GetMeanMotion() - prevMeanMotion);
        double deltaInclination = abs(data.GetTLE().GetInclination() - prevInclination);
        if ((deltaMotion > 0.0001 || deltaInclination > 0.0001) && !newTrain.empty())
        {
            // Sort by longitude
            std::sort(newTrain.begin(), newTrain.end(), [](const app355::OrbitalData& inLHS, const app355::OrbitalData& inRHS) -> bool
            {
                return inLHS.GetLongitude() < inRHS.GetLongitude();
            });

            // Filter out wandering satellites
            if (newTrain.size() > 3)
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
                assert(static_cast<std::ptrdiff_t>(j) >= 0);
                trainVector.erase(trainVector.begin() + static_cast<std::ptrdiff_t>(j));
                --j;
            }
        }
    }
    
    return trainVector;
}

void SatOrbitSingle::OnPrintTrains(const std::vector<std::vector<app355::OrbitalData>>& inTrainVector)
{
    int trainCount = 0;
    // Print the contents of the train list

    std::for_each(inTrainVector.begin(), inTrainVector.end(), [&](auto& train)
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

// Helper
/*static*/ bool SatOrbitSingle::SortPredicate(const app355::OrbitalData& inLHS, const app355::OrbitalData& inRHS)
{
    return inLHS.GetTLE().GetMeanMotion() < inRHS.GetTLE().GetMeanMotion();
}

#pragma endregion {}

//----------------------------------------
#pragma region SatOrbitMulti

class SatOrbitMulti : public SatOrbitSingle
{
// Interface
public:
    SatOrbitMulti(std::size_t inNumThreads) :
        mNumThreads{inNumThreads}
    {
        // Do nothing
    }
    ~SatOrbitMulti() override = default;
    static std::unique_ptr<SatOrbitMulti> Make(std::size_t inThreads = 4);

// Types
private:
    using tle_const_iterator = std::vector<sat355::TLE>::const_iterator;
    using IteratorPairVector = std::vector<std::tuple<orbit_iterator, orbit_iterator>>;

// Implementation
private:
    // SatOrbit
    void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector) override;
    void OnSortOrbitalVector(std::vector<app355::OrbitalData>& ioOrbitalVector) override;

    // SatOrbitMulti
    virtual void OnCalculateOrbitalDataMulti(const tle_const_iterator& inTleBegin, const tle_const_iterator& inTleEnd, OrbitalDataVector& ioDataVector);
    virtual void OnSortOrbitalVectorMulti(orbit_iterator& inBegin, orbit_iterator& inEnd);
    virtual void OnSortMergeVectorMulti(orbit_iterator& ioBegin, orbit_iterator& ioMid, orbit_iterator& ioEnd);

// Data Members
private:
    std::size_t mNumThreads{0};
};

std::unique_ptr<SatOrbitMulti> SatOrbitMulti::Make(std::size_t inThreads)
{
    auto satOrbit = std::make_unique<SatOrbitMulti>(inThreads);
    return satOrbit;
}

 // SatOrbit
void SatOrbitMulti::OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector)
{
        OnCalculateOrbitalDataMulti(inTLEVector.begin(), inTLEVector.end(), ioDataVector);
}

void SatOrbitMulti::OnSortOrbitalVector(std::vector<app355::OrbitalData>& ioOrbitalVector)
{
    // Multithreaded sort
    auto begin = ioOrbitalVector.begin();
    auto end = ioOrbitalVector.end();
    OnSortOrbitalVectorMulti(begin, end);
}

// SatOrbitMulti
void SatOrbitMulti::OnCalculateOrbitalDataMulti(const tle_const_iterator& inTleBegin, const tle_const_iterator& inTleEnd, OrbitalDataVector& ioDataVector)
{
    const auto size = static_cast<std::size_t>(std::distance(inTleBegin, inTleEnd));
    std::vector<app355::OrbitalData> orbitalVector{};
    orbitalVector.reserve(size);
    std::for_each(inTleBegin, inTleEnd, [&](const sat355::TLE& inTLE)
    {
        double out_tleage = 0.0;
        double out_latdegs = 0.0;
        double out_londegs = 0.0;
        double out_altkm = 0.0;

        // Update TLE list with web address
        // https://celestrak.org/NORAD/elements/gp.php?NAME=Starlink&FORMAT=TLE
        // get current time as a long long in seconds
        
        long long testTime = time(nullptr);
        //long long testTime = 1705781559; // Time TLEs stored in StarlinkTLE.txt were recorded

        int result = orbit_to_lla(testTime, inTLE.GetName().data(), inTLE.GetLine1().data(), inTLE.GetLine2().data(), &out_tleage, &out_latdegs, &out_londegs, &out_altkm);
        if (result == 0)
        {
            // Warning! Cannot use tle anymore as it has been moved!
            app355::OrbitalData data(inTLE, out_latdegs, out_londegs, out_altkm);
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

void SatOrbitMulti::OnSortOrbitalVectorMulti(orbit_iterator& inBegin, orbit_iterator& inEnd)
{
    // Sort by mean motion
    std::sort(inBegin, inEnd, [](const app355::OrbitalData& inLHS, const app355::OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

void SatOrbitMulti::OnSortMergeVectorMulti(orbit_iterator& ioBegin, orbit_iterator& ioMid, orbit_iterator& ioEnd)
{
    std::inplace_merge(ioBegin, ioMid, ioEnd, [](const app355::OrbitalData& inLHS, const app355::OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

#pragma endregion{}

//----------------------------------------
#pragma region Timer

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
#pragma endregion {}
} // anonymous namespace

// All methods declared within a namespace in .hpp should be defined within the same namespace in .cpp
namespace app355 {
//----------------------------------------
#pragma region SatOrbit

// Public Non-Virtual Interface
std::unique_ptr<SatOrbit> SatOrbit::Make(SatOrbitKind inKind)
{
    std::unique_ptr<SatOrbit> result = nullptr;
    const std::size_t coreCount = std::thread::hardware_concurrency();

    switch (inKind)
    {
    case SatOrbitKind::kDefault:
        // [[fallthrough]] tells the compiler that the fallthrough is intentional, and to not make a warning
        [[fallthrough]];
    case SatOrbitKind::kMulti:
        result = SatOrbitMulti::Make(coreCount);
        break;
    case SatOrbitKind::kSingle:
        result = SatOrbitSingle::Make();
        break;
    }

    return result;
}

std::vector<sat355::TLE> SatOrbit::ReadFromFile(int inArgc, char* inArgv[])
{
    return OnReadFromFile(inArgc, inArgv);
}

std::vector<OrbitalData> SatOrbit::CalculateOrbitalData(const std::vector<sat355::TLE>& inTleVector)
{
    OrbitalDataVector orbitalVector{};
    OnCalculateOrbitalData(inTleVector, orbitalVector);
    return std::move(std::get<1>(orbitalVector));
}

void SatOrbit::SortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector)
{
    OnSortOrbitalVector(ioOrbitalVector);
}

std::vector<std::vector<OrbitalData>> SatOrbit::CreateTrains(const std::vector<OrbitalData>& inOrbitalVector)
{
    return OnCreateTrains(inOrbitalVector);
}

void SatOrbit::PrintTrains(const std::vector<std::vector<OrbitalData>>& inTrainVector)
{
    OnPrintTrains(inTrainVector);
}

#pragma endregion {}

} // namespace app355

//----------------------------------------
// Main is the only function in global namespace
int main(int inArgc, char* inArgv[])
{
#if 0
    // HACK mnfitz 25mar2024: sample bad code for testing address sanitizer
    /*
    AddressSanitizer is designed to detect various memory-related errors in C++ programs:
    
    Buffer Overflows 
    Use-After-Free
    Memory Leaks
    Invalid Memory Access
    Stack Buffer Overflows
    Heap Buffer Overflows
    Use of Uninitialized Variables
    Double Free
    Memory Access Alignment Issues
    Global Buffer Overflows
    */
    char boompis[3] = { 1, 2, 3 };
    boompis[3] = 52;

    const char* name = "ISS(ZARYA)";
    const char* line1 = "1 25544U 98067A   23320.50172660  .00012336  00000+0  22877-3 0  9990";
    const char* line2 = "2 25544  51.6432 294.0998 0000823 293.3188 166.8114 15.49366195425413";

    auto beempis = std::make_unique<sat355::TLE>(name, line1, line2);
    beempis->GetName();
    beempis.reset();
    //beempis->GetName();

    auto leempis = std::make_unique<sat355::TLE>(name, line1, line2);
    auto doompis = leempis.release();
    doompis->GetName();
    delete doompis;
    //delete doompis;

#elif 1
    // HACK mnfitz 25mar2024: sample bad code for testing undefined behavior sanitizer
    /*
    UndefinedBehaviorSanitizer is designed to catch various kinds of undefined behavior in C++ programs during execution:
    
    Array Subscript Out of Bounds
    Bitwise Shifts Out of Bounds
    Dereferencing Misaligned or Null Pointers
    Signed Integer Overflow
    Floating-Point Type Conversions That Overflow
    Conversion to, from, or Between Floating-Point Types That Would Overflow
    Invalid use of unsigned integers
    Division by zero
    */
    int k = 0x7fffffff; // Maximum positive value for a signed int
    k += 1;

    //int y = 1 << 40;

    //int* ptr = nullptr;
    //*ptr = 42;

    int z = INT_MAX;
    float f = static_cast<float>(z);

#elif 0
    /*
    ThreadSanitizer is designed to detect data race bugs in C/C++ programs:

    Data Races: Data races occur when two threads access the same variable concurrently, 
    and at least one of the accesses is a write. TSan detects these races, 
    which are common and notoriously difficult to debug in concurrent systems. 
    The C++11 standard officially considers data races as undefined behavior
    */
#endif

    // measure total time in milliseconds using chrono 
    auto startTotal = std::chrono::high_resolution_clock::now();

    // 4 threads is baseline
    auto stdPtr = app355::SatOrbit::Make(app355::SatOrbit::SatOrbitKind::kMulti);
    app355::unique_ptr<app355::SatOrbit> satOrbit2(stdPtr.release());
    app355::shared_ptr<app355::SatOrbit> satOrbit(satOrbit2.release());
    {
        app355::shared_ptr<app355::SatOrbit> shared2(satOrbit);
        app355::shared_ptr<app355::SatOrbit> shared3 = satOrbit;
        app355::weak_ptr<app355::SatOrbit> shared4 = satOrbit;
    }

    // meaure time for each section in milliseconds using chrono
    Timer totalTimer{};
    Timer timer{};
    totalTimer.Start();

    timer.Start();
    std::vector<sat355::TLE> tleVector{satOrbit->ReadFromFile(inArgc, inArgv)};
    std::cout << "Read from file: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    std::vector<app355::OrbitalData> orbitalVector{satOrbit->CalculateOrbitalData(tleVector)};
    std::cout << "Calculate orbital data: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    satOrbit->SortOrbitalVector(orbitalVector);
    std::cout << "Sort orbital list: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    std::vector<std::vector<app355::OrbitalData>> trainVector{satOrbit->CreateTrains(orbitalVector)};
    std::cout << "Create trains: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    satOrbit->PrintTrains(trainVector);
    std::cout << "Print trains: " << timer.Stop() << " ms" << std::endl;
    
    std::cout << "Total: " << totalTimer.Stop() << " ms" << std::endl;

    return 0;
}