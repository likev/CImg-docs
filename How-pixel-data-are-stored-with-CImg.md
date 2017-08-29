# How pixel data are stored with CImg.

## Source Code of CImg data members
```c++
  template<typename T>
  struct CImg {

    unsigned int _width, _height, _depth, _spectrum;
    bool _is_shared;
    T *_data;
    
    explicit CImg(const unsigned int size_x, const unsigned int size_y=1,
                  const unsigned int size_z=1, const unsigned int size_c=1):
      _is_shared(false) {
      size_t siz = (size_t)size_x*size_y*size_z*size_c;
      if (siz) {
        _width = size_x; _height = size_y; _depth = size_z; _spectrum = size_c;
        try { 
           //allocate siz counts of type T
            _data = new T[siz]; 
        
        } catch (...) {
          _width = _height = _depth = _spectrum = 0; _data = 0;
          throw CImgInstanceException(_cimg_instance
                                      "CImg(): Failed to allocate memory (%s) for image (%u,%u,%u,%u).",
                                      cimg_instance,
                                      cimg::strbuffersize(sizeof(T)*size_x*size_y*size_z*size_c),
                                      size_x,size_y,size_z,size_c);
        }
      } else { _width = _height = _depth = _spectrum = 0; _data = 0; }
    }
    
    ...
    
    ~CImg() {
      if (!_is_shared) delete[] _data;
    }
```

## Details
First, CImg<T> are very basic structures, which means that there are no memory tricks, weird memory alignments or 
disk caches used to store pixel data of images. When an image is instanced, all its pixel values are stored in memory 
at the same time (yes, you should avoid working with huge images when dealing with CImg, if you have only 64kb of RAM).
**A CImg<T> is basically a 4th-dimensional array (width,height,depth,dim)**, and its pixel data are stored linearly in a 
single memory buffer of general size (width*height*depth*dim). Nothing more, nothing less. 

The address of this memory buffer can be retrieved by the function CImg<T>::data(). As each image value is stored 
as a type T (T being known by the programmer of course), this pointer is a `T*`, or a `const T*` if your image is `const`. 
so, `T *ptr = img.data()` gives you the pointer to the first value of the image 'img'. The overall size of the used memory 
for one instance image (in bytes) is then `width*height*depth*dim*sizeof(T)`.Now, the ordering of the pixel values in this buffer
follows these rules : The values are not interleaved, and are ordered first along the X,Y,Z and V axis respectively 
(corresponding to the width,height,depth,dim dimensions), starting from the upper-left pixel to the bottom-right 
pixel of the instane image, with a classical scanline run.
  
So, a color image with dim=3 and depth=1, will be stored in memory as :R1R2R3R4R5R6......G1G2G3G4G5G6.......B1B2B3B4B5B6.... (i.e following a 'planar' structure)
and not as R1G1B1R2G2B2R3G3B3... (interleaved channels), where R1 = img(0,0,0,0) is the first upper-left pixel of 
the red component of the image, R2 is img(1,0,0,0), G1 = img(0,0,0,1), G2 = img(1,0,0,1), B1 = img(0,0,0,2), and so on...

Another example, a (1x5x1x1) CImg<T> (column vector A) will be stored as : A1A2A3A4A5 where A1 = img(0,0), A2 = img(0,1), ... , 
A5 = img(0,4).As you see, it is very simple and intuitive : no interleaving, no padding, just simple. This is cool not only 
because it is simple, but this has in fact a number of interesting properties. For instance, a 2D color image is stored in memory 
exactly as a 3D scalar image having a depth=3, meaning that **when you are dealing with 2D color images, you can write `img(x,y,k)` 
instead of `img(x,y,0,k)` to access the kth channel of the (x,y) pixel**. More generally, if you have one dimension that is 1 
in your image, you can just skip it in the call to the operator(). Similarly, values of a column vector stored as an image 
with width=depth=spectrum=1 can be accessed by `img(y)` instead of `img(0,y)`. This is very convenient.

Another cool thing is that it allows you to work easily with 'shared' images. A shared image is a CImg<T> instance that shares its memory 
with another one (the 'base' image). Destroying a shared image does nothing in fact. Shared images is a convenient way 
of modifying only portions (consecutive in memory) of an image. For instance, if 'img' is a 2D color image, you can 
write :
```c++
img.get_shared_channel(0).blur(2); 
img.get_shared_channels(1,2).mirror('x');
```
which just blur the red channel of the image, and mirror the two others along the X-axis. This is possible since channels of an image are not interleaved 
but are stored as different consecutive planes in memory, so you see that constructing a shared image is possible (and trivial). 


