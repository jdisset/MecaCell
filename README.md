#MecaCell
MecaCell is an open-source cellular physics agent-based simulation platform. It is designed with artificial life and morphogenetic engineering in mind and tries to stay not too invasive, lightweight and easily extensible.
It comes in the form of two libraries :
- **MecaCell** is the _core_ library which contains everything you need to run simulations in console or with your own viewer. It is written in C++11 and has no dependencies other than the C++ standard library.
- **MecaCellViewer** is a _viewer_ library. It is written in C++, uses OpenGL and depends on Qt5.



##Installation
You will need cmake 2.8+, a recent C++ compiler (C++11 support is required) and Qt 5.2+
1. Clone this repository
2. cd in it and create a build directory
3. cmake .. && make && make install

##Basic usage, simple example






