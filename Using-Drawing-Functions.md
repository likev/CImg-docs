# Using Drawing Functions.

This section tells more about drawing features in CImg images. Drawing functions list can be found in the CImg functions list (section Drawing Functions), and are all defined on a common basis. Here are the important points to understand before using drawing functions :

- Drawing is performed on the instance image. Drawing functions parameters are defined as const variables and return a reference to the current instance (*this), so that drawing functions can be pipelined (see examples below). Drawing is usually done in 2D color images but can be performed in 3D images with any vector-valued dimension, and with any possible pixel type.
- A color parameter is always needed to draw features in an image. The color must be defined as a C-style array whose dimension is at least spectrum()
