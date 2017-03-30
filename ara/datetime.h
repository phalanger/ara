
#ifndef ARA_DATETIME_H
#define ARA_DATETIME_H

#include "ref_string.h"
#include "internal/standard_datetime.h"

///	date_time
///		Normal usage:
///			ara::date_time	now = ara::date_time::get_current();
///			ara::date_time	that_day(2011,3,28,10,30,00);
///			ara::date_time	day1 = "2016-02-21 11:23:45"_date;
///			ara::date_time	day2(time(NULL));
///			ara::date_time	day3( std::chrono::system_clock::now() );
///			ara::date_time	day4( "2016 02 21 11 23 45", "%Y %m %d %H %M %S" );
///		Extention:
///			define self storage
///			class DateTimeWithMSTraits
///			{
///			public:
///				static inline void	set_time(uint64_t & tar, time_t src) {
///					tar = (src * 1000) | (tar % 1000);
///				}
///				static inline time_t get_time(uint64_t src) {
///					return src / 1000;
///				}
///			};
///			class date_time_ms : public ara::internal::date_time_imp< uint64_t, DateTimeWithMSTraits >
///
///	timer_val
///		usage:
///			ara::timer_val t1 = ara::timer_val::current_time();
///			ara::timer_val t2(3, 1000);
///			const auto ms1 = 1000_ms;
///			const auto ms2 = 1_sec;
///			auto ms3 = 1_sec + 20_ms;



struct timeval;

namespace ara {

#define  DEFAULT_DATETIME_FORMAT		"%Y-%m-%d %H:%M:%S"
#define  DEFAULT_DATETIME_FORMAT_L		L"%Y-%m-%d %H:%M:%S"

	namespace internal {
		template<typename T, typename traits>
		class date_time_imp
		{
		public:
			//! the max time
			static date_time_imp max_val;

			static uint64_t	get_microseconds_per_day() {
				return (uint64_t)24L * 60 * 60 * 1000 * 1000;
			}
			static uint64_t	get_microseconds_per_hour() {
				return (uint64_t)60L * 60 * 1000 * 1000;
			}
			static uint64_t	get_microseconds_per_minute() {
				return (uint64_t)60L * 1000 * 1000;
			}
			static uint64_t	get_microseconds_per_second() {
				return (uint64_t)1000 * 1000;
			}
			static uint64_t	get_milliseconds_per_second() {
				return (uint64_t)1000;
			}
			static uint64_t	get_microseconds_per_millisecond() {
				return (uint64_t)1000;
			}
			static uint64_t	get_nanoseconds_per_microseconds() {
				return (uint64_t)1000;
			}
			static uint64_t	get_nanoseconds_per_millisecond() {
				return (uint64_t)1000000LL;
			}
			static uint64_t	get_nanoseconds_per_seconds() {
				return (uint64_t)1000000000LL;
			}

			static uint64_t	get_microseconds_before_1970() {
				return uint64_t(62135769600000000);
			}

			static int	get_seconds_per_minute() {
				return 60;
			}
			static int	get_minute_per_hour() {
				return 60;
			}
			static int	get_hour_per_day() {
				return 24;
			}
			static int	get_month_per_year() {
				return 12;
			}
			static int	get_seconds_per_hour() {
				return 60 * 60;
			}
			static int	get_seconds_per_day() {
				return 60 * 60 * 24;
			}

			static int	get_days_before_julian() {
				return 639797;
			}

		public:
			//! Default constructor
			date_time_imp() : time_(0) {}
			//! Copy constructor
			date_time_imp(const date_time_imp & d) : time_(d.time_) {}
			//! Constructor with a 64bits integer
			explicit date_time_imp(const time_t t) { traits::set_time(time_, t); }
			//! Constructor with Date and Time
			date_time_imp(int year, int month, int day,
				int hour, int minute, int second, int isdst = 0) : time_(0) {
				set(year, month, day, hour, minute, second, isdst);
			}
			explicit date_time_imp(const ref_string & strDate, const char * format = nullptr) : time_(0) {
				std::string		s(strDate.data(), strDate.size());
				parse(s.c_str(), format);
			}
			explicit date_time_imp(const std::string & strDate, const char * format = nullptr) : time_(0) {
				parse(strDate.c_str(), format);
			}
			explicit date_time_imp(const char * lpDate, const char * format = nullptr) : time_(0) {
				parse(lpDate, format);
			}

