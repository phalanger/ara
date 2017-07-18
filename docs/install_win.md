# Install BOOST and OPENSSL at Windows platform

## install BOOST such like this

* Download the BOOST source code package from <http://www.boost.org>
* Suggest to use version **1.64.0** or above
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

## install OPENSSL such like this

* Download the OPENSSL compiled package from <https://slproweb.com/products/Win32OpenSSL.html>
* Suggest to use version **1.1.0f** or above
* Run the Win64OpenSSL-x_x_xx.exe and Win32OpenSSL-x_x_xx.exe to install OPENSSL
* the OPENSSL header files will be found at the INSTALL_DIR/include
* the OPENSSL lib files will be found at the INSTALL_DIR/lib