/*
	//handle path
	ara::file_sys::to_path(std::string("C:\\abcd"))								--> "C:\\abcd\\"
	ara::file_sys::join_to_path(std::string("C:\\abcd"), std::string("def"))	--> "C:\\abcd\\def\\"
	ara::file_sys::join_to_file(std::string("C:\\abcd"), std::string("def"))	--> "C:\\abcd\\def"
	ara::file_sys::fix_path(std::string("C:\\\\abcd\\..\\.\\def\\..\\hij\\"))	--> "C:\\hij\\"
	ara::file_sys::fix_path(std::wstring(L"//abcd/.././def/../hij/"))			--> L"/hij/"

	//file stat
	ara::file_adv_attr	attr;
	if (ara::file_sys::get_file_attr("C:\\123", attr)) {
		if (attr.is_dir()) {
			//Output
		}
	}

	//split path
	std::wstring	wstrPath = L"C:\\Windows\\abc\\def";
	for (auto it : ara::file_sys::split_path(wstrPath)) {
		std::cout << "SubItem:" << it << std::endl;
		// C:
		// Windows
		// abc
		// def
	}

	//scan dir
	for (auto it : ara::scan_dir("C:\\")) {
		std::cout << "File Name:" << it << std::endl;		
	}

*/

#ifndef ARA_FILESYS_H
#define ARA_FILESYS_H

#include "ara_def.h"
#include "stringext.h"
#include "datetime.h"
#include "internal/raw_file.h"

#include <string>

#if defined(ARA_WIN32_VER)
	#include <windows.h>
	#include <sys/types.h>
	#include <sys/stat.h>
#else//ARA_WIN32_VC_VER
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <fnmatch.h>
#endif//ARA_WIN32_VC_VER

namespace ara {

	class file_attr
	{
	public:
		enum {
			IS_DIR = 0x01,
			IS_LINK = 0x02,
			IS_SLINK = 0x04,
			IS_COMPRESS = 0x08,
			IS_ENC = 0x10,
			IS_HIDDEN = 0x20,
			IS_SYS = 0x40,
			IS_READONLY = 0x80,
		};
		int				flags = 0;
		date_time		create_time;
		date_time		modify_time;
		date_time		access_time;
		uint64_t		size = 0;
		unsigned short	mode = 0;

		bool	is_dir() const { return (flags & IS_DIR) != 0; }
		bool	is_link() const { return (flags & IS_LINK) != 0; }
		bool	is_soft_link() const { return (flags & IS_SLINK) != 0; }
		bool	is_compressed() const { return (flags & IS_COMPRESS) != 0; }
		bool	is_encrypted() const { return (flags & IS_ENC) != 0; }
		bool	is_hidden() const { return (flags & IS_HIDDEN) != 0; }
		bool	is_sys() const { return (flags & IS_SYS) != 0; }
		bool	is_readonly() const { return (flags & IS_READONLY) != 0; }
	};

	class file_adv_attr : public file_attr
	{
	public:
		short			uid = 0;
		short			gid = 0;
		short			nlink = 0;
		unsigned int	dev = 0;
		unsigned int	ino = 0;
		unsigned int	rdev = 0;
		unsigned int	blocks = 0;
		unsigned int	blocksize = 0;
	};

	class file_sys
	{
	public:
		static bool		isPathSlashChar(int ch) {
#ifdef ARA_WIN32_VER
			return ch == '/' || ch == '\\';
#else
			return ch == '/';
#endif
		}

		static inline char		path_slash() {
#ifdef ARA_WIN32_VER
			return '\\';
#else
			return '/';
#endif
		}

		template<class typeStr>
		static inline char		detect_path_slash(const typeStr & s) {
#ifdef ARA_WIN32_VER
			if (string_traits<typeStr>::find(s, '/', 0) != string_traits<typeStr>::npos
				&& string_traits<typeStr>::find(s, '\\', 0) == string_traits<typeStr>::npos
				)
				return '/';
			return '\\';
#else
			return '/';
#endif
		}

		template<class typeStr>
		static	typeStr		to_path(const typeStr & sPath) {

			size_t nSize = string_traits<typeStr>::size(sPath);
			if (nSize == 0)
				return	sPath;

			int ch = static_cast<int>(*(string_traits<typeStr>::data(sPath) + nSize - 1));
			if (isPathSlashChar(ch))
				return sPath;

			typeStr res = sPath;
			string_traits<typeStr>::append(res, 1, detect_path_slash(sPath));
			return res;
		}

