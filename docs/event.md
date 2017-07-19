# event

## [Header file](../ara/event.h)

~~~C++
#include "ara/event.h"
~~~

## Descriptions

std::mutex + std::condition_variable + Number like object -> event

## Example

~~~C++
enum MY_EVENT {
  EVENT_INIT,
  EVENT_OPEN_THE_DOOR,
  EVENT_OPEN_THE_WINDOW
};

ara::event<MY_EVENT> n(EVENT_INIT);
~~~

In thread 1

~~~C++
  foo1();
  n.singal_all(EVENT_OPEN_THE_WINDOW);
~~~

In thread 2

~~~C++
if (n.wait(EVENT_OPEN_THE_WINDOW))
  foo2();
~~~

Then thread 2 will keep waitting for the event EVENT_OPEN_THE_WINDOW. While thread 1 finished foo1(), it will singal all other threads who are waiting for event EVENT_OPEN_THE_WINDOW, then thread 2 can keep on going.

And by using event, foo2() will run after foo1() finished.

## Notes

* **event** also define these functions :
  * wait_until
  * signal_one
  * inc_signal_all
  * inc_signal_one
  * value
  * rest

## See also

* test and exsample :
  * [test/test_async_queue.cpp](../test/test_async_queue.cpp)
