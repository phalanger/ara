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

If need the **async** class, which depend on BOOST **asio**, need to build the BOOST library.

### Windows platform

install BOOST such like this:

~~~~~~~~~~bat
    bootstrap.bat
    b2 variant=debug link=static threading=multi runtime-link=static --without-python
    b2 variant=release link=static threading=multi runtime-link=static --without-python
    b2 variant=debug link=static threading=multi runtime-link=static address-model=64 --without-python
    b2 variant=release link=static threading=multi runtime-link=static address-model=64 --without-python
~~~~~~~~~~

### linux platform

install BOOST such like this:

~~~~~~~~~~bat
    b2 -j 4 toolset=gcc link=static  stage debug
    b2 -j 4 toolset=gcc-32 link=static  stage debug
    b2 -j 4 toolset=gcc-64 link=static  stage debug
~~~~~~~~~~