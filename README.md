# `libsat355`
Libsat355 is a library encompassing all Zeptomoby orbital calculation code, which is then utilized in Mobilesat. </br>
Libsat355 is multi-platform compatible, for Windows, iOS, and MacOS. While the library is written using C++, its C briding header allows the program to work on multiple platforms. </br>
The Mac build differs from the iOS build as iOS is saved as a static library, rather than a dynamic one. </br>
It utilizes NVI archetecture, which allows the implementation to be updated without having to change the interface. </br>
This functionality has already been used to allow the implementation to utilize multithreading. </br>
The profiler indicates that the majority of the time is currently spent sorting the TLE data. </br>
As this time is mainly spent in the quicksort method, not much optimization is needed, other than possibly finding a more specialized sorting method. </br>

## Components:
Library
+   DLL (Windows)
+   Dylib (Mac)
+   Static Library (iOS)

### Unit Tests
+ test1.cpp
+ StarlinkTLE.txt

### Build Instructions
```
cd libsat355
Invoke the following CMake command: `cmake --build ./build`
```

### Build Requirements
+ Visual Studio Code v 1.86.1 or + Visual Studio 22
+ Xcode version 15 (latest)