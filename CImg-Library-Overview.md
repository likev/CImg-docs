
# CImg Library Overview
The CImg Library is an image processing library, designed for C++ programmers. It provides useful classes and functions to load/save, display and process various types of images.

## Library structure

The CImg Library consists in a single header file CImg.h providing a set of C++ template classes that can be used in your own sources, to load/save, process and display images or list of images. Very portable (Unix/X11,Windows, MacOS X, FreeBSD,..), efficient, simple to use, it's a pleasant toolkit for coding image processing stuff in C++.

The header file CImg.h contains all the classes and functions that compose the library itself. This is one originality of the CImg Library. This particularly means that :

- No pre-compilation of the library is needed, since the compilation of the CImg functions is done at the same time as the compilation of your own C++ code.
- No complex dependencies have to be handled : Just include the CImg.h file, and you get a working C++ image processing toolkit.
- The compilation is done on the fly : only CImg functionalities really used by your program are compiled and appear in the compiled executable program. This leads to very compact code, without any unused stuff.
- Class members and functions are inlined, leading to better performance during the program execution.

The CImg Library is structured as follows :

- All library classes and functions are defined in the namespace cimg_library. This namespace encapsulates the library functionalities and avoid any class name collision that could happen with other includes. Generally, one uses this namespace as a default namespace :
  ```c++
  #include "CImg.h"
  using namespace cimg_library;
  ...
  ```
- The namespace `cimg_library::cimg` defines a set of low-level functions and variables used by the library. Documented functions in this namespace can be safely used in your own program. But, never use the cimg_library::cimg namespace as a default namespace, since it contains functions whose names are already defined in the standard C/C++ library.
- The class `cimg_library::CImg` represents images up to 4-dimensions wide, containing pixels of type T (template parameter). This is actually the main class of the library.
- The class `cimg_library::CImgList` represents lists of cimg_library::CImg<T> images. It can be used for instance to store different frames of an image sequence.
- The class `cimg_library::CImgDisplay` is able to display images or image lists into graphical display windows. As you may guess, the code of this class is highly system-dependent but this is transparent for the programmer, as environment variables are automatically set by the CImg library (see also Setting Environment Variables).
- The class `cimg_library::CImgException` (and its subclasses) are used by the library to throw exceptions when errors occur. Those exceptions can be caught with a `try { ..} catch (CImgException) { .. }` block. Subclasses define precisely the type of encountered errors.

Knowing these four classes is enough to get benefit of the CImg Library functionalities.

## CImg version of "Hello world".

Below is some very simple code that creates a "Hello World" image. This shows you basically how a CImg program looks like.
```c++
#include "CImg.h"
using namespace cimg_library;
int main() {
  CImg<unsigned char> img(640,400,1,3);  // Define a 640x400 color image with 8 bits per color component.
  img.fill(0);                           // Set pixel values to 0 (color : black)
  unsigned char purple[] = { 255,0,255 };        // Define a purple color
  img.draw_text(100,100,"Hello World",purple); // Draw a purple "Hello world" at coordinates (100,100).
  img.display("My first CImg code");             // Display the image in a display window.
  return 0;
}
```

Which can be also written in a more compact way as :
```c++
#include "CImg.h"
using namespace cimg_library;
int main() {
  const unsigned char purple[] = { 255,0,255 };
  CImg<unsigned char>(640,400,1,3,0).draw_text(100,100,"Hello World",purple).display("My first CImg code");
  return 0;
}
```

Generally, you can write very small code that performs complex image processing tasks. The CImg Library is very simple to use and provides a lot of interesting algorithms for image manipulation.

## How to compile ?

The CImg library is a very light and user-friendly library : only standard system libraries are used. It avoids handling complex dependencies and problems with library compatibility. The only thing you need is a (quite modern) C++ compiler :
- Microsoft Visual Studio Community 2017 on Win7 : Just `#include "CImg.h"`
- Microsoft Visual C++ 6.0, Visual Studio.NET and Visual Express Edition : Use the project files and solution files provided in the CImg Library package (directory 'compilation/') to see how it works.
  
- Intel ICL compiler : Use the following command to compile a CImg-based program with ICL :
  ```c++
  icl /Ox hello_world.cpp user32.lib gdi32.lib
  ```
- g++ (MingW windows version) : Use the following command to compile a CImg-based program with g++, on Windows :
  ```c++
  g++ -o hello_word.exe hello_word.cpp -O2 -lgdi32
  ```
- g++ (Linux version) : Use the following command to compile a CImg-based program with g++, on Linux :
  ```c++
  g++ -o hello_word.exe hello_world.cpp -O2 -L/usr/X11R6/lib -lm -lpthread -lX11
  ```
- g++ (Solaris version) : Use the following command to compile a CImg-based program with g++, on Solaris :
  ```c++
  g++ -o hello_word.exe hello_world.cpp -O2 -lm -lpthread -R/usr/X11R6/lib -lrt -lnsl -lsocket
  ```
- g++ (Mac OS X version) : Use the following command to compile a CImg-based program with g++, on Mac OS X :
  ```c++
  g++ -o hello_word.exe hello_world.cpp -O2 -lm -lpthread -I/usr/X11R6/include -L/usr/X11R6/lib -lm -lpthread -lX11
  ```
- Dev-Cpp : Use the project file provided in the CImg library package to see how it works.

If you are using other compilers and encounter problems, please write me since maintaining compatibility is one of the priorities of the CImg Library. Nevertheless, old compilers that do not respect the C++ standard will not support the CImg Library.

## What's next ?

If you are ready to get more, and to start writing more serious programs with CImg, you are invited to go to the Tutorial : Getting Started. section.