		template<class typeStr, class typeStr2>
		static	typeStr		join_to_path(const typeStr & sPath, const typeStr2 & sSub) {
			return to_path(join_to_file(sPath, sSub));
		}
		template<typename typeStrResult, typename typeStr2, typename ...typeStr>
		static	typeStrResult		join_to_path(const typeStrResult & sSub, const typeStr2 & sSub2, typeStr&&... sPath) {
			return join_to_path(join_to_path(sSub, sSub2), std::forward<typeStr>(sPath)...);
		}
		template<class typeStr>
		static	typeStr		join_to_path(const typeStr & sPath, const char * p) {
			return join_to_path(sPath, std::string(p));
		}
		template<class typeStr>
		static	typeStr		join_to_path(const typeStr & sPath, const wchar_t * p) {
			return join_to_path(sPath, std::wstring(p));
		}

		template<class typeStr, class typeStr2>
		static	typeStr		join_to_file(const typeStr & sPath, const typeStr2 & sSub) {

			auto nSize = string_traits<typeStr2>::size(sSub);
			if (nSize == 0)
				return	sPath;
			typename string_traits<typeStr>::size_type off = 0;
			auto p = string_traits<typeStr2>::data(sSub);
			for (; off < nSize; ++off, ++p)
				if (!isPathSlashChar(*p))
					break;

			typeStr res = to_path(sPath);
			strext(res) += sSub.substr(off);
			return  res;
		}
		template<class typeStr>
		static	typeStr		join_to_file(const typeStr & sPath, const char * p) {
			return join_to_file(sPath, std::string(p));
		}
		template<class typeStr>
		static	typeStr		join_to_file(const typeStr & sPath, const wchar_t * p) {
			return join_to_file(sPath, std::wstring(p));
		}
		template<typename typeStrResult, typename typeStr2, class...typeStr>
		static	typeStrResult		join_to_file(const typeStrResult & sSub, const typeStr2 & sSub2, typeStr&&...sPath) {
			return join_to_file( join_to_path(sSub, sSub2), std::forward<typeStr>(sPath)...);
		}

		template<class typeStr>
		static bool		is_path(const typeStr & sPath) {
			size_t nSize = string_traits<typeStr>::size(sPath);
			if (nSize == 0)
				return false;
			int ch = *(string_traits<typeStr>::data(sPath) + nSize - 1);
			return isPathSlashChar(ch);
		}

		template<class typeStr>
		static bool		is_parent_path(const typeStr & sPath) {
			size_t nSize = string_traits<typeStr>::size(sPath);
			if (nSize != 2)
				return false;
			else if (*string_traits<typeStr>::data(sPath) != '.')
				return false;
			return (*(string_traits<typeStr>::data(sPath) + 1) == '.');
		}

		template<class typeStr>
		static bool		is_self_path(const typeStr & sPath) {
			size_t nSize = string_traits<typeStr>::size(sPath);
			if (nSize != 1)
				return false;
			return (*(string_traits<typeStr>::data(sPath)) == '.');
		}

		template<class typeStr>
		static	void		split_path(const typeStr & sFullPath, typeStr & sPath, typeStr & sFile) {
			size_t nSize = string_traits<typeStr>::size(sFullPath);
			if (nSize == 0)
				return;
			auto p = string_traits<typeStr>::data(sFullPath);
			auto pEnd = p + nSize;
			if (isPathSlashChar(*(pEnd - 1)))
				--pEnd;
			auto pFin = pEnd;

			string_traits<typeStr>::clear(sPath);
			string_traits<typeStr>::clear(sFile);

			for (; pEnd != p; --pEnd) {
				int ch = *(pEnd - 1);
				if (isPathSlashChar(ch)) {
					string_traits<typeStr>::append(sPath, p, pEnd - p);
					string_traits<typeStr>::append(sFile, pEnd, pFin - pEnd);
					return;
				}
			}

			string_traits<typeStr>::append(sFile, sFullPath);
		}

#if defined(ARA_WIN32_VER)

		static time_t filetime_to_timet(const FILETIME& ft) {
			ULARGE_INTEGER ull;
			ull.LowPart = ft.dwLowDateTime;
			ull.HighPart = ft.dwHighDateTime;
			return ull.QuadPart / 10000000ULL - 11644473600ULL;
		}

