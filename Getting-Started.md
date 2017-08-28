# Tutorial : Getting Started.

Let's start to write our first program to get the idea. This will demonstrate how to load and create images, as well as handle image display and mouse events. Assume we want to load a color image lena.jpg, smooth it, display it in a windows, and enter an event loop so that clicking a point in the image will draw the (R,G,B) intensity profiles of the corresponding image line (in another window). Yes, that sounds quite complex for a first code, but don't worry, it will be very simple using the CImg library ! Well, just look at the code below, it does the task :
```c++
#include "CImg.h"
using namespace cimg_library;
int main() {
  CImg<unsigned char> image("lena.jpg"), visu(500,400,1,3,0);

  const unsigned char red[] = { 255,0,0 }, green[] = { 0,255,0 }, blue[] = { 0,0,255 };
  
  image.blur(2.5);
  
  CImgDisplay main_disp(image,"Click a point"), draw_disp(visu,"Intensity profile");
  
  while (!main_disp.is_closed() && !draw_disp.is_closed()) {
    main_disp.wait();
    if (main_disp.button() && main_disp.mouse_y()>=0) {
      const int y = main_disp.mouse_y();
      visu.fill(0).draw_graph(image.get_crop(0,y,0,0,image.width()-1,y,0,0),red,1,1,0,255,0);
      visu.draw_graph(image.get_crop(0,y,0,1,image.width()-1,y,0,1),green,1,1,0,255,0);
      visu.draw_graph(image.get_crop(0,y,0,2,image.width()-1,y,0,2),blue,1,1,0,255,0).display(draw_disp);
      }
    }
  return 0;
}
```

Here is a screenshot of the resulting program :
![tutorial-screenshot.jpg](http://cimg.eu/img/tutorial.jpg)

And here is the detailled explanation of the source, line by line :
```c++
#include "CImg.h" 
```
Include the main and only header file of the CImg library.

```c++
using namespace cimg_library; 
```
Use the library namespace to ease the declarations afterward.

```c++
int main() { 
```
Definition of the main function.

```c++
CImg<unsigned char> image("lena.jpg"), visu(500,400,1,3,0); 
```
Creation of two instances of images of unsigned char pixels. The first image image is initialized by reading an image file from the disk. Here, lena.jpg must be in the same directory as the current program. Note that you must also have installed the ImageMagick package in order to be able to read JPG images. The second image visu is initialized as a black color image with dimension dx=500, dy=400, dz=1 (here, it is a 2D image, not a 3D one), and dv=3 (each pixel has 3 'vector' channels R,G,B). The last argument in the constructor defines the default value of the pixel values (here 0, which means that visu will be initially black).

```c++
const unsigned char red[] = { 255,0,0 }, green[] = { 0,255,0 }, blue[] = { 0,0,255 }; 
```
Definition of three different colors as array of unsigned char. This will be used to draw plots with different colors.

```c++
image.blur(2.5); 
```
Blur the image, with a gaussian blur and a standard variation of 2.5. Note that most of the CImg functions have two versions : one that acts in-place (which is the case of blur), and one that returns the result as a new image (the name of the function begins then with get_ ). In this case, one could have also written image = image.get_blur(2.5); (more expensive, since it needs an additional copy operation).

```c++
CImgDisplay main_disp(image,"Click a point"), draw_disp(visu,"Intensity profile"); 
```
Creation of two display windows, one for the input image image, and one for the image visu which will be display intensity profiles. By default, CImg displays handles events (mouse,keyboard,..). On Windows, there is a way to create fullscreen displays.

```c++
while (!main_disp.is_closed() && !draw_disp.is_closed()) { 
```
Enter the event loop, the code will exit when one of the two display windows is closed.

```c++
main_disp.wait(); 
```
Wait for an event (mouse, keyboard,..) in the display window main_disp.

```c++
if (main_disp.button() && main_disp.mouse_y()>=0) { 
```
Test if the mouse button has been clicked on the image area. One may distinguish between the 3 different mouse buttons, but in this case it is not necessary

```c++
const int y = main_disp.mouse_y(); 
```
Get the image line y-coordinate that has been clicked.

```c++
visu.fill(0).draw_graph(image.get_crop(0,y,0,0,image.width()-1,y,0,0),red,1,0,256,0); 
```
This line illustrates the pipeline property of most of the CImg class functions. The first function fill(0) simply sets all pixel values with 0 (i.e. clear the image visu). The interesting thing is that it returns a reference to visu and then, can be pipelined with the function draw_graph() which draws a plot in the image visu. The plot data are given by another image (the first argument of draw_graph()). In this case, the given image is the red-component of the line y of the original image, retrieved by the function get_crop() which returns a sub-image of the image image. Remember that images coordinates are 4D (x,y,z,c) and for color images, the R,G,B channels are respectively given by v=0, v=1 and v=2.

```c++
visu.draw_graph(image.get_crop(0,y,0,1,image.width()-1,y,0,1),green,1,0,256,0); 
```
Plot the intensity profile for the green channel of the clicked line.

```c++
visu.draw_graph(image.get_crop(0,y,0,2,image.width()-1,y,0,2),blue,1,0,256,0).display(draw_disp); 
```
Same thing for the blue channel. Note how the function (which return a reference to visu) is pipelined with the function display() that just paints the image visu in the corresponding display window.
...till the end 

I don't think you need more explanations !As you have noticed, the CImg library allows to write very small and intuitive code. Note also that this source will perfectly work on Unix and Windows systems. Take also a look to the examples provided in the CImg package ( directory examples/ ). It will show you how CImg-based code can be surprisingly small. Moreover, there is surely one example close to what you want to do. A good start will be to look at the file CImg_demo.cpp which contains small and various examples of what you can do with the CImg Library. All CImg classes are used in this source, and the code can be easily modified to see what happens. 
