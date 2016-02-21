# ara

bootstrap.bat
b2 variant=debug link=static threading=multi runtime-link=static --without-python
b2 variant=release link=static threading=multi runtime-link=static --without-python
b2 variant=debug link=static threading=multi runtime-link=static address-model=64 --without-python
b2 variant=release link=static threading=multi runtime-link=static address-model=64 --without-python

b2 -j 4 toolset=gcc link=static  stage debug
b2 -j 4 toolset=gcc-32 link=static  stage debug
b2 -j 4 toolset=gcc-64 link=static  stage debug