		static	void stat_to_file_adv_attr(const struct __stat64 & buf, file_adv_attr & attr) {
			attr.create_time = static_cast<time_t>(buf.st_ctime);
			attr.modify_time = static_cast<time_t>(buf.st_mtime);
			attr.access_time = static_cast<time_t>(buf.st_atime);
			attr.size = static_cast<uint64_t>(buf.st_size);
			attr.mode = buf.st_mode;
			attr.uid = buf.st_uid;
			attr.gid = buf.st_gid;
			attr.dev = buf.st_dev;
			attr.ino = buf.st_ino;
			attr.rdev = buf.st_rdev;
			attr.nlink = buf.st_nlink;
			attr.blocks = 0;
			attr.blocksize = 0;
			attr.flags = buf.st_mode;
		}

		static bool		get_file_attr(const std::string & sFile, file_adv_attr & attr) {
			struct __stat64 buf;
			if (_stat64(sFile.c_str(), &buf) != 0)
				return false;
			stat_to_file_adv_attr(buf, attr);
			return true;
		}

		static bool		get_file_attr(const std::wstring & sFile, file_adv_attr & attr) {
			struct __stat64 buf;
			if (_wstat64(sFile.c_str(), &buf) != 0)
				return false;
			stat_to_file_adv_attr(buf, attr);
			return true;
		}
#else
		static bool		get_file_attr(const std::string & sFile, file_adv_attr & attr) {
			struct stat buf;
			if (stat(sFile.c_str(), &buf) != 0)
				return false;

			attr.create_time = static_cast<time_t>(buf.st_ctime);
			attr.modify_time = static_cast<time_t>(buf.st_mtime);
			attr.access_time = static_cast<time_t>(buf.st_atime);
			attr.size = static_cast<uint64_t>(buf.st_size);
			attr.mode = buf.st_mode;
			attr.uid = buf.st_uid;
			attr.gid = buf.st_gid;
			attr.dev = buf.st_dev;
			attr.ino = buf.st_ino;
			attr.rdev = buf.st_rdev;
			attr.nlink = buf.st_nlink;
			attr.blocks = buf.st_blocks;
			attr.blocksize = buf.st_blksize;
			if (buf.st_mode & S_IFDIR)
				attr.flags |= file_attr::IS_DIR;
			if (buf.st_mode & S_IFLNK)
				attr.flags |= file_attr::IS_LINK;
			return true;
		}

		static bool		get_file_attr(const std::wstring & sFile, file_adv_attr & attr) {
			return get_file_attr(strext(sFile).to<std::string>().c_str(), attr);
		}
#endif

		template<class typeStrIn>
		class path_splitor
		{
		public:
			typedef typename std::remove_reference<typeStrIn>::type	typeStrInNoRef;
			typedef typename std::remove_const<typeStrInNoRef>::type	typeStr;
			typedef typename string_traits<typeStr>::value_type	value_type;
			typedef typename string_traits<typeStr>::size_type	size_type;

			class iterator
			{
			public:
				iterator(const value_type * p, size_type n) : p_(p), end_(p + n), token_end_(p) { fetch(); }
				iterator() : p_(nullptr), end_(nullptr), token_end_(nullptr) {}
				iterator(const iterator & i) : p_(i.p_), end_(i.end_), token_end_(i.token_end_) {}

				typeStr	operator*() const {
					if (p_ == nullptr)
						return typeStr();
					return string_traits<typeStr>::make(p_, token_end_ - p_);
				}
				bool operator==(const iterator &) const {
					return p_ == nullptr;
				}
				bool operator!=(const iterator &) const {
					return p_ != nullptr;
				}
				iterator & operator++() {
					if (p_ != nullptr)
						fetch();
					return *this;
				}
			protected:
				bool fetch() {
					if (p_ == nullptr)
						return false;
					else if (!fetchImp() || p_ == end_) {
						p_ = end_ = token_end_ = nullptr;
						return false;
					}
					return true;
				}

				bool fetchImp() {
					for (p_ = token_end_; p_ != end_; ++p_)
						if (!isPathSlashChar(*p_))
							break;
					if (p_ == end_)
						return false;
					for (token_end_ = p_; token_end_ != end_; ++token_end_)
						if (isPathSlashChar(*token_end_))
							break;
					return token_end_ != p_;
				}
				const value_type *	p_;
				const value_type *	end_;
				const value_type *	token_end_;
			};