			template <class _Clock, class _Duration>
			explicit date_time_imp(const std::chrono::time_point<_Clock, _Duration>& __atime) : time_(0) {
				set(__atime);
			}

			static date_time_imp	get_current() {
				return date_time_imp(time(NULL));
			}

			// operators
		public:
			//! return true when it is a leap year
			static bool year_is_leap(int year) {
				if (year < 1752)
					return ((year & 3) == 0);
				else
					return (((year & 3) == 0 && (year % 100) != 0) || (year % 400) == 0);
			}
			//! return true when the object is a valid date time
			bool	empty() const {
				return time_ == T();
			}
			//! Set the 0 value of the date time
			void	clear() {
				time_ = T();
			}
			//! Assign operator
			const date_time_imp & operator=(const date_time_imp& d) {
				time_ = d.time_;
				return *this;
			}
			//! Assign operator, assign a 64bits integer
			const date_time_imp& operator=(const time_t t) {
				traits::set_time(time_, t);
				return *this;
			}

			void swap(date_time_imp & n) {
				std::swap(time_, n.time_);
			}

			//! Compare
			int compare(const date_time_imp & date) const {
				if (time_ < date.time_)
					return -1;
				else if (time_ > date.time_)
					return 1;
				return 0;
			}

			//! Compare
			bool operator==(const date_time_imp& date) const {
				return time_ == date.time_;
			}
			//! Compare
			bool operator!=(const date_time_imp& date) const {
				return time_ != date.time_;
			}
			//! Compare
			bool operator<=(const date_time_imp& date) const {
				return time_ <= date.time_;
			}
			//! Compare
			bool operator>=(const date_time_imp& date) const {
				return time_ >= date.time_;
			}
			//! Compare
			bool operator< (const date_time_imp& date) const {
				return time_ < date.time_;
			}
			//! Compare
			bool operator> (const date_time_imp& date) const {
				return time_ > date.time_;
			}

			// TDateTime math
			//! Add TDateTime values
			const date_time_imp operator+(size_t sec) const {
				return date_time_imp(static_cast<time_t>(get() + sec));
			}
			//! Subtract TDateTime values
			const date_time_imp operator-(size_t sec) const {
				return date_time_imp(static_cast<time_t>(get() - sec));
			}
			//! Add a TDateTime value from this TDateTime object.
			const date_time_imp& operator+=(size_t sec) {
				set(get() + sec);
				return *this;
			}
			//! Subtract a TDateTime value from this TDateTime object.
			const date_time_imp& operator-=(size_t sec) {
				set(get() - sec);
				return *this;
			}

			time_t	get() const {
				return traits::get_time(time_);
			}

			date_time_imp & set(time_t t) {
				traits::set_time(time_, t);
				return *this;
			}

		public:
			//! Sets the value of this TDateTime object to the specified date/time value.
			const date_time_imp&  set(int year, int month, int day,
				int hour, int minute, int second, int isdst = 0) {
				traits::set_time(time_, standard_time_op::mktime(year, month, day, hour, minute, second, isdst));
				return *this;
			}

			//! Sets the value of this TDateTime object to the specified date-only value.
			const date_time_imp&  set_date(int year, int month, int day) {
				int year1 = 0, month1 = 0, day1 = 0, hour = 0, minute = 0, second = 0, yday = 0, wday = 0, isdst = 0;
				get(year1, month1, day1, hour, minute, second, yday, wday, isdst);
				set(year, month, day, hour, minute, second, isdst);
				return *this;
			}
			//! Sets the value of this TDateTime object to the specified time-only value.
			const date_time_imp&  set_time(int hour, int min, int sec) {
				int year = 0, month = 0, day = 0, hour1 = 0, minute1 = 0, second1 = 0, yday = 0, wday = 0, isdst = 0;
				get(year, month, day, hour1, minute1, second1, yday, wday, isdst);
				set(year, month, day, hour, min, sec, isdst);
				return *this;
			}
			//! Set struct tm
			const date_time_imp & set(const struct tm & data) {
				traits::set_time(time_, standard_time_op::mktime(data));
				return *this;
			}

			template <class _Clock, class _Duration>
			const date_time_imp & set(const std::chrono::time_point<_Clock, _Duration>& __atime) {
				std::chrono::time_point<_Clock, std::chrono::seconds> __s = std::chrono::time_point_cast<std::chrono::seconds>(__atime);
				set(static_cast<time_t>(__s.time_since_epoch().count()));
				return *this;
			}

