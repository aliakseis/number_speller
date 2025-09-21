![Image](https://github.com/user-attachments/assets/05f6a9f0-769a-46c9-9a76-4c8d41de4660)
Number Speller
==============

A C++ console application that reads integers from standard input and outputs their spelled-out forms in both English and Russian. It supports negative values, zero, and numbers up to the billions.

Features
--------

*   Converts any 32-bit signed integer into words
    
*   Bilingual output (English and Russian)
    
*   Handles negative numbers with a “Minus/минус” prefix
    
*   Modular design using a hierarchy of ISpeller strategies
    
*   Zero-dependency (uses only the C++ standard library)
    

Prerequisites
-------------

*   A C++17-compatible compiler (GCC, Clang, MSVC, etc.)
    
*   CMake 3.10 or newer (optional, but recommended for cross-platform builds)
    

Building
--------

### Using CMake

`   mkdir build  cd build  cmake ..  cmake --build .   `

This will generate an executable named number\_speller (or number\_speller.exe on Windows) in the build directory.

### Manual Compilation

`   # Linux / macOS (with GCC or Clang)  g++ -std=c++17 -O2 ../number_speller.cpp -o number_speller  # Windows (with MSVC)  cl /EHsc /std:c++17 ..\number_speller.cpp   `

Usage
-----

Run the program and type integers. Each line you enter will produce two lines of output: English, then Russian.

`$ ./number_speller  123  One Hundred Twenty Three   сто двадцать три   -45  Minus Forty Five   минус сорок пять` 

To exit, send an end-of-file (Ctrl+D on Linux/macOS, Ctrl+Z on Windows).


License
-------

This project is released under the MIT License. Feel free to fork, inspect, and extend
