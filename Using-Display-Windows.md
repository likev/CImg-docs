# Using Display Windows

When opening a display window, you can choose the way the pixel values will be normalized before being displayed on the screen. Screen displays only support color values between [0,255], and someWhen displaying an image into the display window using CImgDisplay::display(), values of the image pixels can be eventually linearly normalized between [0,255] for visualization purposes. This may be useful for instance when displaying CImg<double> images with pixel values between [0,1]. The normalization behavior depends on the value of normalize which can be either 0,1 or 2 :

- 0 : No pixel normalization is performed when displaying an image. This is the fastest process, but you must be sure your displayed image have pixel values inside the range [0,255].
- 1 : Pixel value normalization is done for each new image display. Image pixels are not modified themselves, only displayed pixels are normalized.
- 2 : Pixel value normalization is done for the first image display, then the normalization parameters are kept and used for all the next image displays.
