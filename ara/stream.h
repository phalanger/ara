
#ifndef ARA_STREAM_H
#define ARA_STREAM_H

#include "ara_def.h"
#include <string>
#include <iostream>

namespace ara
{
    namespace stream
    {
        template <typename Stream>
        class stream_write_traist
        {
        public:
            static int write(Stream &s, const void *p, size_t n)
            {
                return static_cast<int>(s.write(p, n));
            }
            static void flush(Stream &s)
            {
                s.flush();
            }
            static void prepare(Stream &s, size_t n)
            {
            }
        };

        template <typename Stream>
        class stream_write_seekp_traist : public stream_write_traist<Stream>
        {
        public:
            static int64_t seekp(Stream &s, int64_t offset, std::ios::seekdir nType)
            {
                return static_cast<int64_t>(s.seek(offset, nType));
            }
        };

        template <typename Stream>
        class stream_read_traist
        {
        public:
            static int read(Stream &s, void *p, size_t n)
            {
                return static_cast<int>(s.read(p, n));
            }
        };

        template <typename Stream>
        class stream_read_seekg_traist : public stream_read_traist<Stream>
        {
        public:
            static int64_t seekg(Stream &s, int64_t offset, std::ios::seekdir nType)
            {
                return static_cast<int64_t>(s.seek(offset, nType));
            }
        };

        /////////////////////////////////////////////////

        template <>
        class stream_write_traist<std::string>
        {
        public:
            static int write(std::string &s, const void *p, size_t n)
            {
                s.append(reinterpret_cast<const char *>(p), n);
                return static_cast<int>(n);
            }
            static void flush(std::string &s)
            {
            }
            static void prepare(std::string &s, size_t n)
            {
                s.reserve(s.size() + n);
            }
        };

        template <>
        class stream_write_traist<std::ostream>
        {
        public:
            static int write(std::ostream &s, const void *p, size_t n)
            {
                s.write(reinterpret_cast<const char *>(p), n);
                return static_cast<int>(n);
            }
            static void flush(std::ostream &s)
            {
                s.flush();
            }
            static void prepare(std::ostream &s, size_t n)
            {
            }
        };

        template <>
        class stream_write_seekp_traist<std::ostream> : public stream_write_traist<std::ostream>
        {
        public:
            static int64_t seekp(std::ostream &s, int64_t offset, std::ios::seekdir nType)
            {
                s.seekp(offset, nType);
                return static_cast<int64_t>(s.tellp());
            }
        };

        template <>
        class stream_read_traist<std::istream>
        {
        public:
            static int read(std::istream &s, void *p, size_t n)
            {
                return static_cast<int>(s.readsome(reinterpret_cast<char *>(p), n));
            }
        };

        template <>
        class stream_read_seekg_traist<std::istream> : public stream_read_traist<std::istream>
        {
        public:
            static int64_t seekg(std::istream &s, int64_t offset, std::ios::seekdir nType)
            {
                s.seekg(offset, nType);
                return static_cast<int64_t>(s.tellg());
            }
        };

    } // namespace stream
} // namespace ara

#endif //ARA_STREAM_H
