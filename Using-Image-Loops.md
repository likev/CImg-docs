# Using Image Loops.

The CImg Library provides different macros that define useful iterative loops over an image. Basically, it can be used to replace one or several for(..) instructions, but it also proposes interesting extensions to classical loops. Below is a list of all existing loop macros, classified in four different categories :

- Loops over the pixel buffer
- Loops over image dimensions
- Loops over interior regions and borders.
- Loops using neighborhoods.

## Loops over the pixel buffer

Loops over the pixel buffer are really basic loops that iterate a pointer on the pixel data buffer of a `cimg_library::CImg` image. Two macros are defined for this purpose :

- `cimg_for(img,ptr,T)` : This macro loops over the pixel data buffer of the image img, using a pointer T* ptr, starting from the beginning of the buffer (first pixel) till the end of the buffer (last pixel).
  - img must be a (non empty) `cimg_library::CImg` image of pixels T.
  - ptr is a pointer of type T*. This kind of loop should not appear a lot in your own source code, since this is a low-level loop and many functions of the CImg class may be used instead. Here is an example of use :
        
  ```c++
  CImg<float> img(320,200);
  cimg_for(img,ptr,float) { *ptr=0; }        // Equivalent to 'img.fill(0);'
  ```
- `cimg_rof(img,ptr,T)` : This macro does the same as cimg_for() but from the end to the beginning of the pixel buffer.
- `cimg_foroff(img,off)` : This macro loops over the pixel data buffer of the image img, using an offset , starting from the beginning of the buffer (first pixel, off=0) till the end of the buffer (last pixel value, off = img.size()-1).
  - img must be a (non empty) `cimg_library::CImg<T>` image of pixels T.
  - off is an inner-loop variable, only defined inside the scope of the loop.

Here is an example of use :

```c++
CImg<float> img(320,200);
cimg_foroff(img,off) { img[off]=0; }    // Equivalent to 'img.fill(0);'
```

## Loops over image dimensions

The following loops are probably the most used loops in image processing programs. They allow to loop over the image along one or several dimensions, along a raster scan course. Here is the list of such loop macros for a single dimension :

- `cimg_forX(img,x)` : equivalent to : for (int x = 0; x<img.width(); ++x).
- `cimg_forY(img,y)` : equivalent to : for (int y = 0; y<img.height(); ++y).
- `cimg_forZ(img,z)` : equivalent to : for (int z = 0; z<img.depth(); ++z).
- `cimg_forC(img,c)` : equivalent to : for (int c = 0; c<img.spectrum(); ++c).

Combinations of these macros are also defined as other loop macros, allowing to loop directly over 2D, 3D or 4D images :

- `cimg_forXY(img,x,y)` : equivalent to : cimg_forY(img,y) cimg_forX(img,x).
- `cimg_forXZ(img,x,z)` : equivalent to : cimg_forZ(img,z) cimg_forX(img,x).
- `cimg_forYZ(img,y,z)` : equivalent to : cimg_forZ(img,z) cimg_forY(img,y).
- `cimg_forXC(img,x,c)` : equivalent to : cimg_forC(img,c) cimg_forX(img,x).
- `cimg_forYC(img,y,c)` : equivalent to : cimg_forC(img,c) cimg_forY(img,y).
- `cimg_forZC(img,z,c)` : equivalent to : cimg_forC(img,c) cimg_forZ(img,z).
- `cimg_forXYZ(img,x,y,z)` : equivalent to : cimg_forZ(img,z) cimg_forXY(img,x,y).
- `cimg_forXYC(img,x,y,c)` : equivalent to : cimg_forC(img,c) cimg_forXY(img,x,y).
- `cimg_forXZC(img,x,z,c)` : equivalent to : cimg_forC(img,c) cimg_forXZ(img,x,z).
- `cimg_forYZC(img,y,z,c)` : equivalent to : cimg_forC(img,c) cimg_forYZ(img,y,z).
- `cimg_forXYZC(img,x,y,z,c)` : equivalent to : cimg_forC(img,c) cimg_forXYZ(img,x,y,z).
    
For all these loops, x,y,z and v are inner-defined variables only visible inside the scope of the loop. They don't have to be defined before the call of the macro.
img must be a (non empty) `cimg_library::CImg` image.

Here is an example of use that creates an image with a smooth color gradient :

