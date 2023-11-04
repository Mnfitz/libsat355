#ifndef DLL_EXPORT
	#define DLL_EXPORT /* NOTHING */

	#if defined(WIN32) || defined(WIN64)
		#undef DLL_EXPORT
		#if defined(DLL_EXPORTS)
			#define DLL_EXPORT __declspec(dllexport)
		#else
			#define DLL_EXPORT __declspec(dllimport)
		#endif // defined(DLL_EXPORTS)
	#endif // defined(WIN32) || defined(WIN64)

	#if defined(__GNUC__) || defined(__APPLE__) || defined(LINUX)
		#if defined(DLL_EXPORTS)
			#undef DLL_EXPORT
			#define DLL_EXPORT __attribute__((visibility("default")))
		#endif // defined(DLL_EXPORTS)
	#endif // defined(__GNUC__) || defined(__APPLE__) || defined(LINUX)

#endif // !defined(DLL_EXPORT)