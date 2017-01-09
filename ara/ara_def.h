
#ifndef ARA_DEF_H
#define ARA_DEF_H

#if defined(_WIN32) || defined(__WIN32__) || defined(WINDOWS) || defined(WIN32)
	#define ARA_WIN32_VER
	#ifdef _WIN64
		#define ARA_WIN64_VER
	#endif

	#ifdef _MSC_VER
		#if (_MSC_VER >= 1900)
			#define ARA_WIN32_VS2015_VER
			#define ARA_WIN32_MSVC14_VER
		#elif (_MSC_VER >= 1800)
			#define ARA_WIN32_VS2013_VER
			#define ARA_WIN32_MSVC12_VER
		#elif (_MSC_VER >= 1700)
			#define ARA_WIN32_VS2012_VER
			#define ARA_WIN32_MSVC11_VER
		#elif (_MSC_VER >= 1600)
			#define ARA_WIN32_VS2010_VER
			#define ARA_WIN32_MSVC10_VER
		#elif (_MSC_VER >= 1500)
			#define ARA_WIN32_VS2008_VER
			#define ARA_WIN32_MSVC9_VER
		#elif (_MSC_VER >= 1400)
			#define ARA_WIN32_VS2005_VER
			#define ARA_WIN32_MSVC8_VER
		#elif (_MSC_VER >= 1310)
			#define ARA_WIN32_VS2003_VER
		#elif (_MSC_VER >= 1300)
			#define ARA_WIN32_MSVC7_VER
		#else
			#define ARA_WIN32_VS6_VER
		#endif
		#define ARA_WIN32_VC_VER
	#elif defined( __BCPLUSPLUS__ )
		#define ARA_WIN32_BCB_VER
	#endif	//_MSC_VER , __BCPLUSPLUS__

	#ifdef __GNUC__
		#define ARA_WIN32_GCC_VER
	#endif

	#ifndef __GNUC__
		#pragma warning (disable : 4290)
		#pragma warning (disable : 4786)
		#pragma warning (disable : 4291)
		#pragma warning (disable : 4355)
		#pragma warning (disable : 4996)
		#pragma warning (disable : 4503)
		#pragma warning (disable : 4819)
	#endif
#endif

#if defined(__CYGWIN32__) 
	#define ARA_WIN32_CYGWIN_VER
#endif
#if defined(__MINGW32__)	
	#define ARA_WIN32_MINGW_VER
#endif
#if  defined(LINUX) || defined(__linux)
	#define ARA_LINUX_VER
#endif  //LINUX
#if defined(__APPLE__)
	#define ARA_APPLE_VER
#endif //__APPLE__    

#ifdef __GNUC__
	#define ARA_GCC_VER
	#if __GNUC__ > 3
		#define ARA_GCC_4_VER
	#elif __GNUC__ > 2
		#define ARA_GCC_3_VER
	#elif __GNUC__ > 1
		#define ARA_GCC_2_VER
	#endif
#endif

#undef LIKELY
#undef UNLIKELY

#if defined(__GNUC__) && __GNUC__ >= 4
	#define LIKELY(x) (__builtin_expect((x), 1))
	#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
	#define LIKELY(x) (x)
	#define UNLIKELY(x) (x)
#endif

#if defined(__GNUC__)
	#define ARA_FUNC_NAME		__PRETTY_FUNCTION__
#elif defined(ARA_WIN32_VC_VER)
	#define ARA_FUNC_NAME		__FUNCTION__
#else
	#define ARA_FUNC_NAME		__func__
#endif

#endif // ARA_DEF_H
