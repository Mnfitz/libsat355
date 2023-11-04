#include "helloworld.hpp"
#include <windows.h>
void HelloWorld() 
{
	MessageBox(NULL, L"Exported test() function loaded", L"Success", 0);
}