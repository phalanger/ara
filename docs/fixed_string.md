# fixed_string

## [Header file](../ara/fixed_string.h)

~~~C++
#include "ara/fixed_string.h"
~~~

## Descriptions

A simple class wrap a vector of CHARs, and let them look like a string class.

## Example

* simple example

~~~C++
char buf[32];
ara::fixed_string str1(buf, 32);

str1 = "hello world";
str1 += "\r\n";
~~~

* initial with content

~~~C++
char buf3[64] = "abcdefghijk";
ara::fixed_string str2(buf3, strlen(buf3), 64);
str2 += "\r\n";
~~~

## Notes

The fixed_string object's life cycle should be as the same as the CHARs.

While appending data to a fixed_string, the data which overflow the bounds of the vector, will be discarded.

## See also

* test and exsample :
  * [test/test_fixedstring.cpp](../test/test_fixedstring.cpp)
