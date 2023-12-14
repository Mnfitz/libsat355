#ifndef APP355_H
#define APP355_H
#if (!WIN32)
#define gmtime_s(x, y) (gmtime_r(y, x))
#define _get_timezone(x)
#define _snprintf_s (snprintf)
#endif // WIN32

int main(int argc, char* argv[]);

#endif // APP355_H