## Image channels
```c++
// Return the number of image channels, i.e. the image dimension along the C-axis.

int spectrum() const {
  return (int)_spectrum;
}
```
- The spectrum() of an empty image is equal to 0.
- spectrum() is typically equal to 1 when considering scalar-valued images, to 3
 for RGB-coded color images, and to 4 for RGBA-coded color images (with alpha-channel).
 The number of channels of an image instance is not limited. The meaning of the pixel 
 values is not linked up to the number of channels (e.g. a 4-channel image may indifferently 
 stands for a RGBA or CMYK color image).
- spectrum() returns an int, although the image spectrum is internally stored as an unsigned int.
 Using an int is safer and prevents arithmetic traps possibly encountered when 
 doing calculations involving unsigned int variables.
 Access to the initial unsigned int variable is possible (though not recommended) by `(*this)._spectrum`.
 
 
## Source Code of CImg access function members
```c++
    int width() const {
      return (int)_width;
    }
    
    int height() const {
      return (int)_height;
    }

    int depth() const {
      return (int)_depth;
    }
    
    int spectrum() const {
      return (int)_spectrum;
    }
    
    ulongT size() const {
      return (ulongT)_width*_height*_depth*_spectrum;
    }
    
    T* data() {
      return _data;
    }

    T* data(const unsigned int x, const unsigned int y=0, const unsigned int z=0, const unsigned int c=0) {
      return _data + x + (ulongT)y*_width + (ulongT)z*_width*_height + (ulongT)c*_width*_height*_depth;
    }
    
    longT offset(const int x, const int y=0, const int z=0, const int c=0) const {
      return x + (longT)y*_width + (longT)z*_width*_height + (longT)c*_width*_height*_depth;
    }
    
    iterator begin() {
      return _data;
    }

    iterator end() {
      return _data + size();
    }

    T& front() {
      return *_data;
    }

    T& back() {
      return *(_data + size() - 1);
    }
    
    //! Access to a pixel value at a specified offset, using Dirichlet boundary conditions.
    /**
       - Writing img.at(offset,out_value) is similar to img[offset] , except that if offset
         is outside bounds (e.g. offset<0 or offset>=img.size()), a reference to a value out_value
         is safely returned instead.
       - Due to the additional boundary checking operation, this method is slower than operator()(). 
         Use it when you are not sure about the validity of the specified pixel offset.
    **/
    T& at(const int offset, const T& out_value) {
      return (offset<0 || offset>=(int)size())?(cimg::temporary(out_value)=out_value):(*this)[offset];
    }
    
    T& _at(const int offset) {
      const unsigned int siz = (unsigned int)size();
      return (*this)[offset<0?0:(unsigned int)offset>=siz?siz - 1:offset];
    }
    
    T& atX(const int x, const int y, const int z, const int c, const T& out_value) {
      return (x<0 || x>=width())?(cimg::temporary(out_value)=out_value):(*this)(x,y,z,c);
    }
    T& atXY(const int x, const int y, const int z, const int c, const T& out_value) {
      return (x<0 || y<0 || x>=width() || y>=height()) 
      ? (cimg::temporary(out_value)=out_value)
      : (*this)(x,y,z,c);
    }
    T& atXYZ(const int x, const int y, const int z, const int c, const T& out_value) {
      return (x<0 || y<0 || z<0 || x>=width() || y>=height() || z>=depth())?
        (cimg::temporary(out_value)=out_value):(*this)(x,y,z,c);
    }
    T& atXYZC(const int x, const int y, const int z, const int c, const T& out_value) {
      return (x<0 || y<0 || z<0 || c<0 || x>=width() || y>=height() || z>=depth() || c>=spectrum())?
        (cimg::temporary(out_value)=out_value):(*this)(x,y,z,c);
    }
    
    T& operator()(const unsigned int x) {
      return _data[x];
    }
    
    T& operator()(const unsigned int x, const unsigned int y) {
      return _data[x + y*_width];
    }
    
   T& operator()(const unsigned int x, const unsigned int y, const unsigned int z) {
      return _data[x + y*(ulongT)_width + z*(ulongT)_width*_height];
   }
   
    T& operator()(const unsigned int x, const unsigned int y, const unsigned int z, 
                  const unsigned int c) {
      return _data[x + y*(ulongT)_width + z*(ulongT)_width*_height 
            + c*(ulongT)_width*_height*_depth];
    }
    T& operator()(const unsigned int x, const unsigned int y, const unsigned int z, 
                  const unsigned int, const ulongT wh) {
      return _data[x + y*_width + z*wh];
    }
    T& operator()(const unsigned int x, const unsigned int y, const unsigned int z, 
                  const unsigned int c, const ulongT wh, const ulongT whd) {
      return _data[x + y*_width + z*wh + c*whd];
    }
```
