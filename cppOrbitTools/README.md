# Integrating Zeptomoby Source Code 
The zeptomoby OrbitTools source library provides the SGP4 & SDP4 algorithms used by libsat355. This source code can be found here:<br/>
 http://www.zeptomoby.com/satellites/cppOrbitTools.zip

### ISSUE: 
The zeptomoby code assumes windows system header files.
- This will prevent compilation for macOS and iOS platforms <br/>
- For instance, cEci.cpp includes stdafx.h, which includes tchar.h; a windows only header file <br/>

### FIX: 
Use decoy headers to override poisonous stdafx.h and orbitLib.h headers + manual renaming of offending zeptomoby header files
- A set of provided decoy header files are placed in the `./overrides` directory <br/>
- Manual renaming of problem headers in Zeptomoby source drop is also needed

Note: CLang always prioritizes reading header files from the same directory as the source file currently being processed. This circumvents any `-I` header search path, or `PCH` solution.<br/>

### MANUAL WORKAROUND: 
When updating to any future versions of zeptomoby source code, ensure that offending header files are overrided and renamed.
- Affected headers:
    - cppOrbitTools/orbitTools/core/stdafx.h
    - cppOrbitTools/orbitTools/orbit/stdafx.h
    - cppOrbitTools/orbitTools/orbit/orbitLib.h

- Renamed to:
    - cppOrbitTools/orbitTools/core/_stdafx.h
    - cppOrbitTools/orbitTools/orbit/_stdafx.h
    - cppOrbitTools/orbitTools/orbit/_orbitLib.h