```c++
CImg<unsigned char> img(256,256,1,3);       // Define a 256x256 color image
cimg_forXYC(img,x,y,c) { img(x,y,c) = (x+y)*(c+1)/6; }
img.display("Color gradient");
```
## Loops over interior regions and borders.

Similar macros are also defined to loop only on the border of an image, or inside the image (excluding the border). The border may be several pixel wide :

- `cimg_for_insideX(img,x,n)` : Loop along the x-axis, except for pixels inside a border of n pixels wide.
- `cimg_for_insideY(img,y,n)` : Loop along the y-axis, except for pixels inside a border of n pixels wide.
- `cimg_for_insideZ(img,z,n)` : Loop along the z-axis, except for pixels inside a border of n pixels wide.
- `cimg_for_insideC(img,c,n)` : Loop along the c-axis, except for pixels inside a border of n pixels wide.
- `cimg_for_insideXY(img,x,y,n)` : Loop along the (x,y)-axes, excepted for pixels inside a border of n pixels wide.
- `cimg_for_insideXYZ(img,x,y,z,n)` : Loop along the (x,y,z)-axes, excepted for pixels inside a border of n pixels wide.

And also :

- `cimg_for_borderX(img,x,n)` : Loop along the x-axis, only for pixels inside a border of n pixels wide.
- `cimg_for_borderY(img,y,n)` : Loop along the y-axis, only for pixels inside a border of n pixels wide.
- `cimg_for_borderZ(img,z,n)` : Loop along the z-axis, only for pixels inside a border of n pixels wide.
- `cimg_for_borderC(img,c,n)` : Loop along the c-axis, only for pixels inside a border of n pixels wide.
- `cimg_for_borderXY(img,x,y,n)` : Loop along the (x,y)-axes, only for pixels inside a border of n pixels wide.
- `cimg_for_borderXYZ(img,x,y,z,n)` : Loop along the (x,y,z)-axes, only for pixels inside a border of n pixels wide.
    For all these loops, x,y,z and c are inner-defined variables only visible inside the scope of the loop. They don't have to be defined before the call of the macro.
    img must be a (non empty) cimg_library::CImg image.
    The constant n stands for the size of the border.

Here is an example of use, to create a 2d grayscale image with two different intensity gradients :

```c++
CImg<> img(256,256);
cimg_for_insideXY(img,x,y,50) img(x,y) = x+y;
cimg_for_borderXY(img,x,y,50) img(x,y) = x-y;
img.display();
```

## Loops using neighborhoods.

Inside an image loop, it is often useful to get values of neighborhood pixels of the current pixel at the loop location. The CImg Library provides a very smart and fast mechanism for this purpose, with the definition of several loop macros that remember the neighborhood values of the pixels. The use of these macros can highly optimize your code, and also simplify your program.

### Neighborhood-based loops for 2D images

For 2D images, the neighborhood-based loop macros are :

- `cimg_for2x2(img,x,y,z,c,I,T)` : Loop along the (x,y)-axes using a centered 2x2 neighborhood.
- `cimg_for3x3(img,x,y,z,c,I,T)` : Loop along the (x,y)-axes using a centered 3x3 neighborhood.
- `cimg_for4x4(img,x,y,z,c,I,T)` : Loop along the (x,y)-axes using a centered 4x4 neighborhood.
- `cimg_for5x5(img,x,y,z,c,I,T)` : Loop along the (x,y)-axes using a centered 5x5 neighborhood.

For all these loops, x and y are inner-defined variables only visible inside the scope of the loop. They don't have to be defined before the call of the macro. img is a non empty CImg<T> image. z and c are constants that define on which image slice and vector channel the loop must apply (usually both 0 for grayscale 2D images). Finally, I is the 2x2, 3x3, 4x4 or 5x5 neighborhood of type T that will be updated with the correct pixel values during the loop (see Defining neighborhoods).

### Neighborhood-based loops for 3D images

For 3D images, the neighborhood-based loop macros are :

- `cimg_for2x2x2(img,x,y,z,c,I,T)` : Loop along the (x,y,z)-axes using a centered 2x2x2 neighborhood.
- `cimg_for3x3x3(img,x,y,z,c,I,T)` : Loop along the (x,y,z)-axes using a centered 3x3x3 neighborhood.

For all these loops, x, y and z are inner-defined variables only visible inside the scope of the loop. They don't have to be defined before the call of the macro. img is a non empty CImg<T> image. c is a constant that defines on which image channel the loop must apply (usually 0 for grayscale 3D images). Finally, I is the 2x2x2 or 3x3x3 neighborhood of type T that will be updated with the correct pixel values during the loop (see Defining neighborhoods).

