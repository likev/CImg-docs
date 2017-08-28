# FAQ Summary

1. General information and availability
    1. What is the CImg Library ?
    2. What platforms are supported ?
    3. How is CImg distributed ?
    4. What kind of people are concerned by CImg ?
    5. What are the specificities of the CeCILL license ?
    6. Who is behind CImg ?
2. C++ related questions
    1. What is the level of C++ knowledge needed to use CImg ?
    2. How to use CImg in my own C++ program ?
    3. Why is CImg entirely contained in a single header file ?
3. Other resources
    1. Translations

## 1. General information and availability
### 1.1. What is the CImg Library ?

The CImg Library is an open-source C++ toolkit for image processing.
It mainly consists in a (big) single header file CImg.h providing a set of C++ classes and functions that can be used in your own sources, to load/save, manage/process and display generic images. It's actually a very simple and pleasant toolkit for coding image processing stuff in C++ : Just include the header file CImg.h, and you are ready to handle images in your C++ programs.
### 1.2. What platforms are supported ?

CImg has been designed with portability in mind. It is regularly tested on different architectures and compilers, and should also work on any decent OS having a decent C++ compiler. Before each release, the CImg Library is compiled under these different configurations :
- PC Linux 32 bits, with g++.
- PC Windows 32 bits, with Visual C++ 6.0.
- PC Windows 32 bits, with Visual C++ Express Edition.
- Sun SPARC Solaris 32 bits, with g++.
- Mac PPC with OS X and g++.

CImg has a minimal number of dependencies. In its minimal version, it can be compiled only with standard C++ headers. Anyway, it has interesting extension capabilities and can use external libraries to perform specific tasks more efficiently (Fourier Transform computation using FFTW for instance).
### 1.3. How is CImg distributed ?

The CImg Library is freely distributed as a complete .zip compressed package, hosted at the CImg server.
The package is distributed under the CeCILL license.

This package contains :

- The main library file CImg.h (C++ header file).
- Several C++ source code showing examples of using CImg.
- A complete library documentation, in HTML and PDF formats.
- Additional library plug-ins that can be used to extend library capabilities for specific uses.

The CImg Library is a quite lightweight library which is easy to maintain (due to its particular structure), and thus has a fast rythm of release. A new version of the CImg package is released approximately every three months.
### 1.4. What kind of people are concerned by CImg ?

The CImg library is an image processing library, primarily intended for computer scientists or students working in the fields of image processing or computer vision, and knowing bases of C++. As the library is handy and really easy to use, it can be also used by any programmer needing occasional tools for dealing with images in C++, since there are no standard library yet for this purpose.
### 1.5. What are the specificities of the CeCILL license ?

  The CeCILL license governs the use of the CImg Library. This is an open-source license which gives you rights to access, use, modify and redistribute the source code, under certains conditions. There are two different variants of the CeCILL license used in CImg (namely CeCILL and CeCILL-C, all open-source), corresponding to different constraints on the source files :

- The CeCILL-C license is the most permissive one, close to the GNU LGPL license, and applies only on the main library file CImg.h. Basically, this license allows to use CImg.h in a closed-source product without forcing you to redistribute the entire software source code. Anyway, if one modifies the CImg.h source file, one has to redistribute the modified version of the file that must be governed by the same CeCILL-C license.
  
- The CeCILL license applies to all other files (source examples, plug-ins and documentation) of the CImg Library package, and is close (even compatible) with the GNU GPL license. It does not allow the use of these files in closed-source products.

You are invited to read the complete descriptions of the the CeCILL-C and CeCILL licenses before releasing a software based on the CImg Library.
### 1.6. Who is behind CImg ?

CImg has been started by David Tschumperle at the beginning of his PhD thesis, in October 1999. He is still the main coordinator of the project. Since the first release, a growing number of contributors has appeared. Due to the very simple and compact form of the library, submitting a contribution is quite easy and can be fastly integrated into the supported releases. List of contributors can be found on the front page.
## 2. C++ related questions
### 2.1 What is the level of C++ knowledge needed to use CImg ?

  The CImg Library has been designed using C++ templates and object-oriented programming techniques, but in a very accessible level. There are only public classes without any derivation (just like C structures) and there is at most one template parameter for each CImg class (defining the pixel type of the images). The design is simple but clean, making the library accessible even for non professional C++ programmers, while proposing strong extension capabilities for C++ experts.
### 2.2 How to use CImg in my own C++ program ?

Basically, you need to add these two lines in your C++ source code, in order to be able to work with CImg images :
```c++
#include "CImg.h"
using namespace cimg_library;
```
### 2.3 Why is CImg entirely contained in a single header file ?

  People are often surprised to see that the complete code of the library is contained in a single (big) C++ header file CImg.h. There are good practical and technical reasons to do that. Some arguments are listed below to justify this approach, so (I hope) you won't think this is a awkwardly C++ design of the CImg library :

- First, the library is based on template datatypes (images with generic pixel type), meaning that the programmer is free to decide what type of image he instanciates in his code. Even if there are roughly a limited number of fully supported types (basically, the "atomic" types of C++ : unsigned char, int, float, ...), this is not imaginable to pre-compile the library classes and functions for all possible atomic datatypes, since many functions and methods can have two or three arguments having different template parameters. This really means a huge number of possible combinations. The size of the object binary file generated to cover all possible cases would be just colossal. Is the STL library a pre-compiled one ? No, CImg neither. CImg is not using a classical .cpp and .h mechanism, just like the STL. Architectures of C++ template-based libraries are somewhat special in this sense. This is a proven technical fact.

- Second, why CImg does not have several header files, just like the STL does (one for each class for instance) ? This would be possible of course. There are only 4 classes in CImg, the two most important being CImg<T> and CImgList<T> representing respectively an image and a collection of images. But contrary to the STL library, these two CImg classes are strongly inter-dependent. All CImg algorithms are actually not defined as separate functions acting on containers (as the STL does with his header <algorithm>), but are directly methods of the image and image collection classes. This inter-dependence practically means that you will undoubtly need these two main classes at the same time if you are using CImg. If they were defined in separate header files, you would be forced to include both of them. What is the gain then ? No gain.
    
  Concerning the two other classes : You can disable the third most important class CImgDisplay of the CImg library, by setting the compilation macro cimg_display to 0, avoiding thus to compile this class if you don't use display capabilities of CImg in your code. But to be honest, this is a quite small class and doing this doesn't save much compilation time. The last and fourth class is CImgException, which is only few lines long and is obviously required in almost all methods of CImg. Including this one is mandatory.
    
  As a consequence, having a single header file instead of several ones is just a way for you to avoid including all of them, without any consequences on compilation time. This is both good technical and practical reasons to do like this.
    
- Third, having a single header file has plenty of advantages : Simplicity for the user, and for the developers (maintenance is in fact easier). Look at the CImg.h file, it looks like a mess at a first glance, but it is in fact very well organized and structured. Finding pieces of code in CImg functions or methods is particularly easy and fast. Also, how about the fact that library installation problems just disappear ? Just bring CImg.h with you, put it in your source directory, and the library is ready to go !


I admit the compilation time of CImg-based programs can be sometime long, but don't think that it is due to the fact that you are using a single header file. Using several header files wouldn't arrange anything since you would need all of them. Having a pre-compiled library object would be the only solution to speed up compilation time, but it is not possible at all, due to the too much generic nature of the library.
## 3. Other resources
### 3.1 Translations

This FAQ has been translated to Serbo-Croatian language by Web Geeks . 
