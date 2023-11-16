// dllmain.cpp : Defines the entry point for the DLL application.
#include <windows.h>

/*
TRICKY: BOOL is a typedef that arrives from <windows.h>.
For an API that is cross-platform, it will cause undefined symbol compile errors when used on say Mac.
A cross-platform return type would have to be something like int instead.

That being said: dllmain.cpp is "windows specific".
so this is the exception where BOOL is fine, because your Mac version will have a completely different
source file that is used for it's "shared object (.so, instead of .dll)" entry point.
*/

BOOL APIENTRY DllMain( HMODULE inModule,
                       DWORD  inCall,
                       LPVOID inReserved)
{
    switch (inCall)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
