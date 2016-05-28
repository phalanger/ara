#ifndef ARA_RAW_FILE_H_201605
#define ARA_RAW_FILE_H_201605

#include "../ara_def.h"
#include <string>

#if defined(ARA_WIN32_VER)
	#include <fcntl.h>
	#include <Windows.h>
#endif

namespace ara {
	namespace internal {

		class raw_file_imp;

		template<typename typeStr>
		class open_flag
		{
		public:
			open_flag(raw_file_imp & f, const typeStr & strFileName) : f_(f), name_(strFileName), flags_(0) {}
			
			bool	done();

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

			bool		open_imp(const std::string & strName, int nFlags);
			bool		open_imp(const std::wstring & strName, int nFlags);
			off_t		seek_imp(off_t, std::ios::seek_dir from);
			off_t		tell_imp();
			int			read_imp(void * buf, size_t n);
			int			write_imp(const void * buf, size_t n);
			off_t		truncat_imp(size_t nNewSize);

		protected:
#if defined(ARA_WIN32_VER)
			HANDLE		fd_ = INVALID_HANDLE_VALUE;
#else
			int			fd_ = -1;
#endif
		};

		template<typename typeStr>
		inline bool open_flag<typeStr>::done() {
			return f_.open_imp(name_, flags_);
		}

	}//internal

}

#endif//ARA_RAW_FILE_H_201605

