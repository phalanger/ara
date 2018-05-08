# Install BOOST and OPENSSL at Linux platform

## Install BOOST such like this

* Download the BOOST source code package from <http://www.boost.org>
* Suggest to use version **1.67.0** or above
* Unzip the BOOST source code package and change to the directory
* Run these commands in Shell

~~~~~~~~~~bat
    b2 -j 4 toolset=gcc link=static  stage debug
    b2 -j 4 toolset=gcc-32 link=static  stage debug
    b2 -j 4 toolset=gcc-64 link=static  stage debug
~~~~~~~~~~

## Build and run unit test

~~~~~~~~~~bat
cd ara
mkdir bin
cmake ..
make
make test
~~~~~~~~~~

## If **cmake** can not find the BOOST

set then BOOST env setting before running cmake

~~~~~~~~~~~bat
export BOOST_ROOT=/usr/local/boost/
export BOOST_INCLUDE_DIRS=/usr/local/boost/include/
export BOOST_LIBRARY_DIRS=/usr/local/boost/lib/
~~~~~~~~~~~
