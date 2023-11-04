#ifndef LIBSAT355_H
#define LIBSAT355_H

#include <stdio.h>
#include "dllmain.h"

// TRICKY: Make sure only C++ compiles this.
// When compiled from C or SwiftUI, it is excluded
#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT void HelloWorld();

#ifdef __cplusplus
} // extern "C"
#endif 

#endif // LIBSAT355_H