			//! Step the value of this TDateTime object
			const date_time_imp&  step(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) {
				struct tm info = { 0 };
				get(info);
				info.tm_year += year;
				info.tm_mon += month;
				info.tm_mday += day;
				info.tm_hour += hour;
				info.tm_min += minute;
				info.tm_sec += second;
				return set(info);
			}

			//! Return the date/time value
			inline void get(int & year, int & month, int & day, int & hour, int & minute, int & second) const {
				int yday = 0, wday = 0, isdst = 0;
				get(year, month, day, hour, minute, second, yday, wday, isdst);
			}
			inline void get(int & year, int & month, int & day, int & hour, int & minute, int & second, int & yday, int & wday, int & isdst) const {
				standard_time_op::local_time(traits::get_time(time_), year, month, day, hour, minute, second, yday, wday, isdst);
			}

			//! Return the date-only value
			inline void get_date(int & year, int & month, int & day) const {
				int hour = 0, min = 0, sec = 0;
				get(year, month, day, hour, min, sec);
			}
			//! Return the time-only value
			inline void get_time(int & hour, int & min, int & sec) const {
				int year = 0, month = 0, day = 0;
				get(year, month, day, hour, min, sec);
			}
			//! get as struct tm
			inline void get(struct tm & data) const {
				standard_time_op::local_time(traits::get_time(time_), data);
			}
			inline void get_gmt(struct tm & data) const {
				standard_time_op::gmt_time(traits::get_time(time_), data);
			}

			//! Return the day of week, 0=Sun, 1=Mon, ..., 6=Sat
			inline int day_of_week() const {
				struct tm info = { 0 };
				get(info);
				return info.tm_wday;
			}
			//! Return days since start of year, Jan 1 = 1
			inline int day_of_year() const {
				struct tm info = { 0 };
				get(info);
				return info.tm_yday;
			}

			// formatting
			//! Format the Date time string
			//! \param lpszFormat the string format, if lpszFormat==nullptr, then will use the standard format : %Y-%m-%d %H:%M:%S
			//! \return the date time string.
			std::wstring 	wformat(const wchar_t * pattern = nullptr) const {
				return standard_time_op::local_format<wchar_t, std::wstring>(traits::get_time(time_), pattern == nullptr ? DEFAULT_DATETIME_FORMAT_L : pattern);
			}
			std::string 	format(const char * pattern = nullptr) const {
				return standard_time_op::local_format<char, std::string>(traits::get_time(time_), pattern == nullptr ? DEFAULT_DATETIME_FORMAT : pattern);
			}
			std::wstring 	wformat_gmt(const wchar_t * pattern = nullptr) const {
				return standard_time_op::gmt_format<wchar_t, std::wstring>(traits::get_time(time_), pattern == nullptr ? DEFAULT_DATETIME_FORMAT_L : pattern);
			}
			std::string 	format_gmt(const char * pattern = nullptr) const {
				return standard_time_op::gmt_format<char, std::string>(traits::get_time(time_), pattern == nullptr ? DEFAULT_DATETIME_FORMAT : pattern);
			}

			//! Parse the DateTime string
			bool		parse(const char * date, const char * pattern) {
				struct tm info = { 0 };
				bool bo = standard_time_op::parse(info, pattern == nullptr ? DEFAULT_DATETIME_FORMAT : pattern, date);
				if (bo)
					set(info);
				return bo;
			}
			bool		parse(const wchar_t * date, const wchar_t * pattern) {
				struct tm info = { 0 };
				bool bo = standard_time_op::parse(info, pattern == nullptr ? DEFAULT_DATETIME_FORMAT_L : pattern, date);
				if (bo)
					set(info);
				return bo;
			}

			template<class Archive>
			void serialize(Archive & ar, const unsigned int) {
				ar & time_;
			}
		protected:
			T		get_raw() const { return time_; }
			T	&	get_raw() { return time_; }
		private:
			T				time_;
		}; //class TDateTime

		template<typename T, typename traits>
		date_time_imp<T, traits>		date_time_imp<T, traits>::max_val = date_time_imp<T, traits>(time_t(-1));

		template<typename T, typename traits>
		std::ostream & operator<<(std::ostream & out, const date_time_imp<T, traits> & t) {
			out << t.format();
			return out;
		}
		template<typename T, typename traits>
		std::wostream & operator<<(std::wostream & out, const date_time_imp<T, traits> & t) {
			out << t.wformat();
			return out;
		}
	}//internal

