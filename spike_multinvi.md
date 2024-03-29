Needs original implementation, current implementation, proposal for 2 classes; single and multithreaded.

This is the original single threaded implementation. It generated TLE orbit data and implemented TLE calculations and sorting.
Original:
```
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
private:
    using OrbitalDataVector = std::tuple<std::mutex, std::vector<OrbitalData>>;
    using orbit_iterator = std::vector<OrbitalData>::iterator;

// Implementation
private:
    virtual std::vector<sat355::TLE> OnReadFromFile(int argc, char* argv[]);
    virtual void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector);
    virtual void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);
    virtual std::vector<std::vector<OrbitalData>> OnCreateTrains(const std::vector<OrbitalData>& orbitalVector);
    virtual void OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector);
};
```

This is the current multi-threaded implementation. After the initial viersion was finished, I added multithreading features such that TLE calculate and sort could run on multiple threads. This is the form it now took following these changed. *Note* The interface is unchanged, but the implementation the interface calls did change. My development approach was to keep the original single threaded implementations working, and then extend the functions to have additional multithreaded implementations. This way, we'd always have a baseline which we know works.
Current:
```
class SatOrbit
{
 // Interface
public:
+   SatOrbit(std::size_t inNumThreads = 1);
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
+   using IteratorPairVector = std::vector<std::tuple<orbit_iterator, orbit_iterator>>;

// Implementation
private:
    virtual std::vector<sat355::TLE> OnReadFromFile(int argc, char* argv[]);
    virtual void OnCalculateOrbitalData(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector);
+   virtual void OnCalculateOrbitalDataMulti(const tle_const_iterator& tleBegin, const tle_const_iterator& tleEnd, OrbitalDataVector& ioDataVector);
    virtual void OnSortOrbitalVector(std::vector<OrbitalData>& ioOrbitalVector);
+   virtual void OnSortOrbitalVectorMulti(orbit_iterator& inBegin, orbit_iterator& inEnd);
+   virtual void OnSortMergeVector(orbit_iterator& ioBegin, orbit_iterator& ioMid, orbit_iterator& ioEnd);
    virtual std::vector<std::vector<OrbitalData>> OnCreateTrains(const std::vector<OrbitalData>& orbitalVector);
    virtual void OnPrintTrains(const std::vector<std::vector<OrbitalData>>& trainVector);

+// Helper
+private:
+   static bool SortPredicate(const OrbitalData& inLHS, const OrbitalData& inRHS);
+   void CalculateOrbitalDataMulti(const std::vector<sat355::TLE>& inTLEVector, OrbitalDataVector& ioDataVector);
+   void SortOrbitalVectorMulti(std::vector<OrbitalData>& ioOrbitalVector);

+// Data Members
+private:
+   std::size_t mNumThreads{0};
};
```

Q: Could multithreading be achieved by making a new Multithreading class, as opposed to adding multithreading to a working single threaded class? 
Q: Can we benefit from increased modularity and adds increased functionality without disrupting an already working implementation?
Q: Is-a VS Has-a? Pros and cons of each? 
Proposed:
```
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
```

- Has-a
Pros:
Increased modularity
Simpler to implement
Interface is independent of any dependencies; meaning if any dependencies change, the interface is unaffected
Aggregation is preferred 9 times out of 10 over inheritance, since it allows objects to share information and operations. 

Cons:
Users of the old API must update their code to utilize new APIs
No backwards compatibility


- Is-a
Pros:
Follows conventions of NVI
Inheritance allows for multiple implementations
Allows updating of API behind the scenes, so users won't have to update API to get new features

Cons:
More complex
Inheritance not typically preferred, as it leads to higher coupling between classes, making it more rigid and less modular
Violates encapsulation principle, as methods and data members can be accessed by derived classes


Is-a is less commonly used, but would be preferred for this project, since we want high coupling and to be able to modify the implementation for existing API.