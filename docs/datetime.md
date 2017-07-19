# datetime

## [Header file](../ara/datetime.h)

~~~C++
#include "ara/datetime.h"
~~~

## Descriptions

Usually C/C++ handle the date/time (such as year/month/day/hours/minute/second)  use **time_t** and the **struct tm**

**ara::date_time** is a simple class to maintain the date/time functions.

~~~C++
    ara::date_time now = ara::date_time::get_current();
    ara::date_time that_day(2011,3,28,10,30,00);
    ara::date_time day1 = "2016-02-21 11:23:45"_date;
    ara::date_time day2(time(NULL));
    ara::date_time day3( std::chrono::system_clock::now() );
    ara::date_time day4( "2016-02-21 11:23:45", "%Y-%m-%d %H:%M:%S" );

    ara::date_time tomorrow = now.step( 0, 0, 1);
    int tomorrow_day_of_year = tomorrow.day_of_year();

    auto local_time = tomorrow.format();
    auto gmt_time = tomorrow.format_gmt("%Y%m%d");
~~~

**ara::date_time** default precision is **second**. But sometimes may need store **micorsecond** or **millisecond** with the date. We can define our datetime storage traits and extend the **date_time** class.

~~~C++
class date_time_with_ms_traits
{
public:
    static inline void set_time(uint64_t & tar, time_t src) {
        tar = (src * 1000) | (tar % 1000);
    }
    static inline time_t get_time(uint64_t src) {
        return src / 1000;
    }
};

class date_time_ms : public ara::internal::date_time_imp< uint64_t, date_time_with_ms_traits >
{
public:
    //Define the ms handle functions
    void    set_ms(size_t n) {
        auto & t = get_raw();
        t = t - (t % 1000) + (n % 1000);
    }
    size_t  get_ms() const {
        retrun get_raw() % 1000;
    }
}
~~~

## See also

* test and exsample :
  * [test/test_datetime.cpp](../test/test_datetime.cpp)