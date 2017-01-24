
#ifndef ARA_INTERNAL_STANDARD_DATETIME_H
#define ARA_INTERNAL_STANDARD_DATETIME_H

#include "../ara_def.h"
#include "string_convert.h"
#include <time.h>

#include <ctime>
#include <iomanip>
#include <chrono>
#include <memory>

#ifdef ARA_WIN32_VER
	#include <windows.h>
#endif

namespace ara {

	namespace internal {
		class standard_time_traits
		{
		public:
			static inline void	set_time(time_t & tar, time_t src) {
				tar = src;
			}
			static inline time_t get_time(time_t src) {
				return src;
			}
		};

		class standard_time_op 
		{
		public:
			static inline time_t	mktime(int year, int month, int day, int hour, int minuite, int sec, int isdst) {
				struct tm  timeinfo = { 0 };
				timeinfo.tm_year = year - 1900;
				timeinfo.tm_mon = month - 1;
				timeinfo.tm_mday = day;
				timeinfo.tm_hour = hour;
				timeinfo.tm_min = minuite;
				timeinfo.tm_sec = sec;
				timeinfo.tm_isdst = isdst;
				return ::mktime(&timeinfo);
			}

			static inline time_t mktime(const struct tm & data) {
				return ::mktime( const_cast<struct tm *>(&data) );
			}

			static inline void local_time(time_t t, struct tm & info) {
#ifdef ARA_WIN32_VER
				::localtime_s(&info, &t);
#else
				::localtime_r(&t, &info);
#endif
			}
			static inline void gmt_time(time_t t, struct tm & info) {
#ifdef ARA_WIN32_VER
				::gmtime_s(&info, &t);
#else
				::gmtime_r(&t, &info);
#endif
			}

			static inline void local_time(time_t t, int & year, int & month, int & day, int & hour, int & minuite, int & sec, int & yday, int & wday, int & isdst) {
				struct tm info = { 0 };
#ifdef ARA_WIN32_VER
				::localtime_s(&info, &t);
#else
				::localtime_r(&t, &info);
#endif
				year = info.tm_year + 1900;
				month = info.tm_mon + 1;
				day = info.tm_mday;
				hour = info.tm_hour;
				minuite = info.tm_min;
				sec = info.tm_sec;
				yday = info.tm_yday;
				wday = info.tm_wday;
				isdst = info.tm_isdst;
			}

			static std::wstring time_format(const struct tm & info, const wchar_t * lpFormat) {
				const size_t nDefaultSize = 256;
				wchar_t buf[nDefaultSize] = { 0 };

				size_t nBufSize = nDefaultSize;
				wchar_t * p = buf;
				std::unique_ptr<wchar_t[]>	pBuf;

				while (std::wcsftime(p, nBufSize - 1, lpFormat, &info) == 0) {
					nBufSize += (nBufSize >> 1);
					pBuf.reset(new wchar_t[nBufSize]);
					p = pBuf.get();
				}
				return std::wstring(p);
			}

			static std::string time_format(const struct tm & info, const char * lpFormat) {
				const size_t nDefaultSize = 256;
				char buf[nDefaultSize] = { 0 };

				size_t nBufSize = nDefaultSize;
				char * p = buf;
				std::unique_ptr<char[]>	pBuf;

				while (std::strftime(p, nBufSize - 1, lpFormat, &info) == 0) {
					nBufSize += (nBufSize >> 1);
					pBuf.reset(new char[nBufSize]);
					p = pBuf.get();
				}
				return std::string(p);
			}

			template<typename T, typename Ret>
			static Ret local_format(time_t t, const T * lpFormat) {
				struct tm info = { 0 };
#ifdef ARA_WIN32_VER
				::localtime_s(&info, &t);
#else
				::localtime_r(&t, &info);
#endif
				return time_format(info, lpFormat);
			}

			template<typename T, typename Ret>
			static Ret gmt_format(time_t t, const T * lpFormat) {
				struct tm info = { 0 };
#ifdef ARA_WIN32_VER
				gmtime_s(&info, &t);
#else
				gmtime_r(&t, &info);
#endif
				return time_format(info, lpFormat);
			}

			static bool  parse(struct tm & info, const char * lpFormat, const char * content) {
#ifdef ARA_WIN32_VER
				std::stringstream ss(content);
				ss >> std::get_time(&info, lpFormat);
				return !ss.fail();
#else
				return ::strptime(content, lpFormat, &info) != NULL;
#endif
			}

			static bool  parse(struct tm & info, const wchar_t * lpFormat, const wchar_t * content) {
#ifdef ARA_WIN32_VER
				std::wstringstream ss(content);
				ss >> std::get_time(&info, lpFormat);
				return !ss.fail();
#else
				std::string strFormat, strContent;
				string_convert::append(strFormat, lpFormat, std::wcslen(lpFormat));
				string_convert::append(strContent, content, std::wcslen(content));
				return ::strptime(strContent.c_str(), strFormat.c_str(), &info) != NULL;
#endif
			}


			static void get_current(time_t & t, long & ns) {
#ifdef ARA_WIN32_VER
				static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);
				SYSTEMTIME  system_time;
				FILETIME    file_time;
				uint64_t    time;

				GetSystemTime(&system_time);
				SystemTimeToFileTime(&system_time, &file_time);
				time = ((uint64_t)file_time.dwLowDateTime);
				time += ((uint64_t)file_time.dwHighDateTime) << 32;

				t = (time_t)((time - EPOCH) / 10000000L);
				ns = (long)(system_time.wMilliseconds * 1000000);
#else
				struct timespec ts;
				clock_gettime(CLOCK_MONOTONIC, &ts);
				t = ts.tv_sec;
				ns = ts.tv_nsec;
#endif
			}

		};
	}
}

#endif//ARA_INTERNAL_STANDARD_DATETIME_H
