# file_sys

## [Header file](../ara/filesys.h)

~~~C++
#include "ara/filesys.h"
~~~

## Descriptions

A simple class defined some functions to handle file system operations, such as split path name, unlink file, etc.

**file_sys** is not really a class, it looks more like a namespace definition.

## Example

* return the path slash charactor depend on OS, return '\\' on windows and return '/' on Linux/OSX
~~~C++
ara::file_sys::path_slash()
~~~

* detect the charactor is whether a path slash.

| char | Windows | Linux/OSX |
|------|---------|-----------|
|   /  |   true  |  true     |
|  \\  |   true  |  false    |
|others|   false |  false    |
~~~C++
ara::file_sys::isPathSlashChar( ch )
~~~

* auto add an end slash charactor to a path string
~~~C++
ara::file_sys::to_path( "/abc/def/ghi" );    //return "/abc/def/ghi/"
ara::file_sys::to_path( "/abc/def/ghi/" );   //return "/abc/def/ghi/"
ara::file_sys::to_path( "/abc\\def/ghi" );   //return "/abc\\def/ghi\\"
ara::file_sys::to_path( "/abc\\def/ghi/" );   //return "/abc\\def/ghi/"
~~~



## Notes

It's important to initiate the root node by call member function **as_root()**. The root node's pointers are both point to root node self. And the normal node's pointers are both point to nullptr.

## See also

* test and exsample :
  * [test/test_filesys.cpp](../test/test_filesys.cpp)
