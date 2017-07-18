# Install BOOST and OPENSSL at Linux platform

## install BOOST such like this

* Download the BOOST source code package from <http://www.boost.org>
* Suggest to use version **1.64.0** or above
* Unzip the BOOST source code package and change to the directory
* Run these commands in Shell

~~~~~~~~~~bat
    b2 -j 4 toolset=gcc link=static  stage debug
    b2 -j 4 toolset=gcc-32 link=static  stage debug
    b2 -j 4 toolset=gcc-64 link=static  stage debug
~~~~~~~~~~