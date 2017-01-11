
///		get system last error
///			cout << ara::error::code();
///			cout << ara::error::info(EAGAIN);
///			cout << ara::error::info();

#ifndef ARA_ERROR_H_201608
#define ARA_ERROR_H_201608

#include "ara_def.h"

#include <string>

#if defined(ARA_WIN32_VER)
	#include <windows.h>
#else
	#include <errno.h>
#endif//

namespace ara {

	class error
	{
	public:
		static int code() {
#if defined(ARA_WIN32_VER)
			return static_cast<int>(GetLastError());
#else
			return errno;
#endif
		}

		static std::string	info(int nCode) {

#if defined(ARA_WIN32_VER)
			DWORD errorMessageID = static_cast<DWORD>(nCode);
			if ( errorMessageID == 0 )
				return std::string();

			LPSTR messageBuffer = nullptr;
			size_t size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

			std::string message(messageBuffer, size);
			::LocalFree(messageBuffer);

			return message;			
#else
			char _buf[_POSIX_C_SOURCE];
			return ::strerror_r(nCode, _buf, _POSIX_C_SOURCE);
#endif
		}

		static std::string	info() {
			return info(code());
		}

	};

}

#endif//ARA_ERROR_H_201608