	typedef  internal::date_time_imp<time_t, internal::standard_time_traits>	date_time;

	/**
	* @class timer_val
	*
	* @brief Operations on "timeval" structures, which express time in
	* seconds (secs) and microseconds (usecs).
	* These time values are typically used in conjunction with OS
	* mechanisms like <select>, <poll>, or <cond_timedwait>.
	*/
	namespace internal {
		template<typename TSec, typename TNs = long>
		class timer_val_imp
		{
		public:
			/// Constant "0".
			static const timer_val_imp zero;

			/**
			* Constant for maximum time representable.  Note that this time is
			* not intended for use with <select> or other calls that may have
			* *their own* implementation-specific maximum time representations.
			* Its primary use is in time computations such as those used by the
			* dynamic sub priority strategies in the ACE_Dynamic_Message_Queue
			* class.
			*/
			static const timer_val_imp max_time;

			// = Initialization methods.
			/// Default Constructor.
			timer_val_imp() : sec_(0), nsec_(0) {}
			/// Constructor.
			timer_val_imp(TSec sec, TNs nsec = 0) : sec_(sec), nsec_(nsec) {}
			/// Copy Constructor
			timer_val_imp(const timer_val_imp & val) : sec_(val.sec_), nsec_(val.nsec_) {}

			template<typename _Clock, typename _Duration>
			timer_val_imp(const std::chrono::time_point<_Clock, _Duration>& __atime) : sec_(0), nsec_(0) {
				std::chrono::nanoseconds __ns = std::chrono::duration_cast<std::chrono::nanoseconds>(__atime.time_since_epoch());
				set_total_nano_sec(__ns.count());
			}

			template<typename _Rep, typename _Period>
			timer_val_imp(const std::chrono::duration<_Rep, _Period>& __rtime) : sec_(0), nsec_(0) {
				std::chrono::seconds __s = std::chrono::duration_cast<std::chrono::seconds>(__rtime);
				std::chrono::nanoseconds __ns = std::chrono::duration_cast<std::chrono::nanoseconds>(__rtime - __s);
				sec_ = static_cast<TSec>(__s.count());
				nsec_ = static_cast<TNs>(__ns.count());
			}

			/// Initializes the ACE_Time_Value from two longs.
			void set(TSec sec, TNs nsec = 0) { sec_ = sec; nsec_ = nsec; }

			static timer_val_imp	current_time() {
				time_t t = 0;
				long ns = 0;
				internal::standard_time_op::get_current(t, ns);
				return timer_val_imp(t, ns);
			}

			static inline timer_val_imp	make_by_sec(unsigned long long n) {
				return timer_val_imp(n, 0);
			}
			static inline timer_val_imp	make_by_milli_sec(unsigned long long n) {
				return timer_val_imp(static_cast<TSec>(n / date_time::get_milliseconds_per_second()),
					static_cast<TNs>((n % date_time::get_milliseconds_per_second()) * date_time::get_nanoseconds_per_millisecond()));
			}
			static inline timer_val_imp	make_by_micro_sec(unsigned long long n) {
				return timer_val_imp(static_cast<TSec>(n / date_time::get_microseconds_per_second()),
					static_cast<TNs>((n % date_time::get_microseconds_per_second()) * date_time::get_nanoseconds_per_microseconds()));
			}
			static inline timer_val_imp	make_by_nano_sec(unsigned long long n) {
				return timer_val_imp(static_cast<TSec>(n / date_time::get_nanoseconds_per_seconds()),
					static_cast<TNs>(n % date_time::get_nanoseconds_per_seconds()));
			}

			/// Converts from timer_val_imp format into milli-seconds format.
			/**
			* @return Sum of second field (in milliseconds) and nanosecond field (in milliseconds).
			* @note The semantics of this method differs from the sec() and
			*       nsec() methods.  There is no analogous "millisecond"
			*       component in an timer_val_imp. */
			uint64_t	get_total_milli_sec() const {
				return static_cast<uint64_t>(sec_) * date_time::get_milliseconds_per_second() + static_cast<uint64_t>(nsec_) / date_time::get_nanoseconds_per_millisecond();
			}

