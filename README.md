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