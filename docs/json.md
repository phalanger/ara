# json

## [Header file](../ara/json.h)

~~~C++
#include "ara/json.h"
~~~

## Descriptions

A simple class to store a [variant](../ara/variant.h) object as json format.
Using [rapidjson](https://github.com/Tencent/rapidjson) to load/save the [variant](../ara/variant.h) object.

## Example

* a simple parse json example

~~~C++
ara::var v;
v = ara::json::parse("{  \
    \"a\" : 100, \
    \"b\" : \"hello world\" \
}");
~~~

* initial with json format string

~~~C++
ara::var v = "[{\"abcdefghijk\" : 0.6, \"1234567890asd\" : \"11111111111111\"}, 100]"_json;
~~~

* Specially, the json source string hold the data life cycle longer than the json object, json can parse the string by reference. And the variant object needn't copy all the data from string, just reference to it if it's possible.

~~~C++
std::wstring str3 = L"{  \
    \"a\" : 100,  \
    \"b\" : \"hello world\"  \
}";
ara::var v = ara::json::parse_ref( str3 );
~~~

* save a variant to a json string

~~~C++
std::string s2 = ara::json::save<std::string>(v);
std::wstring s3 = ara::json::pretty_save<std::wstring>(v);
~~~

## Notes

The unicode string will be changed to utf-8 string into variant.

## See also

* test and exsample :
  * [test/test_json.cpp](../test/test_json.cpp)
