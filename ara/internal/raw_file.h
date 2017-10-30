#ifndef ARA_RAW_FILE_H_201605
#define ARA_RAW_FILE_H_201605

#include "../ara_def.h"
#include <string>

#include <fcntl.h>

#if defined(ARA_WIN32_VER)
	#include <Windows.h>
#else
	#include <unistd.h>
#endif

namespace ara {
	namespace internal {

		class raw_file_imp;

		template<typename typeStr>
		class open_flag
		{
		public:
			open_flag(raw_file_imp & f, const typeStr & strFileName) : f_(f), name_(strFileName) {}
			
			bool	done();

			open_flag &	mod(int nMod) { mod_ = nMod; return *this; }
			open_flag &	flags(int flag) { flags_ |= flag; return *this; }

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_TEXT
#define O_TEXT 0
#endif
#ifndef O_TEMPORARY
#define O_TEMPORARY 0
#endif
#ifndef O_RANDOM
#define O_RANDOM 0
#endif
#define DECLEAR_FLAG(name, val)		open_flag &	name() { flags_ |= val; return *this; }
			DECLEAR_FLAG(read_only, O_RDONLY)
			DECLEAR_FLAG(write_only, O_WRONLY)
			DECLEAR_FLAG(read_write, O_RDWR)
			DECLEAR_FLAG(create, O_CREAT)
			DECLEAR_FLAG(append, O_APPEND)
			DECLEAR_FLAG(truncat, O_TRUNC)
			DECLEAR_FLAG(exclusive, O_EXCL)
			DECLEAR_FLAG(binary, O_BINARY)
			DECLEAR_FLAG(text, O_TEXT)
			DECLEAR_FLAG(temporary, O_TEMPORARY)
			DECLEAR_FLAG(random, O_RANDOM)
#undef	DECLEAR_FLAG

		protected:
			raw_file_imp &	f_;
			typeStr			name_;
			int				flags_ = 0;
			int				mod_ = -1;
		};

		class raw_file_imp
		{
		public:
#if defined(ARA_WIN32_VER)
			typedef		HANDLE		file_handle;
#else
			typedef		int			file_handle;
#endif

			void		close_imp() {
				if (!is_opened_imp())
					return;
#if defined(ARA_WIN32_VER)
				::CloseHandle(fd_);
				fd_ = INVALID_HANDLE_VALUE;
#else
				::close(fd_);
				fd_ = -1;
#endif
			}

			bool		is_opened_imp() const {
#if defined(ARA_WIN32_VER)
				return (fd_ != INVALID_HANDLE_VALUE);
#else
				return fd_ != -1;
#endif
			}

			template<class typeString>
			bool		open_imp(const typeString & strName, int nFlags, int mod) {
				close_imp();
#if defined(ARA_WIN32_VER)
				DWORD dwDesiredAccess = 0, dwShareMode = 0, dwCreationDisposition = 0, dwFlagsAndAttributes = 0;
				if (nFlags & O_WRONLY)
					dwDesiredAccess = GENERIC_WRITE;
				else if (nFlags & O_RDWR)
					dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
				else
					dwDesiredAccess = GENERIC_READ;

				dwShareMode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

				if (nFlags & O_CREAT) {
					if (nFlags & O_TRUNC)
						dwCreationDisposition = CREATE_ALWAYS;
					else if (nFlags & O_EXCL)
						dwCreationDisposition = CREATE_NEW;
					else
						dwCreationDisposition = OPEN_ALWAYS;
				} else {
					if (nFlags & O_TRUNC)
						dwCreationDisposition = TRUNCATE_EXISTING;
					else
						dwCreationDisposition = OPEN_EXISTING;
				}
				
				if (nFlags & O_TEMPORARY)
					dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY;
				else
					dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
				if (nFlags & O_RANDOM)
					dwFlagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS;
				if (mod != -1)
					dwFlagsAndAttributes |= static_cast<DWORD>(mod);

				fd_ = _open_imp(strName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes);
				if (fd_ == INVALID_HANDLE_VALUE)
					return false;
				if (nFlags & O_APPEND)
					::SetFilePointerEx(fd_, LARGE_INTEGER{0}, 0, FILE_END);
				return true;
#else
				if (mod == -1)
					mod = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
				fd_ = _open_imp(strName, nFlags, mod);
				return fd_ >= 0;
#endif
			}

