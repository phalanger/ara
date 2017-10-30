# key_string

## [Header file](../ara/key_string.h)

~~~C++
#include "ara/key_string.h"
~~~

## Descriptions

**key** string field in std::map and other key-value container, always look like these:

* Const object. It can not be modified after insert into the container key field.

* It is duplicate in one multimap or in many containers. Specially in **json** like conatiner, the root object is holding a vector of objects. And each object has the same attributes such like "name", "param" ... The **key** name of attributes may be duplicate many times.

* It can just reference to a static const char data by pointer. Such like my_map["name"] = xxxx, and "name" is a static data stored in the DATA Segment in a process.

* the key string data is too short. Sometimes it is shorter than a pointer.

**key_string** is designed for those key field.

**key_string**  has three type storage for string data.

* Reference to a static const char data.

~~~C++
    const char * tmp = "age";
    ara::key_string str = ara::key_string::ref(tmp);

    ara::key_string str2 = ara::key_string::ref( "name" );
    ara::key_string str3( "address" );
~~~

But sometimes need to copy the string data.

~~~C++
    const char * tmp = "age";
    ara::key_string str = ara::key_string::copy(tmp);
    ara::key_string str2(tmp, 2); //copy 2 char from string. It's difference from the simple construtor with a static const char pointer.
~~~

* store data in key_string object directly while string size less than size of a pointer. (4 byte or 8 byte depond on OS and application type)

* a simple shared data by reference count. While copy the string data, just increase the reference count. And decrease counter while destruct it, release data after no onre reference to.

## Example

* Detect string storage type

~~~C++
    ara::key_string s = "Hello world";
    if (s.data_type() == ara::key_string::TYPE_CONST) {
        //foo, string is refernec to a static string data.
    } else if (s.data_type() == ara::key_string::TYPE_BUF) {
        //foo, string data is stored in the key_string object.
    } else if (s.data_type() == ara::key_string::TYPE_STORE) {
        //foo, string data is stored in heap, controlled by a reference counter
    }
~~~

* Get the reference count

~~~C++
    ara::key_string s = "Hello world";
    ara::key_string s2 = s;
    ara::key_string s3 = s;
    ara::key_string s4 = s2;
    if (s.ref_count() == 4) {
        //foo
    }
~~~

## Notes

**key_string** works like a standard string. But it can not be modified after construct.

No matter which storage type it used, **key_string** can be compared with other difference storage type object.

## See also

* test and exsample :
  * [test/test_keystring.cpp](../test/test_keystring.cpp)
* other classes used it:
  * [ara/variant.h](../ara/variant.h)