			path_splitor(const typeStr & s) : s_(s) {}
			path_splitor(typeStr && s) : s_(std::move(s)) {}
			path_splitor(const path_splitor & s) : s_(s.s_) {}
			path_splitor(path_splitor && s) : s_(std::move(s.s_)) {}

			iterator	begin() const {
				return iterator(string_traits<typeStr>::data(s_), string_traits<typeStr>::size(s_));
			}
			iterator	end() const {
				return iterator();
			}
		protected:
			typeStr		s_;
		};

		template<class typeStr>
		static path_splitor<typeStr>	split_path(typeStr && s) {
			return path_splitor<typeStr>(std::forward<typeStr>(s));
		}

		template<class typeStr>
		static typeStr		fix_path(const typeStr & path) {
			std::vector<typeStr>	vecItems;
			typeStr		res;
			bool needPrefix = false;
#ifdef ARA_WIN32_VER
			bool withDriver = false;
#endif

			if (!string_traits<typeStr>::empty(path)) {
				if(*string_traits<typeStr>::data(path) == '/')
					needPrefix = true;
#ifdef ARA_WIN32_VER
				else if (string_traits<typeStr>::size(path) > 1 && *(string_traits<typeStr>::data(path) + 1) == ':')
					withDriver = true;
#endif
			}

			for (auto it : split_path(path)) {
				if (is_parent_path(it)) {
					if (!vecItems.empty()) {
#ifdef ARA_WIN32_VER
						if (!withDriver || vecItems.size() > 1)
#endif
						vecItems.pop_back();
					}
					if (vecItems.empty())
						needPrefix = true;
				}
				else if (is_self_path(it))
					continue;
				else
					vecItems.push_back(it);
			}
			char ch = detect_path_slash(path);
			for (auto & it2 : vecItems) {
				if (!string_traits<typeStr>::empty(res)) {
					string_traits<typeStr>::append(res, ch);
				} else if (needPrefix) {
					needPrefix = false;
					string_traits<typeStr>::append(res, ch);
				}
				string_traits<typeStr>::append(res, it2);
			}
			if (is_path(path))
				string_traits<typeStr>::append(res, ch);
			return res;
		}

		static bool	unlink(const std::string & strFile) {
#ifdef ARA_WIN32_VER
			return ::DeleteFileA(strFile.c_str()) == TRUE;
#else
			return ::unlink(strFile.c_str()) == 0;
#endif
		}
		static bool	unlink(const std::wstring & strFile) {
#ifdef ARA_WIN32_VER
			return ::DeleteFileW(strFile.c_str()) == TRUE;
#else
			return ::unlink(strext(strFile).to<std::string>().c_str()) == 0;
#endif
		}

		static bool path_exist(const std::string & strFile) {
#ifdef ARA_WIN32_VER
			return (::GetFileAttributesA(strFile.c_str()) != INVALID_FILE_ATTRIBUTES);
#else
			return (access(strFile.c_str(), F_OK) == 0);
#endif
		}
		static bool path_exist(const std::wstring & strFile) {
#ifdef ARA_WIN32_VER
			return (::GetFileAttributesW(strFile.c_str()) != INVALID_FILE_ATTRIBUTES);
#else
			return (access(strext(strFile).to<std::string>().c_str(), F_OK) == 0);
#endif
		}

		static bool	get_temp_folder(std::string & strFolder) {
#ifdef ARA_WIN32_VER
			char lpTempPathBuffer[MAX_PATH];
			DWORD dw = ::GetTempPathA(MAX_PATH, lpTempPathBuffer);
			if (dw == 0)
				return false;
			strFolder.assign(lpTempPathBuffer, dw);
			return true;
#else
			const char * tp = getenv("TMPDIR");
			strFolder = (tp == nullptr ? "/tmp" : tp);
			return true;
#endif
		}

		static bool	get_temp_folder(std::wstring & strFolder) {
#ifdef ARA_WIN32_VER
			wchar_t lpTempPathBuffer[MAX_PATH];
			DWORD dw = ::GetTempPathW(MAX_PATH, lpTempPathBuffer);
			if (dw == 0)
				return false;
			strFolder.assign(lpTempPathBuffer, dw);
			return true;
#else
			std::string tmp;
			if (!get_temp_folder(tmp))
				return false;
			strFolder = strext(tmp).to<std::wstring>();
			return true;
#endif
		}
	};//file sys

#if defined(ARA_WIN32_VER)
	class dir_iterator
	{
	public:
		dir_iterator() noexcept : FindFileDataA_() {}
		dir_iterator(const std::string & strPath, const std::string & strFilter = "") {
			std::string str = file_sys::join_to_file(strPath, strFilter.empty() ? std::string("*.*") : strFilter);
			hFind_ = ::FindFirstFileA(str.c_str(), &FindFileDataA_);
		}

