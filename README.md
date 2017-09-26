# ara

|Platform Build status| Master | Develop |
| :---------------------------------: | :---------------: | :-----------------: |
|[Linux/Mac (on Travis CI)](https://travis-ci.org/phalanger/ara)| ![lin-master-badge] | ![lin-dev-badge]        |
|[Windows (on AppVeyor)](https://ci.appveyor.com/project/phalanger/ara)| ![win-master-badge] | ![win-dev-badge]  |

[lin-master-badge]: https://travis-ci.org/phalanger/ara.svg?branch=master "linux master build status"
[lin-dev-badge]: https://travis-ci.org/phalanger/ara.svg?branch=develop "linux deleveop build status"
[win-master-badge]: https://ci.appveyor.com/api/projects/status/842088lgtg7gnyx8/branch/master "windows master build status"
[win-dev-badge]: https://ci.appveyor.com/api/projects/status/842088lgtg7gnyx8/branch/develop "windows deleveop build status"

"ara" is a simple template C++14 library.

* ara is **small**
* ara is **header only**
* ara depend on STL
* ara dosn't depend on BOOST unless need the **async** class.

## Installation

ara is a header-only C++ library. Just copy the `ara` folder to system or project's include path.

If need the **async** class, which depend on BOOST **asio** and **openssl**, need to build the BOOST library.

[Windows platform install guide](docs/install_win.md)

[Linux platform install guide](docs/install_linux.md)

## Reference section

* [datetime](docs/datetime.md) : a simple class to handle the date/time
* [timer_val](docs/timer_val.md) : a simple class look likes **timeval** to store **seconds** and **nanoseconds**
* [dlist](docs/dlist.md) : a simple template to help any class to implement doubly linked list, which is easy to unlink object self from list.
* [event](docs/event.md) : similar to CEvent, providing synchronized methods to indicate something happend or to wait for something has happened. And also define the **sleep**/**sleep_until** functions using **event** class.
* [file_sys](docs/filesys.md) : define some class to handle file system operations, such as handle path name, unlink file.
* [scan_dir](docs/scan_dir.md) : a simple class to navigate directory.
* [raw_file](docs/raw_file.md) : a simple class wrapping file **HANDLE** operations.
* [fixed_string](docs/fixed_string.md) :
* [json](docs/json.md) :
* [key_string](docs/key_string.md) :
* [log](docs/log.md) :
* [promise](docs/promise.md) :
* [ref_string](docs/ref_string.md) :
* [session](docs/session.md) :
* [singleton](docs/singleton.md) :
* [stringex](docs/stringex.md) :
* [threadex](docs/threadex.md) :
* [token](docs/token.md) :
* [utf8](docs/utf8.md) :
* [variant](docs/variant.md) :