			off_t		seek_imp(off_t n, std::ios::seek_dir from) {
				if (!is_opened_imp())
					return off_t(-1);
#if defined(ARA_WIN32_VER)
				LARGE_INTEGER	off = {};
				LARGE_INTEGER	NewFilePointer = {};
				off.QuadPart = static_cast<LONGLONG>(n);
				if (!::SetFilePointerEx(fd_, off, &NewFilePointer, from == std::ios::beg ? FILE_BEGIN : (from == std::ios::cur ? FILE_CURRENT : FILE_END)))
					return off_t(-1);
				return static_cast<off_t>(NewFilePointer.QuadPart);
#else
				return ::lseek(fd_, n, from == std::ios::beg ? SEEK_SET : (from == std::ios::cur ? SEEK_CUR : SEEK_END));
#endif
			}

			int			read_imp(void * buf, size_t n) {
				if (!is_opened_imp())
					return -1;
#if defined(ARA_WIN32_VER)
				DWORD dwRead = 0;
				if (!::ReadFile(fd_, buf, static_cast<DWORD>(n), &dwRead, NULL))
					return -1;
				return static_cast<int>(dwRead);
#else
				return ::read(fd_, buf, n);
#endif
			}
			int			write_imp(const void * buf, size_t n) {
				if (!is_opened_imp())
					return -1;
#if defined(ARA_WIN32_VER)
				DWORD dwWrite = 0;
				if (!::WriteFile(fd_, buf, static_cast<DWORD>(n), &dwWrite, NULL))
					return -1;
				return static_cast<int>(dwWrite);
#else
				return ::write(fd_, buf, n);
#endif
			}
			bool		truncat_imp(uint64_t nNewSize) {
				if (!is_opened_imp())
					return false;
#if defined(ARA_WIN32_VER)
				LARGE_INTEGER	off = { 0 };
				off.QuadPart = static_cast<decltype(off.QuadPart)>(nNewSize);
				if (!::SetFilePointerEx(fd_, off, 0, FILE_BEGIN))
					return false;
				return true;
#else
				return ::ftruncate(fd_, static_cast<off_t>(nNewSize)) == 0;
#endif
			}

			bool		sync_imp() {
				if (!is_opened_imp())
					return false;
#if defined(ARA_WIN32_VER)
				return ::FlushFileBuffers(fd_) == TRUE;
#else
				return ::fsync(fd_) == 0;
#endif
			}
			bool		data_sync_imp() {
				if (!is_opened_imp())
					return false;
#if defined(ARA_WIN32_VER)
				return ::FlushFileBuffers(fd_) == TRUE;
#elif defined(ARA_LINUX_VER)
				return ::fdatasync(fd_) == 0;
#else
				return ::fsync(fd_) == 0;
#endif
			}

		protected:
#if defined(ARA_WIN32_VER)
			HANDLE		fd_ = INVALID_HANDLE_VALUE;
			static HANDLE		_open_imp(const std::string & strName, DWORD d1, DWORD d2, DWORD d3, DWORD d4) {
				return ::CreateFileA(strName.c_str(), d1, d2, NULL, d3, d4, NULL);
			}
			static HANDLE		_open_imp(const std::wstring & strName, DWORD d1, DWORD d2, DWORD d3, DWORD d4) {
				return ::CreateFileW(strName.c_str(), d1, d2, NULL, d3, d4, NULL);
		}
#else
			int			fd_ = -1;
			static int		_open_imp(const std::string & strName, int a, int n) {
				return ::open(strName.c_str(), a, n);
			}
			static int		_open_imp(const std::wstring & strName, int a, int n) {
				return ::open(strext(strName).to<std::string>().c_str(), a, n);
			}
#endif
		};

		template<typename typeStr>
		inline bool open_flag<typeStr>::done() {
			return f_.open_imp(name_, flags_, mod_);
		}

	}//internal

}

#endif//ARA_RAW_FILE_H_201605