### Defining neighborhoods

A neighborhood is defined as an instance of a class having operator[] defined. This particularly includes classical C-array, as well as CImg<T> objects.

For instance, a 3x3 neighborhood can be defined either as a 'float[9]' or a 'CImg<float>(3,3)' variable.

### Using alternate variable names

There are also some useful macros that can be used to define variables that reference the neighborhood elements. There are :

- `CImg_2x2(I,type)` : Define a 2x2 neighborhood named I, of type type.
- `CImg_3x3(I,type)` : Define a 3x3 neighborhood named I, of type type.
- `CImg_4x4(I,type)` : Define a 4x4 neighborhood named I, of type type.
- `CImg_5x5(I,type)` : Define a 5x5 neighborhood named I, of type type.
- `CImg_2x2x2(I,type)` : Define a 2x2x2 neighborhood named I, of type type.
- `CImg_3x3x3(I,type)` : Define a 3x3x3 neighborhood named I, of type type.

Actually, I is a generic name for the neighborhood. In fact, these macros declare a set of new variables. For instance, defining a 3x3 neighborhood CImg_3x3(I,float) declares 9 different float variables Ipp,Icp,Inp,Ipc,Icc,Inc,Ipn,Icn,Inn which correspond to each pixel value of a 3x3 neighborhood. Variable indices are p,c or n, and stand respectively for 'previous', 'current' and 'next'. First indice denotes the x-axis, second indice denotes the y-axis. Then, the names of the variables are directly related to the position of the corresponding pixels in the neighborhood. For 3D neighborhoods, a third indice denotes the z-axis. Then, inside a neighborhood loop, you will have the following equivalence :

- `Ipp = img(x-1,y-1)`
- `Icn = img(x,y+1)`
- `Inp = img(x+1,y-1)`
- `Inpc = img(x+1,y-1,z)`
- `Ippn = img(x-1,y-1,z+1)`
    and so on...

For bigger neighborhoods, such as 4x4 or 5x5 neighborhoods, two additionnal indices are introduced : a (stands for 'after') and b (stands for 'before'), so that :

- `Ibb = img(x-2,y-2)`
- `Ina = img(x+1,y+2)`
    and so on...

The value of a neighborhood pixel outside the image range (image border problem) is automatically set to the same values as the nearest valid pixel in the image (this is also called the Neumann border condition).

### Example codes

More than a long discussion, the above example will demonstrate how to compute the gradient norm of a 3D volume using the cimg_for3x3x3() loop macro :

```c++
CImg<float> volume("IRM.hdr");    // Load an IRM volume from an Analyze7.5 file
CImg_3x3x3(I,float);                      // Define a 3x3x3 neighborhood
CImg<float> gradnorm(volume);     // Create an image with same size as 'volume'
cimg_for3x3x3(volume,x,y,z,0,I,float) { // Loop over the volume, using the neighborhood I
  const float ix = 0.5f*(Incc-Ipcc);      // Compute the derivative along the x-axis.
  const float iy = 0.5f*(Icnc-Icpc);      // Compute the derivative along the y-axis.
  const float iz = 0.5f*(Iccn-Iccp);      // Compute the derivative along the z-axis.
  gradnorm(x,y,z) = std::sqrt(ix*ix+iy*iy+iz*iz);  // Set the gradient norm in the destination image
}
gradnorm.display("Gradient norm");
```

And the following example shows how to deal with neighborhood references to blur a color image by averaging pixel values on a 5x5 neighborhood.

```c++
CImg<unsigned char> src("image_color.jpg"), dest(src,false);  // Image definitions.
typedef unsigned char uchar;               // Avoid space in the second parameter 
                                           //of the macro CImg_5x5x1 below.
CImg<> N(5,5);                     // Define a 5x5 neighborhood as a 5x5 image.
cimg_forC(src,k)                           // Standard loop on color channels
   cimg_for5x5(src,x,y,0,k,N,float)        // 5x5 neighborhood loop.
     dest(x,y,k) = N.sum()/(5*5);        // Averaging pixels to filter the color image.
CImgList<unsigned char> visu(src,dest);
visu.display("Original + Filtered");       // Display both original and filtered image.
```

As you can see, explaining the use of the CImg neighborhood macros is actually more difficult than using them !