		dir_iterator(dir_iterator && r) noexcept {
			hFind_ = r.hFind_;
			r.hFind_ = INVALID_HANDLE_VALUE;
			memcpy(&FindFileDataA_, &FindFileDataA_, sizeof(FindFileDataA_));
		}

		~dir_iterator() {
			if (hFind_ != INVALID_HANDLE_VALUE) {
				::FindClose(hFind_);
				hFind_ = INVALID_HANDLE_VALUE;
			}
		}

		bool operator==(const dir_iterator &) const {
			return hFind_ == INVALID_HANDLE_VALUE;
		}
		bool operator!=(const dir_iterator &) const {
			return hFind_ != INVALID_HANDLE_VALUE;
		}

		std::string operator*() const {
			return FindFileDataA_.cFileName;
		}

		file_attr get_attr() const {
			file_attr attr;
			get_attr(attr);
			return attr;
		}

		dir_iterator & operator++() {
			if (hFind_ != INVALID_HANDLE_VALUE) {
				if (!::FindNextFileA(hFind_, &FindFileDataA_)) {
					::FindClose(hFind_);
					hFind_ = INVALID_HANDLE_VALUE;
				}
			}
			return *this;
		}
	protected:
		void get_attr(file_attr & attr) const {
			attr.access_time = file_sys::filetime_to_timet(FindFileDataA_.ftLastAccessTime);
			attr.create_time = file_sys::filetime_to_timet(FindFileDataA_.ftCreationTime);
			attr.modify_time = file_sys::filetime_to_timet(FindFileDataA_.ftLastWriteTime);

			ULARGE_INTEGER ull;
			ull.LowPart = FindFileDataA_.nFileSizeLow;
			ull.HighPart = FindFileDataA_.nFileSizeHigh;
			attr.size = static_cast<uint64_t>(ull.QuadPart);

			DWORD n = FindFileDataA_.dwFileAttributes;
			if (n & FILE_ATTRIBUTE_COMPRESSED)
				attr.flags |= file_attr::IS_COMPRESS;
			if (n & FILE_ATTRIBUTE_DIRECTORY)
				attr.flags |= file_attr::IS_DIR;
			if (n & FILE_ATTRIBUTE_ENCRYPTED)
				attr.flags |= file_attr::IS_ENC;
			if (n & FILE_ATTRIBUTE_HIDDEN)
				attr.flags |= file_attr::IS_HIDDEN;
			if (n & FILE_ATTRIBUTE_READONLY)
				attr.flags |= file_attr::IS_READONLY;
			if (n & FILE_ATTRIBUTE_SYSTEM)
				attr.flags |= file_attr::IS_SYS;
		}
		dir_iterator(const dir_iterator &) = delete;

		HANDLE hFind_ = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA FindFileDataA_;
	};
	class wdir_iterator
	{
	public:
		wdir_iterator() noexcept : FindFileDataW_() {}
		wdir_iterator(const std::wstring & strPath, const std::wstring & strFilter = L"") {
			std::wstring str = file_sys::join_to_file(strPath, strFilter.empty() ? std::wstring(L"*.*") : strFilter);
			hFind_ = ::FindFirstFileW(str.c_str(), &FindFileDataW_);
		}

		wdir_iterator(wdir_iterator && r) noexcept {
			hFind_ = r.hFind_;
			r.hFind_ = INVALID_HANDLE_VALUE;
			memcpy(&FindFileDataW_, &FindFileDataW_, sizeof(FindFileDataW_));
		}

		~wdir_iterator() {
			if (hFind_ != INVALID_HANDLE_VALUE) {
				::FindClose(hFind_);
				hFind_ = INVALID_HANDLE_VALUE;
			}
		}

		bool operator==(const wdir_iterator &) const {
			return hFind_ == INVALID_HANDLE_VALUE;
		}
		bool operator!=(const wdir_iterator &) const {
			return hFind_ != INVALID_HANDLE_VALUE;
		}

