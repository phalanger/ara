
#ifndef ARA_INTERNAL_HTTPZLIB_H
#define ARA_INTERNAL_HTTPZLIB_H

#include <string>

#include "zlib.h"

namespace ara {
	namespace zlib {

		template<typename Obj>
		int writeable_object_write(Obj& obj, const void* data, size_t n) {
			return static_cast<int>(obj.writeData(data, n));
		}
		template<typename Obj>
		void writeable_object_flush(Obj& obj) {
			obj.flushData();
		}
		template<>
		int writeable_object_write(std::string & obj, const void* data, size_t n) {
			obj.append(static_cast<const char*>(data), n);
			return static_cast<int>(n);
		}
		template<>
		void writeable_object_flush(std::string& obj) {
		}

#ifdef ZLIB_H
		template<typename Obj>
		class compress_writer
		{
		public:
			compress_writer(Obj& obj) : obj_(obj) {}

			void	init(int Level, size_t nCacheSize) {
				defstream_.zalloc = Z_NULL;
				defstream_.zfree = Z_NULL;
				defstream_.opaque = Z_NULL;
				deflateInit(&defstream_, nLevel);
				cache_.resize(nCacheSize);
				data_ = 0;
			}
			void	reset() {
			}
			int		write(const void* pSrc, size_t nSrc) {

				defstream_.avail_in = static_cast<uInt>(nSrc);
				defstream_.next_in = (Bytef*)pSrc;

				while (defstream_.avail_in != 0) {

					int nOut = cache_.size() - data_;
					char* pTar = const_cast<char *>(cache_.data() + data_);
					defstream_.avail_out = (uInt)nOut; // size of output
					defstream_.next_out = (Bytef*)bTar; // output char array

					int r = deflate(&defstream_, Z_NO_FLUSH);
					if (r == Z_OK)
					{
						if (defstream_.avail_out == 0)
						{
							data_ = cache_.size();
							_outpout();
						}
						else
							break;
					}
					else
					{
						reset();
						return -1;
					}
				}
				return static_cast<int>(nSrc);
			}
			void	flush() {

			}
		protected:
			void	_outpout() {
				writeable_object_write(obj_, cache_.data(), data_);
				data_ == 0;
			}
			Obj& obj_;
			z_stream defstream_;
			std::string		cache_;
			size_t			data_ = 0;
		};

		template<typename Obj>
		class decompress_writer
		{
		public:
			decompress_writer(Obj& obj) : obj_(obj) {}

			void	init(size_t nCacheSize);
			void	reset();
			int		write(const void* pData, size_t n);
			void	flush();
		protected:
			Obj& obj_;
			z_stream infstream_;
			std::string		cache_;
		};
#else
		template<typename Obj>
		class compress_writer
		{
		public:
			compress_writer(Obj& obj) : obj_(obj) {}
			void	init(int Level, size_t nCacheSize) {}
			void	reset() {}
			int		write(const void* pData, size_t n) {
				return writeable_object_write(obj_, pData, n);
			}
			void	flush() {
				writeable_object_flush(obj_);
			}
		protected:
			Obj& obj_;
		};
		template<typename Obj>
		class decompress_writer
		{
		public:
			compress_writer(Obj& obj) : obj_(obj) {}
			void	init(int Level, size_t nCacheSize) {}
			void	reset() {}
			int		write(const void* pData, size_t n) {
				return writeable_object_write(obj_, pData, n);
			}
			void	flush() {
				writeable_object_flush(obj_);
			}
		protected:
			Obj& obj_;
		};
#endif
	}
}


#endif//ARA_INTERNAL_HTTPZIP_H

