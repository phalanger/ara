# Install BOOST and OPENSSL at Windows platform

## install BOOST such like this

* Download the BOOST source code package from <http://www.boost.org>
* Suggest to use version **1.67.0** or above
* Unzip the BOOST source code package and change to the directory
* Run these commands in Shell

~~~~~~~~~~bat
    bootstrap.bat
    b2 variant=debug link=static threading=multi runtime-link=static --without-python
    b2 variant=release link=static threading=multi runtime-link=static --without-python
    b2 variant=debug link=static threading=multi runtime-link=static address-model=64 --without-python
    b2 variant=release link=static threading=multi runtime-link=static address-model=64 --without-python
~~~~~~~~~~

* Maybe compile the source code in difference toolset

| toolset          |   IDE               |
|------------------|---------------------|
| toolset=msvc-14.1 |  VisualStudio 2017 |
| toolset=msvc-14 |  VisualStudio 2015 |
| toolset=msvc-12 |  VisualStudio 2013 |

## Install OPENSSL such like this

* Download the OPENSSL compiled package from <https://slproweb.com/products/Win32OpenSSL.html>
* Suggest to use version **1.1.0f** or above
* Run the Win64OpenSSL-x_x_xx.exe and Win32OpenSSL-x_x_xx.exe to install OPENSSL
* the OPENSSL header files will be found at the INSTALL_DIR/include
* the OPENSSL lib files will be found at the INSTALL_DIR/lib

## Open the VS2015 / VS2017 sln file directly

* open the IDE solution file in **ara/build/vc2015/ara.sln** and **ara/build/vc2017/ara.sln**
* set the BOOST and OPENSSL directories
  * Property Manager
    * -> test / benchmark project
    * -> Debug | Win32 / Debug | x64
    * -> Microsoft.Cpp.Win32.user / Microsoft.Cpp.x64.user
    * -> Properties
    * -> Common Properties
    * -> VC++ Directories
    * -> Input Directories (add the BOOST/OPENSSL include path)
    * -> Library Directories (add the BOOST/OPENSSL Library path)