			/// Converts from milli-seconds format into timer_val_imp format.
			/**
			* @note The semantics of this method differs from the sec() and
			*       usec() methods.  There is no analogous "millisecond"
			*       component in an timer_val_imp.*/
			void set_total_milli_sec(uint64_t nVal) {
				sec_ = static_cast<TSec>(nVal / date_time::get_milliseconds_per_second());
				nsec_ = static_cast<TNs>((nVal % date_time::get_milliseconds_per_second()) * date_time::get_nanoseconds_per_millisecond());
			}

			/// Converts from timer_val_imp format into micro-seconds format.
			/**
			* @return Sum of second field (in microseconds) and nanosecond field (in microseconds).
			* @note The semantics of this method differs from the sec() and
			*       nsec() methods.  There is no analogous "microsecond"
			*       component in an timer_val_imp. */
			uint64_t get_total_micro_sec() const {
				return static_cast<uint64_t>(sec_) * date_time::get_microseconds_per_second() + static_cast<uint64_t>(nsec_) / date_time::get_nanoseconds_per_microseconds();
			}

			/// Converts from micro-seconds format into timer_val_imp format.
			/**
			* @note The semantics of this method differs from the sec() and
			*       usec() methods.  There is no analogous "microsecond"
			*       component in an timer_val_imp.*/
			void set_total_micro_sec(uint64_t nVal) {
				sec_ = static_cast<TSec>(nVal / date_time::get_microseconds_per_second());
				nsec_ = static_cast<TNs>((nVal % date_time::get_microseconds_per_second()) * date_time::get_nanoseconds_per_microseconds());
			}

			uint64_t	get_total_nano_sec() const {
				return static_cast<uint64_t>(sec_) * date_time::get_nanoseconds_per_seconds() + static_cast<uint64_t>(nsec_);
			}

			void	set_total_nano_sec(uint64_t nVal) {
				sec_ = static_cast<TSec>(nVal / date_time::get_nanoseconds_per_seconds());
				nsec_ = static_cast<TNs>(nVal % date_time::get_nanoseconds_per_seconds());
			}

			// = The following are accessor/mutator methods.
			/// Get seconds.
			/**
			* @return The second field/component of this timer_val_imp.
			* @note The semantics of this method differs from the convert_to_milli_sec() and convert_to_micro_sec() method.*/
			TSec sec() const { return sec_; }

			/// Set seconds.
			void sec(TSec sec) { sec_ = sec; }

			/// Get nanoseconds.
			/**
			* @return The nanosecond field/component of this timer_val.
			* @note The semantics of this method differs from the convert_to_milli_sec() and convert_to_micro_sec() method.*/
			TNs nano_sec() const { return nsec_; }

			/// Set nanoseconds.
			void nano_sec(TNs nsec) { nsec_ = nsec; }

			/// Get microseconds.
			/**
			* @return The microsecond field/component of this timer_val.
			* @note The semantics of this method differs from the convert_to_milli_sec() and convert_to_micro_sec() method.*/
			long micro_sec() const { return static_cast<long>(nsec_ / date_time::get_nanoseconds_per_microseconds()); }

			/// Set microseconds.
			void micro_sec(long micro_sec) { nsec_ = static_cast<TNs>(micro_sec * date_time::get_nanoseconds_per_microseconds()); }

			/// Get milliseconds.
			/**
			* @return The millisecond field/component of this timer_val.
			* @note The semantics of this method differs from the convert_to_milli_sec() and convert_to_micro_sec() method.*/
			long milli_sec() const { return static_cast<long>(nsec_ / date_time::get_nanoseconds_per_millisecond()); }

			/// Set milliseconds.
			void milli_sec(long milli_sec) { nsec_ = static_cast<TNs>(milli_sec * date_time::get_nanoseconds_per_millisecond()); }

			template<typename _Clock, typename _Duration>
			std::chrono::time_point<_Clock, _Duration>	to_time_point() const {
				std::chrono::seconds __s = sec_;
				std::chrono::nanoseconds __ns = nsec_;
				return __s + __ns;
			}

			std::chrono::nanoseconds	to_duration() const {
				std::chrono::seconds __s(sec_);
				std::chrono::nanoseconds __ns(nsec_);
				return __s + __ns;
			}

			int			compare(const timer_val_imp & nVal) const {
				uint64_t val = (static_cast<uint64_t>(sec_) << 32) | static_cast<uint64_t>(nsec_ & 0xffffffff);
				uint64_t val2 = (static_cast<uint64_t>(nVal.sec_) << 32) | static_cast<uint64_t>(nVal.nsec_ & 0xffffffff);
				if (val < val2)
					return -1;
				else if (val > val2)
					return 1;
				return 0;
			}

