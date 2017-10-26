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
ara::file_sys::to_path<std::string>("/abc/def/ghi");    //return std::string("/abc/def/ghi/")
ara::file_sys::to_path<std::string>("/abc/def/ghi/");   //return std::string("/abc/def/ghi/")
ara::file_sys::to_path<std::string>("/abc\\def/ghi");   //return std::string("/abc\\def/ghi\\")
ara::file_sys::to_path(std::string("/abc\\def/ghi/"));   //return std::string("/abc\\def/ghi/")
~~~

* join parent path and sub path to a path name string

~~~C++
ara::file_sys::join_to_path<std::string>("/abcd", "def") //return std::string("/abcd/def/")
~~~

* join the path and file name to a file name string

~~~C++
ara::file_sys::join_to_file<std::string>("C:\\abcd\\", "\\def") //return std::string("C:\\abcd\\def")
~~~

* detect the string looks like a file name or a path name

~~~C++
ara::file_sys::is_path<std::string>("//abcde//")   //return true
ara::file_sys::is_path<std::string>("//abcde")     //return false
ara::file_sys::is_path<std::string>("C:\\abcde\\") //return true
ara::file_sys::is_path<std::string>("C:\\abcde")   //return false
~~~

* splite string into parent path part and file part or sub path part

~~~C++
std::string full, path, sub;
ara::file_sys::split_path(full, path, sub);
~~~

| full             | path      | sub      |
|------------------|-----------|----------|
| C:\\123\\456.txt | C:\\123\\ | 456.txt  |
| C:\\123\\456\\   | C:\\123\\ | 456      |
| C:\\123          | C:\\      | 123      |
| /abc/def.txt     | /abc/     | def.txt  |
| /abc/def/        | /abc/     | def      |
| /abc             | /         | abc      |

* split the path into a navigatable container

~~~C++
std::wstring wstrPath = L"C:\\Windows\\abc\\def";
for (auto it : ara::file_sys::split_path(wstrPath)) {
    //C:  Windows  abc  def
}
~~~

* get the temp folder path

~~~C++
std::string path;
if (ara::file_sys::get_temp_folder(path)) {
    //foo
}
~~~

 | OS | path |
 |----|------|
 | Windows | ::GetTempPath/::GetTempPathW |
 | Linux/OSX | getenv("TMPDIR") or "/tmp" |

* get the file or path attribute

~~~C++
ara::file_adv_attr attr;
if (ara::file_sys::get_file_attr("C:\\Windows", attr)) {
    if (attr.is_dir()) {
        //foo
    }
}
~~~

* scan the path and naviagte all items in the path including file and sub-dir

~~~C++
std::vector<std::string> vectPathName;
for (auto it : ara::scan_dir("/home")) {
    vectPathName.push_back(it);
}
~~~

## Notes

Under Linux/OSX system, file name with unicode will be changed to utf-8 format before handle it.

## See also

* test and exsample :
  * [test/test_filesys.cpp](../test/test_filesys.cpp)