		std::wstring operator*() const {
			return FindFileDataW_.cFileName;
		}

		file_attr get_attr() const {
			file_attr attr;
			get_attr(attr);
			return attr;
		}

		wdir_iterator & operator++() {
			if (hFind_ != INVALID_HANDLE_VALUE) {
				if (!::FindNextFileW(hFind_, &FindFileDataW_)) {
					::FindClose(hFind_);
					hFind_ = INVALID_HANDLE_VALUE;
				}
			}
			return *this;
		}
	protected:
		void get_attr(file_attr & attr) const {
			attr.access_time = file_sys::filetime_to_timet(FindFileDataW_.ftLastAccessTime);
			attr.create_time = file_sys::filetime_to_timet(FindFileDataW_.ftCreationTime);
			attr.modify_time = file_sys::filetime_to_timet(FindFileDataW_.ftLastWriteTime);

			ULARGE_INTEGER ull;
			ull.LowPart = FindFileDataW_.nFileSizeLow;
			ull.HighPart = FindFileDataW_.nFileSizeHigh;
			attr.size = static_cast<uint64_t>(ull.QuadPart);

			DWORD n = FindFileDataW_.dwFileAttributes;
			if (n & FILE_ATTRIBUTE_COMPRESSED)
				attr.flags |= file_attr::IS_COMPRESS;
			if (n & FILE_ATTRIBUTE_DIRECTORY)
				attr.flags |= file_attr::IS_DIR;
			if (n & FILE_ATTRIBUTE_ENCRYPTED)
				attr.flags |= file_attr::IS_ENC;
			if (n & FILE_ATTRIBUTE_HIDDEN)
				attr.flags |= file_attr::IS_HIDDEN;
			if (n & FILE_ATTRIBUTE_READONLY)
				attr.flags |= file_attr::IS_READONLY;
			if (n & FILE_ATTRIBUTE_SYSTEM)
				attr.flags |= file_attr::IS_SYS;
		}
		wdir_iterator(const wdir_iterator &) = delete;

		HANDLE hFind_ = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAW FindFileDataW_;
	};
#else// !ARA_WIN32_VC_VER && !ARA_WIN32_MINGW_VER
	class dir_iterator
	{
	public:
		dir_iterator() {}
		dir_iterator(const std::string & strPath, const std::string & strFilter = "") : parent_(strPath), filter_(strFilter) {
			entry_ = reinterpret_cast<struct dirent *>(new char[sizeof(struct dirent) + pathconf(strPath.c_str(), _PC_NAME_MAX) + 1]);
			if ((dir_ = ::opendir(strPath.c_str())) != nullptr)
				fetch_and_check();
			if (filter_ == "*")
				filter_.clear();
		}

		dir_iterator(dir_iterator && r) {
			dir_ = r.dir_;
			r.dir_ = nullptr;
			entry_ = r.entry_;
			r.entry_ = nullptr;
			parent_ = std::move(r.parent_);
			filter_ = std::move(r.filter_);
		}

		~dir_iterator() {
			if (dir_ != nullptr)
				::closedir(dir_);
			char * p = reinterpret_cast<char *>(entry_);
			delete[]p;
		}

		bool operator==(const dir_iterator &) const {
			return dir_ == nullptr;
		}
		bool operator!=(const dir_iterator &) const {
			return dir_ != nullptr;
		}

		std::string operator*() const {
			return entry_->d_name;
		}

		file_adv_attr get_attr() const {
			file_adv_attr attr;
			file_sys::get_file_attr(file_sys::join_to_file(parent_, std::string(entry_->d_name)), attr);
			return attr;
		}

		dir_iterator & operator++() {
			fetch_and_check();
			return *this;
		}
	protected:
		bool	fetch_and_check() {
			if (dir_ == nullptr)
				return false;
			for (;;) {
				if (!fetch())
					return false;
				else if (filter_.empty())
					return true;
				else if (::fnmatch(filter_.c_str(), entry_->d_name, FNM_FILE_NAME) == 0)
					return true;
			}
			return false;
		}
		bool	fetch() {
			struct dirent * dp;
			if (::readdir_r(dir_, entry_, &dp) != 0 || dp == nullptr) {
				::closedir(dir_);
				dir_ = nullptr;
				return false;
			}
			return true;
		}
		dir_iterator(const dir_iterator &) = delete;

