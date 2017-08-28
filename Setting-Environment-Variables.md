# Setting Environment Variables

The CImg library is a multiplatform library, working on a wide variety of systems. This implies the existence of 
some environment variables that must be correctly defined depending on your current system. Most of the time, 
the CImg Library defines these variables **automatically** (for popular systems). Anyway, if your system is not recognized, 
you will have to set the environment variables by hand. Here is a quick explanations of environment variables.
Setting the environment variables is done with the `#define` keyword. 
**This setting must be done before including the file CImg.h** in your source code. 

For instance, defining the environment variable cimg_display would be done like this :
```cpp
#define cimg_display 0
#include "CImg.h"
...
```

Here are the different environment variables used by the CImg Library :

- `cimg_OS` : This variable defines the type of your Operating System. It can be set to 1 (Unix), 2 (Windows), or 0 (Other configuration). It should be actually auto-detected by the CImg library. If this is not the case (cimg_OS=0), you will probably have to tune the environment variables described below.
- `cimg_display` : This variable defines the type of graphical library used to display images in windows. It can be set to 0 (no display library available), 1 (X11-based display) or 2 (Windows-GDI display). If you are running on a system without X11 or Windows-GDI ability, please set this variable to 0. This will disable the display support, since the CImg Library doesn't contain the necessary code to display images on systems other than X11 or Windows GDI.
- `cimg_use_vt100` : This variable tells the library if the system terminal has VT100 color capabilities. It can be defined or not defined. Define this variable to get colored output on your terminal, when using the CImg Library.
- `cimg_verbosity` : This variable defines the level of run-time debug messages that will be displayed by the CImg Library. It can be set to 0 (no debug messages), 1 (normal debug messages displayed on standard error), 2 (normal debug messages displayed in modal windows, which is the default value), or 3 (high debug messages). Note that setting this value to 3 may slow down your program since more debug tests are made by the library (particularly to check if pixel access is made outside image boundaries). See also CImgException to better understand how debug messages are working.
- `cimg_plugin` : This variable tells the library to use a plugin file to add features to the CImg<T> class. Define it with the path of your plugin file, if you want to add member functions to the CImg<T> class, without having to modify directly the "<tt>CImg.h</tt>" file. An include of the plugin file is performed in the CImg<T> class. If cimg_plugin if not specified (default), no include is done.
- `cimglist_plugin` : Same as cimg_plugin, but to add features to the CImgList<T> class.
- `cimgdisplay_plugin` : Same as cimg_plugin, but to add features to the CImgDisplay<T> class.

All these compilation variables can be checked, using the function `cimg_library::cimg::info()`, which displays a list of the different configuration variables and their values on the standard error output. 

```
 CImg Library 2.0.3, compiled Aug 28 2017 ( 15:19:43 ) with the following flags:


  > Operating System:       Windows       ('cimg_OS'=2)
  > CPU endianness:         Little Endian
  > Verbosity mode:         Dialog        ('cimg_verbosity'=2)
  > Stricts warnings:       No            ('cimg_strict_warnings' undefined)
  > Support for C++11:      No            ('cimg_use_cpp11'=0)
  > Using VT100 messages:   No            ('cimg_use_vt100' undefined)
  > Display type:           Windows GDI   ('cimg_display'=2)
  > Using OpenMP:           No            ('cimg_use_openmp' undefined)
  > Using PNG library:      No            ('cimg_use_png' undefined)
  > Using JPEG library:     No            ('cimg_use_jpeg' undefined)
  > Using TIFF library:     No            ('cimg_use_tiff' undefined)
  > Using Magick++ library: No            ('cimg_use_magick' undefined)
  > Using FFTW3 library:    No            ('cimg_use_fftw3' undefined)
  > Using LAPACK library:   No            ('cimg_use_lapack' undefined)
  > Path of ImageMagick:    ".\convert.exe"
  > Path of GraphicsMagick: "gm.exe"
  > Path of 'medcon':       "medcon.exe"
  > Temporary path:         "C:\Users\work\AppData\Local\Temp"
```
