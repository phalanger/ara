#ifndef ARA_ZSTREAM_H
#define ARA_ZSTREAM_H

#include "stream.h"

#include "zlib.h"

namespace ara
{
    namespace stream
    {
#ifdef ZLIB_H
        template <class Stream, class Traist = stream_write_traist<Stream>>
        class zlib_compress_write_stream
        {
        public:
            zlib_compress_write_stream(Stream &s) : stream_(s), data_(0) {}
            ~zlib_compress_write_stream() {
                reset();
            }

            bool init(int nLevel = Z_DEFAULT_COMPRESSION, size_t nCacheSize = 1024 * 1024)
            {
                if (!cache_.empty())
                    reset();
                strm_.zalloc = nullptr;
                strm_.zfree = nullptr;
                strm_.opaque = nullptr;
                ret = deflateInit(&strm, nLevel);
                if (ret != Z_OK)
                    return false;
                cache_.resize(nCacheSize);
                data_ = 0;
            }

            void reset()
            {
                deflateEnd(&strm_);
                cache_.swap(std::string());
            }

            int     write(const void *p, size_t n)
            {
                strm_.avail_in = static_cast<uInt>(n);
                strm_.next_in = reinterpret_cast<Bytef *>(p);
                while (strm_.avail_in > 0) 
                {
                    uInt nRest = static_cast<uInt>(cache_.size() - data_);
                    strm_.avail_out = nRest;
                    strm_.next_out = (Bytef *)const_cast<char *>(cache_.data() + data_);
                    int ret = deflate(&strm_, Z_NO_FLUSH);
                    if (ret == Z_STREAM_ERROR)
                        return -1;
                    data_ += nRest - strm_.avail_out;
                    if (strm_.avail_out == 0 && !_output()) 
                        return -1;
                }
                return static_cast<int>(n);
            }

            void    flush()
            {
                strm_.avail_in = 0;
                strm_.next_in = nullptr;
                for(;;) {
                    uInt nRest = static_cast<uInt>(cache_.size() - data_);
                    strm_.avail_out = nRest;
                    strm_.next_out = (Bytef *)const_cast<char *>(cache_.data() + data_);
                    int ret = deflate(&strm_, Z_FINISH);
                    if (ret == Z_STREAM_END)
                        break;
                    else if (ret == Z_STREAM_ERROR)
                        return;
                    data_ += nRest - strm_.avail_out;
                    if (strm_.avail_out == 0 && !_output())
                        return; 
                }
                _output();
            }
        protected:
            bool    _output()
            {
                if (data_ == 0)
                    return true;
                else if (Traist::write(stream_, cache_.data(), data_) != static_cast<int>(data_))
                    return false;
                data_ = 0;
                return true;
            }

            Stream &stream_;
            std::string cache_;
            size_t data_ = 0;
            z_stream strm_;
        };

        template<class Stream, class Traist = stream_write_traist<Stream>>
        class zlib_decompress_write_stream
        {
            zlib_decompress_write_stream(Stream &s) : stream_(s), data_(0) {}
            ~zlib_decompress_write_stream() {
                reset();
            }

            bool init(size_t nCacheSize = 1024 * 1024)
            {
                if (!cache_.empty())
                    reset();
                strm_.zalloc = nullptr;
                strm_.zfree = nullptr;
                strm_.opaque = nullptr;
                ret = inflateInit(&strm);
                if (ret != Z_OK)
                    return false;
                cache_.resize(nCacheSize);
                data_ = 0;
            }

            void reset()
            {
                inflateEnd(&strm_);
                cache_.swap(std::string());
            }

            int     write(const void *p, size_t n)
            {
                strm_.avail_in = static_cast<uInt>(n);
                strm_.next_in = reinterpret_cast<Bytef *>(p);
                while (strm_.avail_in > 0) 
                {
                    uInt nRest = static_cast<uInt>(cache_.size() - data_);
                    strm_.avail_out = nRest;
                    strm_.next_out = (Bytef *)const_cast<char *>(cache_.data() + data_);
                    int ret = inflate(&strm_, Z_NO_FLUSH);
                    if (ret == Z_NEED_DICT)
                        break;
                    else if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
                        return -1;
                    data_ += nRest - strm_.avail_out;
                    if (strm_.avail_out == 0 && !_output()) 
                        return -1;
                }
                return static_cast<int>(n);
            }

            void    flush()
            {
                strm_.avail_in = 0;
                strm_.next_in = nullptr;
                for(;;) {
                    uInt nRest = static_cast<uInt>(cache_.size() - data_);
                    strm_.avail_out = nRest;
                    strm_.next_out = (Bytef *)const_cast<char *>(cache_.data() + data_);
                    int ret = inflate(&strm_, Z_FINISH);
                    if (ret == Z_STREAM_END)
                        break;
                    else if (ret == Z_NEED_DICT || ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
                        return;
                    data_ += nRest - strm_.avail_out;
                    if (strm_.avail_out == 0 && !_output())
                        return; 
                }
                _output();
            }
        protected:
            bool    _output()
            {
                if (data_ == 0)
                    return true;
                else if (Traist::write(stream_, cache_.data(), data_) != static_cast<int>(data_))
                    return false;
                data_ = 0;
                return true;
            }

            Stream &stream_;
            std::string cache_;
            size_t data_ = 0;
            z_stream strm_;
        };
#else
        template <class Stream, class Traist = stream_write_traist<Stream>>
        class zlib_compress_write_stream
        {
        public:
            zlib_compress_write_stream(Stream &s) : stream_(s) {}

            bool init(int nLevel, size_t nCacheSize) { return true; }
            void  reset() {}
            int write(const void *p, size_t n) {
                return Traist::write(stream_, p, n);
            }
            void flush() {
                Traist::flush(stream_);
            }
        protected:
            Stream &stream_;
        };
        template <class Stream, class Traist = stream_write_traist<Stream>>
        class zlib_decompress_write_stream
        {
        public:
            zlib_decompress_write_stream(Stream &s) : stream_(s) {}

            bool init(size_t nCacheSize) { return true; }
            void  reset() {}
            int write(const void *p, size_t n) {
                return Traist::write(stream_, p, n);
            }
            void flush() {
                Traist::flush(stream_);
            }
        protected:
            Stream &stream_;
        };
#endif
    } // namespace stream
} // namespace ara

#endif // ARA_ZSTREAM_H