		DIR	*				dir_ = nullptr;
		struct dirent *		entry_ = nullptr;
		std::string			parent_;
		std::string			filter_;
	};
	class wdir_iterator : public dir_iterator
	{
	public:
		wdir_iterator() {}
		wdir_iterator(const std::wstring & strPath, const std::wstring & strFilter = L"") :
			dir_iterator( strext(strPath).to<std::string>(), strext(strFilter).to<std::string>()) {}

		wdir_iterator(wdir_iterator && r) : dir_iterator(std::move(r)) {}

		std::wstring operator*() const {
			return strext(std::string(entry_->d_name)).to<std::wstring>();
		}
	};
#endif//ARA_WIN32_VC_VER

	class scan_dir {
	public:
		scan_dir(const std::string & strPath, const std::string & strFilter = "") : path_(strPath), filter_(strFilter) {}

		dir_iterator	begin() const {
			return dir_iterator(path_, filter_);
		}
		dir_iterator	end() const {
			return dir_iterator();
		}
	protected:
		std::string path_;
		std::string filter_;
	};
	class scan_wdir {
	public:
		scan_wdir(const std::wstring & strPath, const std::wstring & strFilter = L"") : path_(strPath), filter_(strFilter) {}

		wdir_iterator	begin() const {
			return wdir_iterator(path_, filter_);
		}
		wdir_iterator	end() const {
			return wdir_iterator();
		}
	protected:
		std::wstring path_;
		std::wstring filter_;
	};

	class raw_file;

	class raw_file : protected internal::raw_file_imp
	{
	public:
		typedef internal::raw_file_imp::file_handle	file_handle;

		file_handle		get_handle() const {
			return fd_;
		}

		raw_file() noexcept {}
		~raw_file() {
			internal::raw_file_imp::close_imp();
		}

		inline internal::open_flag<std::string>		open(const std::string & strName) {
			return internal::open_flag<std::string>(*this, strName);
		}
		inline internal::open_flag<std::wstring>		open(const std::wstring & strName) {
			return internal::open_flag<std::wstring>(*this, strName);
		}
		inline bool	open(const std::string & strName, int nFlags, int mod = -1) {
			return open_imp(strName, nFlags, mod);
		}
		inline bool	open(const std::wstring & strName, int nFlags, int mod = -1) {
			return open_imp(strName, nFlags, mod);
		}
		inline bool	is_opened() const {
			return is_opened_imp();
		}
		inline int		read(void * buf, size_t n) {
			return read_imp(buf, n);
		}
		inline int		write(const void * buf, size_t n) {
			return write_imp(buf, n);
		}
		inline bool	truncat(uint64_t n) {
			return truncat_imp(n);
		}
		inline off_t	seek(off_t n, std::ios::seek_dir from) {
			return seek_imp(n, from);
		}
		inline off_t	tell() {
			return seek_imp(0, std::ios::cur);
		}
		inline bool	sync() {
			return sync_imp();
		}
		inline bool	data_sync() {
			return data_sync_imp();
		}

		template<class typeFileNameString>
		static bool		load_data_from_file(const typeFileNameString & strFileName, std::string & data) {
			raw_file	f;
			if (!f.open(strFileName).read_only().binary().done())
				return false;
			off_t s = f.seek(0, std::ios::end);
			size_t nOldSize = data.size();
			data.resize(nOldSize + static_cast<size_t>(s));
			f.seek(0, std::ios::beg);
			char * p = const_cast<char *>(data.data() + nOldSize);
			while (s > 0) {
				int n = f.read(p, s);
				if (n <= 0)
					break;
				p += n;
				s -= n;
			}
			if (s)
				data.resize( static_cast<size_t>(data.size() - s) );
			return true;
		}
		template<class typeFileNameString>
		static bool		save_data_to_file(const typeFileNameString & strFileName, const std::string & data) {
			return save_data_to_file(strFileName, data.data(), data.size());
		}
		template<class typeFileNameString>
		static bool		save_data_to_file(const typeFileNameString & strFileName, const void * pData, size_t s) {
			raw_file	f;
			if (!f.open(strFileName).write_only().create().truncat().binary().done())
				return false;
			const char * p = static_cast<const char *>(pData);
			while (s > 0) {
				int n = f.write(p, s);
				if (n <= 0)
					return false;
				p += n;
				s -= n;
			}
			return true;
		}
	};
}

#endif//ARA_FILESYS_H
