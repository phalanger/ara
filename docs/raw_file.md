# raw_file

## [Header file](../ara/filesys.h)

~~~C++
#include "ara/filesys.h"
~~~

## Descriptions

**ara::raw_file** is a simple class wrap the file operations. No cache, just a **HANDLE** value on Windows and a **int** value on Linux/OSX.

## Example

Open a file

~~~C++
std::string file = "~/123.txt";
ara::raw_file rf;
char buf[1024];
if ( rf.open(file).read_only().binary().done() ) {
    rf.read(buf, 1024);
}
~~~

ara::raw_file::open() function will return an object to control open file flags.
| flag | function |
|------|----------|
| O_RDONLY | read_only() |
| O_WRONLY | write_only() |
| O_RDWR | read_write() |
| O_CREAT | create() |
| O_APPEND | append() |
| O_TRUNC | truncat() |
| O_EXCL | exclusive() |
| O_BINARY | binary() |
| O_TEXT | text() |
| O_TEMPORARY | temporary() |
| O_RANDOM | random() |

control object also can set the open flags by bitset. Above exsample can also write like this:

~~~C++
if (rf.open(file).flags(O_RDONLY|O_BINARY).done()) {
    rf.read(buf, 1024);
}
~~~

## Notes

Remember to call done() while finished set the open file flags.

## See also

* test and exsample :
  * [test/test_filesys.cpp](../test/test_filesys.cpp)
* other classes used it:
  * [ara/internal/log_appender_rollingfile.h](../ara/internal/log_appender_rollingfile.h)
  