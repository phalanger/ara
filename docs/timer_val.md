# timer_val

## [Header file](../ara/datetime.h)

~~~C++
#include "ara/datetime.h"
~~~

## Descriptions

Usually C/C++ handle the timedout setting by **struct timeval**. The structure is used to specify a time interval. But **struct timeval** just store seconds and microseconds. Sometimes we need handle **nanoseconds**.

**ara::timer_val** is a simple class to maintain the **seconds** and **nanoseconds** data.

## Example

~~~C++
ara::timer_val t1 = ara::timer_val::current_time();
ara::timer_val t2(3, 1000);
const auto ms1 = 1000_ms;
const auto ms2 = 1_sec;
auto ms3 = 1_sec + 20_ms + + 30_us + 40_ns;
~~~

C++11 add some template class for **Clock**, such like **std::chrono::time_point** and **std::chrono::duration**. **timer_val** can convert to those C++11 **Clock** objects. And also can convert from them.

~~~C++
const auto ms1 = 1000_ms;
auto d1 = ms1.to_duration();

ara::timer_val t1( std::chrono::system_clock::now() );
auto t2 = t1.to_time_point<std::chrono::system_clock>();
~~~

## See also

* test and exsample :
  * [test/test_datetime.cpp](../test/test_datetime.cpp)
* other classes used it:
  * [ara/async_queue.h](../ara/async_queue.h)
  * [ara/async_rwqueue.h](../ara/async_rwqueue.h)