			bool operator==(const timer_val_imp & val) const { return sec_ == val.sec_ && nsec_ == val.nsec_; }
			bool operator!=(const timer_val_imp & val) const { return !(*this == val); }
			bool operator< (const timer_val_imp & val) const { return compare(val) < 0; }
			bool operator>=(const timer_val_imp & val) const { return compare(val) >= 0; }
			bool operator> (const timer_val_imp & val) const { return compare(val) > 0; }
			bool operator<=(const timer_val_imp & val) const { return compare(val) <= 0; }

			const timer_val_imp & operator+=(const timer_val_imp & rv) {
				sec_ += rv.sec_;
				nsec_ += rv.nsec_;
				if (nsec_ > static_cast<TNs>(date_time::get_nanoseconds_per_seconds())) {
					nsec_ -= static_cast<TNs>(date_time::get_nanoseconds_per_seconds());
					++sec_;
				}
				return *this;
			}
			const timer_val_imp & operator-=(const timer_val_imp & rv) {
				if (nsec_ < rv.nsec_) {
					nsec_ += static_cast<TNs>(date_time::get_nanoseconds_per_seconds());
					--sec_;
				}
				sec_ -= rv.sec_;
				nsec_ -= rv.nsec_;
				return *this;
			}

			timer_val_imp operator+(const timer_val_imp & rv) const {
				timer_val_imp	tmp(*this);
				tmp += rv;
				return tmp;
			}
			timer_val_imp operator-(const timer_val_imp & rv) const {
				timer_val_imp	tmp(*this);
				tmp -= rv;
				return tmp;
			}

			template<class Archive>
			void serialize(Archive & ar, const unsigned int) {
				ar & sec_ & nsec_;
			}

		private:
			TSec    sec_;         /// seconds
			TNs     nsec_;        /// and nanoseconds 
		};

		template<typename TSec, typename TNs>
		const timer_val_imp<TSec, TNs> timer_val_imp<TSec, TNs>::zero = timer_val_imp<TSec, TNs>();

		template<typename TSec, typename TNs>
		const timer_val_imp<TSec, TNs> timer_val_imp<TSec, TNs>::max_time = timer_val_imp<TSec, TNs>(TSec(-1), TNs(-1));

		template<typename TSec, typename TNs, typename Ch>
		std::basic_ostream<Ch> & operator<<(std::basic_ostream<Ch> & out, const timer_val_imp<TSec, TNs> & t) {
			out << t.sec() << "s";
			TNs ns = t.nano_sec();
			if (ns)
			{
				TNs ms = static_cast<TNs>(ns / date_time::get_nanoseconds_per_millisecond());	ns -= static_cast<TNs>(ms * date_time::get_nanoseconds_per_millisecond());
				if (ms)
					out << ms << "ms";
				if (ns)
				{
					long us = static_cast<TNs>(ns / date_time::get_nanoseconds_per_microseconds()); ns -= static_cast<TNs>(us * date_time::get_nanoseconds_per_microseconds());
					if (us)
						out << us << "us";
					if (ns)
						out << ns << "ns";
				}
			}
			return out;
		}
	}//internal

	typedef internal::timer_val_imp<time_t, long>		timer_val;

}//namespace ara {

inline ara::date_time operator "" _date(const char * p, size_t n) {
	return ara::date_time(p);
}

inline ara::timer_val operator "" _ms(unsigned long long n) {
	return ara::timer_val::make_by_milli_sec(n);
}

inline ara::timer_val operator "" _us(unsigned long long n) {
	return ara::timer_val::make_by_micro_sec(n);
}

inline ara::timer_val operator "" _ns(unsigned long long n) {
	return ara::timer_val::make_by_nano_sec(n);
}

inline ara::timer_val operator "" _sec(unsigned long long n) {
	return ara::timer_val::make_by_sec(n);
}

inline ara::timer_val operator "" _minute(unsigned long long n) {
	return ara::timer_val::make_by_sec(n * ara::date_time::get_seconds_per_minute());
}

inline ara::timer_val operator "" _hour(unsigned long long n) {
	return ara::timer_val::make_by_sec(n * ara::date_time::get_seconds_per_hour());
}

inline ara::timer_val operator "" _day(unsigned long long n) {
	return ara::timer_val::make_by_sec(n * ara::date_time::get_seconds_per_day());
}

#endif //ARA_DATETIME_H
