// Self
#include "app355.h"

namespace {
//----------------------------------------
#pragma region SatOrbitSingle

class SatOrbitSingle : public SatOrbit
{
public:
    SatOrbitSingle() = default;
    virtual ~SatOrbitSingle() = default;
    static std::unique_ptr<SatOrbitSingle> Make();

// Helper
protected:
    static bool SortPredicate(const OrbitalData& inLHS, const OrbitalData& inRHS);

// Implementation
private:
    // SatOrbit
    std::vector<sat355::TLE> OnReadFromFile(int argc, char* argv[]) override;
    void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector) override;
    void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector) override;
    std::vector<std::vector<OrbitalData>> OnCreateTrains(const std::vector<OrbitalData>& orbitalVector) override;
    void OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector) override;
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

void SatOrbitSingle::OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector)
{
    std::sort(ioOrbitalVector.begin(), ioOrbitalVector.end(), [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

// Thread Independent
std::vector<sat355::TLE> SatOrbitSingle::OnReadFromFile(int argc, char* argv[])
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

std::vector<std::vector<OrbitalData>> SatOrbitSingle::OnCreateTrains(const std::vector<OrbitalData>& orbitalVector)
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

void SatOrbitSingle::OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector)
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

// Helper
/*static*/ bool SatOrbitSingle::SortPredicate(const OrbitalData& inLHS, const OrbitalData& inRHS)
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
    void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector) override;

    // SatOrbitMulti
    virtual void OnCalculateOrbitalDataMulti(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataVector& ioDataVector);
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

void SatOrbitMulti::OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector)
{
    // Multithreaded sort
    OnSortOrbitalVectorMulti(ioOrbitalVector.begin(), ioOrbitalVector.end());
}

// SatOrbitMulti
void SatOrbitMulti::OnCalculateOrbitalDataMulti(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataVector& ioDataVector)
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

void SatOrbitMulti::OnSortOrbitalVectorMulti(orbit_iterator& inBegin, orbit_iterator& inEnd)
{
    // Sort by mean motion
    std::sort(inBegin, inEnd, [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

void SatOrbitMulti::OnSortMergeVectorMulti(orbit_iterator& ioBegin, orbit_iterator& ioMid, orbit_iterator& ioEnd)
{
    std::inplace_merge(ioBegin, ioMid, ioEnd, [](const OrbitalData& inLHS, const OrbitalData& inRHS) -> bool
    {
        return SortPredicate(inLHS, inRHS);
    });
}

#pragma endregion{}

//----------------------------------------
#pragma region SatOrbit

// Public Non-Virtual Interface
std::unique_ptr<SatOrbit> SatOrbit::Make(SatOrbitKind inKind)
{
    std::unique_ptr<SatOrbit> result = nullptr;
    std::size_t coreCount = std::thread::hardware_concurrency();

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

std::vector<sat355::TLE> SatOrbit::ReadFromFile(int argc, char* argv[])
{
    return OnReadFromFile(argc, argv);
}

std::vector<OrbitalData> SatOrbit::CalculateOrbitalData(const std::vector<sat355::TLE>& tleVector)
{
    OrbitalDataVector orbitalVector{};
    OnCalculateOrbitalData(tleVector, orbitalVector);
    return std::move(std::get<1>(orbitalVector));
}

void SatOrbit::SortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector)
{
    OnSortOrbitalVector(ioOrbitalVector);
}

std::vector<std::vector<OrbitalData>> SatOrbit::CreateTrains(const std::vector<OrbitalData>& orbitalVector)
{
    return OnCreateTrains(orbitalVector);
}

void SatOrbit::PrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector)
{
    OnPrintTrains(trainVector);
}

#pragma endregion {}

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
} // anonymous namespace
#pragma endregion {}

//----------------------------------------
int main(int argc, char* argv[])
{
    // measure total time in milliseconds using chrono 
    auto startTotal = std::chrono::high_resolution_clock::now();

    // 4 threads is baseline
    auto satOrbit = SatOrbit::Make(SatOrbit::SatOrbitKind::kMulti);

    // meaure time for each section in milliseconds using chrono
    Timer totalTimer{};
    Timer timer{};
    totalTimer.Start();

    timer.Start();
    std::vector<sat355::TLE> tleVector{satOrbit->ReadFromFile(argc, argv)};
    std::cout << "Read from file: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    std::vector<OrbitalData> orbitalVector{satOrbit->CalculateOrbitalData(tleVector)};
    std::cout << "Calculate orbital data: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    satOrbit->SortOrbitalVector(orbitalVector);
    std::cout << "Sort orbital list: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    std::vector<std::vector<OrbitalData>> trainVector{satOrbit->CreateTrains(orbitalVector)};
    std::cout << "Create trains: " << timer.Stop() << " ms" << std::endl;

    timer.Start();
    satOrbit->PrintTrains(trainVector);
    std::cout << "Print trains: " << timer.Stop() << " ms" << std::endl;
    
    std::cout << "Total: " << totalTimer.Stop() << " ms" << std::endl;

    return 0;
}