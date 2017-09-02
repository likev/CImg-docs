#ifndef _CIMG_LIST
#define _CIMG_LIST


#include "classCImg03.h"

namespace cimg_library_suffixed {
/*
#-----------------------------------------
#
#
#
# Definition of the CImgList<T> structure
#
#
#
#------------------------------------------
*/
//! Represent a list of images CImg<T>.
template<typename T>
struct CImgList {
	unsigned int _width, _allocated_width;
	CImg<T> *_data;

	//! Simple iterator type, to loop through each image of a list.
	/**
	\note
	- The \c CImgList<T>::iterator type is defined as a <tt>CImg<T>*</tt>.
	- You may use it like this:
	\code
	CImgList<> list;   // Assuming this image list is not empty.
	for (CImgList<>::iterator it = list.begin(); it<list.end(); ++it) (*it).mirror('x');
	\endcode
	- Using the loop macro \c cimglist_for is another (more concise) alternative:
	\code
	cimglist_for(list,l) list[l].mirror('x');
	\endcode
	**/
	typedef CImg<T>* iterator;

	//! Simple const iterator type, to loop through each image of a \c const list instance.
	/**
	\note
	- The \c CImgList<T>::const_iterator type is defined to be a <tt>const CImg<T>*</tt>.
	- Similar to CImgList<T>::iterator, but for constant list instances.
	**/
	typedef const CImg<T>* const_iterator;

	//! Pixel value type.
	/**
	Refer to the pixels value type of the images in the list.
	\note
	- The \c CImgList<T>::value_type type of a \c CImgList<T> is defined to be a \c T.
	It is then similar to CImg<T>::value_type.
	- \c CImgList<T>::value_type is actually not used in %CImg methods. It has been mainly defined for
	compatibility with STL naming conventions.
	**/
	typedef T value_type;

	// Define common types related to template type T.
	typedef typename cimg::superset<T, bool>::type Tbool;
	typedef typename cimg::superset<T, unsigned char>::type Tuchar;
	typedef typename cimg::superset<T, char>::type Tchar;
	typedef typename cimg::superset<T, unsigned short>::type Tushort;
	typedef typename cimg::superset<T, short>::type Tshort;
	typedef typename cimg::superset<T, unsigned int>::type Tuint;
	typedef typename cimg::superset<T, int>::type Tint;
	typedef typename cimg::superset<T, cimg_ulong>::type Tulong;
	typedef typename cimg::superset<T, cimg_long>::type Tlong;
	typedef typename cimg::superset<T, float>::type Tfloat;
	typedef typename cimg::superset<T, double>::type Tdouble;
	typedef typename cimg::last<T, bool>::type boolT;
	typedef typename cimg::last<T, unsigned char>::type ucharT;
	typedef typename cimg::last<T, char>::type charT;
	typedef typename cimg::last<T, unsigned short>::type ushortT;
	typedef typename cimg::last<T, short>::type shortT;
	typedef typename cimg::last<T, unsigned int>::type uintT;
	typedef typename cimg::last<T, int>::type intT;
	typedef typename cimg::last<T, cimg_ulong>::type ulongT;
	typedef typename cimg::last<T, cimg_long>::type longT;
	typedef typename cimg::last<T, cimg_uint64>::type uint64T;
	typedef typename cimg::last<T, cimg_int64>::type int64T;
	typedef typename cimg::last<T, float>::type floatT;
	typedef typename cimg::last<T, double>::type doubleT;

	//@}
	//---------------------------
	//
	//! \name Plugins
	//@{
	//---------------------------
#ifdef cimglist_plugin
#include cimglist_plugin
#endif
#ifdef cimglist_plugin1
#include cimglist_plugin1
#endif
#ifdef cimglist_plugin2
#include cimglist_plugin2
#endif
#ifdef cimglist_plugin3
#include cimglist_plugin3
#endif
#ifdef cimglist_plugin4
#include cimglist_plugin4
#endif
#ifdef cimglist_plugin5
#include cimglist_plugin5
#endif
#ifdef cimglist_plugin6
#include cimglist_plugin6
#endif
#ifdef cimglist_plugin7
#include cimglist_plugin7
#endif
#ifdef cimglist_plugin8
#include cimglist_plugin8
#endif

	//@}
	//--------------------------------------------------------
	//
	//! \name Constructors / Destructor / Instance Management
	//@{
	//--------------------------------------------------------

	//! Destructor.
	/**
	Destroy current list instance.
	\note
	- Any allocated buffer is deallocated.
	- Destroying an empty list does nothing actually.
	**/
	~CImgList() {
		delete[] _data;
	}

	//! Default constructor.
	/**
	Construct a new empty list instance.
	\note
	- An empty list has no pixel data and its dimension width() is set to \c 0, as well as its
	image buffer pointer data().
	- An empty list may be reassigned afterwards, with the family of the assign() methods.
	In all cases, the type of pixels stays \c T.
	**/
	CImgList() :
		_width(0), _allocated_width(0), _data(0) {}

	//! Construct list containing empty images.
	/**
	\param n Number of empty images.
	\note Useful when you know by advance the number of images you want to manage, as
	it will allocate the right amount of memory for the list, without needs for reallocation
	(that may occur when starting from an empty list and inserting several images in it).
	**/
	explicit CImgList(const unsigned int n) :_width(n) {
		if (n) _data = new CImg<T>[_allocated_width = std::max(16U, (unsigned int)cimg::nearest_pow2(n))];
		else { _allocated_width = 0; _data = 0; }
	}

	//! Construct list containing images of specified size.
	/**
	\param n Number of images.
	\param width Width of images.
	\param height Height of images.
	\param depth Depth of images.
	\param spectrum Number of channels of images.
	\note Pixel values are not initialized and may probably contain garbage.
	**/
	CImgList(const unsigned int n, const unsigned int width, const unsigned int height = 1,
		const unsigned int depth = 1, const unsigned int spectrum = 1) :
		_width(0), _allocated_width(0), _data(0) {
		assign(n);
		cimglist_apply(*this, assign)(width, height, depth, spectrum);
	}

	//! Construct list containing images of specified size, and initialize pixel values.
	/**
	\param n Number of images.
	\param width Width of images.
	\param height Height of images.
	\param depth Depth of images.
	\param spectrum Number of channels of images.
	\param val Initialization value for images pixels.
	**/
	CImgList(const unsigned int n, const unsigned int width, const unsigned int height,
		const unsigned int depth, const unsigned int spectrum, const T& val) :
		_width(0), _allocated_width(0), _data(0) {
		assign(n);
		cimglist_apply(*this, assign)(width, height, depth, spectrum, val);
	}

	//! Construct list containing images of specified size, and initialize pixel values from a sequence of integers.
	/**
	\param n Number of images.
	\param width Width of images.
	\param height Height of images.
	\param depth Depth of images.
	\param spectrum Number of channels of images.
	\param val0 First value of the initializing integers sequence.
	\param val1 Second value of the initializing integers sequence.
	\warning You must specify at least <tt>width*height*depth*spectrum</tt> values in your argument list,
	or you will probably segfault.
	**/
	CImgList(const unsigned int n, const unsigned int width, const unsigned int height,
		const unsigned int depth, const unsigned int spectrum, const int val0, const int val1, ...) :
		_width(0), _allocated_width(0), _data(0) {
#define _CImgList_stdarg(t) { \
	assign(n,width,height,depth,spectrum); \
	const ulongT siz = (ulongT)width*height*depth*spectrum, nsiz = siz*n; \
	T *ptrd = _data->_data; \
	va_list ap; \
	va_start(ap,val1); \
	for (ulongT l = 0, s = 0, i = 0; i<nsiz; ++i) { \
	  *(ptrd++) = (T)(i==0?val0:(i==1?val1:va_arg(ap,t))); \
	  if ((++s)==siz) { ptrd = _data[++l]._data; s = 0; } \
	} \
	va_end(ap); \
      }
		_CImgList_stdarg(int);
	}

	//! Construct list containing images of specified size, and initialize pixel values from a sequence of doubles.
	/**
	\param n Number of images.
	\param width Width of images.
	\param height Height of images.
	\param depth Depth of images.
	\param spectrum Number of channels of images.
	\param val0 First value of the initializing doubles sequence.
	\param val1 Second value of the initializing doubles sequence.
	\warning You must specify at least <tt>width*height*depth*spectrum</tt> values in your argument list,
	or you will probably segfault.
	**/
	CImgList(const unsigned int n, const unsigned int width, const unsigned int height,
		const unsigned int depth, const unsigned int spectrum, const double val0, const double val1, ...) :
		_width(0), _allocated_width(0), _data(0) {
		_CImgList_stdarg(double);
	}

	//! Construct list containing copies of an input image.
	/**
	\param n Number of images.
	\param img Input image to copy in the constructed list.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of \c img.
	**/
	template<typename t>
	CImgList(const unsigned int n, const CImg<t>& img, const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(n);
		cimglist_apply(*this, assign)(img, is_shared);
	}

	//! Construct list from one image.
	/**
	\param img Input image to copy in the constructed list.
	\param is_shared Tells if the element of the list is a shared or non-shared copy of \c img.
	**/
	template<typename t>
	explicit CImgList(const CImg<t>& img, const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(1);
		_data[0].assign(img, is_shared);
	}

	//! Construct list from two images.
	/**
	\param img1 First input image to copy in the constructed list.
	\param img2 Second input image to copy in the constructed list.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of input images.
	**/
	template<typename t1, typename t2>
	CImgList(const CImg<t1>& img1, const CImg<t2>& img2, const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(2);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared);
	}

	//! Construct list from three images.
	/**
	\param img1 First input image to copy in the constructed list.
	\param img2 Second input image to copy in the constructed list.
	\param img3 Third input image to copy in the constructed list.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of input images.
	**/
	template<typename t1, typename t2, typename t3>
	CImgList(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(3);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
	}

	//! Construct list from four images.
	/**
	\param img1 First input image to copy in the constructed list.
	\param img2 Second input image to copy in the constructed list.
	\param img3 Third input image to copy in the constructed list.
	\param img4 Fourth input image to copy in the constructed list.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of input images.
	**/
	template<typename t1, typename t2, typename t3, typename t4>
	CImgList(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(4);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared);
	}

	//! Construct list from five images.
	/**
	\param img1 First input image to copy in the constructed list.
	\param img2 Second input image to copy in the constructed list.
	\param img3 Third input image to copy in the constructed list.
	\param img4 Fourth input image to copy in the constructed list.
	\param img5 Fifth input image to copy in the constructed list.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of input images.
	**/
	template<typename t1, typename t2, typename t3, typename t4, typename t5>
	CImgList(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const CImg<t5>& img5, const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(5);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared); _data[4].assign(img5, is_shared);
	}

	//! Construct list from six images.
	/**
	\param img1 First input image to copy in the constructed list.
	\param img2 Second input image to copy in the constructed list.
	\param img3 Third input image to copy in the constructed list.
	\param img4 Fourth input image to copy in the constructed list.
	\param img5 Fifth input image to copy in the constructed list.
	\param img6 Sixth input image to copy in the constructed list.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of input images.
	**/
	template<typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
	CImgList(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const CImg<t5>& img5, const CImg<t6>& img6, const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(6);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared); _data[4].assign(img5, is_shared); _data[5].assign(img6, is_shared);
	}

	//! Construct list from seven images.
	/**
	\param img1 First input image to copy in the constructed list.
	\param img2 Second input image to copy in the constructed list.
	\param img3 Third input image to copy in the constructed list.
	\param img4 Fourth input image to copy in the constructed list.
	\param img5 Fifth input image to copy in the constructed list.
	\param img6 Sixth input image to copy in the constructed list.
	\param img7 Seventh input image to copy in the constructed list.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of input images.
	**/
	template<typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
	CImgList(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const CImg<t5>& img5, const CImg<t6>& img6, const CImg<t7>& img7, const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(7);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared); _data[4].assign(img5, is_shared); _data[5].assign(img6, is_shared);
		_data[6].assign(img7, is_shared);
	}

	//! Construct list from eight images.
	/**
	\param img1 First input image to copy in the constructed list.
	\param img2 Second input image to copy in the constructed list.
	\param img3 Third input image to copy in the constructed list.
	\param img4 Fourth input image to copy in the constructed list.
	\param img5 Fifth input image to copy in the constructed list.
	\param img6 Sixth input image to copy in the constructed list.
	\param img7 Seventh input image to copy in the constructed list.
	\param img8 Eighth input image to copy in the constructed list.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of input images.
	**/
	template<typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
	CImgList(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const CImg<t5>& img5, const CImg<t6>& img6, const CImg<t7>& img7, const CImg<t8>& img8,
		const bool is_shared = false) :
		_width(0), _allocated_width(0), _data(0) {
		assign(8);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared); _data[4].assign(img5, is_shared); _data[5].assign(img6, is_shared);
		_data[6].assign(img7, is_shared); _data[7].assign(img8, is_shared);
	}

	//! Construct list copy.
	/**
	\param list Input list to copy.
	\note The shared state of each element of the constructed list is kept the same as in \c list.
	**/
	template<typename t>
	CImgList(const CImgList<t>& list) :_width(0), _allocated_width(0), _data(0) {
		assign(list._width);
		cimglist_for(*this, l) _data[l].assign(list[l], false);
	}

	//! Construct list copy \specialization.
	CImgList(const CImgList<T>& list) :_width(0), _allocated_width(0), _data(0) {
		assign(list._width);
		cimglist_for(*this, l) _data[l].assign(list[l], list[l]._is_shared);
	}

	//! Construct list copy, and force the shared state of the list elements.
	/**
	\param list Input list to copy.
	\param is_shared Tells if the elements of the list are shared or non-shared copies of input images.
	**/
	template<typename t>
	CImgList(const CImgList<t>& list, const bool is_shared) :_width(0), _allocated_width(0), _data(0) {
		assign(list._width);
		cimglist_for(*this, l) _data[l].assign(list[l], is_shared);
	}

	//! Construct list by reading the content of a file.
	/**
	\param filename Filename, as a C-string.
	**/
	explicit CImgList(const char *const filename) :_width(0), _allocated_width(0), _data(0) {
		assign(filename);
	}

	//! Construct list from the content of a display window.
	/**
	\param disp Display window to get content from.
	\note Constructed list contains a single image only.
	**/
	explicit CImgList(const CImgDisplay& disp) :_width(0), _allocated_width(0), _data(0) {
		assign(disp);
	}

	//! Return a list with elements being shared copies of images in the list instance.
	/**
	\note <tt>list2 = list1.get_shared()</tt> is equivalent to <tt>list2.assign(list1,true)</tt>.
	**/
	CImgList<T> get_shared() {
		CImgList<T> res(_width);
		cimglist_for(*this, l) res[l].assign(_data[l], true);
		return res;
	}

	//! Return a list with elements being shared copies of images in the list instance \const.
	const CImgList<T> get_shared() const {
		CImgList<T> res(_width);
		cimglist_for(*this, l) res[l].assign(_data[l], true);
		return res;
	}

	//! Destructor \inplace.
	/**
	\see CImgList().
	**/
	CImgList<T>& assign() {
		delete[] _data;
		_width = _allocated_width = 0;
		_data = 0;
		return *this;
	}

	//! Destructor \inplace.
	/**
	Equivalent to assign().
	\note Only here for compatibility with STL naming conventions.
	**/
	CImgList<T>& clear() {
		return assign();
	}

	//! Construct list containing empty images \inplace.
	/**
	\see CImgList(unsigned int).
	**/
	CImgList<T>& assign(const unsigned int n) {
		if (!n) return assign();
		if (_allocated_width<n || _allocated_width>(n << 2)) {
			delete[] _data;
			_data = new CImg<T>[_allocated_width = std::max(16U, (unsigned int)cimg::nearest_pow2(n))];
		}
		_width = n;
		return *this;
	}

	//! Construct list containing images of specified size \inplace.
	/**
	\see CImgList(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int).
	**/
	CImgList<T>& assign(const unsigned int n, const unsigned int width, const unsigned int height = 1,
		const unsigned int depth = 1, const unsigned int spectrum = 1) {
		assign(n);
		cimglist_apply(*this, assign)(width, height, depth, spectrum);
		return *this;
	}

	//! Construct list containing images of specified size, and initialize pixel values \inplace.
	/**
	\see CImgList(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, const T).
	**/
	CImgList<T>& assign(const unsigned int n, const unsigned int width, const unsigned int height,
		const unsigned int depth, const unsigned int spectrum, const T& val) {
		assign(n);
		cimglist_apply(*this, assign)(width, height, depth, spectrum, val);
		return *this;
	}

	//! Construct list with images of specified size, and initialize pixel values from a sequence of integers \inplace.
	/**
	\see CImgList(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, const int, const int, ...).
	**/
	CImgList<T>& assign(const unsigned int n, const unsigned int width, const unsigned int height,
		const unsigned int depth, const unsigned int spectrum, const int val0, const int val1, ...) {
		_CImgList_stdarg(int);
		return *this;
	}

	//! Construct list with images of specified size, and initialize pixel values from a sequence of doubles \inplace.
	/**
	\see CImgList(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,const double,const double,...).
	**/
	CImgList<T>& assign(const unsigned int n, const unsigned int width, const unsigned int height,
		const unsigned int depth, const unsigned int spectrum,
		const double val0, const double val1, ...) {
		_CImgList_stdarg(double);
		return *this;
	}

	//! Construct list containing copies of an input image \inplace.
	/**
	\see CImgList(unsigned int, const CImg<t>&, bool).
	**/
	template<typename t>
	CImgList<T>& assign(const unsigned int n, const CImg<t>& img, const bool is_shared = false) {
		assign(n);
		cimglist_apply(*this, assign)(img, is_shared);
		return *this;
	}

	//! Construct list from one image \inplace.
	/**
	\see CImgList(const CImg<t>&, bool).
	**/
	template<typename t>
	CImgList<T>& assign(const CImg<t>& img, const bool is_shared = false) {
		assign(1);
		_data[0].assign(img, is_shared);
		return *this;
	}

	//! Construct list from two images \inplace.
	/**
	\see CImgList(const CImg<t>&, const CImg<t>&, bool).
	**/
	template<typename t1, typename t2>
	CImgList<T>& assign(const CImg<t1>& img1, const CImg<t2>& img2, const bool is_shared = false) {
		assign(2);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared);
		return *this;
	}

	//! Construct list from three images \inplace.
	/**
	\see CImgList(const CImg<t>&, const CImg<t>&, const CImg<t>&, bool).
	**/
	template<typename t1, typename t2, typename t3>
	CImgList<T>& assign(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const bool is_shared = false) {
		assign(3);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		return *this;
	}

	//! Construct list from four images \inplace.
	/**
	\see CImgList(const CImg<t>&, const CImg<t>&, const CImg<t>&, const CImg<t>&, bool).
	**/
	template<typename t1, typename t2, typename t3, typename t4>
	CImgList<T>& assign(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const bool is_shared = false) {
		assign(4);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared);
		return *this;
	}

	//! Construct list from five images \inplace.
	/**
	\see CImgList(const CImg<t>&, const CImg<t>&, const CImg<t>&, const CImg<t>&, const CImg<t>&, bool).
	**/
	template<typename t1, typename t2, typename t3, typename t4, typename t5>
	CImgList<T>& assign(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const CImg<t5>& img5, const bool is_shared = false) {
		assign(5);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared); _data[4].assign(img5, is_shared);
		return *this;
	}

	//! Construct list from six images \inplace.
	/**
	\see CImgList(const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&, bool).
	**/
	template<typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
	CImgList<T>& assign(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const CImg<t5>& img5, const CImg<t6>& img6, const bool is_shared = false) {
		assign(6);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared); _data[4].assign(img5, is_shared); _data[5].assign(img6, is_shared);
		return *this;
	}

	//! Construct list from seven images \inplace.
	/**
	\see CImgList(const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&,
	const CImg<t>&, bool).
	**/
	template<typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
	CImgList<T>& assign(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const CImg<t5>& img5, const CImg<t6>& img6, const CImg<t7>& img7, const bool is_shared = false) {
		assign(7);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared); _data[4].assign(img5, is_shared); _data[5].assign(img6, is_shared);
		_data[6].assign(img7, is_shared);
		return *this;
	}

	//! Construct list from eight images \inplace.
	/**
	\see CImgList(const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&,const CImg<t>&,
	const CImg<t>&, const CImg<t>&, bool).
	**/
	template<typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
	CImgList<T>& assign(const CImg<t1>& img1, const CImg<t2>& img2, const CImg<t3>& img3, const CImg<t4>& img4,
		const CImg<t5>& img5, const CImg<t6>& img6, const CImg<t7>& img7, const CImg<t8>& img8,
		const bool is_shared = false) {
		assign(8);
		_data[0].assign(img1, is_shared); _data[1].assign(img2, is_shared); _data[2].assign(img3, is_shared);
		_data[3].assign(img4, is_shared); _data[4].assign(img5, is_shared); _data[5].assign(img6, is_shared);
		_data[6].assign(img7, is_shared); _data[7].assign(img8, is_shared);
		return *this;
	}

	//! Construct list as a copy of an existing list and force the shared state of the list elements \inplace.
	/**
	\see CImgList(const CImgList<t>&, bool is_shared).
	**/
	template<typename t>
	CImgList<T>& assign(const CImgList<t>& list, const bool is_shared = false) {
		cimg::unused(is_shared);
		assign(list._width);
		cimglist_for(*this, l) _data[l].assign(list[l], false);
		return *this;
	}

	//! Construct list as a copy of an existing list and force shared state of elements \inplace \specialization.
	CImgList<T>& assign(const CImgList<T>& list, const bool is_shared = false) {
		if (this == &list) return *this;
		CImgList<T> res(list._width);
		cimglist_for(res, l) res[l].assign(list[l], is_shared);
		return res.move_to(*this);
	}

	//! Construct list by reading the content of a file \inplace.
	/**
	\see CImgList(const char *const).
	**/
	CImgList<T>& assign(const char *const filename) {
		return load(filename);
	}

	//! Construct list from the content of a display window \inplace.
	/**
	\see CImgList(const CImgDisplay&).
	**/
	CImgList<T>& assign(const CImgDisplay &disp) {
		return assign(CImg<T>(disp));
	}

	//! Transfer the content of the list instance to another list.
	/**
	\param list Destination list.
	\note When returning, the current list instance is empty and the initial content of \c list is destroyed.
	**/
	template<typename t>
	CImgList<t>& move_to(CImgList<t>& list) {
		list.assign(_width);
		bool is_one_shared_element = false;
		cimglist_for(*this, l) is_one_shared_element |= _data[l]._is_shared;
		if (is_one_shared_element) cimglist_for(*this, l) list[l].assign(_data[l]);
		else cimglist_for(*this, l) _data[l].move_to(list[l]);
		assign();
		return list;
	}

	//! Transfer the content of the list instance at a specified position in another list.
	/**
	\param list Destination list.
	\param pos Index of the insertion in the list.
	\note When returning, the list instance is empty and the initial content of \c list is preserved
	(only images indexes may be modified).
	**/
	template<typename t>
	CImgList<t>& move_to(CImgList<t>& list, const unsigned int pos) {
		if (is_empty()) return list;
		const unsigned int npos = pos>list._width ? list._width : pos;
		list.insert(_width, npos);
		bool is_one_shared_element = false;
		cimglist_for(*this, l) is_one_shared_element |= _data[l]._is_shared;
		if (is_one_shared_element) cimglist_for(*this, l) list[npos + l].assign(_data[l]);
		else cimglist_for(*this, l) _data[l].move_to(list[npos + l]);
		assign();
		return list;
	}

	//! Swap all fields between two list instances.
	/**
	\param list List to swap fields with.
	\note Can be used to exchange the content of two lists in a fast way.
	**/
	CImgList<T>& swap(CImgList<T>& list) {
		cimg::swap(_width, list._width, _allocated_width, list._allocated_width);
		cimg::swap(_data, list._data);
		return list;
	}

	//! Return a reference to an empty list.
	/**
	\note Can be used to define default values in a function taking a CImgList<T> as an argument.
	\code
	void f(const CImgList<char>& list=CImgList<char>::empty());
	\endcode
	**/
	static CImgList<T>& empty() {
		static CImgList<T> _empty;
		return _empty.assign();
	}

	//! Return a reference to an empty list \const.
	static const CImgList<T>& const_empty() {
		static const CImgList<T> _empty;
		return _empty;
	}

	//@}
	//------------------------------------------
	//
	//! \name Overloaded Operators
	//@{
	//------------------------------------------

	//! Return a reference to one image element of the list.
	/**
	\param pos Indice of the image element.
	**/
	CImg<T>& operator()(const unsigned int pos) {
#if cimg_verbosity>=3
		if (pos >= _width) {
			cimg::warn(_cimglist_instance
				"operator(): Invalid image request, at position [%u].",
				cimglist_instance,
				pos);
			return *_data;
		}
#endif
		return _data[pos];
	}

	//! Return a reference to one image of the list.
	/**
	\param pos Indice of the image element.
	**/
	const CImg<T>& operator()(const unsigned int pos) const {
		return const_cast<CImgList<T>*>(this)->operator()(pos);
	}

	//! Return a reference to one pixel value of one image of the list.
	/**
	\param pos Indice of the image element.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\note <tt>list(n,x,y,z,c)</tt> is equivalent to <tt>list[n](x,y,z,c)</tt>.
	**/
	T& operator()(const unsigned int pos, const unsigned int x, const unsigned int y = 0,
		const unsigned int z = 0, const unsigned int c = 0) {
		return (*this)[pos](x, y, z, c);
	}

	//! Return a reference to one pixel value of one image of the list \const.
	const T& operator()(const unsigned int pos, const unsigned int x, const unsigned int y = 0,
		const unsigned int z = 0, const unsigned int c = 0) const {
		return (*this)[pos](x, y, z, c);
	}

	//! Return pointer to the first image of the list.
	/**
	\note Images in a list are stored as a buffer of \c CImg<T>.
	**/
	operator CImg<T>*() {
		return _data;
	}

	//! Return pointer to the first image of the list \const.
	operator const CImg<T>*() const {
		return _data;
	}

	//! Construct list from one image \inplace.
	/**
	\param img Input image to copy in the constructed list.
	\note <tt>list = img;</tt> is equivalent to <tt>list.assign(img);</tt>.
	**/
	template<typename t>
	CImgList<T>& operator=(const CImg<t>& img) {
		return assign(img);
	}

	//! Construct list from another list.
	/**
	\param list Input list to copy.
	\note <tt>list1 = list2</tt> is equivalent to <tt>list1.assign(list2);</tt>.
	**/
	template<typename t>
	CImgList<T>& operator=(const CImgList<t>& list) {
		return assign(list);
	}

	//! Construct list from another list \specialization.
	CImgList<T>& operator=(const CImgList<T>& list) {
		return assign(list);
	}

	//! Construct list by reading the content of a file \inplace.
	/**
	\see CImgList(const char *const).
	**/
	CImgList<T>& operator=(const char *const filename) {
		return assign(filename);
	}

	//! Construct list from the content of a display window \inplace.
	/**
	\see CImgList(const CImgDisplay&).
	**/
	CImgList<T>& operator=(const CImgDisplay& disp) {
		return assign(disp);
	}

	//! Return a non-shared copy of a list.
	/**
	\note <tt>+list</tt> is equivalent to <tt>CImgList<T>(list,false)</tt>.
	It forces the copy to have non-shared elements.
	**/
	CImgList<T> operator+() const {
		return CImgList<T>(*this, false);
	}

	//! Return a copy of the list instance, where image \c img has been inserted at the end.
	/**
	\param img Image inserted at the end of the instance copy.
	\note Define a convenient way to create temporary lists of images, as in the following code:
	\code
	(img1,img2,img3,img4).display("My four images");
	\endcode
	**/
	template<typename t>
	CImgList<T>& operator,(const CImg<t>& img) {
		return insert(img);
	}

	//! Return a copy of the list instance, where image \c img has been inserted at the end \const.
	template<typename t>
	CImgList<T> operator,(const CImg<t>& img) const {
		return (+*this).insert(img);
	}

	//! Return a copy of the list instance, where all elements of input list \c list have been inserted at the end.
	/**
	\param list List inserted at the end of the instance copy.
	**/
	template<typename t>
	CImgList<T>& operator,(const CImgList<t>& list) {
		return insert(list);
	}

	//! Return a copy of the list instance, where all elements of input \c list have been inserted at the end \const.
	template<typename t>
	CImgList<T>& operator,(const CImgList<t>& list) const {
		return (+*this).insert(list);
	}

	//! Return image corresponding to the appending of all images of the instance list along specified axis.
	/**
	\param axis Appending axis. Can be <tt>{ 'x' | 'y' | 'z' | 'c' }</tt>.
	\note <tt>list>'x'</tt> is equivalent to <tt>list.get_append('x')</tt>.
	**/
	CImg<T> operator>(const char axis) const {
		return get_append(axis, 0);
	}

	//! Return list corresponding to the splitting of all images of the instance list along specified axis.
	/**
	\param axis Axis used for image splitting.
	\note <tt>list<'x'</tt> is equivalent to <tt>list.get_split('x')</tt>.
	**/
	CImgList<T> operator<(const char axis) const {
		return get_split(axis);
	}

	//@}
	//-------------------------------------
	//
	//! \name Instance Characteristics
	//@{
	//-------------------------------------

	//! Return the type of image pixel values as a C string.
	/**
	Return a \c char* string containing the usual type name of the image pixel values
	(i.e. a stringified version of the template parameter \c T).
	\note
	- The returned string may contain spaces (as in \c "unsigned char").
	- If the pixel type \c T does not correspond to a registered type, the string <tt>"unknown"</tt> is returned.
	**/
	static const char* pixel_type() {
		return cimg::type<T>::string();
	}

	//! Return the size of the list, i.e. the number of images contained in it.
	/**
	\note Similar to size() but returns result as a (signed) integer.
	**/
	int width() const {
		return (int)_width;
	}

	//! Return the size of the list, i.e. the number of images contained in it.
	/**
	\note Similar to width() but returns result as an unsigned integer.
	**/
	unsigned int size() const {
		return _width;
	}

	//! Return pointer to the first image of the list.
	/**
	\note Images in a list are stored as a buffer of \c CImg<T>.
	**/
	CImg<T> *data() {
		return _data;
	}

	//! Return pointer to the first image of the list \const.
	const CImg<T> *data() const {
		return _data;
	}

	//! Return pointer to the pos-th image of the list.
	/**
	\param pos Indice of the image element to access.
	\note <tt>list.data(n);</tt> is equivalent to <tt>list.data + n;</tt>.
	**/
#if cimg_verbosity>=3
	CImg<T> *data(const unsigned int pos) {
		if (pos >= size())
			cimg::warn(_cimglist_instance
				"data(): Invalid pointer request, at position [%u].",
				cimglist_instance,
				pos);
		return _data + pos;
	}

	const CImg<T> *data(const unsigned int l) const {
		return const_cast<CImgList<T>*>(this)->data(l);
	}
#else
	CImg<T> *data(const unsigned int l) {
		return _data + l;
	}

	//! Return pointer to the pos-th image of the list \const.
	const CImg<T> *data(const unsigned int l) const {
		return _data + l;
	}
#endif

	//! Return iterator to the first image of the list.
	/**
	**/
	iterator begin() {
		return _data;
	}

	//! Return iterator to the first image of the list \const.
	const_iterator begin() const {
		return _data;
	}

	//! Return iterator to one position after the last image of the list.
	/**
	**/
	iterator end() {
		return _data + _width;
	}

	//! Return iterator to one position after the last image of the list \const.
	const_iterator end() const {
		return _data + _width;
	}

	//! Return reference to the first image of the list.
	/**
	**/
	CImg<T>& front() {
		return *_data;
	}

	//! Return reference to the first image of the list \const.
	const CImg<T>& front() const {
		return *_data;
	}

	//! Return a reference to the last image of the list.
	/**
	**/
	const CImg<T>& back() const {
		return *(_data + _width - 1);
	}

	//! Return a reference to the last image of the list \const.
	CImg<T>& back() {
		return *(_data + _width - 1);
	}

	//! Return pos-th image of the list.
	/**
	\param pos Indice of the image element to access.
	**/
	CImg<T>& at(const int pos) {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"at(): Empty instance.",
				cimglist_instance);

		return _data[cimg::cut(pos, 0, width() - 1)];
	}

	//! Access to pixel value with Dirichlet boundary conditions.
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\param out_value Default value returned if \c offset is outside image bounds.
	\note <tt>list.atNXYZC(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZC(x,y,z,c);</tt>.
	**/
	T& atNXYZC(const int pos, const int x, const int y, const int z, const int c, const T& out_value) {
		return (pos<0 || pos >= (int)_width) ? (cimg::temporary(out_value) = out_value) : _data[pos].atXYZC(x, y, z, c, out_value);
	}

	//! Access to pixel value with Dirichlet boundary conditions \const.
	T atNXYZC(const int pos, const int x, const int y, const int z, const int c, const T& out_value) const {
		return (pos<0 || pos >= (int)_width) ? out_value : _data[pos].atXYZC(x, y, z, c, out_value);
	}

	//! Access to pixel value with Neumann boundary conditions.
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\note <tt>list.atNXYZC(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZC(x,y,z,c);</tt>.
	**/
	T& atNXYZC(const int pos, const int x, const int y, const int z, const int c) {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atNXYZC(): Empty instance.",
				cimglist_instance);

		return _atNXYZC(pos, x, y, z, c);
	}

	//! Access to pixel value with Neumann boundary conditions \const.
	T atNXYZC(const int pos, const int x, const int y, const int z, const int c) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atNXYZC(): Empty instance.",
				cimglist_instance);

		return _atNXYZC(pos, x, y, z, c);
	}

	T& _atNXYZC(const int pos, const int x, const int y, const int z, const int c) {
		return _data[cimg::cut(pos, 0, width() - 1)].atXYZC(x, y, z, c);
	}

	T _atNXYZC(const int pos, const int x, const int y, const int z, const int c) const {
		return _data[cimg::cut(pos, 0, width() - 1)].atXYZC(x, y, z, c);
	}

	//! Access pixel value with Dirichlet boundary conditions for the 3 coordinates (\c pos, \c x,\c y,\c z).
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\param out_value Default value returned if \c offset is outside image bounds.
	\note <tt>list.atNXYZ(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZ(x,y,z,c);</tt>.
	**/
	T& atNXYZ(const int pos, const int x, const int y, const int z, const int c, const T& out_value) {
		return (pos<0 || pos >= (int)_width) ? (cimg::temporary(out_value) = out_value) : _data[pos].atXYZ(x, y, z, c, out_value);
	}

	//! Access pixel value with Dirichlet boundary conditions for the 3 coordinates (\c pos, \c x,\c y,\c z) \const.
	T atNXYZ(const int pos, const int x, const int y, const int z, const int c, const T& out_value) const {
		return (pos<0 || pos >= (int)_width) ? out_value : _data[pos].atXYZ(x, y, z, c, out_value);
	}

	//! Access to pixel value with Neumann boundary conditions for the 4 coordinates (\c pos, \c x,\c y,\c z).
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\note <tt>list.atNXYZ(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZ(x,y,z,c);</tt>.
	**/
	T& atNXYZ(const int pos, const int x, const int y, const int z, const int c = 0) {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atNXYZ(): Empty instance.",
				cimglist_instance);

		return _atNXYZ(pos, x, y, z, c);
	}

	//! Access to pixel value with Neumann boundary conditions for the 4 coordinates (\c pos, \c x,\c y,\c z) \const.
	T atNXYZ(const int pos, const int x, const int y, const int z, const int c = 0) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atNXYZ(): Empty instance.",
				cimglist_instance);

		return _atNXYZ(pos, x, y, z, c);
	}

	T& _atNXYZ(const int pos, const int x, const int y, const int z, const int c = 0) {
		return _data[cimg::cut(pos, 0, width() - 1)].atXYZ(x, y, z, c);
	}

	T _atNXYZ(const int pos, const int x, const int y, const int z, const int c = 0) const {
		return _data[cimg::cut(pos, 0, width() - 1)].atXYZ(x, y, z, c);
	}

	//! Access to pixel value with Dirichlet boundary conditions for the 3 coordinates (\c pos, \c x,\c y).
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\param out_value Default value returned if \c offset is outside image bounds.
	\note <tt>list.atNXYZ(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZ(x,y,z,c);</tt>.
	**/
	T& atNXY(const int pos, const int x, const int y, const int z, const int c, const T& out_value) {
		return (pos<0 || pos >= (int)_width) ? (cimg::temporary(out_value) = out_value) : _data[pos].atXY(x, y, z, c, out_value);
	}

	//! Access to pixel value with Dirichlet boundary conditions for the 3 coordinates (\c pos, \c x,\c y) \const.
	T atNXY(const int pos, const int x, const int y, const int z, const int c, const T& out_value) const {
		return (pos<0 || pos >= (int)_width) ? out_value : _data[pos].atXY(x, y, z, c, out_value);
	}

	//! Access to pixel value with Neumann boundary conditions for the 3 coordinates (\c pos, \c x,\c y).
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\note <tt>list.atNXYZ(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZ(x,y,z,c);</tt>.
	**/
	T& atNXY(const int pos, const int x, const int y, const int z = 0, const int c = 0) {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atNXY(): Empty instance.",
				cimglist_instance);

		return _atNXY(pos, x, y, z, c);
	}

	//! Access to pixel value with Neumann boundary conditions for the 3 coordinates (\c pos, \c x,\c y) \const.
	T atNXY(const int pos, const int x, const int y, const int z = 0, const int c = 0) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atNXY(): Empty instance.",
				cimglist_instance);

		return _atNXY(pos, x, y, z, c);
	}

	T& _atNXY(const int pos, const int x, const int y, const int z = 0, const int c = 0) {
		return _data[cimg::cut(pos, 0, width() - 1)].atXY(x, y, z, c);
	}

	T _atNXY(const int pos, const int x, const int y, const int z = 0, const int c = 0) const {
		return _data[cimg::cut(pos, 0, width() - 1)].atXY(x, y, z, c);
	}

	//! Access to pixel value with Dirichlet boundary conditions for the 2 coordinates (\c pos,\c x).
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\param out_value Default value returned if \c offset is outside image bounds.
	\note <tt>list.atNXYZ(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZ(x,y,z,c);</tt>.
	**/
	T& atNX(const int pos, const int x, const int y, const int z, const int c, const T& out_value) {
		return (pos<0 || pos >= (int)_width) ? (cimg::temporary(out_value) = out_value) : _data[pos].atX(x, y, z, c, out_value);
	}

	//! Access to pixel value with Dirichlet boundary conditions for the 2 coordinates (\c pos,\c x) \const.
	T atNX(const int pos, const int x, const int y, const int z, const int c, const T& out_value) const {
		return (pos<0 || pos >= (int)_width) ? out_value : _data[pos].atX(x, y, z, c, out_value);
	}

	//! Access to pixel value with Neumann boundary conditions for the 2 coordinates (\c pos, \c x).
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\note <tt>list.atNXYZ(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZ(x,y,z,c);</tt>.
	**/
	T& atNX(const int pos, const int x, const int y = 0, const int z = 0, const int c = 0) {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atNX(): Empty instance.",
				cimglist_instance);

		return _atNX(pos, x, y, z, c);
	}

	//! Access to pixel value with Neumann boundary conditions for the 2 coordinates (\c pos, \c x) \const.
	T atNX(const int pos, const int x, const int y = 0, const int z = 0, const int c = 0) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atNX(): Empty instance.",
				cimglist_instance);

		return _atNX(pos, x, y, z, c);
	}

	T& _atNX(const int pos, const int x, const int y = 0, const int z = 0, const int c = 0) {
		return _data[cimg::cut(pos, 0, width() - 1)].atX(x, y, z, c);
	}

	T _atNX(const int pos, const int x, const int y = 0, const int z = 0, const int c = 0) const {
		return _data[cimg::cut(pos, 0, width() - 1)].atX(x, y, z, c);
	}

	//! Access to pixel value with Dirichlet boundary conditions for the coordinate (\c pos).
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\param out_value Default value returned if \c offset is outside image bounds.
	\note <tt>list.atNXYZ(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZ(x,y,z,c);</tt>.
	**/
	T& atN(const int pos, const int x, const int y, const int z, const int c, const T& out_value) {
		return (pos<0 || pos >= (int)_width) ? (cimg::temporary(out_value) = out_value) : (*this)(pos, x, y, z, c);
	}

	//! Access to pixel value with Dirichlet boundary conditions for the coordinate (\c pos) \const.
	T atN(const int pos, const int x, const int y, const int z, const int c, const T& out_value) const {
		return (pos<0 || pos >= (int)_width) ? out_value : (*this)(pos, x, y, z, c);
	}

	//! Return pixel value with Neumann boundary conditions for the coordinate (\c pos).
	/**
	\param pos Indice of the image element to access.
	\param x X-coordinate of the pixel value.
	\param y Y-coordinate of the pixel value.
	\param z Z-coordinate of the pixel value.
	\param c C-coordinate of the pixel value.
	\note <tt>list.atNXYZ(p,x,y,z,c);</tt> is equivalent to <tt>list[p].atXYZ(x,y,z,c);</tt>.
	**/
	T& atN(const int pos, const int x = 0, const int y = 0, const int z = 0, const int c = 0) {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atN(): Empty instance.",
				cimglist_instance);
		return _atN(pos, x, y, z, c);
	}

	//! Return pixel value with Neumann boundary conditions for the coordinate (\c pos) \const.
	T atN(const int pos, const int x = 0, const int y = 0, const int z = 0, const int c = 0) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"atN(): Empty instance.",
				cimglist_instance);
		return _atN(pos, x, y, z, c);
	}

	T& _atN(const int pos, const int x = 0, const int y = 0, const int z = 0, const int c = 0) {
		return _data[cimg::cut(pos, 0, width() - 1)](x, y, z, c);
	}

	T _atN(const int pos, const int x = 0, const int y = 0, const int z = 0, const int c = 0) const {
		return _data[cimg::cut(pos, 0, width() - 1)](x, y, z, c);
	}

	//! Return a C-string containing the values of all images in the instance list.
	/**
	\param separator Character separator set between consecutive pixel values.
	\param max_size Maximum size of the returned string.
	\note The result is returne as a <tt>CImg<char></tt> image whose pixel buffer contains the desired C-string.
	**/
	CImg<charT> value_string(const char separator = ',', const unsigned int max_size = 0) const {
		if (is_empty()) return CImg<ucharT>(1, 1, 1, 1, 0);
		CImgList<charT> items;
		for (unsigned int l = 0; l<_width - 1; ++l) {
			CImg<charT> item = _data[l].value_string(separator, 0);
			item.back() = separator;
			item.move_to(items);
		}
		_data[_width - 1].value_string(separator, 0).move_to(items);
		CImg<charT> res; (items>'x').move_to(res);
		if (max_size) { res.crop(0, max_size); res(max_size) = 0; }
		return res;
	}

	//@}
	//-------------------------------------
	//
	//! \name Instance Checking
	//@{
	//-------------------------------------

	//! Return \c true if list is empty.
	/**
	**/
	bool is_empty() const {
		return (!_data || !_width);
	}

	//! Test if number of image elements is equal to specified value.
	/**
	\param size_n Number of image elements to test.
	**/
	bool is_sameN(const unsigned int size_n) const {
		return _width == size_n;
	}

	//! Test if number of image elements is equal between two images lists.
	/**
	\param list Input list to compare with.
	**/
	template<typename t>
	bool is_sameN(const CImgList<t>& list) const {
		return is_sameN(list._width);
	}

	// Define useful functions to check list dimensions.
	// (cannot be documented because macro-generated).
#define _cimglist_def_is_same1(axis) \
    bool is_same##axis(const unsigned int val) const { \
      bool res = true; \
      for (unsigned int l = 0; l<_width && res; ++l) res = _data[l].is_same##axis(val); return res; \
    } \
    bool is_sameN##axis(const unsigned int n, const unsigned int val) const { \
      return is_sameN(n) && is_same##axis(val); \
    } \

#define _cimglist_def_is_same2(axis1,axis2) \
    bool is_same##axis1##axis2(const unsigned int val1, const unsigned int val2) const { \
      bool res = true; \
      for (unsigned int l = 0; l<_width && res; ++l) res = _data[l].is_same##axis1##axis2(val1,val2); return res; \
    } \
    bool is_sameN##axis1##axis2(const unsigned int n, const unsigned int val1, const unsigned int val2) const { \
      return is_sameN(n) && is_same##axis1##axis2(val1,val2); \
    } \

#define _cimglist_def_is_same3(axis1,axis2,axis3) \
    bool is_same##axis1##axis2##axis3(const unsigned int val1, const unsigned int val2, \
                                      const unsigned int val3) const { \
      bool res = true; \
      for (unsigned int l = 0; l<_width && res; ++l) res = _data[l].is_same##axis1##axis2##axis3(val1,val2,val3); \
      return res; \
    } \
    bool is_sameN##axis1##axis2##axis3(const unsigned int n, const unsigned int val1, \
                                       const unsigned int val2, const unsigned int val3) const { \
      return is_sameN(n) && is_same##axis1##axis2##axis3(val1,val2,val3); \
    } \

#define _cimglist_def_is_same(axis) \
    template<typename t> bool is_same##axis(const CImg<t>& img) const { \
      bool res = true; for (unsigned int l = 0; l<_width && res; ++l) res = _data[l].is_same##axis(img); return res; \
    } \
    template<typename t> bool is_same##axis(const CImgList<t>& list) const { \
      const unsigned int lmin = std::min(_width,list._width); \
      bool res = true; for (unsigned int l = 0; l<lmin && res; ++l) res = _data[l].is_same##axis(list[l]); return res; \
    } \
    template<typename t> bool is_sameN##axis(const unsigned int n, const CImg<t>& img) const { \
      return (is_sameN(n) && is_same##axis(img)); \
    } \
    template<typename t> bool is_sameN##axis(const CImgList<t>& list) const { \
      return (is_sameN(list) && is_same##axis(list)); \
    }

	_cimglist_def_is_same(XY)
		_cimglist_def_is_same(XZ)
		_cimglist_def_is_same(XC)
		_cimglist_def_is_same(YZ)
		_cimglist_def_is_same(YC)
		_cimglist_def_is_same(XYZ)
		_cimglist_def_is_same(XYC)
		_cimglist_def_is_same(YZC)
		_cimglist_def_is_same(XYZC)
		_cimglist_def_is_same1(X)
		_cimglist_def_is_same1(Y)
		_cimglist_def_is_same1(Z)
		_cimglist_def_is_same1(C)
		_cimglist_def_is_same2(X, Y)
		_cimglist_def_is_same2(X, Z)
		_cimglist_def_is_same2(X, C)
		_cimglist_def_is_same2(Y, Z)
		_cimglist_def_is_same2(Y, C)
		_cimglist_def_is_same2(Z, C)
		_cimglist_def_is_same3(X, Y, Z)
		_cimglist_def_is_same3(X, Y, C)
		_cimglist_def_is_same3(X, Z, C)
		_cimglist_def_is_same3(Y, Z, C)

		//! Test if dimensions of each image of the list match specified arguments.
		/**
		\param dx Checked image width.
		\param dy Checked image height.
		\param dz Checked image depth.
		\param dc Checked image spectrum.
		**/
		bool is_sameXYZC(const unsigned int dx, const unsigned int dy,
			const unsigned int dz, const unsigned int dc) const {
		bool res = true;
		for (unsigned int l = 0; l<_width && res; ++l) res = _data[l].is_sameXYZC(dx, dy, dz, dc);
		return res;
	}

	//! Test if list dimensions match specified arguments.
	/**
	\param n Number of images in the list.
	\param dx Checked image width.
	\param dy Checked image height.
	\param dz Checked image depth.
	\param dc Checked image spectrum.
	**/
	bool is_sameNXYZC(const unsigned int n,
		const unsigned int dx, const unsigned int dy,
		const unsigned int dz, const unsigned int dc) const {
		return is_sameN(n) && is_sameXYZC(dx, dy, dz, dc);
	}

	//! Test if list contains one particular pixel location.
	/**
	\param n Index of the image whom checked pixel value belong to.
	\param x X-coordinate of the checked pixel value.
	\param y Y-coordinate of the checked pixel value.
	\param z Z-coordinate of the checked pixel value.
	\param c C-coordinate of the checked pixel value.
	**/
	bool containsNXYZC(const int n, const int x = 0, const int y = 0, const int z = 0, const int c = 0) const {
		if (is_empty()) return false;
		return n >= 0 && n<(int)_width && x >= 0 && x<_data[n].width() && y >= 0 && y<_data[n].height() &&
			z >= 0 && z<_data[n].depth() && c >= 0 && c<_data[n].spectrum();
	}

	//! Test if list contains image with specified indice.
	/**
	\param n Index of the checked image.
	**/
	bool containsN(const int n) const {
		if (is_empty()) return false;
		return n >= 0 && n<(int)_width;
	}

	//! Test if one image of the list contains the specified referenced value.
	/**
	\param pixel Reference to pixel value to test.
	\param[out] n Index of image containing the pixel value, if test succeeds.
	\param[out] x X-coordinate of the pixel value, if test succeeds.
	\param[out] y Y-coordinate of the pixel value, if test succeeds.
	\param[out] z Z-coordinate of the pixel value, if test succeeds.
	\param[out] c C-coordinate of the pixel value, if test succeeds.
	\note If true, set coordinates (n,x,y,z,c).
	**/
	template<typename t>
	bool contains(const T& pixel, t& n, t& x, t&y, t& z, t& c) const {
		if (is_empty()) return false;
		cimglist_for(*this, l) if (_data[l].contains(pixel, x, y, z, c)) { n = (t)l; return true; }
		return false;
	}

	//! Test if one of the image list contains the specified referenced value.
	/**
	\param pixel Reference to pixel value to test.
	\param[out] n Index of image containing the pixel value, if test succeeds.
	\param[out] x X-coordinate of the pixel value, if test succeeds.
	\param[out] y Y-coordinate of the pixel value, if test succeeds.
	\param[out] z Z-coordinate of the pixel value, if test succeeds.
	\note If true, set coordinates (n,x,y,z).
	**/
	template<typename t>
	bool contains(const T& pixel, t& n, t& x, t&y, t& z) const {
		t c;
		return contains(pixel, n, x, y, z, c);
	}

	//! Test if one of the image list contains the specified referenced value.
	/**
	\param pixel Reference to pixel value to test.
	\param[out] n Index of image containing the pixel value, if test succeeds.
	\param[out] x X-coordinate of the pixel value, if test succeeds.
	\param[out] y Y-coordinate of the pixel value, if test succeeds.
	\note If true, set coordinates (n,x,y).
	**/
	template<typename t>
	bool contains(const T& pixel, t& n, t& x, t&y) const {
		t z, c;
		return contains(pixel, n, x, y, z, c);
	}

	//! Test if one of the image list contains the specified referenced value.
	/**
	\param pixel Reference to pixel value to test.
	\param[out] n Index of image containing the pixel value, if test succeeds.
	\param[out] x X-coordinate of the pixel value, if test succeeds.
	\note If true, set coordinates (n,x).
	**/
	template<typename t>
	bool contains(const T& pixel, t& n, t& x) const {
		t y, z, c;
		return contains(pixel, n, x, y, z, c);
	}

	//! Test if one of the image list contains the specified referenced value.
	/**
	\param pixel Reference to pixel value to test.
	\param[out] n Index of image containing the pixel value, if test succeeds.
	\note If true, set coordinates (n).
	**/
	template<typename t>
	bool contains(const T& pixel, t& n) const {
		t x, y, z, c;
		return contains(pixel, n, x, y, z, c);
	}

	//! Test if one of the image list contains the specified referenced value.
	/**
	\param pixel Reference to pixel value to test.
	**/
	bool contains(const T& pixel) const {
		unsigned int n, x, y, z, c;
		return contains(pixel, n, x, y, z, c);
	}

	//! Test if the list contains the image 'img'.
	/**
	\param img Reference to image to test.
	\param[out] n Index of image in the list, if test succeeds.
	\note If true, returns the position (n) of the image in the list.
	**/
	template<typename t>
	bool contains(const CImg<T>& img, t& n) const {
		if (is_empty()) return false;
		const CImg<T> *const ptr = &img;
		cimglist_for(*this, i) if (_data + i == ptr) { n = (t)i; return true; }
		return false;
	}

	//! Test if the list contains the image img.
	/**
	\param img Reference to image to test.
	**/
	bool contains(const CImg<T>& img) const {
		unsigned int n;
		return contains(img, n);
	}

	//@}
	//-------------------------------------
	//
	//! \name Mathematical Functions
	//@{
	//-------------------------------------

	//! Return a reference to the minimum pixel value of the instance list.
	/**
	**/
	T& min() {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"min(): Empty instance.",
				cimglist_instance);
		T *ptr_min = _data->_data;
		T min_value = *ptr_min;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_for(img, ptrs, T) if (*ptrs<min_value) min_value = *(ptr_min = ptrs);
		}
		return *ptr_min;
	}

	//! Return a reference to the minimum pixel value of the instance list \const.
	const T& min() const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"min(): Empty instance.",
				cimglist_instance);
		const T *ptr_min = _data->_data;
		T min_value = *ptr_min;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_for(img, ptrs, T) if (*ptrs<min_value) min_value = *(ptr_min = ptrs);
		}
		return *ptr_min;
	}

	//! Return a reference to the maximum pixel value of the instance list.
	/**
	**/
	T& max() {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"max(): Empty instance.",
				cimglist_instance);
		T *ptr_max = _data->_data;
		T max_value = *ptr_max;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_for(img, ptrs, T) if (*ptrs>max_value) max_value = *(ptr_max = ptrs);
		}
		return *ptr_max;
	}

	//! Return a reference to the maximum pixel value of the instance list \const.
	const T& max() const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"max(): Empty instance.",
				cimglist_instance);
		const T *ptr_max = _data->_data;
		T max_value = *ptr_max;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_for(img, ptrs, T) if (*ptrs>max_value) max_value = *(ptr_max = ptrs);
		}
		return *ptr_max;
	}

	//! Return a reference to the minimum pixel value of the instance list and return the maximum vvalue as well.
	/**
	\param[out] max_val Value of the maximum value found.
	**/
	template<typename t>
	T& min_max(t& max_val) {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"min_max(): Empty instance.",
				cimglist_instance);
		T *ptr_min = _data->_data;
		T min_value = *ptr_min, max_value = min_value;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_for(img, ptrs, T) {
				const T val = *ptrs;
				if (val<min_value) { min_value = val; ptr_min = ptrs; }
				if (val>max_value) max_value = val;
			}
		}
		max_val = (t)max_value;
		return *ptr_min;
	}

	//! Return a reference to the minimum pixel value of the instance list and return the maximum vvalue as well \const.
	/**
	\param[out] max_val Value of the maximum value found.
	**/
	template<typename t>
	const T& min_max(t& max_val) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"min_max(): Empty instance.",
				cimglist_instance);
		const T *ptr_min = _data->_data;
		T min_value = *ptr_min, max_value = min_value;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_for(img, ptrs, T) {
				const T val = *ptrs;
				if (val<min_value) { min_value = val; ptr_min = ptrs; }
				if (val>max_value) max_value = val;
			}
		}
		max_val = (t)max_value;
		return *ptr_min;
	}

	//! Return a reference to the minimum pixel value of the instance list and return the minimum value as well.
	/**
	\param[out] min_val Value of the minimum value found.
	**/
	template<typename t>
	T& max_min(t& min_val) {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"max_min(): Empty instance.",
				cimglist_instance);
		T *ptr_max = _data->_data;
		T min_value = *ptr_max, max_value = min_value;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_for(img, ptrs, T) {
				const T val = *ptrs;
				if (val>max_value) { max_value = val; ptr_max = ptrs; }
				if (val<min_value) min_value = val;
			}
		}
		min_val = (t)min_value;
		return *ptr_max;
	}

	//! Return a reference to the minimum pixel value of the instance list and return the minimum value as well \const.
	template<typename t>
	const T& max_min(t& min_val) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"max_min(): Empty instance.",
				cimglist_instance);
		const T *ptr_max = _data->_data;
		T min_value = *ptr_max, max_value = min_value;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_for(img, ptrs, T) {
				const T val = *ptrs;
				if (val>max_value) { max_value = val; ptr_max = ptrs; }
				if (val<min_value) min_value = val;
			}
		}
		min_val = (t)min_value;
		return *ptr_max;
	}

	//@}
	//---------------------------
	//
	//! \name List Manipulation
	//@{
	//---------------------------

	//! Insert a copy of the image \c img into the current image list, at position \c pos.
	/**
	\param img Image to insert a copy to the list.
	\param pos Index of the insertion.
	\param is_shared Tells if the inserted image is a shared copy of \c img or not.
	**/
	template<typename t>
	CImgList<T>& insert(const CImg<t>& img, const unsigned int pos = ~0U, const bool is_shared = false) {
		const unsigned int npos = pos == ~0U ? _width : pos;
		if (npos>_width)
			throw CImgArgumentException(_cimglist_instance
				"insert(): Invalid insertion request of specified image (%u,%u,%u,%u,%p) "
				"at position %u.",
				cimglist_instance,
				img._width, img._height, img._depth, img._spectrum, img._data, npos);
		if (is_shared)
			throw CImgArgumentException(_cimglist_instance
				"insert(): Invalid insertion request of specified shared image "
				"CImg<%s>(%u,%u,%u,%u,%p) at position %u (pixel types are different).",
				cimglist_instance,
				img.pixel_type(), img._width, img._height, img._depth, img._spectrum, img._data, npos);

		CImg<T> *const new_data = (++_width>_allocated_width) ? new CImg<T>[_allocated_width ? (_allocated_width <<= 1) :
			(_allocated_width = 16)] : 0;
		if (!_data) { // Insert new element into empty list.
			_data = new_data;
			*_data = img;
		}
		else {
			if (new_data) { // Insert with re-allocation.
				if (npos) std::memcpy(new_data, _data, sizeof(CImg<T>)*npos);
				if (npos != _width - 1) std::memcpy(new_data + npos + 1, _data + npos, sizeof(CImg<T>)*(_width - 1 - npos));
				std::memset(_data, 0, sizeof(CImg<T>)*(_width - 1));
				delete[] _data;
				_data = new_data;
			}
			else if (npos != _width - 1) // Insert without re-allocation.
				std::memmove(_data + npos + 1, _data + npos, sizeof(CImg<T>)*(_width - 1 - npos));
			_data[npos]._width = _data[npos]._height = _data[npos]._depth = _data[npos]._spectrum = 0;
			_data[npos]._data = 0;
			_data[npos] = img;
		}
		return *this;
	}

	//! Insert a copy of the image \c img into the current image list, at position \c pos \specialization.
	CImgList<T>& insert(const CImg<T>& img, const unsigned int pos = ~0U, const bool is_shared = false) {
		const unsigned int npos = pos == ~0U ? _width : pos;
		if (npos>_width)
			throw CImgArgumentException(_cimglist_instance
				"insert(): Invalid insertion request of specified image (%u,%u,%u,%u,%p) "
				"at position %u.",
				cimglist_instance,
				img._width, img._height, img._depth, img._spectrum, img._data, npos);
		CImg<T> *const new_data = (++_width>_allocated_width) ? new CImg<T>[_allocated_width ? (_allocated_width <<= 1) :
			(_allocated_width = 16)] : 0;
		if (!_data) { // Insert new element into empty list.
			_data = new_data;
			if (is_shared && img) {
				_data->_width = img._width;
				_data->_height = img._height;
				_data->_depth = img._depth;
				_data->_spectrum = img._spectrum;
				_data->_is_shared = true;
				_data->_data = img._data;
			}
			else *_data = img;
		}
		else {
			if (new_data) { // Insert with re-allocation.
				if (npos) std::memcpy(new_data, _data, sizeof(CImg<T>)*npos);
				if (npos != _width - 1) std::memcpy(new_data + npos + 1, _data + npos, sizeof(CImg<T>)*(_width - 1 - npos));
				if (is_shared && img) {
					new_data[npos]._width = img._width;
					new_data[npos]._height = img._height;
					new_data[npos]._depth = img._depth;
					new_data[npos]._spectrum = img._spectrum;
					new_data[npos]._is_shared = true;
					new_data[npos]._data = img._data;
				}
				else {
					new_data[npos]._width = new_data[npos]._height = new_data[npos]._depth = new_data[npos]._spectrum = 0;
					new_data[npos]._data = 0;
					new_data[npos] = img;
				}
				std::memset(_data, 0, sizeof(CImg<T>)*(_width - 1));
				delete[] _data;
				_data = new_data;
			}
			else { // Insert without re-allocation.
				if (npos != _width - 1) std::memmove(_data + npos + 1, _data + npos, sizeof(CImg<T>)*(_width - 1 - npos));
				if (is_shared && img) {
					_data[npos]._width = img._width;
					_data[npos]._height = img._height;
					_data[npos]._depth = img._depth;
					_data[npos]._spectrum = img._spectrum;
					_data[npos]._is_shared = true;
					_data[npos]._data = img._data;
				}
				else {
					_data[npos]._width = _data[npos]._height = _data[npos]._depth = _data[npos]._spectrum = 0;
					_data[npos]._data = 0;
					_data[npos] = img;
				}
			}
		}
		return *this;
	}

	//! Insert a copy of the image \c img into the current image list, at position \c pos \newinstance.
	template<typename t>
	CImgList<T> get_insert(const CImg<t>& img, const unsigned int pos = ~0U, const bool is_shared = false) const {
		return (+*this).insert(img, pos, is_shared);
	}

	//! Insert n empty images img into the current image list, at position \p pos.
	/**
	\param n Number of empty images to insert.
	\param pos Index of the insertion.
	**/
	CImgList<T>& insert(const unsigned int n, const unsigned int pos = ~0U) {
		CImg<T> empty;
		if (!n) return *this;
		const unsigned int npos = pos == ~0U ? _width : pos;
		for (unsigned int i = 0; i<n; ++i) insert(empty, npos + i);
		return *this;
	}

	//! Insert n empty images img into the current image list, at position \p pos \newinstance.
	CImgList<T> get_insert(const unsigned int n, const unsigned int pos = ~0U) const {
		return (+*this).insert(n, pos);
	}

	//! Insert \c n copies of the image \c img into the current image list, at position \c pos.
	/**
	\param n Number of image copies to insert.
	\param img Image to insert by copy.
	\param pos Index of the insertion.
	\param is_shared Tells if inserted images are shared copies of \c img or not.
	**/
	template<typename t>
	CImgList<T>& insert(const unsigned int n, const CImg<t>& img, const unsigned int pos = ~0U,
		const bool is_shared = false) {
		if (!n) return *this;
		const unsigned int npos = pos == ~0U ? _width : pos;
		insert(img, npos, is_shared);
		for (unsigned int i = 1; i<n; ++i) insert(_data[npos], npos + i, is_shared);
		return *this;
	}

	//! Insert \c n copies of the image \c img into the current image list, at position \c pos \newinstance.
	template<typename t>
	CImgList<T> get_insert(const unsigned int n, const CImg<t>& img, const unsigned int pos = ~0U,
		const bool is_shared = false) const {
		return (+*this).insert(n, img, pos, is_shared);
	}

	//! Insert a copy of the image list \c list into the current image list, starting from position \c pos.
	/**
	\param list Image list to insert.
	\param pos Index of the insertion.
	\param is_shared Tells if inserted images are shared copies of images of \c list or not.
	**/
	template<typename t>
	CImgList<T>& insert(const CImgList<t>& list, const unsigned int pos = ~0U, const bool is_shared = false) {
		const unsigned int npos = pos == ~0U ? _width : pos;
		if ((void*)this != (void*)&list) cimglist_for(list, l) insert(list[l], npos + l, is_shared);
		else insert(CImgList<T>(list), npos, is_shared);
		return *this;
	}

	//! Insert a copy of the image list \c list into the current image list, starting from position \c pos \newinstance.
	template<typename t>
	CImgList<T> get_insert(const CImgList<t>& list, const unsigned int pos = ~0U, const bool is_shared = false) const {
		return (+*this).insert(list, pos, is_shared);
	}

	//! Insert n copies of the list \c list at position \c pos of the current list.
	/**
	\param n Number of list copies to insert.
	\param list Image list to insert.
	\param pos Index of the insertion.
	\param is_shared Tells if inserted images are shared copies of images of \c list or not.
	**/
	template<typename t>
	CImgList<T>& insert(const unsigned int n, const CImgList<t>& list, const unsigned int pos = ~0U,
		const bool is_shared = false) {
		if (!n) return *this;
		const unsigned int npos = pos == ~0U ? _width : pos;
		for (unsigned int i = 0; i<n; ++i) insert(list, npos, is_shared);
		return *this;
	}

	//! Insert n copies of the list \c list at position \c pos of the current list \newinstance.
	template<typename t>
	CImgList<T> get_insert(const unsigned int n, const CImgList<t>& list, const unsigned int pos = ~0U,
		const bool is_shared = false) const {
		return (+*this).insert(n, list, pos, is_shared);
	}

	//! Remove all images between from indexes.
	/**
	\param pos1 Starting index of the removal.
	\param pos2 Ending index of the removal.
	**/
	CImgList<T>& remove(const unsigned int pos1, const unsigned int pos2) {
		const unsigned int
			npos1 = pos1<pos2 ? pos1 : pos2,
			tpos2 = pos1<pos2 ? pos2 : pos1,
			npos2 = tpos2<_width ? tpos2 : _width - 1;
		if (npos1 >= _width)
			throw CImgArgumentException(_cimglist_instance
				"remove(): Invalid remove request at positions %u->%u.",
				cimglist_instance,
				npos1, tpos2);
		else {
			if (tpos2 >= _width)
				throw CImgArgumentException(_cimglist_instance
					"remove(): Invalid remove request at positions %u->%u.",
					cimglist_instance,
					npos1, tpos2);

			for (unsigned int k = npos1; k <= npos2; ++k) _data[k].assign();
			const unsigned int nb = 1 + npos2 - npos1;
			if (!(_width -= nb)) return assign();
			if (_width>(_allocated_width >> 2) || _allocated_width <= 16) { // Removing items without reallocation.
				if (npos1 != _width) std::memmove(_data + npos1, _data + npos2 + 1, sizeof(CImg<T>)*(_width - npos1));
				std::memset(_data + _width, 0, sizeof(CImg<T>)*nb);
			}
			else { // Removing items with reallocation.
				_allocated_width >>= 2;
				while (_allocated_width>16 && _width<(_allocated_width >> 1)) _allocated_width >>= 1;
				CImg<T> *const new_data = new CImg<T>[_allocated_width];
				if (npos1) std::memcpy(new_data, _data, sizeof(CImg<T>)*npos1);
				if (npos1 != _width) std::memcpy(new_data + npos1, _data + npos2 + 1, sizeof(CImg<T>)*(_width - npos1));
				if (_width != _allocated_width) std::memset(new_data + _width, 0, sizeof(CImg<T>)*(_allocated_width - _width));
				std::memset(_data, 0, sizeof(CImg<T>)*(_width + nb));
				delete[] _data;
				_data = new_data;
			}
		}
		return *this;
	}

	//! Remove all images between from indexes \newinstance.
	CImgList<T> get_remove(const unsigned int pos1, const unsigned int pos2) const {
		return (+*this).remove(pos1, pos2);
	}

	//! Remove image at index \c pos from the image list.
	/**
	\param pos Index of the image to remove.
	**/
	CImgList<T>& remove(const unsigned int pos) {
		return remove(pos, pos);
	}

	//! Remove image at index \c pos from the image list \newinstance.
	CImgList<T> get_remove(const unsigned int pos) const {
		return (+*this).remove(pos);
	}

	//! Remove last image.
	/**
	**/
	CImgList<T>& remove() {
		return remove(_width - 1);
	}

	//! Remove last image \newinstance.
	CImgList<T> get_remove() const {
		return (+*this).remove();
	}

	//! Reverse list order.
	CImgList<T>& reverse() {
		for (unsigned int l = 0; l<_width / 2; ++l) (*this)[l].swap((*this)[_width - 1 - l]);
		return *this;
	}

	//! Reverse list order \newinstance.
	CImgList<T> get_reverse() const {
		return (+*this).reverse();
	}

	//! Return a sublist.
	/**
	\param pos0 Starting index of the sublist.
	\param pos1 Ending index of the sublist.
	**/
	CImgList<T>& images(const unsigned int pos0, const unsigned int pos1) {
		return get_images(pos0, pos1).move_to(*this);
	}

	//! Return a sublist \newinstance.
	CImgList<T> get_images(const unsigned int pos0, const unsigned int pos1) const {
		if (pos0>pos1 || pos1 >= _width)
			throw CImgArgumentException(_cimglist_instance
				"images(): Specified sub-list indices (%u->%u) are out of bounds.",
				cimglist_instance,
				pos0, pos1);
		CImgList<T> res(pos1 - pos0 + 1);
		cimglist_for(res, l) res[l].assign(_data[pos0 + l]);
		return res;
	}

	//! Return a shared sublist.
	/**
	\param pos0 Starting index of the sublist.
	\param pos1 Ending index of the sublist.
	**/
	CImgList<T> get_shared_images(const unsigned int pos0, const unsigned int pos1) {
		if (pos0>pos1 || pos1 >= _width)
			throw CImgArgumentException(_cimglist_instance
				"get_shared_images(): Specified sub-list indices (%u->%u) are out of bounds.",
				cimglist_instance,
				pos0, pos1);
		CImgList<T> res(pos1 - pos0 + 1);
		cimglist_for(res, l) res[l].assign(_data[pos0 + l], _data[pos0 + l] ? true : false);
		return res;
	}

	//! Return a shared sublist \newinstance.
	const CImgList<T> get_shared_images(const unsigned int pos0, const unsigned int pos1) const {
		if (pos0>pos1 || pos1 >= _width)
			throw CImgArgumentException(_cimglist_instance
				"get_shared_images(): Specified sub-list indices (%u->%u) are out of bounds.",
				cimglist_instance,
				pos0, pos1);
		CImgList<T> res(pos1 - pos0 + 1);
		cimglist_for(res, l) res[l].assign(_data[pos0 + l], _data[pos0 + l] ? true : false);
		return res;
	}

	//! Return a single image which is the appending of all images of the current CImgList instance.
	/**
	\param axis Appending axis. Can be <tt>{ 'x' | 'y' | 'z' | 'c' }</tt>.
	\param align Appending alignment.
	**/
	CImg<T> get_append(const char axis, const float align = 0) const {
		if (is_empty()) return CImg<T>();
		if (_width == 1) return +((*this)[0]);
		unsigned int dx = 0, dy = 0, dz = 0, dc = 0, pos = 0;
		CImg<T> res;
		switch (cimg::lowercase(axis)) {
		case 'x': { // Along the X-axis.
			cimglist_for(*this, l) {
				const CImg<T>& img = (*this)[l];
				if (img) {
					dx += img._width;
					dy = std::max(dy, img._height);
					dz = std::max(dz, img._depth);
					dc = std::max(dc, img._spectrum);
				}
			}
			res.assign(dx, dy, dz, dc, (T)0);
			if (res) cimglist_for(*this, l) {
				const CImg<T>& img = (*this)[l];
				if (img) res.draw_image(pos,
					(int)(align*(dy - img._height)),
					(int)(align*(dz - img._depth)),
					(int)(align*(dc - img._spectrum)),
					img);
				pos += img._width;
			}
		} break;
		case 'y': { // Along the Y-axis.
			cimglist_for(*this, l) {
				const CImg<T>& img = (*this)[l];
				if (img) {
					dx = std::max(dx, img._width);
					dy += img._height;
					dz = std::max(dz, img._depth);
					dc = std::max(dc, img._spectrum);
				}
			}
			res.assign(dx, dy, dz, dc, (T)0);
			if (res) cimglist_for(*this, l) {
				const CImg<T>& img = (*this)[l];
				if (img) res.draw_image((int)(align*(dx - img._width)),
					pos,
					(int)(align*(dz - img._depth)),
					(int)(align*(dc - img._spectrum)),
					img);
				pos += img._height;
			}
		} break;
		case 'z': { // Along the Z-axis.
			cimglist_for(*this, l) {
				const CImg<T>& img = (*this)[l];
				if (img) {
					dx = std::max(dx, img._width);
					dy = std::max(dy, img._height);
					dz += img._depth;
					dc = std::max(dc, img._spectrum);
				}
			}
			res.assign(dx, dy, dz, dc, (T)0);
			if (res) cimglist_for(*this, l) {
				const CImg<T>& img = (*this)[l];
				if (img) res.draw_image((int)(align*(dx - img._width)),
					(int)(align*(dy - img._height)),
					pos,
					(int)(align*(dc - img._spectrum)),
					img);
				pos += img._depth;
			}
		} break;
		default: { // Along the C-axis.
			cimglist_for(*this, l) {
				const CImg<T>& img = (*this)[l];
				if (img) {
					dx = std::max(dx, img._width);
					dy = std::max(dy, img._height);
					dz = std::max(dz, img._depth);
					dc += img._spectrum;
				}
			}
			res.assign(dx, dy, dz, dc, (T)0);
			if (res) cimglist_for(*this, l) {
				const CImg<T>& img = (*this)[l];
				if (img) res.draw_image((int)(align*(dx - img._width)),
					(int)(align*(dy - img._height)),
					(int)(align*(dz - img._depth)),
					pos,
					img);
				pos += img._spectrum;
			}
		}
		}
		return res;
	}

	//! Return a list where each image has been split along the specified axis.
	/**
	\param axis Axis to split images along.
	\param nb Number of spliting parts for each image.
	**/
	CImgList<T>& split(const char axis, const int nb = -1) {
		return get_split(axis, nb).move_to(*this);
	}

	//! Return a list where each image has been split along the specified axis \newinstance.
	CImgList<T> get_split(const char axis, const int nb = -1) const {
		CImgList<T> res;
		cimglist_for(*this, l) _data[l].get_split(axis, nb).move_to(res, ~0U);
		return res;
	}

	//! Insert image at the end of the list.
	/**
	\param img Image to insert.
	**/
	template<typename t>
	CImgList<T>& push_back(const CImg<t>& img) {
		return insert(img);
	}

	//! Insert image at the front of the list.
	/**
	\param img Image to insert.
	**/
	template<typename t>
	CImgList<T>& push_front(const CImg<t>& img) {
		return insert(img, 0);
	}

	//! Insert list at the end of the current list.
	/**
	\param list List to insert.
	**/
	template<typename t>
	CImgList<T>& push_back(const CImgList<t>& list) {
		return insert(list);
	}

	//! Insert list at the front of the current list.
	/**
	\param list List to insert.
	**/
	template<typename t>
	CImgList<T>& push_front(const CImgList<t>& list) {
		return insert(list, 0);
	}

	//! Remove last image.
	/**
	**/
	CImgList<T>& pop_back() {
		return remove(_width - 1);
	}

	//! Remove first image.
	/**
	**/
	CImgList<T>& pop_front() {
		return remove(0);
	}

	//! Remove image pointed by iterator.
	/**
	\param iter Iterator pointing to the image to remove.
	**/
	CImgList<T>& erase(const iterator iter) {
		return remove(iter - _data);
	}

	//@}
	//----------------------------------
	//
	//! \name Data Input
	//@{
	//----------------------------------

	//! Display a simple interactive interface to select images or sublists.
	/**
	\param disp Window instance to display selection and user interface.
	\param feature_type Can be \c false to select a single image, or \c true to select a sublist.
	\param axis Axis along whom images are appended for visualization.
	\param align Alignment setting when images have not all the same size.
	\param exit_on_anykey Exit function when any key is pressed.
	\return A one-column vector containing the selected image indexes.
	**/
	CImg<intT> get_select(CImgDisplay &disp, const bool feature_type = true,
		const char axis = 'x', const float align = 0,
		const bool exit_on_anykey = false) const {
		return _select(disp, 0, feature_type, axis, align, exit_on_anykey, 0, false, false, false);
	}

	//! Display a simple interactive interface to select images or sublists.
	/**
	\param title Title of a new window used to display selection and user interface.
	\param feature_type Can be \c false to select a single image, or \c true to select a sublist.
	\param axis Axis along whom images are appended for visualization.
	\param align Alignment setting when images have not all the same size.
	\param exit_on_anykey Exit function when any key is pressed.
	\return A one-column vector containing the selected image indexes.
	**/
	CImg<intT> get_select(const char *const title, const bool feature_type = true,
		const char axis = 'x', const float align = 0,
		const bool exit_on_anykey = false) const {
		CImgDisplay disp;
		return _select(disp, title, feature_type, axis, align, exit_on_anykey, 0, false, false, false);
	}

	CImg<intT> _select(CImgDisplay &disp, const char *const title, const bool feature_type,
		const char axis, const float align, const bool exit_on_anykey,
		const unsigned int orig, const bool resize_disp,
		const bool exit_on_rightbutton, const bool exit_on_wheel) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"select(): Empty instance.",
				cimglist_instance);

		// Create image correspondence table and get list dimensions for visualization.
		CImgList<uintT> _indices;
		unsigned int max_width = 0, max_height = 0, sum_width = 0, sum_height = 0;
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			const unsigned int
				w = CImgDisplay::_fitscreen(img._width, img._height, img._depth, 128, -85, false),
				h = CImgDisplay::_fitscreen(img._width, img._height, img._depth, 128, -85, true);
			if (w>max_width) max_width = w;
			if (h>max_height) max_height = h;
			sum_width += w; sum_height += h;
			if (axis == 'x') CImg<uintT>(w, 1, 1, 1, (unsigned int)l).move_to(_indices);
			else CImg<uintT>(h, 1, 1, 1, (unsigned int)l).move_to(_indices);
		}
		const CImg<uintT> indices0 = _indices>'x';

		// Create display window.
		if (!disp) {
			if (axis == 'x') disp.assign(cimg_fitscreen(sum_width, max_height, 1), title ? title : 0, 1);
			else disp.assign(cimg_fitscreen(max_width, sum_height, 1), title ? title : 0, 1);
			if (!title) disp.set_title("CImgList<%s> (%u)", pixel_type(), _width);
		}
		else if (title) disp.set_title("%s", title);
		if (resize_disp) {
			if (axis == 'x') disp.resize(cimg_fitscreen(sum_width, max_height, 1), false);
			else disp.resize(cimg_fitscreen(max_width, sum_height, 1), false);
		}

		const unsigned int old_normalization = disp.normalization();
		bool old_is_resized = disp.is_resized();
		disp._normalization = 0;
		disp.show().set_key(0);
		static const unsigned char foreground_color[] = { 255,255,255 }, background_color[] = { 0,0,0 };

		// Enter event loop.
		CImg<ucharT> visu0, visu;
		CImg<uintT> indices;
		CImg<intT> positions(_width, 4, 1, 1, -1);
		int oindice0 = -1, oindice1 = -1, indice0 = -1, indice1 = -1;
		bool is_clicked = false, is_selected = false, text_down = false, update_display = true;
		unsigned int key = 0;

		while (!is_selected && !disp.is_closed() && !key) {

			// Create background image.
			if (!visu0) {
				visu0.assign(disp._width, disp._height, 1, 3, 0); visu.assign();
				(indices0.get_resize(axis == 'x' ? visu0._width : visu0._height, 1)).move_to(indices);
				unsigned int ind = 0;
				const CImg<T> onexone(1, 1, 1, 1, (T)0);
				if (axis == 'x')
					cimg_pragma_openmp(parallel for cimg_openmp_if(_width >= 4))
					cimglist_for(*this, ind) {
					unsigned int x0 = 0;
					while (x0<visu0._width && indices[x0++] != (unsigned int)ind) {}
					unsigned int x1 = x0;
					while (x1<visu0._width && indices[x1++] == (unsigned int)ind) {}
					const CImg<T> &src = _data[ind] ? _data[ind] : onexone;
					CImg<ucharT> res;
					src.__get_select(disp, old_normalization, (src._width - 1) / 2, (src._height - 1) / 2, (src._depth - 1) / 2).
						move_to(res);
					const unsigned int h = CImgDisplay::_fitscreen(res._width, res._height, 1, 128, -85, true);
					res.resize(x1 - x0, std::max(32U, h*disp._height / max_height), 1, res._spectrum == 1 ? 3 : -100);
					positions(ind, 0) = positions(ind, 2) = (int)x0;
					positions(ind, 1) = positions(ind, 3) = (int)(align*(visu0.height() - res.height()));
					positions(ind, 2) += res._width;
					positions(ind, 3) += res._height - 1;
					visu0.draw_image(positions(ind, 0), positions(ind, 1), res);
				}
				else
					cimg_pragma_openmp(parallel for cimg_openmp_if(_width >= 4))
					cimglist_for(*this, ind) {
					unsigned int y0 = 0;
					while (y0<visu0._height && indices[y0++] != (unsigned int)ind) {}
					unsigned int y1 = y0;
					while (y1<visu0._height && indices[y1++] == (unsigned int)ind) {}
					const CImg<T> &src = _data[ind] ? _data[ind] : onexone;
					CImg<ucharT> res;
					src.__get_select(disp, old_normalization, (src._width - 1) / 2, (src._height - 1) / 2, (src._depth - 1) / 2).
						move_to(res);
					const unsigned int w = CImgDisplay::_fitscreen(res._width, res._height, 1, 128, -85, false);
					res.resize(std::max(32U, w*disp._width / max_width), y1 - y0, 1, res._spectrum == 1 ? 3 : -100);
					positions(ind, 0) = positions(ind, 2) = (int)(align*(visu0.width() - res.width()));
					positions(ind, 1) = positions(ind, 3) = (int)y0;
					positions(ind, 2) += res._width - 1;
					positions(ind, 3) += res._height;
					visu0.draw_image(positions(ind, 0), positions(ind, 1), res);
				}
				if (axis == 'x') --positions(ind, 2); else --positions(ind, 3);
				update_display = true;
			}

			if (!visu || oindice0 != indice0 || oindice1 != indice1) {
				if (indice0 >= 0 && indice1 >= 0) {
					visu.assign(visu0, false);
					const int indm = std::min(indice0, indice1), indM = std::max(indice0, indice1);
					for (int ind = indm; ind <= indM; ++ind) if (positions(ind, 0) >= 0) {
						visu.draw_rectangle(positions(ind, 0), positions(ind, 1), positions(ind, 2), positions(ind, 3),
							background_color, 0.2f);
						if ((axis == 'x' && positions(ind, 2) - positions(ind, 0) >= 8) ||
							(axis != 'x' && positions(ind, 3) - positions(ind, 1) >= 8))
							visu.draw_rectangle(positions(ind, 0), positions(ind, 1), positions(ind, 2), positions(ind, 3),
								foreground_color, 0.9f, 0xAAAAAAAA);
					}
					const int yt = (int)text_down ? visu.height() - 13 : 0;
					if (is_clicked) visu.draw_text(0, yt, " Images #%u - #%u, Size = %u",
						foreground_color, background_color, 0.7f, 13,
						orig + indm, orig + indM, indM - indm + 1);
					else visu.draw_text(0, yt, " Image #%u (%u,%u,%u,%u)", foreground_color, background_color, 0.7f, 13,
						orig + indice0,
						_data[indice0]._width,
						_data[indice0]._height,
						_data[indice0]._depth,
						_data[indice0]._spectrum);
					update_display = true;
				}
				else visu.assign();
			}
			if (!visu) { visu.assign(visu0, true); update_display = true; }
			if (update_display) { visu.display(disp); update_display = false; }
			disp.wait();

			// Manage user events.
			const int xm = disp.mouse_x(), ym = disp.mouse_y();
			int indice = -1;

			if (xm >= 0) {
				indice = (int)indices(axis == 'x' ? xm : ym);
				if (disp.button() & 1) {
					if (!is_clicked) { is_clicked = true; oindice0 = indice0; indice0 = indice; }
					oindice1 = indice1; indice1 = indice;
					if (!feature_type) is_selected = true;
				}
				else {
					if (!is_clicked) { oindice0 = oindice1 = indice0; indice0 = indice1 = indice; }
					else is_selected = true;
				}
			}
			else {
				if (is_clicked) {
					if (!(disp.button() & 1)) { is_clicked = is_selected = false; indice0 = indice1 = -1; }
					else indice1 = -1;
				}
				else indice0 = indice1 = -1;
			}

			if (disp.button() & 4) { is_clicked = is_selected = false; indice0 = indice1 = -1; }
			if (disp.button() & 2 && exit_on_rightbutton) { is_selected = true; indice1 = indice0 = -1; }
			if (disp.wheel() && exit_on_wheel) is_selected = true;

			CImg<charT> filename(32);
			switch (key = disp.key()) {
#if cimg_OS!=2
			case cimg::keyCTRLRIGHT:
#endif
			case 0: case cimg::keyCTRLLEFT: key = 0; break;
			case cimg::keyD: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				disp.set_fullscreen(false).
					resize(CImgDisplay::_fitscreen(3 * disp.width() / 2, 3 * disp.height() / 2, 1, 128, -100, false),
						CImgDisplay::_fitscreen(3 * disp.width() / 2, 3 * disp.height() / 2, 1, 128, -100, true), false).
					_is_resized = true;
				disp.set_key(key, false); key = 0; visu0.assign();
			} break;
			case cimg::keyC: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				disp.set_fullscreen(false).
					resize(cimg_fitscreen(2 * disp.width() / 3, 2 * disp.height() / 3, 1), false)._is_resized = true;
				disp.set_key(key, false); key = 0; visu0.assign();
			} break;
			case cimg::keyR: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				disp.set_fullscreen(false).
					resize(cimg_fitscreen(axis == 'x' ? sum_width : max_width, axis == 'x' ? max_height : sum_height, 1), false).
					_is_resized = true;
				disp.set_key(key, false); key = 0; visu0.assign();
			} break;
			case cimg::keyF: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				disp.resize(disp.screen_width(), disp.screen_height(), false).toggle_fullscreen()._is_resized = true;
				disp.set_key(key, false); key = 0; visu0.assign();
			} break;
			case cimg::keyS: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				static unsigned int snap_number = 0;
				std::FILE *file;
				do {
					cimg_snprintf(filename, filename._width, cimg_appname "_%.4u.bmp", snap_number++);
					if ((file = std_fopen(filename, "r")) != 0) cimg::fclose(file);
				} while (file);
				if (visu0) {
					(+visu0).draw_text(0, 0, " Saving snapshot... ",
						foreground_color, background_color, 0.7f, 13).display(disp);
					visu0.save(filename);
					(+visu0).draw_text(0, 0, " Snapshot '%s' saved. ",
						foreground_color, background_color, 0.7f, 13, filename._data).display(disp);
				}
				disp.set_key(key, false).wait(); key = 0;
			} break;
			case cimg::keyO:
				if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
					static unsigned int snap_number = 0;
					std::FILE *file;
					do {
#ifdef cimg_use_zlib
						cimg_snprintf(filename, filename._width, cimg_appname "_%.4u.cimgz", snap_number++);
#else
						cimg_snprintf(filename, filename._width, cimg_appname "_%.4u.cimg", snap_number++);
#endif
						if ((file = std_fopen(filename, "r")) != 0) cimg::fclose(file);
					} while (file);
					(+visu0).draw_text(0, 0, " Saving instance... ",
						foreground_color, background_color, 0.7f, 13).display(disp);
					save(filename);
					(+visu0).draw_text(0, 0, " Instance '%s' saved. ",
						foreground_color, background_color, 0.7f, 13, filename._data).display(disp);
					disp.set_key(key, false).wait(); key = 0;
				} break;
			}
			if (disp.is_resized()) { disp.resize(false); visu0.assign(); }
			if (ym >= 0 && ym<13) { if (!text_down) { visu.assign(); text_down = true; } }
			else if (ym >= visu.height() - 13) { if (text_down) { visu.assign(); text_down = false; } }
			if (!exit_on_anykey && key && key != cimg::keyESC &&
				(key != cimg::keyW || (!disp.is_keyCTRLLEFT() && !disp.is_keyCTRLRIGHT()))) {
				key = 0;
			}
		}
		CImg<intT> res(1, 2, 1, 1, -1);
		if (is_selected) {
			if (feature_type) res.fill(std::min(indice0, indice1), std::max(indice0, indice1));
			else res.fill(indice0);
		}
		if (!(disp.button() & 2)) disp.set_button();
		disp._normalization = old_normalization;
		disp._is_resized = old_is_resized;
		disp.set_key(key);
		return res;
	}

	//! Load a list from a file.
	/**
	\param filename Filename to read data from.
	**/
	CImgList<T>& load(const char *const filename) {
		if (!filename)
			throw CImgArgumentException(_cimglist_instance
				"load(): Specified filename is (null).",
				cimglist_instance);

		if (!cimg::strncasecmp(filename, "http://", 7) || !cimg::strncasecmp(filename, "https://", 8)) {
			CImg<charT> filename_local(256);
			load(cimg::load_network(filename, filename_local));
			std::remove(filename_local);
			return *this;
		}

		const bool is_stdin = *filename == '-' && (!filename[1] || filename[1] == '.');
		const char *const ext = cimg::split_filename(filename);
		const unsigned int omode = cimg::exception_mode();
		cimg::exception_mode(0);
		bool is_loaded = true;
		try {
#ifdef cimglist_load_plugin
			cimglist_load_plugin(filename);
#endif
#ifdef cimglist_load_plugin1
			cimglist_load_plugin1(filename);
#endif
#ifdef cimglist_load_plugin2
			cimglist_load_plugin2(filename);
#endif
#ifdef cimglist_load_plugin3
			cimglist_load_plugin3(filename);
#endif
#ifdef cimglist_load_plugin4
			cimglist_load_plugin4(filename);
#endif
#ifdef cimglist_load_plugin5
			cimglist_load_plugin5(filename);
#endif
#ifdef cimglist_load_plugin6
			cimglist_load_plugin6(filename);
#endif
#ifdef cimglist_load_plugin7
			cimglist_load_plugin7(filename);
#endif
#ifdef cimglist_load_plugin8
			cimglist_load_plugin8(filename);
#endif
			if (!cimg::strcasecmp(ext, "tif") ||
				!cimg::strcasecmp(ext, "tiff")) load_tiff(filename);
			else if (!cimg::strcasecmp(ext, "gif")) load_gif_external(filename);
			else if (!cimg::strcasecmp(ext, "cimg") ||
				!cimg::strcasecmp(ext, "cimgz") ||
				!*ext) load_cimg(filename);
			else if (!cimg::strcasecmp(ext, "rec") ||
				!cimg::strcasecmp(ext, "par")) load_parrec(filename);
			else if (!cimg::strcasecmp(ext, "avi") ||
				!cimg::strcasecmp(ext, "mov") ||
				!cimg::strcasecmp(ext, "asf") ||
				!cimg::strcasecmp(ext, "divx") ||
				!cimg::strcasecmp(ext, "flv") ||
				!cimg::strcasecmp(ext, "mpg") ||
				!cimg::strcasecmp(ext, "m1v") ||
				!cimg::strcasecmp(ext, "m2v") ||
				!cimg::strcasecmp(ext, "m4v") ||
				!cimg::strcasecmp(ext, "mjp") ||
				!cimg::strcasecmp(ext, "mp4") ||
				!cimg::strcasecmp(ext, "mkv") ||
				!cimg::strcasecmp(ext, "mpe") ||
				!cimg::strcasecmp(ext, "movie") ||
				!cimg::strcasecmp(ext, "ogm") ||
				!cimg::strcasecmp(ext, "ogg") ||
				!cimg::strcasecmp(ext, "ogv") ||
				!cimg::strcasecmp(ext, "qt") ||
				!cimg::strcasecmp(ext, "rm") ||
				!cimg::strcasecmp(ext, "vob") ||
				!cimg::strcasecmp(ext, "wmv") ||
				!cimg::strcasecmp(ext, "xvid") ||
				!cimg::strcasecmp(ext, "mpeg")) load_video(filename);
			else if (!cimg::strcasecmp(ext, "gz")) load_gzip_external(filename);
			else is_loaded = false;
		}
		catch (CImgIOException&) { is_loaded = false; }

		// If nothing loaded, try to guess file format from magic number in file.
		if (!is_loaded && !is_stdin) {
			std::FILE *const file = std_fopen(filename, "rb");
			if (!file) {
				cimg::exception_mode(omode);
				throw CImgIOException(_cimglist_instance
					"load(): Failed to open file '%s'.",
					cimglist_instance,
					filename);
			}

			const char *const f_type = cimg::ftype(file, filename);
			std::fclose(file);
			is_loaded = true;
			try {
				if (!cimg::strcasecmp(f_type, "gif")) load_gif_external(filename);
				else if (!cimg::strcasecmp(f_type, "tif")) load_tiff(filename);
				else is_loaded = false;
			}
			catch (CImgIOException&) { is_loaded = false; }
		}

		// If nothing loaded, try to load file as a single image.
		if (!is_loaded) {
			assign(1);
			try {
				_data->load(filename);
			}
			catch (CImgIOException&) {
				cimg::exception_mode(omode);
				throw CImgIOException(_cimglist_instance
					"load(): Failed to recognize format of file '%s'.",
					cimglist_instance,
					filename);
			}
		}
		cimg::exception_mode(omode);
		return *this;
	}

	//! Load a list from a file \newinstance.
	static CImgList<T> get_load(const char *const filename) {
		return CImgList<T>().load(filename);
	}

	//! Load a list from a .cimg file.
	/**
	\param filename Filename to read data from.
	**/
	CImgList<T>& load_cimg(const char *const filename) {
		return _load_cimg(0, filename);
	}

	//! Load a list from a .cimg file \newinstance.
	static CImgList<T> get_load_cimg(const char *const filename) {
		return CImgList<T>().load_cimg(filename);
	}

	//! Load a list from a .cimg file.
	/**
	\param file File to read data from.
	**/
	CImgList<T>& load_cimg(std::FILE *const file) {
		return _load_cimg(file, 0);
	}

	//! Load a list from a .cimg file \newinstance.
	static CImgList<T> get_load_cimg(std::FILE *const file) {
		return CImgList<T>().load_cimg(file);
	}

	CImgList<T>& _load_cimg(std::FILE *const file, const char *const filename) {
#ifdef cimg_use_zlib
#define _cimgz_load_cimg_case(Tss) { \
   Bytef *const cbuf = new Bytef[csiz]; \
   cimg::fread(cbuf,csiz,nfile); \
   raw.assign(W,H,D,C); \
   uLongf destlen = (ulongT)raw.size()*sizeof(Tss); \
   uncompress((Bytef*)raw._data,&destlen,cbuf,csiz); \
   delete[] cbuf; \
   if (endian!=cimg::endianness()) cimg::invert_endianness(raw._data,raw.size()); \
   raw.move_to(img); \
}
#else
#define _cimgz_load_cimg_case(Tss) \
   throw CImgIOException(_cimglist_instance \
                         "load_cimg(): Unable to load compressed data from file '%s' unless zlib is enabled.", \
                         cimglist_instance, \
                         filename?filename:"(FILE*)");
#endif

#define _cimg_load_cimg_case(Ts,Tss) \
      if (!loaded && !cimg::strcasecmp(Ts,str_pixeltype)) { \
        for (unsigned int l = 0; l<N; ++l) { \
          j = 0; while ((i=std::fgetc(nfile))!='\n' && i>=0 && j<255) tmp[j++] = (char)i; tmp[j] = 0; \
          W = H = D = C = 0; csiz = 0; \
          if ((err = cimg_sscanf(tmp,"%u %u %u %u #%lu",&W,&H,&D,&C,&csiz))<4) \
            throw CImgIOException(_cimglist_instance \
                                  "load_cimg(): Invalid specified size (%u,%u,%u,%u) of image %u in file '%s'.", \
                                  cimglist_instance, \
                                  W,H,D,C,l,filename?filename:("(FILE*)")); \
          if (W*H*D*C>0) { \
            CImg<Tss> raw; \
            CImg<T> &img = _data[l]; \
            if (err==5) _cimgz_load_cimg_case(Tss) \
            else { \
              img.assign(W,H,D,C); \
              T *ptrd = img._data; \
              for (ulongT to_read = img.size(); to_read; ) { \
                raw.assign((unsigned int)std::min(to_read,cimg_iobuffer)); \
                cimg::fread(raw._data,raw._width,nfile); \
                if (endian!=cimg::endianness()) cimg::invert_endianness(raw._data,raw.size()); \
                const Tss *ptrs = raw._data; \
                for (ulongT off = (ulongT)raw._width; off; --off) *(ptrd++) = (T)*(ptrs++); \
                to_read-=raw._width; \
              } \
            } \
          } \
        } \
        loaded = true; \
      }

		if (!filename && !file)
			throw CImgArgumentException(_cimglist_instance
				"load_cimg(): Specified filename is (null).",
				cimglist_instance);

		const ulongT cimg_iobuffer = (ulongT)24 * 1024 * 1024;
		std::FILE *const nfile = file ? file : cimg::fopen(filename, "rb");
		bool loaded = false, endian = cimg::endianness();
		CImg<charT> tmp(256), str_pixeltype(256), str_endian(256);
		*tmp = *str_pixeltype = *str_endian = 0;
		unsigned int j, N = 0, W, H, D, C;
		unsigned long csiz;
		int i, err;
		do {
			j = 0; while ((i = std::fgetc(nfile)) != '\n' && i >= 0 && j<255) tmp[j++] = (char)i; tmp[j] = 0;
		} while (*tmp == '#' && i >= 0);
		err = cimg_sscanf(tmp, "%u%*c%255[A-Za-z64_]%*c%255[sA-Za-z_ ]",
			&N, str_pixeltype._data, str_endian._data);
		if (err<2) {
			if (!file) cimg::fclose(nfile);
			throw CImgIOException(_cimglist_instance
				"load_cimg(): CImg header not found in file '%s'.",
				cimglist_instance,
				filename ? filename : "(FILE*)");
		}
		if (!cimg::strncasecmp("little", str_endian, 6)) endian = false;
		else if (!cimg::strncasecmp("big", str_endian, 3)) endian = true;
		assign(N);
		_cimg_load_cimg_case("bool", bool);
		_cimg_load_cimg_case("unsigned_char", unsigned char);
		_cimg_load_cimg_case("uchar", unsigned char);
		_cimg_load_cimg_case("char", char);
		_cimg_load_cimg_case("unsigned_short", unsigned short);
		_cimg_load_cimg_case("ushort", unsigned short);
		_cimg_load_cimg_case("short", short);
		_cimg_load_cimg_case("unsigned_int", unsigned int);
		_cimg_load_cimg_case("uint", unsigned int);
		_cimg_load_cimg_case("int", int);
		_cimg_load_cimg_case("unsigned_long", ulongT);
		_cimg_load_cimg_case("ulong", ulongT);
		_cimg_load_cimg_case("long", longT);
		_cimg_load_cimg_case("unsigned_int64", uint64T);
		_cimg_load_cimg_case("uint64", uint64T);
		_cimg_load_cimg_case("int64", int64T);
		_cimg_load_cimg_case("float", float);
		_cimg_load_cimg_case("double", double);

		if (!loaded) {
			if (!file) cimg::fclose(nfile);
			throw CImgIOException(_cimglist_instance
				"load_cimg(): Unsupported pixel type '%s' for file '%s'.",
				cimglist_instance,
				str_pixeltype._data, filename ? filename : "(FILE*)");
		}
		if (!file) cimg::fclose(nfile);
		return *this;
	}

	//! Load a sublist list from a (non compressed) .cimg file.
	/**
	\param filename Filename to read data from.
	\param n0 Starting index of images to read (~0U for max).
	\param n1 Ending index of images to read (~0U for max).
	\param x0 Starting X-coordinates of image regions to read.
	\param y0 Starting Y-coordinates of image regions to read.
	\param z0 Starting Z-coordinates of image regions to read.
	\param c0 Starting C-coordinates of image regions to read.
	\param x1 Ending X-coordinates of image regions to read (~0U for max).
	\param y1 Ending Y-coordinates of image regions to read (~0U for max).
	\param z1 Ending Z-coordinates of image regions to read (~0U for max).
	\param c1 Ending C-coordinates of image regions to read (~0U for max).
	**/
	CImgList<T>& load_cimg(const char *const filename,
		const unsigned int n0, const unsigned int n1,
		const unsigned int x0, const unsigned int y0,
		const unsigned int z0, const unsigned int c0,
		const unsigned int x1, const unsigned int y1,
		const unsigned int z1, const unsigned int c1) {
		return _load_cimg(0, filename, n0, n1, x0, y0, z0, c0, x1, y1, z1, c1);
	}

	//! Load a sublist list from a (non compressed) .cimg file \newinstance.
	static CImgList<T> get_load_cimg(const char *const filename,
		const unsigned int n0, const unsigned int n1,
		const unsigned int x0, const unsigned int y0,
		const unsigned int z0, const unsigned int c0,
		const unsigned int x1, const unsigned int y1,
		const unsigned int z1, const unsigned int c1) {
		return CImgList<T>().load_cimg(filename, n0, n1, x0, y0, z0, c0, x1, y1, z1, c1);
	}

	//! Load a sub-image list from a (non compressed) .cimg file \overloading.
	CImgList<T>& load_cimg(std::FILE *const file,
		const unsigned int n0, const unsigned int n1,
		const unsigned int x0, const unsigned int y0,
		const unsigned int z0, const unsigned int c0,
		const unsigned int x1, const unsigned int y1,
		const unsigned int z1, const unsigned int c1) {
		return _load_cimg(file, 0, n0, n1, x0, y0, z0, c0, x1, y1, z1, c1);
	}

	//! Load a sub-image list from a (non compressed) .cimg file \newinstance.
	static CImgList<T> get_load_cimg(std::FILE *const file,
		const unsigned int n0, const unsigned int n1,
		const unsigned int x0, const unsigned int y0,
		const unsigned int z0, const unsigned int c0,
		const unsigned int x1, const unsigned int y1,
		const unsigned int z1, const unsigned int c1) {
		return CImgList<T>().load_cimg(file, n0, n1, x0, y0, z0, c0, x1, y1, z1, c1);
	}

	CImgList<T>& _load_cimg(std::FILE *const file, const char *const filename,
		const unsigned int n0, const unsigned int n1,
		const unsigned int x0, const unsigned int y0,
		const unsigned int z0, const unsigned int c0,
		const unsigned int x1, const unsigned int y1,
		const unsigned int z1, const unsigned int c1) {
#define _cimg_load_cimg_case2(Ts,Tss) \
      if (!loaded && !cimg::strcasecmp(Ts,str_pixeltype)) { \
        for (unsigned int l = 0; l<=nn1; ++l) { \
          j = 0; while ((i=std::fgetc(nfile))!='\n' && i>=0) tmp[j++] = (char)i; tmp[j] = 0; \
          W = H = D = C = 0; \
          if (cimg_sscanf(tmp,"%u %u %u %u",&W,&H,&D,&C)!=4) \
            throw CImgIOException(_cimglist_instance \
                                  "load_cimg(): Invalid specified size (%u,%u,%u,%u) of image %u in file '%s'", \
                                  cimglist_instance, \
                                  W,H,D,C,l,filename?filename:"(FILE*)"); \
          if (W*H*D*C>0) { \
            if (l<nn0 || nx0>=W || ny0>=H || nz0>=D || nc0>=C) cimg::fseek(nfile,W*H*D*C*sizeof(Tss),SEEK_CUR); \
            else { \
              const unsigned int \
                _nx1 = nx1==~0U?W - 1:nx1, \
                _ny1 = ny1==~0U?H - 1:ny1, \
                _nz1 = nz1==~0U?D - 1:nz1, \
                _nc1 = nc1==~0U?C - 1:nc1; \
              if (_nx1>=W || _ny1>=H || _nz1>=D || _nc1>=C) \
                throw CImgArgumentException(_cimglist_instance \
                                            "load_cimg(): Invalid specified coordinates " \
                                            "[%u](%u,%u,%u,%u) -> [%u](%u,%u,%u,%u) " \
                                            "because image [%u] in file '%s' has size (%u,%u,%u,%u).", \
                                            cimglist_instance, \
                                            n0,x0,y0,z0,c0,n1,x1,y1,z1,c1,l,filename?filename:"(FILE*)",W,H,D,C); \
              CImg<Tss> raw(1 + _nx1 - nx0); \
              CImg<T> &img = _data[l - nn0]; \
              img.assign(1 + _nx1 - nx0,1 + _ny1 - ny0,1 + _nz1 - nz0,1 + _nc1 - nc0); \
              T *ptrd = img._data; \
              ulongT skipvb = nc0*W*H*D*sizeof(Tss); \
              if (skipvb) cimg::fseek(nfile,skipvb,SEEK_CUR); \
              for (unsigned int c = 1 + _nc1 - nc0; c; --c) { \
                const ulongT skipzb = nz0*W*H*sizeof(Tss); \
                if (skipzb) cimg::fseek(nfile,skipzb,SEEK_CUR); \
                for (unsigned int z = 1 + _nz1 - nz0; z; --z) { \
                  const ulongT skipyb = ny0*W*sizeof(Tss); \
                  if (skipyb) cimg::fseek(nfile,skipyb,SEEK_CUR); \
                  for (unsigned int y = 1 + _ny1 - ny0; y; --y) { \
                    const ulongT skipxb = nx0*sizeof(Tss); \
                    if (skipxb) cimg::fseek(nfile,skipxb,SEEK_CUR); \
                    cimg::fread(raw._data,raw._width,nfile); \
                    if (endian!=cimg::endianness()) cimg::invert_endianness(raw._data,raw._width); \
                    const Tss *ptrs = raw._data; \
                    for (unsigned int off = raw._width; off; --off) *(ptrd++) = (T)*(ptrs++); \
                    const ulongT skipxe = (W - 1 - _nx1)*sizeof(Tss); \
                    if (skipxe) cimg::fseek(nfile,skipxe,SEEK_CUR); \
                  } \
                  const ulongT skipye = (H - 1 - _ny1)*W*sizeof(Tss); \
                  if (skipye) cimg::fseek(nfile,skipye,SEEK_CUR); \
                } \
                const ulongT skipze = (D - 1 - _nz1)*W*H*sizeof(Tss); \
                if (skipze) cimg::fseek(nfile,skipze,SEEK_CUR); \
              } \
              const ulongT skipve = (C - 1 - _nc1)*W*H*D*sizeof(Tss); \
              if (skipve) cimg::fseek(nfile,skipve,SEEK_CUR); \
            } \
          } \
        } \
        loaded = true; \
      }

		if (!filename && !file)
			throw CImgArgumentException(_cimglist_instance
				"load_cimg(): Specified filename is (null).",
				cimglist_instance);
		unsigned int
			nn0 = std::min(n0, n1), nn1 = std::max(n0, n1),
			nx0 = std::min(x0, x1), nx1 = std::max(x0, x1),
			ny0 = std::min(y0, y1), ny1 = std::max(y0, y1),
			nz0 = std::min(z0, z1), nz1 = std::max(z0, z1),
			nc0 = std::min(c0, c1), nc1 = std::max(c0, c1);

		std::FILE *const nfile = file ? file : cimg::fopen(filename, "rb");
		bool loaded = false, endian = cimg::endianness();
		CImg<charT> tmp(256), str_pixeltype(256), str_endian(256);
		*tmp = *str_pixeltype = *str_endian = 0;
		unsigned int j, N, W, H, D, C;
		int i, err;
		j = 0; while ((i = std::fgetc(nfile)) != '\n' && i != EOF && j<256) tmp[j++] = (char)i; tmp[j] = 0;
		err = cimg_sscanf(tmp, "%u%*c%255[A-Za-z64_]%*c%255[sA-Za-z_ ]",
			&N, str_pixeltype._data, str_endian._data);
		if (err<2) {
			if (!file) cimg::fclose(nfile);
			throw CImgIOException(_cimglist_instance
				"load_cimg(): CImg header not found in file '%s'.",
				cimglist_instance,
				filename ? filename : "(FILE*)");
		}
		if (!cimg::strncasecmp("little", str_endian, 6)) endian = false;
		else if (!cimg::strncasecmp("big", str_endian, 3)) endian = true;
		nn1 = n1 == ~0U ? N - 1 : n1;
		if (nn1 >= N)
			throw CImgArgumentException(_cimglist_instance
				"load_cimg(): Invalid specified coordinates [%u](%u,%u,%u,%u) -> [%u](%u,%u,%u,%u) "
				"because file '%s' contains only %u images.",
				cimglist_instance,
				n0, x0, y0, z0, c0, n1, x1, y1, z1, c1, filename ? filename : "(FILE*)", N);
		assign(1 + nn1 - n0);
		_cimg_load_cimg_case2("bool", bool);
		_cimg_load_cimg_case2("unsigned_char", unsigned char);
		_cimg_load_cimg_case2("uchar", unsigned char);
		_cimg_load_cimg_case2("char", char);
		_cimg_load_cimg_case2("unsigned_short", unsigned short);
		_cimg_load_cimg_case2("ushort", unsigned short);
		_cimg_load_cimg_case2("short", short);
		_cimg_load_cimg_case2("unsigned_int", unsigned int);
		_cimg_load_cimg_case2("uint", unsigned int);
		_cimg_load_cimg_case2("int", int);
		_cimg_load_cimg_case2("unsigned_long", ulongT);
		_cimg_load_cimg_case2("ulong", ulongT);
		_cimg_load_cimg_case2("long", longT);
		_cimg_load_cimg_case2("unsigned_int64", uint64T);
		_cimg_load_cimg_case2("uint64", uint64T);
		_cimg_load_cimg_case2("int64", int64T);
		_cimg_load_cimg_case2("float", float);
		_cimg_load_cimg_case2("double", double);
		if (!loaded) {
			if (!file) cimg::fclose(nfile);
			throw CImgIOException(_cimglist_instance
				"load_cimg(): Unsupported pixel type '%s' for file '%s'.",
				cimglist_instance,
				str_pixeltype._data, filename ? filename : "(FILE*)");
		}
		if (!file) cimg::fclose(nfile);
		return *this;
	}

	//! Load a list from a PAR/REC (Philips) file.
	/**
	\param filename Filename to read data from.
	**/
	CImgList<T>& load_parrec(const char *const filename) {
		if (!filename)
			throw CImgArgumentException(_cimglist_instance
				"load_parrec(): Specified filename is (null).",
				cimglist_instance);

		CImg<charT> body(1024), filenamepar(1024), filenamerec(1024);
		*body = *filenamepar = *filenamerec = 0;
		const char *const ext = cimg::split_filename(filename, body);
		if (!std::strcmp(ext, "par")) {
			std::strncpy(filenamepar, filename, filenamepar._width - 1);
			cimg_snprintf(filenamerec, filenamerec._width, "%s.rec", body._data);
		}
		if (!std::strcmp(ext, "PAR")) {
			std::strncpy(filenamepar, filename, filenamepar._width - 1);
			cimg_snprintf(filenamerec, filenamerec._width, "%s.REC", body._data);
		}
		if (!std::strcmp(ext, "rec")) {
			std::strncpy(filenamerec, filename, filenamerec._width - 1);
			cimg_snprintf(filenamepar, filenamepar._width, "%s.par", body._data);
		}
		if (!std::strcmp(ext, "REC")) {
			std::strncpy(filenamerec, filename, filenamerec._width - 1);
			cimg_snprintf(filenamepar, filenamepar._width, "%s.PAR", body._data);
		}
		std::FILE *file = cimg::fopen(filenamepar, "r");

		// Parse header file
		CImgList<floatT> st_slices;
		CImgList<uintT> st_global;
		CImg<charT> line(256); *line = 0;
		int err;
		do { err = std::fscanf(file, "%255[^\n]%*c", line._data); } while (err != EOF && (*line == '#' || *line == '.'));
		do {
			unsigned int sn, size_x, size_y, pixsize;
			float rs, ri, ss;
			err = std::fscanf(file, "%u%*u%*u%*u%*u%*u%*u%u%*u%u%u%g%g%g%*[^\n]", &sn, &pixsize, &size_x, &size_y, &ri, &rs, &ss);
			if (err == 7) {
				CImg<floatT>::vector((float)sn, (float)pixsize, (float)size_x, (float)size_y, ri, rs, ss, 0).move_to(st_slices);
				unsigned int i; for (i = 0; i<st_global._width && sn <= st_global[i][2]; ++i) {}
				if (i == st_global._width) CImg<uintT>::vector(size_x, size_y, sn).move_to(st_global);
				else {
					CImg<uintT> &vec = st_global[i];
					if (size_x>vec[0]) vec[0] = size_x;
					if (size_y>vec[1]) vec[1] = size_y;
					vec[2] = sn;
				}
				st_slices[st_slices._width - 1][7] = (float)i;
			}
		} while (err == 7);

		// Read data
		std::FILE *file2 = cimg::fopen(filenamerec, "rb");
		cimglist_for(st_global, l) {
			const CImg<uintT>& vec = st_global[l];
			CImg<T>(vec[0], vec[1], vec[2]).move_to(*this);
		}

		cimglist_for(st_slices, l) {
			const CImg<floatT>& vec = st_slices[l];
			const unsigned int
				sn = (unsigned int)vec[0] - 1,
				pixsize = (unsigned int)vec[1],
				size_x = (unsigned int)vec[2],
				size_y = (unsigned int)vec[3],
				imn = (unsigned int)vec[7];
			const float ri = vec[4], rs = vec[5], ss = vec[6];
			switch (pixsize) {
			case 8: {
				CImg<ucharT> buf(size_x, size_y);
				cimg::fread(buf._data, size_x*size_y, file2);
				if (cimg::endianness()) cimg::invert_endianness(buf._data, size_x*size_y);
				CImg<T>& img = (*this)[imn];
				cimg_forXY(img, x, y) img(x, y, sn) = (T)((buf(x, y)*rs + ri) / (rs*ss));
			} break;
			case 16: {
				CImg<ushortT> buf(size_x, size_y);
				cimg::fread(buf._data, size_x*size_y, file2);
				if (cimg::endianness()) cimg::invert_endianness(buf._data, size_x*size_y);
				CImg<T>& img = (*this)[imn];
				cimg_forXY(img, x, y) img(x, y, sn) = (T)((buf(x, y)*rs + ri) / (rs*ss));
			} break;
			case 32: {
				CImg<uintT> buf(size_x, size_y);
				cimg::fread(buf._data, size_x*size_y, file2);
				if (cimg::endianness()) cimg::invert_endianness(buf._data, size_x*size_y);
				CImg<T>& img = (*this)[imn];
				cimg_forXY(img, x, y) img(x, y, sn) = (T)((buf(x, y)*rs + ri) / (rs*ss));
			} break;
			default:
				cimg::fclose(file);
				cimg::fclose(file2);
				throw CImgIOException(_cimglist_instance
					"load_parrec(): Unsupported %d-bits pixel type for file '%s'.",
					cimglist_instance,
					pixsize, filename);
			}
		}
		cimg::fclose(file);
		cimg::fclose(file2);
		if (!_width)
			throw CImgIOException(_cimglist_instance
				"load_parrec(): Failed to recognize valid PAR-REC data in file '%s'.",
				cimglist_instance,
				filename);
		return *this;
	}

	//! Load a list from a PAR/REC (Philips) file \newinstance.
	static CImgList<T> get_load_parrec(const char *const filename) {
		return CImgList<T>().load_parrec(filename);
	}

	//! Load a list from a YUV image sequence file.
	/**
	\param filename Filename to read data from.
	\param size_x Width of the images.
	\param size_y Height of the images.
	\param first_frame Index of first image frame to read.
	\param last_frame Index of last image frame to read.
	\param step_frame Step applied between each frame.
	\param yuv2rgb Apply YUV to RGB transformation during reading.
	**/
	CImgList<T>& load_yuv(const char *const filename,
		const unsigned int size_x, const unsigned int size_y,
		const unsigned int first_frame = 0, const unsigned int last_frame = ~0U,
		const unsigned int step_frame = 1, const bool yuv2rgb = true) {
		return _load_yuv(0, filename, size_x, size_y, first_frame, last_frame, step_frame, yuv2rgb);
	}

	//! Load a list from a YUV image sequence file \newinstance.
	static CImgList<T> get_load_yuv(const char *const filename,
		const unsigned int size_x, const unsigned int size_y = 1,
		const unsigned int first_frame = 0, const unsigned int last_frame = ~0U,
		const unsigned int step_frame = 1, const bool yuv2rgb = true) {
		return CImgList<T>().load_yuv(filename, size_x, size_y, first_frame, last_frame, step_frame, yuv2rgb);
	}

	//! Load a list from an image sequence YUV file \overloading.
	CImgList<T>& load_yuv(std::FILE *const file,
		const unsigned int size_x, const unsigned int size_y,
		const unsigned int first_frame = 0, const unsigned int last_frame = ~0U,
		const unsigned int step_frame = 1, const bool yuv2rgb = true) {
		return _load_yuv(file, 0, size_x, size_y, first_frame, last_frame, step_frame, yuv2rgb);
	}

	//! Load a list from an image sequence YUV file \newinstance.
	static CImgList<T> get_load_yuv(std::FILE *const file,
		const unsigned int size_x, const unsigned int size_y = 1,
		const unsigned int first_frame = 0, const unsigned int last_frame = ~0U,
		const unsigned int step_frame = 1, const bool yuv2rgb = true) {
		return CImgList<T>().load_yuv(file, size_x, size_y, first_frame, last_frame, step_frame, yuv2rgb);
	}

	CImgList<T>& _load_yuv(std::FILE *const file, const char *const filename,
		const unsigned int size_x, const unsigned int size_y,
		const unsigned int first_frame, const unsigned int last_frame,
		const unsigned int step_frame, const bool yuv2rgb) {
		if (!filename && !file)
			throw CImgArgumentException(_cimglist_instance
				"load_yuv(): Specified filename is (null).",
				cimglist_instance);
		if (size_x % 2 || size_y % 2)
			throw CImgArgumentException(_cimglist_instance
				"load_yuv(): Invalid odd XY dimensions %ux%u in file '%s'.",
				cimglist_instance,
				size_x, size_y, filename ? filename : "(FILE*)");
		if (!size_x || !size_y)
			throw CImgArgumentException(_cimglist_instance
				"load_yuv(): Invalid sequence size (%u,%u) in file '%s'.",
				cimglist_instance,
				size_x, size_y, filename ? filename : "(FILE*)");

		const unsigned int
			nfirst_frame = first_frame<last_frame ? first_frame : last_frame,
			nlast_frame = first_frame<last_frame ? last_frame : first_frame,
			nstep_frame = step_frame ? step_frame : 1;

		CImg<ucharT> tmp(size_x, size_y, 1, 3), UV(size_x / 2, size_y / 2, 1, 2);
		std::FILE *const nfile = file ? file : cimg::fopen(filename, "rb");
		bool stop_flag = false;
		int err;
		if (nfirst_frame) {
			err = cimg::fseek(nfile, nfirst_frame*(size_x*size_y + size_x*size_y / 2), SEEK_CUR);
			if (err) {
				if (!file) cimg::fclose(nfile);
				throw CImgIOException(_cimglist_instance
					"load_yuv(): File '%s' doesn't contain frame number %u.",
					cimglist_instance,
					filename ? filename : "(FILE*)", nfirst_frame);
			}
		}
		unsigned int frame;
		for (frame = nfirst_frame; !stop_flag && frame <= nlast_frame; frame += nstep_frame) {
			tmp.fill(0);
			// *TRY* to read the luminance part, do not replace by cimg::fread!
			err = (int)std::fread((void*)(tmp._data), 1, (ulongT)tmp._width*tmp._height, nfile);
			if (err != (int)(tmp._width*tmp._height)) {
				stop_flag = true;
				if (err>0)
					cimg::warn(_cimglist_instance
						"load_yuv(): File '%s' contains incomplete data or given image dimensions "
						"(%u,%u) are incorrect.",
						cimglist_instance,
						filename ? filename : "(FILE*)", size_x, size_y);
			}
			else {
				UV.fill(0);
				// *TRY* to read the luminance part, do not replace by cimg::fread!
				err = (int)std::fread((void*)(UV._data), 1, (size_t)(UV.size()), nfile);
				if (err != (int)(UV.size())) {
					stop_flag = true;
					if (err>0)
						cimg::warn(_cimglist_instance
							"load_yuv(): File '%s' contains incomplete data or given image dimensions (%u,%u) "
							"are incorrect.",
							cimglist_instance,
							filename ? filename : "(FILE*)", size_x, size_y);
				}
				else {
					cimg_forXY(UV, x, y) {
						const int x2 = x * 2, y2 = y * 2;
						tmp(x2, y2, 1) = tmp(x2 + 1, y2, 1) = tmp(x2, y2 + 1, 1) = tmp(x2 + 1, y2 + 1, 1) = UV(x, y, 0);
						tmp(x2, y2, 2) = tmp(x2 + 1, y2, 2) = tmp(x2, y2 + 1, 2) = tmp(x2 + 1, y2 + 1, 2) = UV(x, y, 1);
					}
					if (yuv2rgb) tmp.YCbCrtoRGB();
					insert(tmp);
					if (nstep_frame>1) cimg::fseek(nfile, (nstep_frame - 1)*(size_x*size_y + size_x*size_y / 2), SEEK_CUR);
				}
			}
		}
		if (stop_flag && nlast_frame != ~0U && frame != nlast_frame)
			cimg::warn(_cimglist_instance
				"load_yuv(): Frame %d not reached since only %u frames were found in file '%s'.",
				cimglist_instance,
				nlast_frame, frame - 1, filename ? filename : "(FILE*)");

		if (!file) cimg::fclose(nfile);
		return *this;
	}

	//! Load an image from a video file, using OpenCV library.
	/**
	\param filename Filename, as a C-string.
	\param first_frame Index of the first frame to read.
	\param last_frame Index of the last frame to read.
	\param step_frame Step value for frame reading.
	\note If step_frame==0, the current video stream is forced to be released (without any frames read).
	**/
	CImgList<T>& load_video(const char *const filename,
		const unsigned int first_frame = 0, const unsigned int last_frame = ~0U,
		const unsigned int step_frame = 1) {
#ifndef cimg_use_opencv
		if (first_frame || last_frame != ~0U || step_frame>1)
			throw CImgArgumentException(_cimglist_instance
				"load_video() : File '%s', arguments 'first_frame', 'last_frame' "
				"and 'step_frame' can be only set when using OpenCV "
				"(-Dcimg_use_opencv must be enabled).",
				cimglist_instance, filename);
		return load_ffmpeg_external(filename);
#else
		static CvCapture *captures[32] = { 0 };
		static CImgList<charT> filenames(32);
		static CImg<uintT> positions(32, 1, 1, 1, 0);
		static int last_used_index = -1;

		// Detect if a video capture already exists for the specified filename.
		cimg::mutex(9);
		int index = -1;
		if (filename) {
			if (last_used_index >= 0 && !std::strcmp(filename, filenames[last_used_index])) {
				index = last_used_index;
			}
			else cimglist_for(filenames, l) if (filenames[l] && !std::strcmp(filename, filenames[l])) {
				index = l; break;
			}
		}
		else index = last_used_index;
		cimg::mutex(9, 0);

		// Release stream if needed.
		if (!step_frame || (index >= 0 && positions[index]>first_frame)) {
			if (index >= 0) {
				cimg::mutex(9);
				cvReleaseCapture(&captures[index]);
				captures[index] = 0; filenames[index].assign(); positions[index] = 0;
				if (last_used_index == index) last_used_index = -1;
				index = -1;
				cimg::mutex(9, 0);
			}
			else
				if (filename)
					cimg::warn(_cimglist_instance
						"load_video() : File '%s', no opened video stream associated with filename found.",
						cimglist_instance, filename);
				else
					cimg::warn(_cimglist_instance
						"load_video() : No opened video stream found.",
						cimglist_instance, filename);
			if (!step_frame) return *this;
		}

		// Find empty slot for capturing video stream.
		if (index<0) {
			if (!filename)
				throw CImgArgumentException(_cimglist_instance
					"load_video(): No already open video reader found. You must specify a "
					"non-(null) filename argument for the first call.",
					cimglist_instance);
			else { cimg::mutex(9); cimglist_for(filenames, l) if (!filenames[l]) { index = l; break; } cimg::mutex(9, 0); }
			if (index<0)
				throw CImgIOException(_cimglist_instance
					"load_video(): File '%s', no video reader slots available. "
					"You have to release some of your previously opened videos.",
					cimglist_instance, filename);
			cimg::mutex(9);
			captures[index] = cvCaptureFromFile(filename);
			CImg<charT>::string(filename).move_to(filenames[index]);
			positions[index] = 0;
			cimg::mutex(9, 0);
			if (!captures[index]) {
				filenames[index].assign();
				std::fclose(cimg::fopen(filename, "rb"));  // Check file availability.
				throw CImgIOException(_cimglist_instance
					"load_video(): File '%s', unable to detect format of video file.",
					cimglist_instance, filename);
			}
		}

		cimg::mutex(9);
		const unsigned int nb_frames = (unsigned int)std::max(0., cvGetCaptureProperty(captures[index],
			CV_CAP_PROP_FRAME_COUNT));
		cimg::mutex(9, 0);
		assign();

		// Skip frames if necessary.
		bool go_on = true;
		unsigned int &pos = positions[index];
		while (pos<first_frame) {
			cimg::mutex(9);
			if (!cvGrabFrame(captures[index])) { cimg::mutex(9, 0); go_on = false; break; }
			cimg::mutex(9, 0);
			++pos;
		}

		// Read and convert frames.
		const IplImage *src = 0;
		if (go_on) {
			const unsigned int _last_frame = std::min(nb_frames ? nb_frames - 1 : ~0U, last_frame);
			while (pos <= _last_frame) {
				cimg::mutex(9);
				src = cvQueryFrame(captures[index]);
				if (src) {
					CImg<T> frame(src->width, src->height, 1, 3);
					const int step = (int)(src->widthStep - 3 * src->width);
					const unsigned char* ptrs = (unsigned char*)src->imageData;
					T *ptr_r = frame.data(0, 0, 0, 0), *ptr_g = frame.data(0, 0, 0, 1), *ptr_b = frame.data(0, 0, 0, 2);
					if (step>0) cimg_forY(frame, y) {
						cimg_forX(frame, x) { *(ptr_b++) = (T)*(ptrs++); *(ptr_g++) = (T)*(ptrs++); *(ptr_r++) = (T)*(ptrs++); }
						ptrs += step;
					}
					else for (ulongT siz = (ulongT)src->width*src->height; siz; --siz) {
						*(ptr_b++) = (T)*(ptrs++); *(ptr_g++) = (T)*(ptrs++); *(ptr_r++) = (T)*(ptrs++);
					}
					frame.move_to(*this);
					++pos;

					bool skip_failed = false;
					for (unsigned int i = 1; i<step_frame && pos <= _last_frame; ++i, ++pos)
						if (!cvGrabFrame(captures[index])) { skip_failed = true; break; }
					if (skip_failed) src = 0;
				}
				cimg::mutex(9, 0);
				if (!src) break;
			}
		}

		if (!src || (nb_frames && pos >= nb_frames)) { // Close video stream when necessary.
			cimg::mutex(9);
			cvReleaseCapture(&captures[index]);
			captures[index] = 0;
			filenames[index].assign();
			positions[index] = 0;
			index = -1;
			cimg::mutex(9, 0);
		}

		cimg::mutex(9);
		last_used_index = index;
		cimg::mutex(9, 0);

		if (is_empty())
			throw CImgIOException(_cimglist_instance
				"load_video(): File '%s', unable to locate frame %u.",
				cimglist_instance, filename, first_frame);
		return *this;
#endif
	}

	//! Load an image from a video file, using OpenCV library \newinstance.
	static CImgList<T> get_load_video(const char *const filename,
		const unsigned int first_frame = 0, const unsigned int last_frame = ~0U,
		const unsigned int step_frame = 1) {
		return CImgList<T>().load_video(filename, first_frame, last_frame, step_frame);
	}

	//! Load an image from a video file using the external tool 'ffmpeg'.
	/**
	\param filename Filename to read data from.
	**/
	CImgList<T>& load_ffmpeg_external(const char *const filename) {
		if (!filename)
			throw CImgArgumentException(_cimglist_instance
				"load_ffmpeg_external(): Specified filename is (null).",
				cimglist_instance);
		std::fclose(cimg::fopen(filename, "rb"));            // Check if file exists.
		CImg<charT> command(1024), filename_tmp(256), filename_tmp2(256);
		std::FILE *file = 0;
		do {
			cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s",
				cimg::temporary_path(), cimg_file_separator, cimg::filenamerand());
			cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s_000001.ppm", filename_tmp._data);
			if ((file = std_fopen(filename_tmp2, "rb")) != 0) cimg::fclose(file);
		} while (file);
		cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s_%%6d.ppm", filename_tmp._data);
#if cimg_OS!=2
		cimg_snprintf(command, command._width, "%s -i \"%s\" \"%s\" >/dev/null 2>&1",
			cimg::ffmpeg_path(),
			CImg<charT>::string(filename)._system_strescape().data(),
			CImg<charT>::string(filename_tmp2)._system_strescape().data());
#else
		cimg_snprintf(command, command._width, "\"%s -i \"%s\" \"%s\"\" >NUL 2>&1",
			cimg::ffmpeg_path(),
			CImg<charT>::string(filename)._system_strescape().data(),
			CImg<charT>::string(filename_tmp2)._system_strescape().data());
#endif
		cimg::system(command, 0);
		const unsigned int omode = cimg::exception_mode();
		cimg::exception_mode(0);
		assign();
		unsigned int i = 1;
		for (bool stop_flag = false; !stop_flag; ++i) {
			cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s_%.6u.ppm", filename_tmp._data, i);
			CImg<T> img;
			try { img.load_pnm(filename_tmp2); }
			catch (CImgException&) { stop_flag = true; }
			if (img) { img.move_to(*this); std::remove(filename_tmp2); }
		}
		cimg::exception_mode(omode);
		if (is_empty())
			throw CImgIOException(_cimglist_instance
				"load_ffmpeg_external(): Failed to open file '%s' with external command 'ffmpeg'.",
				cimglist_instance,
				filename);
		return *this;
	}

	//! Load an image from a video file using the external tool 'ffmpeg' \newinstance.
	static CImgList<T> get_load_ffmpeg_external(const char *const filename) {
		return CImgList<T>().load_ffmpeg_external(filename);
	}

	//! Load gif file, using ImageMagick or GraphicsMagick's external tools.
	/**
	\param filename Filename to read data from.
	**/
	CImgList<T>& load_gif_external(const char *const filename) {
		if (!filename)
			throw CImgArgumentException(_cimglist_instance
				"load_gif_external(): Specified filename is (null).",
				cimglist_instance);
		std::fclose(cimg::fopen(filename, "rb"));            // Check if file exists.
		if (!_load_gif_external(filename, false))
			if (!_load_gif_external(filename, true))
				try { assign(CImg<T>().load_other(filename)); }
		catch (CImgException&) { assign(); }
		if (is_empty())
			throw CImgIOException(_cimglist_instance
				"load_gif_external(): Failed to open file '%s'.",
				cimglist_instance, filename);
		return *this;
	}

	CImgList<T>& _load_gif_external(const char *const filename, const bool use_graphicsmagick = false) {
		CImg<charT> command(1024), filename_tmp(256), filename_tmp2(256);
		std::FILE *file = 0;
		do {
			cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s",
				cimg::temporary_path(), cimg_file_separator, cimg::filenamerand());
			if (use_graphicsmagick) cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s.png.0", filename_tmp._data);
			else cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s-0.png", filename_tmp._data);
			if ((file = std_fopen(filename_tmp2, "rb")) != 0) cimg::fclose(file);
		} while (file);
#if cimg_OS!=2
		if (use_graphicsmagick) cimg_snprintf(command, command._width, "%s convert \"%s\" \"%s.png\" >/dev/null 2>&1",
			cimg::graphicsmagick_path(),
			CImg<charT>::string(filename)._system_strescape().data(),
			CImg<charT>::string(filename_tmp)._system_strescape().data());
		else cimg_snprintf(command, command._width, "%s \"%s\" \"%s.png\" >/dev/null 2>&1",
			cimg::imagemagick_path(),
			CImg<charT>::string(filename)._system_strescape().data(),
			CImg<charT>::string(filename_tmp)._system_strescape().data());
#else
		if (use_graphicsmagick) cimg_snprintf(command, command._width, "\"%s convert \"%s\" \"%s.png\"\" >NUL 2>&1",
			cimg::graphicsmagick_path(),
			CImg<charT>::string(filename)._system_strescape().data(),
			CImg<charT>::string(filename_tmp)._system_strescape().data());
		else cimg_snprintf(command, command._width, "\"%s \"%s\" \"%s.png\"\" >NUL 2>&1",
			cimg::imagemagick_path(),
			CImg<charT>::string(filename)._system_strescape().data(),
			CImg<charT>::string(filename_tmp)._system_strescape().data());
#endif
		cimg::system(command, 0);
		const unsigned int omode = cimg::exception_mode();
		cimg::exception_mode(0);
		assign();

		// Try to read a single frame gif.
		cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s.png", filename_tmp._data);
		CImg<T> img;
		try { img.load_png(filename_tmp2); }
		catch (CImgException&) {}
		if (img) { img.move_to(*this); std::remove(filename_tmp2); }
		else { // Try to read animated gif.
			unsigned int i = 0;
			for (bool stop_flag = false; !stop_flag; ++i) {
				if (use_graphicsmagick) cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s.png.%u", filename_tmp._data, i);
				else cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s-%u.png", filename_tmp._data, i);
				CImg<T> img;
				try { img.load_png(filename_tmp2); }
				catch (CImgException&) { stop_flag = true; }
				if (img) { img.move_to(*this); std::remove(filename_tmp2); }
			}
		}
		cimg::exception_mode(omode);
		return *this;
	}

	//! Load gif file, using ImageMagick or GraphicsMagick's external tools \newinstance.
	static CImgList<T> get_load_gif_external(const char *const filename) {
		return CImgList<T>().load_gif_external(filename);
	}

	//! Load a gzipped list, using external tool 'gunzip'.
	/**
	\param filename Filename to read data from.
	**/
	CImgList<T>& load_gzip_external(const char *const filename) {
		if (!filename)
			throw CImgIOException(_cimglist_instance
				"load_gzip_external(): Specified filename is (null).",
				cimglist_instance);
		std::fclose(cimg::fopen(filename, "rb"));            // Check if file exists.
		CImg<charT> command(1024), filename_tmp(256), body(256);
		const char
			*ext = cimg::split_filename(filename, body),
			*ext2 = cimg::split_filename(body, 0);
		std::FILE *file = 0;
		do {
			if (!cimg::strcasecmp(ext, "gz")) {
				if (*ext2) cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s.%s",
					cimg::temporary_path(), cimg_file_separator, cimg::filenamerand(), ext2);
				else cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s",
					cimg::temporary_path(), cimg_file_separator, cimg::filenamerand());
			}
			else {
				if (*ext) cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s.%s",
					cimg::temporary_path(), cimg_file_separator, cimg::filenamerand(), ext);
				else cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s",
					cimg::temporary_path(), cimg_file_separator, cimg::filenamerand());
			}
			if ((file = std_fopen(filename_tmp, "rb")) != 0) cimg::fclose(file);
		} while (file);
		cimg_snprintf(command, command._width, "%s -c \"%s\" > \"%s\"",
			cimg::gunzip_path(),
			CImg<charT>::string(filename)._system_strescape().data(),
			CImg<charT>::string(filename_tmp)._system_strescape().data());
		cimg::system(command);
		if (!(file = std_fopen(filename_tmp, "rb"))) {
			cimg::fclose(cimg::fopen(filename, "r"));
			throw CImgIOException(_cimglist_instance
				"load_gzip_external(): Failed to open file '%s'.",
				cimglist_instance,
				filename);

		}
		else cimg::fclose(file);
		load(filename_tmp);
		std::remove(filename_tmp);
		return *this;
	}

	//! Load a gzipped list, using external tool 'gunzip' \newinstance.
	static CImgList<T> get_load_gzip_external(const char *const filename) {
		return CImgList<T>().load_gzip_external(filename);
	}

	//! Load a 3d object from a .OFF file.
	/**
	\param filename Filename to read data from.
	\param[out] primitives At return, contains the list of 3d object primitives.
	\param[out] colors At return, contains the list of 3d object colors.
	\return List of 3d object vertices.
	**/
	template<typename tf, typename tc>
	CImgList<T>& load_off(const char *const filename,
		CImgList<tf>& primitives, CImgList<tc>& colors) {
		return get_load_off(filename, primitives, colors).move_to(*this);
	}

	//! Load a 3d object from a .OFF file \newinstance.
	template<typename tf, typename tc>
	static CImgList<T> get_load_off(const char *const filename,
		CImgList<tf>& primitives, CImgList<tc>& colors) {
		return CImg<T>().load_off(filename, primitives, colors)<'x';
	}

	//! Load images from a TIFF file.
	/**
	\param filename Filename to read data from.
	\param first_frame Index of first image frame to read.
	\param last_frame Index of last image frame to read.
	\param step_frame Step applied between each frame.
	\param[out] voxel_size Voxel size, as stored in the filename.
	\param[out] description Description, as stored in the filename.
	**/
	CImgList<T>& load_tiff(const char *const filename,
		const unsigned int first_frame = 0, const unsigned int last_frame = ~0U,
		const unsigned int step_frame = 1,
		float *const voxel_size = 0,
		CImg<charT> *const description = 0) {
		const unsigned int
			nfirst_frame = first_frame<last_frame ? first_frame : last_frame,
			nstep_frame = step_frame ? step_frame : 1;
		unsigned int nlast_frame = first_frame<last_frame ? last_frame : first_frame;
#ifndef cimg_use_tiff
		cimg::unused(voxel_size, description);
		if (nfirst_frame || nlast_frame != ~0U || nstep_frame != 1)
			throw CImgArgumentException(_cimglist_instance
				"load_tiff(): Unable to load sub-images from file '%s' unless libtiff is enabled.",
				cimglist_instance,
				filename);

		return assign(CImg<T>::get_load_tiff(filename));
#else
#if cimg_verbosity<3
		TIFFSetWarningHandler(0);
		TIFFSetErrorHandler(0);
#endif
		TIFF *tif = TIFFOpen(filename, "r");
		if (tif) {
			unsigned int nb_images = 0;
			do ++nb_images; while (TIFFReadDirectory(tif));
			if (nfirst_frame >= nb_images || (nlast_frame != ~0U && nlast_frame >= nb_images))
				cimg::warn(_cimglist_instance
					"load_tiff(): Invalid specified frame range is [%u,%u] (step %u) since "
					"file '%s' contains %u image(s).",
					cimglist_instance,
					nfirst_frame, nlast_frame, nstep_frame, filename, nb_images);

			if (nfirst_frame >= nb_images) return assign();
			if (nlast_frame >= nb_images) nlast_frame = nb_images - 1;
			assign(1 + (nlast_frame - nfirst_frame) / nstep_frame);
			TIFFSetDirectory(tif, 0);
			cimglist_for(*this, l) _data[l]._load_tiff(tif, nfirst_frame + l*nstep_frame, voxel_size, description);
			TIFFClose(tif);
		}
		else throw CImgIOException(_cimglist_instance
			"load_tiff(): Failed to open file '%s'.",
			cimglist_instance,
			filename);
		return *this;
#endif
	}

	//! Load a multi-page TIFF file \newinstance.
	static CImgList<T> get_load_tiff(const char *const filename,
		const unsigned int first_frame = 0, const unsigned int last_frame = ~0U,
		const unsigned int step_frame = 1,
		float *const voxel_size = 0,
		CImg<charT> *const description = 0) {
		return CImgList<T>().load_tiff(filename, first_frame, last_frame, step_frame, voxel_size, description);
	}

	//@}
	//----------------------------------
	//
	//! \name Data Output
	//@{
	//----------------------------------

	//! Print information about the list on the standard output.
	/**
	\param title Label set to the information displayed.
	\param display_stats Tells if image statistics must be computed and displayed.
	**/
	const CImgList<T>& print(const char *const title = 0, const bool display_stats = true) const {
		unsigned int msiz = 0;
		cimglist_for(*this, l) msiz += _data[l].size();
		msiz *= sizeof(T);
		const unsigned int mdisp = msiz<8 * 1024 ? 0U : msiz<8 * 1024 * 1024 ? 1U : 2U;
		CImg<charT> _title(64);
		if (!title) cimg_snprintf(_title, _title._width, "CImgList<%s>", pixel_type());
		std::fprintf(cimg::output(), "%s%s%s%s: %sthis%s = %p, %ssize%s = %u/%u [%u %s], %sdata%s = (CImg<%s>*)%p",
			cimg::t_magenta, cimg::t_bold, title ? title : _title._data, cimg::t_normal,
			cimg::t_bold, cimg::t_normal, (void*)this,
			cimg::t_bold, cimg::t_normal, _width, _allocated_width,
			mdisp == 0 ? msiz : (mdisp == 1 ? (msiz >> 10) : (msiz >> 20)),
			mdisp == 0 ? "b" : (mdisp == 1 ? "Kio" : "Mio"),
			cimg::t_bold, cimg::t_normal, pixel_type(), (void*)begin());
		if (_data) std::fprintf(cimg::output(), "..%p.\n", (void*)((char*)end() - 1));
		else std::fprintf(cimg::output(), ".\n");

		char tmp[16] = { 0 };
		cimglist_for(*this, ll) {
			cimg_snprintf(tmp, sizeof(tmp), "[%d]", ll);
			std::fprintf(cimg::output(), "  ");
			_data[ll].print(tmp, display_stats);
			if (ll == 3 && width()>8) { ll = width() - 5; std::fprintf(cimg::output(), "  ...\n"); }
		}
		std::fflush(cimg::output());
		return *this;
	}

	//! Display the current CImgList instance in an existing CImgDisplay window (by reference).
	/**
	\param disp Reference to an existing CImgDisplay instance, where the current image list will be displayed.
	\param axis Appending axis. Can be <tt>{ 'x' | 'y' | 'z' | 'c' }</tt>.
	\param align Appending alignmenet.
	\note This function displays the list images of the current CImgList instance into an existing
	CImgDisplay window.
	Images of the list are appended in a single temporarly image for visualization purposes.
	The function returns immediately.
	**/
	const CImgList<T>& display(CImgDisplay &disp, const char axis = 'x', const float align = 0) const {
		disp.display(*this, axis, align);
		return *this;
	}

	//! Display the current CImgList instance in a new display window.
	/**
	\param disp Display window.
	\param display_info Tells if image information are displayed on the standard output.
	\param axis Alignment axis for images viewing.
	\param align Apending alignment.
	\param[in,out] XYZ Contains the XYZ coordinates at start / exit of the function.
	\param exit_on_anykey Exit function when any key is pressed.
	\note This function opens a new window with a specific title and displays the list images of the
	current CImgList instance into it.
	Images of the list are appended in a single temporarly image for visualization purposes.
	The function returns when a key is pressed or the display window is closed by the user.
	**/
	const CImgList<T>& display(CImgDisplay &disp, const bool display_info,
		const char axis = 'x', const float align = 0,
		unsigned int *const XYZ = 0, const bool exit_on_anykey = false) const {
		bool is_exit = false;
		return _display(disp, 0, 0, display_info, axis, align, XYZ, exit_on_anykey, 0, true, is_exit);
	}

	//! Display the current CImgList instance in a new display window.
	/**
	\param title Title of the opening display window.
	\param display_info Tells if list information must be written on standard output.
	\param axis Appending axis. Can be <tt>{ 'x' | 'y' | 'z' | 'c' }</tt>.
	\param align Appending alignment.
	\param[in,out] XYZ Contains the XYZ coordinates at start / exit of the function.
	\param exit_on_anykey Exit function when any key is pressed.
	**/
	const CImgList<T>& display(const char *const title = 0, const bool display_info = true,
		const char axis = 'x', const float align = 0,
		unsigned int *const XYZ = 0, const bool exit_on_anykey = false) const {
		CImgDisplay disp;
		bool is_exit = false;
		return _display(disp, title, 0, display_info, axis, align, XYZ, exit_on_anykey, 0, true, is_exit);
	}

	const CImgList<T>& _display(CImgDisplay &disp, const char *const title, const CImgList<charT> *const titles,
		const bool display_info, const char axis, const float align, unsigned int *const XYZ,
		const bool exit_on_anykey, const unsigned int orig, const bool is_first_call,
		bool &is_exit) const {
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"display(): Empty instance.",
				cimglist_instance);
		if (!disp) {
			if (axis == 'x') {
				unsigned int sum_width = 0, max_height = 0;
				cimglist_for(*this, l) {
					const CImg<T> &img = _data[l];
					const unsigned int
						w = CImgDisplay::_fitscreen(img._width, img._height, img._depth, 128, -85, false),
						h = CImgDisplay::_fitscreen(img._width, img._height, img._depth, 128, -85, true);
					sum_width += w;
					if (h>max_height) max_height = h;
				}
				disp.assign(cimg_fitscreen(sum_width, max_height, 1), title ? title : titles ? titles->__display()._data : 0, 1);
			}
			else {
				unsigned int max_width = 0, sum_height = 0;
				cimglist_for(*this, l) {
					const CImg<T> &img = _data[l];
					const unsigned int
						w = CImgDisplay::_fitscreen(img._width, img._height, img._depth, 128, -85, false),
						h = CImgDisplay::_fitscreen(img._width, img._height, img._depth, 128, -85, true);
					if (w>max_width) max_width = w;
					sum_height += h;
				}
				disp.assign(cimg_fitscreen(max_width, sum_height, 1), title ? title : titles ? titles->__display()._data : 0, 1);
			}
			if (!title && !titles) disp.set_title("CImgList<%s> (%u)", pixel_type(), _width);
		}
		else if (title) disp.set_title("%s", title);
		else if (titles) disp.set_title("%s", titles->__display()._data);
		const CImg<char> dtitle = CImg<char>::string(disp.title());
		if (display_info) print(disp.title());
		disp.show().flush();

		if (_width == 1) {
			const unsigned int dw = disp._width, dh = disp._height;
			if (!is_first_call)
				disp.resize(cimg_fitscreen(_data[0]._width, _data[0]._height, _data[0]._depth), false);
			disp.set_title("%s (%ux%ux%ux%u)",
				dtitle.data(), _data[0]._width, _data[0]._height, _data[0]._depth, _data[0]._spectrum);
			_data[0]._display(disp, 0, false, XYZ, exit_on_anykey, !is_first_call);
			if (disp.key()) is_exit = true;
			disp.resize(cimg_fitscreen(dw, dh, 1), false).set_title("%s", dtitle.data());
		}
		else {
			bool disp_resize = !is_first_call;
			while (!disp.is_closed() && !is_exit) {
				const CImg<intT> s = _select(disp, 0, true, axis, align, exit_on_anykey, orig, disp_resize, !is_first_call, true);
				disp_resize = true;
				if (s[0]<0 && !disp.wheel()) { // No selections done.
					if (disp.button() & 2) { disp.flush(); break; }
					is_exit = true;
				}
				else if (disp.wheel()) { // Zoom in/out.
					const int wheel = disp.wheel();
					disp.set_wheel();
					if (!is_first_call && wheel<0) break;
					if (wheel>0 && _width >= 4) {
						const unsigned int
							delta = std::max(1U, (unsigned int)cimg::round(0.3*_width)),
							ind0 = (unsigned int)std::max(0, s[0] - (int)delta),
							ind1 = (unsigned int)std::min(width() - 1, s[0] + (int)delta);
						if ((ind0 != 0 || ind1 != _width - 1) && ind1 - ind0 >= 3) {
							const CImgList<T> sublist = get_shared_images(ind0, ind1);
							CImgList<charT> t_sublist;
							if (titles) t_sublist = titles->get_shared_images(ind0, ind1);
							sublist._display(disp, 0, titles ? &t_sublist : 0, false, axis, align, XYZ, exit_on_anykey,
								orig + ind0, false, is_exit);
						}
					}
				}
				else if (s[0] != 0 || s[1] != width() - 1) {
					const CImgList<T> sublist = get_shared_images(s[0], s[1]);
					CImgList<charT> t_sublist;
					if (titles) t_sublist = titles->get_shared_images(s[0], s[1]);
					sublist._display(disp, 0, titles ? &t_sublist : 0, false, axis, align, XYZ, exit_on_anykey,
						orig + s[0], false, is_exit);
				}
				disp.set_title("%s", dtitle.data());
			}
		}
		return *this;
	}

	// [internal] Return string to describe display title.
	CImg<charT> __display() const {
		CImg<charT> res, str;
		cimglist_for(*this, l) {
			CImg<charT>::string(_data[l]).move_to(str);
			if (l != width() - 1) {
				str.resize(str._width + 1, 1, 1, 1, 0);
				str[str._width - 2] = ',';
				str[str._width - 1] = ' ';
			}
			res.append(str, 'x');
		}
		if (!res) return CImg<charT>(1, 1, 1, 1, 0).move_to(res);
		cimg::strellipsize(res, 128, false);
		if (_width>1) {
			const unsigned int l = (unsigned int)std::strlen(res);
			if (res._width <= l + 16) res.resize(l + 16, 1, 1, 1, 0);
			cimg_snprintf(res._data + l, 16, " (#%u)", _width);
		}
		return res;
	}

	//! Save list into a file.
	/**
	\param filename Filename to write data to.
	\param number When positive, represents an index added to the filename. Otherwise, no number is added.
	\param digits Number of digits used for adding the number to the filename.
	**/
	const CImgList<T>& save(const char *const filename, const int number = -1, const unsigned int digits = 6) const {
		if (!filename)
			throw CImgArgumentException(_cimglist_instance
				"save(): Specified filename is (null).",
				cimglist_instance);
		// Do not test for empty instances, since .cimg format is able to manage empty instances.
		const bool is_stdout = *filename == '-' && (!filename[1] || filename[1] == '.');
		const char *const ext = cimg::split_filename(filename);
		CImg<charT> nfilename(1024);
		const char *const fn = is_stdout ? filename : number >= 0 ? cimg::number_filename(filename, number, digits, nfilename) :
			filename;

#ifdef cimglist_save_plugin
		cimglist_save_plugin(fn);
#endif
#ifdef cimglist_save_plugin1
		cimglist_save_plugin1(fn);
#endif
#ifdef cimglist_save_plugin2
		cimglist_save_plugin2(fn);
#endif
#ifdef cimglist_save_plugin3
		cimglist_save_plugin3(fn);
#endif
#ifdef cimglist_save_plugin4
		cimglist_save_plugin4(fn);
#endif
#ifdef cimglist_save_plugin5
		cimglist_save_plugin5(fn);
#endif
#ifdef cimglist_save_plugin6
		cimglist_save_plugin6(fn);
#endif
#ifdef cimglist_save_plugin7
		cimglist_save_plugin7(fn);
#endif
#ifdef cimglist_save_plugin8
		cimglist_save_plugin8(fn);
#endif
		if (!cimg::strcasecmp(ext, "cimgz")) return save_cimg(fn, true);
		else if (!cimg::strcasecmp(ext, "cimg") || !*ext) return save_cimg(fn, false);
		else if (!cimg::strcasecmp(ext, "yuv")) return save_yuv(fn, true);
		else if (!cimg::strcasecmp(ext, "avi") ||
			!cimg::strcasecmp(ext, "mov") ||
			!cimg::strcasecmp(ext, "asf") ||
			!cimg::strcasecmp(ext, "divx") ||
			!cimg::strcasecmp(ext, "flv") ||
			!cimg::strcasecmp(ext, "mpg") ||
			!cimg::strcasecmp(ext, "m1v") ||
			!cimg::strcasecmp(ext, "m2v") ||
			!cimg::strcasecmp(ext, "m4v") ||
			!cimg::strcasecmp(ext, "mjp") ||
			!cimg::strcasecmp(ext, "mp4") ||
			!cimg::strcasecmp(ext, "mkv") ||
			!cimg::strcasecmp(ext, "mpe") ||
			!cimg::strcasecmp(ext, "movie") ||
			!cimg::strcasecmp(ext, "ogm") ||
			!cimg::strcasecmp(ext, "ogg") ||
			!cimg::strcasecmp(ext, "ogv") ||
			!cimg::strcasecmp(ext, "qt") ||
			!cimg::strcasecmp(ext, "rm") ||
			!cimg::strcasecmp(ext, "vob") ||
			!cimg::strcasecmp(ext, "wmv") ||
			!cimg::strcasecmp(ext, "xvid") ||
			!cimg::strcasecmp(ext, "mpeg")) return save_video(fn);
#ifdef cimg_use_tiff
		else if (!cimg::strcasecmp(ext, "tif") ||
			!cimg::strcasecmp(ext, "tiff")) return save_tiff(fn);
#endif
		else if (!cimg::strcasecmp(ext, "gz")) return save_gzip_external(fn);
		else {
			if (_width == 1) _data[0].save(fn, -1);
			else cimglist_for(*this, l) { _data[l].save(fn, is_stdout ? -1 : l); if (is_stdout) std::fputc(EOF, cimg::_stdout()); }
		}
		return *this;
	}

	//! Tell if an image list can be saved as one single file.
	/**
	\param filename Filename, as a C-string.
	\return \c true if the file format supports multiple images, \c false otherwise.
	**/
	static bool is_saveable(const char *const filename) {
		const char *const ext = cimg::split_filename(filename);
		if (!cimg::strcasecmp(ext, "cimgz") ||
#ifdef cimg_use_tiff
			!cimg::strcasecmp(ext, "tif") ||
			!cimg::strcasecmp(ext, "tiff") ||
#endif
			!cimg::strcasecmp(ext, "yuv") ||
			!cimg::strcasecmp(ext, "avi") ||
			!cimg::strcasecmp(ext, "mov") ||
			!cimg::strcasecmp(ext, "asf") ||
			!cimg::strcasecmp(ext, "divx") ||
			!cimg::strcasecmp(ext, "flv") ||
			!cimg::strcasecmp(ext, "mpg") ||
			!cimg::strcasecmp(ext, "m1v") ||
			!cimg::strcasecmp(ext, "m2v") ||
			!cimg::strcasecmp(ext, "m4v") ||
			!cimg::strcasecmp(ext, "mjp") ||
			!cimg::strcasecmp(ext, "mp4") ||
			!cimg::strcasecmp(ext, "mkv") ||
			!cimg::strcasecmp(ext, "mpe") ||
			!cimg::strcasecmp(ext, "movie") ||
			!cimg::strcasecmp(ext, "ogm") ||
			!cimg::strcasecmp(ext, "ogg") ||
			!cimg::strcasecmp(ext, "ogv") ||
			!cimg::strcasecmp(ext, "qt") ||
			!cimg::strcasecmp(ext, "rm") ||
			!cimg::strcasecmp(ext, "vob") ||
			!cimg::strcasecmp(ext, "wmv") ||
			!cimg::strcasecmp(ext, "xvid") ||
			!cimg::strcasecmp(ext, "mpeg")) return true;
		return false;
	}

	//! Save image sequence as a GIF animated file.
	/**
	\param filename Filename to write data to.
	\param fps Number of desired frames per second.
	\param nb_loops Number of loops (\c 0 for infinite looping).
	**/
	const CImgList<T>& save_gif_external(const char *const filename, const float fps = 25,
		const unsigned int nb_loops = 0) {
		CImg<charT> command(1024), filename_tmp(256), filename_tmp2(256);
		CImgList<charT> filenames;
		std::FILE *file = 0;

#ifdef cimg_use_png
#define _cimg_save_gif_ext "png"
#else
#define _cimg_save_gif_ext "ppm"
#endif

		do {
			cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s",
				cimg::temporary_path(), cimg_file_separator, cimg::filenamerand());
			cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s_000001." _cimg_save_gif_ext, filename_tmp._data);
			if ((file = std_fopen(filename_tmp2, "rb")) != 0) cimg::fclose(file);
		} while (file);
		cimglist_for(*this, l) {
			cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s_%.6u." _cimg_save_gif_ext, filename_tmp._data, l + 1);
			CImg<charT>::string(filename_tmp2).move_to(filenames);
			if (_data[l]._depth>1 || _data[l]._spectrum != 3) _data[l].get_resize(-100, -100, 1, 3).save(filename_tmp2);
			else _data[l].save(filename_tmp2);
		}

#if cimg_OS!=2
		cimg_snprintf(command, command._width, "%s -delay %u -loop %u",
			cimg::imagemagick_path(), (unsigned int)std::max(0.0f, cimg::round(100 / fps)), nb_loops);
		CImg<ucharT>::string(command).move_to(filenames, 0);
		cimg_snprintf(command, command._width, "\"%s\" >/dev/null 2>&1",
			CImg<charT>::string(filename)._system_strescape().data());
		CImg<ucharT>::string(command).move_to(filenames);
#else
		cimg_snprintf(command, command._width, "\"%s -delay %u -loop %u",
			cimg::imagemagick_path(), (unsigned int)std::max(0.0f, cimg::round(100 / fps)), nb_loops);
		CImg<ucharT>::string(command).move_to(filenames, 0);
		cimg_snprintf(command, command._width, "\"%s\"\" >NUL 2>&1",
			CImg<charT>::string(filename)._system_strescape().data());
		CImg<ucharT>::string(command).move_to(filenames);
#endif
		CImg<charT> _command = filenames>'x';
		cimg_for(_command, p, char) if (!*p) *p = ' ';
		_command.back() = 0;

		cimg::system(_command);
		file = std_fopen(filename, "rb");
		if (!file)
			throw CImgIOException(_cimglist_instance
				"save_gif_external(): Failed to save file '%s' with external command 'magick/convert'.",
				cimglist_instance,
				filename);
		else cimg::fclose(file);
		cimglist_for_in(*this, 1, filenames._width - 1, l) std::remove(filenames[l]);
		return *this;
	}

	const CImgList<T>& _save_yuv(std::FILE *const file, const char *const filename, const bool is_rgb) const {
		if (!file && !filename)
			throw CImgArgumentException(_cimglist_instance
				"save_yuv(): Specified filename is (null).",
				cimglist_instance);
		if (is_empty()) { cimg::fempty(file, filename); return *this; }
		if ((*this)[0].width() % 2 || (*this)[0].height() % 2)
			throw CImgInstanceException(_cimglist_instance
				"save_yuv(): Invalid odd instance dimensions (%u,%u) for file '%s'.",
				cimglist_instance,
				(*this)[0].width(), (*this)[0].height(),
				filename ? filename : "(FILE*)");

		std::FILE *const nfile = file ? file : cimg::fopen(filename, "wb");
		cimglist_for(*this, l) {
			CImg<ucharT> YCbCr((*this)[l]);
			if (is_rgb) YCbCr.RGBtoYCbCr();
			cimg::fwrite(YCbCr._data, (size_t)YCbCr._width*YCbCr._height, nfile);
			cimg::fwrite(YCbCr.get_resize(YCbCr._width / 2, YCbCr._height / 2, 1, 3, 3).data(0, 0, 0, 1),
				(size_t)YCbCr._width*YCbCr._height / 2, nfile);
		}
		if (!file) cimg::fclose(nfile);
		return *this;
	}

	//! Save list as a YUV image sequence file.
	/**
	\param filename Filename to write data to.
	\param is_rgb Tells if the RGB to YUV conversion must be done for saving.
	**/
	const CImgList<T>& save_yuv(const char *const filename = 0, const bool is_rgb = true) const {
		return _save_yuv(0, filename, is_rgb);
	}

	//! Save image sequence into a YUV file.
	/**
	\param file File to write data to.
	\param is_rgb Tells if the RGB to YUV conversion must be done for saving.
	**/
	const CImgList<T>& save_yuv(std::FILE *const file, const bool is_rgb = true) const {
		return _save_yuv(file, 0, is_rgb);
	}

	const CImgList<T>& _save_cimg(std::FILE *const file, const char *const filename, const bool is_compressed) const {
		if (!file && !filename)
			throw CImgArgumentException(_cimglist_instance
				"save_cimg(): Specified filename is (null).",
				cimglist_instance);
#ifndef cimg_use_zlib
		if (is_compressed)
			cimg::warn(_cimglist_instance
				"save_cimg(): Unable to save compressed data in file '%s' unless zlib is enabled, "
				"saving them uncompressed.",
				cimglist_instance,
				filename ? filename : "(FILE*)");
#endif
		std::FILE *const nfile = file ? file : cimg::fopen(filename, "wb");
		const char *const ptype = pixel_type(), *const etype = cimg::endianness() ? "big" : "little";
		if (std::strstr(ptype, "unsigned") == ptype) std::fprintf(nfile, "%u unsigned_%s %s_endian\n", _width, ptype + 9, etype);
		else std::fprintf(nfile, "%u %s %s_endian\n", _width, ptype, etype);
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			std::fprintf(nfile, "%u %u %u %u", img._width, img._height, img._depth, img._spectrum);
			if (img._data) {
				CImg<T> tmp;
				if (cimg::endianness()) { tmp = img; cimg::invert_endianness(tmp._data, tmp.size()); }
				const CImg<T>& ref = cimg::endianness() ? tmp : img;
				bool failed_to_compress = true;
				if (is_compressed) {
#ifdef cimg_use_zlib
					const ulongT siz = sizeof(T)*ref.size();
					uLongf csiz = siz + siz / 100 + 16;
					Bytef *const cbuf = new Bytef[csiz];
					if (compress(cbuf, &csiz, (Bytef*)ref._data, siz))
						cimg::warn(_cimglist_instance
							"save_cimg(): Failed to save compressed data for file '%s', saving them uncompressed.",
							cimglist_instance,
							filename ? filename : "(FILE*)");
					else {
						std::fprintf(nfile, " #%lu\n", csiz);
						cimg::fwrite(cbuf, csiz, nfile);
						delete[] cbuf;
						failed_to_compress = false;
					}
#endif
				}
				if (failed_to_compress) { // Write in a non-compressed way.
					std::fputc('\n', nfile);
					cimg::fwrite(ref._data, ref.size(), nfile);
				}
			}
			else std::fputc('\n', nfile);
		}
		if (!file) cimg::fclose(nfile);
		return *this;
	}

	//! Save list into a .cimg file.
	/**
	\param filename Filename to write data to.
	\param is_compressed Tells if data compression must be enabled.
	**/
	const CImgList<T>& save_cimg(const char *const filename, const bool is_compressed = false) const {
		return _save_cimg(0, filename, is_compressed);
	}

	//! Save list into a .cimg file.
	/**
	\param file File to write data to.
	\param is_compressed Tells if data compression must be enabled.
	**/
	const CImgList<T>& save_cimg(std::FILE *file, const bool is_compressed = false) const {
		return _save_cimg(file, 0, is_compressed);
	}

	const CImgList<T>& _save_cimg(std::FILE *const file, const char *const filename,
		const unsigned int n0,
		const unsigned int x0, const unsigned int y0,
		const unsigned int z0, const unsigned int c0) const {
#define _cimg_save_cimg_case(Ts,Tss) \
      if (!saved && !cimg::strcasecmp(Ts,str_pixeltype)) { \
        for (unsigned int l = 0; l<lmax; ++l) { \
          j = 0; while ((i=std::fgetc(nfile))!='\n') tmp[j++]=(char)i; tmp[j] = 0; \
          W = H = D = C = 0; \
          if (cimg_sscanf(tmp,"%u %u %u %u",&W,&H,&D,&C)!=4) \
            throw CImgIOException(_cimglist_instance \
                                  "save_cimg(): Invalid size (%u,%u,%u,%u) of image[%u], for file '%s'.", \
                                  cimglist_instance, \
                                  W,H,D,C,l,filename?filename:"(FILE*)"); \
          if (W*H*D*C>0) { \
            if (l<n0 || x0>=W || y0>=H || z0>=D || c0>=D) cimg::fseek(nfile,W*H*D*C*sizeof(Tss),SEEK_CUR); \
            else { \
              const CImg<T>& img = (*this)[l - n0]; \
              const T *ptrs = img._data; \
              const unsigned int \
                x1 = x0 + img._width - 1, \
                y1 = y0 + img._height - 1, \
                z1 = z0 + img._depth - 1, \
                c1 = c0 + img._spectrum - 1, \
                nx1 = x1>=W?W - 1:x1, \
                ny1 = y1>=H?H - 1:y1, \
                nz1 = z1>=D?D - 1:z1, \
                nc1 = c1>=C?C - 1:c1; \
              CImg<Tss> raw(1 + nx1 - x0); \
              const unsigned int skipvb = c0*W*H*D*sizeof(Tss); \
              if (skipvb) cimg::fseek(nfile,skipvb,SEEK_CUR); \
              for (unsigned int v = 1 + nc1 - c0; v; --v) { \
                const unsigned int skipzb = z0*W*H*sizeof(Tss); \
                if (skipzb) cimg::fseek(nfile,skipzb,SEEK_CUR); \
                for (unsigned int z = 1 + nz1 - z0; z; --z) { \
                  const unsigned int skipyb = y0*W*sizeof(Tss); \
                  if (skipyb) cimg::fseek(nfile,skipyb,SEEK_CUR); \
                  for (unsigned int y = 1 + ny1 - y0; y; --y) { \
                    const unsigned int skipxb = x0*sizeof(Tss); \
                    if (skipxb) cimg::fseek(nfile,skipxb,SEEK_CUR); \
                    raw.assign(ptrs, raw._width); \
                    ptrs+=img._width; \
                    if (endian) cimg::invert_endianness(raw._data,raw._width); \
                    cimg::fwrite(raw._data,raw._width,nfile); \
                    const unsigned int skipxe = (W - 1 - nx1)*sizeof(Tss); \
                    if (skipxe) cimg::fseek(nfile,skipxe,SEEK_CUR); \
                  } \
                  const unsigned int skipye = (H - 1 - ny1)*W*sizeof(Tss); \
                  if (skipye) cimg::fseek(nfile,skipye,SEEK_CUR); \
                } \
                const unsigned int skipze = (D - 1 - nz1)*W*H*sizeof(Tss); \
                if (skipze) cimg::fseek(nfile,skipze,SEEK_CUR); \
              } \
              const unsigned int skipve = (C - 1 - nc1)*W*H*D*sizeof(Tss); \
              if (skipve) cimg::fseek(nfile,skipve,SEEK_CUR); \
            } \
          } \
        } \
        saved = true; \
      }

		if (!file && !filename)
			throw CImgArgumentException(_cimglist_instance
				"save_cimg(): Specified filename is (null).",
				cimglist_instance);
		if (is_empty())
			throw CImgInstanceException(_cimglist_instance
				"save_cimg(): Empty instance, for file '%s'.",
				cimglist_instance,
				filename ? filename : "(FILE*)");

		std::FILE *const nfile = file ? file : cimg::fopen(filename, "rb+");
		bool saved = false, endian = cimg::endianness();
		CImg<charT> tmp(256), str_pixeltype(256), str_endian(256);
		*tmp = *str_pixeltype = *str_endian = 0;
		unsigned int j, N, W, H, D, C;
		int i, err;
		j = 0; while ((i = std::fgetc(nfile)) != '\n' && i != EOF && j<256) tmp[j++] = (char)i; tmp[j] = 0;
		err = cimg_sscanf(tmp, "%u%*c%255[A-Za-z64_]%*c%255[sA-Za-z_ ]", &N, str_pixeltype._data, str_endian._data);
		if (err<2) {
			if (!file) cimg::fclose(nfile);
			throw CImgIOException(_cimglist_instance
				"save_cimg(): CImg header not found in file '%s'.",
				cimglist_instance,
				filename ? filename : "(FILE*)");
		}
		if (!cimg::strncasecmp("little", str_endian, 6)) endian = false;
		else if (!cimg::strncasecmp("big", str_endian, 3)) endian = true;
		const unsigned int lmax = std::min(N, n0 + _width);
		_cimg_save_cimg_case("bool", bool);
		_cimg_save_cimg_case("unsigned_char", unsigned char);
		_cimg_save_cimg_case("uchar", unsigned char);
		_cimg_save_cimg_case("char", char);
		_cimg_save_cimg_case("unsigned_short", unsigned short);
		_cimg_save_cimg_case("ushort", unsigned short);
		_cimg_save_cimg_case("short", short);
		_cimg_save_cimg_case("unsigned_int", unsigned int);
		_cimg_save_cimg_case("uint", unsigned int);
		_cimg_save_cimg_case("int", int);
		_cimg_save_cimg_case("unsigned_int64", uint64T);
		_cimg_save_cimg_case("uint64", uint64T);
		_cimg_save_cimg_case("int64", int64T);
		_cimg_save_cimg_case("float", float);
		_cimg_save_cimg_case("double", double);
		if (!saved) {
			if (!file) cimg::fclose(nfile);
			throw CImgIOException(_cimglist_instance
				"save_cimg(): Unsupported data type '%s' for file '%s'.",
				cimglist_instance,
				filename ? filename : "(FILE*)", str_pixeltype._data);
		}
		if (!file) cimg::fclose(nfile);
		return *this;
	}

	//! Insert the image instance into into an existing .cimg file, at specified coordinates.
	/**
	\param filename Filename to write data to.
	\param n0 Starting index of images to write.
	\param x0 Starting X-coordinates of image regions to write.
	\param y0 Starting Y-coordinates of image regions to write.
	\param z0 Starting Z-coordinates of image regions to write.
	\param c0 Starting C-coordinates of image regions to write.
	**/
	const CImgList<T>& save_cimg(const char *const filename,
		const unsigned int n0,
		const unsigned int x0, const unsigned int y0,
		const unsigned int z0, const unsigned int c0) const {
		return _save_cimg(0, filename, n0, x0, y0, z0, c0);
	}

	//! Insert the image instance into into an existing .cimg file, at specified coordinates.
	/**
	\param file File to write data to.
	\param n0 Starting index of images to write.
	\param x0 Starting X-coordinates of image regions to write.
	\param y0 Starting Y-coordinates of image regions to write.
	\param z0 Starting Z-coordinates of image regions to write.
	\param c0 Starting C-coordinates of image regions to write.
	**/
	const CImgList<T>& save_cimg(std::FILE *const file,
		const unsigned int n0,
		const unsigned int x0, const unsigned int y0,
		const unsigned int z0, const unsigned int c0) const {
		return _save_cimg(file, 0, n0, x0, y0, z0, c0);
	}

	static void _save_empty_cimg(std::FILE *const file, const char *const filename,
		const unsigned int nb,
		const unsigned int dx, const unsigned int dy,
		const unsigned int dz, const unsigned int dc) {
		std::FILE *const nfile = file ? file : cimg::fopen(filename, "wb");
		const ulongT siz = (ulongT)dx*dy*dz*dc * sizeof(T);
		std::fprintf(nfile, "%u %s\n", nb, pixel_type());
		for (unsigned int i = nb; i; --i) {
			std::fprintf(nfile, "%u %u %u %u\n", dx, dy, dz, dc);
			for (ulongT off = siz; off; --off) std::fputc(0, nfile);
		}
		if (!file) cimg::fclose(nfile);
	}

	//! Save empty (non-compressed) .cimg file with specified dimensions.
	/**
	\param filename Filename to write data to.
	\param nb Number of images to write.
	\param dx Width of images in the written file.
	\param dy Height of images in the written file.
	\param dz Depth of images in the written file.
	\param dc Spectrum of images in the written file.
	**/
	static void save_empty_cimg(const char *const filename,
		const unsigned int nb,
		const unsigned int dx, const unsigned int dy = 1,
		const unsigned int dz = 1, const unsigned int dc = 1) {
		return _save_empty_cimg(0, filename, nb, dx, dy, dz, dc);
	}

	//! Save empty .cimg file with specified dimensions.
	/**
	\param file File to write data to.
	\param nb Number of images to write.
	\param dx Width of images in the written file.
	\param dy Height of images in the written file.
	\param dz Depth of images in the written file.
	\param dc Spectrum of images in the written file.
	**/
	static void save_empty_cimg(std::FILE *const file,
		const unsigned int nb,
		const unsigned int dx, const unsigned int dy = 1,
		const unsigned int dz = 1, const unsigned int dc = 1) {
		return _save_empty_cimg(file, 0, nb, dx, dy, dz, dc);
	}

	//! Save list as a TIFF file.
	/**
	\param filename Filename to write data to.
	\param compression_type Compression mode used to write data.
	\param voxel_size Voxel size, to be stored in the filename.
	\param description Description, to be stored in the filename.
	\param use_bigtiff Allow to save big tiff files (>4Gb).
	**/
	const CImgList<T>& save_tiff(const char *const filename, const unsigned int compression_type = 0,
		const float *const voxel_size = 0, const char *const description = 0,
		const bool use_bigtiff = true) const {
		if (!filename)
			throw CImgArgumentException(_cimglist_instance
				"save_tiff(): Specified filename is (null).",
				cimglist_instance);
		if (is_empty()) { cimg::fempty(0, filename); return *this; }

#ifndef cimg_use_tiff
		if (_width == 1) _data[0].save_tiff(filename, compression_type, voxel_size, description, use_bigtiff);
		else cimglist_for(*this, l) {
			CImg<charT> nfilename(1024);
			cimg::number_filename(filename, l, 6, nfilename);
			_data[l].save_tiff(nfilename, compression_type, voxel_size, description, use_bigtiff);
		}
#else
		ulongT siz = 0;
		cimglist_for(*this, l) siz += _data[l].size();
		const bool _use_bigtiff = use_bigtiff && sizeof(siz) >= 8 && siz * sizeof(T) >= 1UL << 31; // No bigtiff for small images.
		TIFF *tif = TIFFOpen(filename, _use_bigtiff ? "w8" : "w4");
		if (tif) {
			for (unsigned int dir = 0, l = 0; l<_width; ++l) {
				const CImg<T>& img = (*this)[l];
				cimg_forZ(img, z) img._save_tiff(tif, dir++, z, compression_type, voxel_size, description);
			}
			TIFFClose(tif);
		}
		else
			throw CImgIOException(_cimglist_instance
				"save_tiff(): Failed to open stream for file '%s'.",
				cimglist_instance,
				filename);
#endif
		return *this;
	}

	//! Save list as a gzipped file, using external tool 'gzip'.
	/**
	\param filename Filename to write data to.
	**/
	const CImgList<T>& save_gzip_external(const char *const filename) const {
		if (!filename)
			throw CImgIOException(_cimglist_instance
				"save_gzip_external(): Specified filename is (null).",
				cimglist_instance);
		CImg<charT> command(1024), filename_tmp(256), body(256);
		const char
			*ext = cimg::split_filename(filename, body),
			*ext2 = cimg::split_filename(body, 0);
		std::FILE *file;
		do {
			if (!cimg::strcasecmp(ext, "gz")) {
				if (*ext2) cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s.%s",
					cimg::temporary_path(), cimg_file_separator, cimg::filenamerand(), ext2);
				else cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s.cimg",
					cimg::temporary_path(), cimg_file_separator, cimg::filenamerand());
			}
			else {
				if (*ext) cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s.%s",
					cimg::temporary_path(), cimg_file_separator, cimg::filenamerand(), ext);
				else cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s.cimg",
					cimg::temporary_path(), cimg_file_separator, cimg::filenamerand());
			}
			if ((file = std_fopen(filename_tmp, "rb")) != 0) cimg::fclose(file);
		} while (file);

		if (is_saveable(body)) {
			save(filename_tmp);
			cimg_snprintf(command, command._width, "%s -c \"%s\" > \"%s\"",
				cimg::gzip_path(),
				CImg<charT>::string(filename_tmp)._system_strescape().data(),
				CImg<charT>::string(filename)._system_strescape().data());
			cimg::system(command);
			file = std_fopen(filename, "rb");
			if (!file)
				throw CImgIOException(_cimglist_instance
					"save_gzip_external(): Failed to save file '%s' with external command 'gzip'.",
					cimglist_instance,
					filename);
			else cimg::fclose(file);
			std::remove(filename_tmp);
		}
		else {
			CImg<charT> nfilename(1024);
			cimglist_for(*this, l) {
				cimg::number_filename(body, l, 6, nfilename);
				if (*ext) cimg_sprintf(nfilename._data + std::strlen(nfilename), ".%s", ext);
				_data[l].save_gzip_external(nfilename);
			}
		}
		return *this;
	}

	//! Save image sequence, using the OpenCV library.
	/**
	\param filename Filename to write data to.
	\param fps Number of frames per second.
	\param codec Type of compression (See http://www.fourcc.org/codecs.php to see available codecs).
	\param keep_open Tells if the video writer associated to the specified filename
	must be kept open or not (to allow frames to be added in the same file afterwards).
	**/
	const CImgList<T>& save_video(const char *const filename, const unsigned int fps = 25,
		const char *codec = 0, const bool keep_open = false) const {
#ifndef cimg_use_opencv
		cimg::unused(codec, keep_open);
		return save_ffmpeg_external(filename, fps);
#else
		static CvVideoWriter *writers[32] = { 0 };
		static CImgList<charT> filenames(32);
		static CImg<intT> sizes(32, 2, 1, 1, 0);
		static int last_used_index = -1;

		// Detect if a video writer already exists for the specified filename.
		cimg::mutex(9);
		int index = -1;
		if (filename) {
			if (last_used_index >= 0 && !std::strcmp(filename, filenames[last_used_index])) {
				index = last_used_index;
			}
			else cimglist_for(filenames, l) if (filenames[l] && !std::strcmp(filename, filenames[l])) {
				index = l; break;
			}
		}
		else index = last_used_index;
		cimg::mutex(9, 0);

		// Find empty slot for capturing video stream.
		if (index<0) {
			if (!filename)
				throw CImgArgumentException(_cimglist_instance
					"save_video(): No already open video writer found. You must specify a "
					"non-(null) filename argument for the first call.",
					cimglist_instance);
			else { cimg::mutex(9); cimglist_for(filenames, l) if (!filenames[l]) { index = l; break; } cimg::mutex(9, 0); }
			if (index<0)
				throw CImgIOException(_cimglist_instance
					"save_video(): File '%s', no video writer slots available. "
					"You have to release some of your previously opened videos.",
					cimglist_instance, filename);
			if (is_empty())
				throw CImgInstanceException(_cimglist_instance
					"save_video(): Instance list is empty.",
					cimglist_instance);
			const unsigned int W = _data ? _data[0]._width : 0, H = _data ? _data[0]._height : 0;
			if (!W || !H)
				throw CImgInstanceException(_cimglist_instance
					"save_video(): Frame [0] is an empty image.",
					cimglist_instance);

#define _cimg_docase(x) ((x)>='a'&&(x)<='z'?(x) + 'A' - 'a':(x))
			const char
				*const _codec = codec && *codec ? codec : "mp4v",
				codec0 = _cimg_docase(_codec[0]),
				codec1 = _codec[0] ? _cimg_docase(_codec[1]) : 0,
				codec2 = _codec[1] ? _cimg_docase(_codec[2]) : 0,
				codec3 = _codec[2] ? _cimg_docase(_codec[3]) : 0;
			cimg::mutex(9);
			writers[index] = cvCreateVideoWriter(filename, CV_FOURCC(codec0, codec1, codec2, codec3),
				fps, cvSize(W, H));
			CImg<charT>::string(filename).move_to(filenames[index]);
			sizes(index, 0) = W; sizes(index, 1) = H;
			cimg::mutex(9, 0);
			if (!writers[index])
				throw CImgIOException(_cimglist_instance
					"save_video(): File '%s', unable to initialize video writer with codec '%c%c%c%c'.",
					cimglist_instance, filename,
					codec0, codec1, codec2, codec3);
		}

		if (!is_empty()) {
			const unsigned int W = sizes(index, 0), H = sizes(index, 1);
			cimg::mutex(9);
			IplImage *ipl = cvCreateImage(cvSize(W, H), 8, 3);
			cimglist_for(*this, l) {
				CImg<T> &src = _data[l];
				if (src.is_empty())
					cimg::warn(_cimglist_instance
						"save_video(): Skip empty frame %d for file '%s'.",
						cimglist_instance, l, filename);
				if (src._depth>1 || src._spectrum>3)
					cimg::warn(_cimglist_instance
						"save_video(): Frame %u has incompatible dimension (%u,%u,%u,%u). "
						"Some image data may be ignored when writing frame into video file '%s'.",
						cimglist_instance, l, src._width, src._height, src._depth, src._spectrum, filename);
				if (src._width == W && src._height == H && src._spectrum == 3) {
					const T *ptr_r = src.data(0, 0, 0, 0), *ptr_g = src.data(0, 0, 0, 1), *ptr_b = src.data(0, 0, 0, 2);
					char *ptrd = ipl->imageData;
					cimg_forXY(src, x, y) {
						*(ptrd++) = (char)*(ptr_b++); *(ptrd++) = (char)*(ptr_g++); *(ptrd++) = (char)*(ptr_r++);
					}
				}
				else {
					CImg<unsigned char> _src(src, false);
					_src.channels(0, std::min(_src._spectrum - 1, 2U)).resize(W, H);
					_src.resize(W, H, 1, 3, _src._spectrum == 1);
					const unsigned char *ptr_r = _src.data(0, 0, 0, 0), *ptr_g = _src.data(0, 0, 0, 1), *ptr_b = _src.data(0, 0, 0, 2);
					char *ptrd = ipl->imageData;
					cimg_forXY(_src, x, y) {
						*(ptrd++) = (char)*(ptr_b++); *(ptrd++) = (char)*(ptr_g++); *(ptrd++) = (char)*(ptr_r++);
					}
				}
				cvWriteFrame(writers[index], ipl);
			}
			cvReleaseImage(&ipl);
			cimg::mutex(9, 0);
		}

		cimg::mutex(9);
		if (!keep_open) {
			cvReleaseVideoWriter(&writers[index]);
			writers[index] = 0;
			filenames[index].assign();
			sizes(index, 0) = sizes(index, 1) = 0;
			last_used_index = -1;
		}
		else last_used_index = index;
		cimg::mutex(9, 0);

		return *this;
#endif
	}

	//! Save image sequence, using the external tool 'ffmpeg'.
	/**
	\param filename Filename to write data to.
	\param fps Number of frames per second.
	\param codec Type of compression.
	\param bitrate Output bitrate
	**/
	const CImgList<T>& save_ffmpeg_external(const char *const filename, const unsigned int fps = 25,
		const char *const codec = 0, const unsigned int bitrate = 2048) const {
		if (!filename)
			throw CImgArgumentException(_cimglist_instance
				"save_ffmpeg_external(): Specified filename is (null).",
				cimglist_instance);
		if (is_empty()) { cimg::fempty(0, filename); return *this; }

		const char
			*const ext = cimg::split_filename(filename),
			*const _codec = codec ? codec : !cimg::strcasecmp(ext, "flv") ? "flv" : "mpeg2video";

		CImg<charT> command(1024), filename_tmp(256), filename_tmp2(256);
		CImgList<charT> filenames;
		std::FILE *file = 0;
		cimglist_for(*this, l) if (!_data[l].is_sameXYZ(_data[0]))
			throw CImgInstanceException(_cimglist_instance
				"save_ffmpeg_external(): Invalid instance dimensions for file '%s'.",
				cimglist_instance,
				filename);
		do {
			cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s",
				cimg::temporary_path(), cimg_file_separator, cimg::filenamerand());
			cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s_000001.ppm", filename_tmp._data);
			if ((file = std_fopen(filename_tmp2, "rb")) != 0) cimg::fclose(file);
		} while (file);
		cimglist_for(*this, l) {
			cimg_snprintf(filename_tmp2, filename_tmp2._width, "%s_%.6u.ppm", filename_tmp._data, l + 1);
			CImg<charT>::string(filename_tmp2).move_to(filenames);
			if (_data[l]._depth>1 || _data[l]._spectrum != 3) _data[l].get_resize(-100, -100, 1, 3).save_pnm(filename_tmp2);
			else _data[l].save_pnm(filename_tmp2);
		}
#if cimg_OS!=2
		cimg_snprintf(command, command._width, "%s -i \"%s_%%6d.ppm\" -vcodec %s -b %uk -r %u -y \"%s\" >/dev/null 2>&1",
			cimg::ffmpeg_path(),
			CImg<charT>::string(filename_tmp)._system_strescape().data(),
			_codec, bitrate, fps,
			CImg<charT>::string(filename)._system_strescape().data());
#else
		cimg_snprintf(command, command._width, "\"%s -i \"%s_%%6d.ppm\" -vcodec %s -b %uk -r %u -y \"%s\"\" >NUL 2>&1",
			cimg::ffmpeg_path(),
			CImg<charT>::string(filename_tmp)._system_strescape().data(),
			_codec, bitrate, fps,
			CImg<charT>::string(filename)._system_strescape().data());
#endif
		cimg::system(command);
		file = std_fopen(filename, "rb");
		if (!file)
			throw CImgIOException(_cimglist_instance
				"save_ffmpeg_external(): Failed to save file '%s' with external command 'ffmpeg'.",
				cimglist_instance,
				filename);
		else cimg::fclose(file);
		cimglist_for(*this, l) std::remove(filenames[l]);
		return *this;
	}

	//! Serialize a CImgList<T> instance into a raw CImg<unsigned char> buffer.
	/**
	\param is_compressed tells if zlib compression must be used for serialization
	(this requires 'cimg_use_zlib' been enabled).
	**/
	CImg<ucharT> get_serialize(const bool is_compressed = false) const {
#ifndef cimg_use_zlib
		if (is_compressed)
			cimg::warn(_cimglist_instance
				"get_serialize(): Unable to compress data unless zlib is enabled, "
				"storing them uncompressed.",
				cimglist_instance);
#endif
		CImgList<ucharT> stream;
		CImg<charT> tmpstr(128);
		const char *const ptype = pixel_type(), *const etype = cimg::endianness() ? "big" : "little";
		if (std::strstr(ptype, "unsigned") == ptype)
			cimg_snprintf(tmpstr, tmpstr._width, "%u unsigned_%s %s_endian\n", _width, ptype + 9, etype);
		else
			cimg_snprintf(tmpstr, tmpstr._width, "%u %s %s_endian\n", _width, ptype, etype);
		CImg<ucharT>::string(tmpstr, false).move_to(stream);
		cimglist_for(*this, l) {
			const CImg<T>& img = _data[l];
			cimg_snprintf(tmpstr, tmpstr._width, "%u %u %u %u", img._width, img._height, img._depth, img._spectrum);
			CImg<ucharT>::string(tmpstr, false).move_to(stream);
			if (img._data) {
				CImg<T> tmp;
				if (cimg::endianness()) { tmp = img; cimg::invert_endianness(tmp._data, tmp.size()); }
				const CImg<T>& ref = cimg::endianness() ? tmp : img;
				bool failed_to_compress = true;
				if (is_compressed) {
#ifdef cimg_use_zlib
					const ulongT siz = sizeof(T)*ref.size();
					uLongf csiz = (ulongT)compressBound(siz);
					Bytef *const cbuf = new Bytef[csiz];
					if (compress(cbuf, &csiz, (Bytef*)ref._data, siz))
						cimg::warn(_cimglist_instance
							"get_serialize(): Failed to save compressed data, saving them uncompressed.",
							cimglist_instance);
					else {
						cimg_snprintf(tmpstr, tmpstr._width, " #%lu\n", csiz);
						CImg<ucharT>::string(tmpstr, false).move_to(stream);
						CImg<ucharT>(cbuf, csiz).move_to(stream);
						delete[] cbuf;
						failed_to_compress = false;
					}
#endif
				}
				if (failed_to_compress) { // Write in a non-compressed way.
					CImg<charT>::string("\n", false).move_to(stream);
					stream.insert(1);
					stream.back().assign((unsigned char*)ref._data, ref.size() * sizeof(T), 1, 1, 1, true);
				}
			}
			else CImg<charT>::string("\n", false).move_to(stream);
		}
		cimglist_apply(stream, unroll)('y');
		return stream>'y';
	}

	//! Unserialize a CImg<unsigned char> serialized buffer into a CImgList<T> list.
	template<typename t>
	static CImgList<T> get_unserialize(const CImg<t>& buffer) {
#ifdef cimg_use_zlib
#define _cimgz_unserialize_case(Tss) { \
        Bytef *cbuf = 0; \
        if (sizeof(t)!=1 || cimg::type<t>::string()==cimg::type<bool>::string()) { \
          cbuf = new Bytef[csiz]; Bytef *_cbuf = cbuf; \
          for (ulongT i = 0; i<csiz; ++i) *(_cbuf++) = (Bytef)*(stream++); \
          is_bytef = false; \
        } else { cbuf = (Bytef*)stream; stream+=csiz; is_bytef = true; } \
        raw.assign(W,H,D,C); \
        uLongf destlen = raw.size()*sizeof(Tss); \
        uncompress((Bytef*)raw._data,&destlen,cbuf,csiz); \
        if (!is_bytef) delete[] cbuf; \
      }
#else
#define _cimgz_unserialize_case(Tss) \
      throw CImgArgumentException("CImgList<%s>::get_unserialize(): Unable to unserialize compressed data " \
                                  "unless zlib is enabled.", \
                                  pixel_type());
#endif

#define _cimg_unserialize_case(Ts,Tss) \
      if (!loaded && !cimg::strcasecmp(Ts,str_pixeltype)) { \
        for (unsigned int l = 0; l<N; ++l) { \
          j = 0; while ((i=(int)*stream)!='\n' && stream<estream && j<255) { ++stream; tmp[j++] = (char)i; } \
          ++stream; tmp[j] = 0; \
          W = H = D = C = 0; csiz = 0; \
          if ((err = cimg_sscanf(tmp,"%u %u %u %u #" cimg_fuint64,&W,&H,&D,&C,&csiz))<4) \
            throw CImgArgumentException("CImgList<%s>::unserialize(): Invalid specified size (%u,%u,%u,%u) for " \
                                        "image #%u in serialized buffer.", \
                                        pixel_type(),W,H,D,C,l); \
          if (W*H*D*C>0) { \
            CImg<Tss> raw; \
            CImg<T> &img = res._data[l]; \
            if (err==5) _cimgz_unserialize_case(Tss) \
            else if (sizeof(Tss)==1) { \
              raw.assign((Tss*)stream,W,H,D,C,true); \
              stream+=raw.size(); \
            } else { \
              raw.assign(W,H,D,C); \
              CImg<ucharT> _raw((unsigned char*)raw._data,W*sizeof(Tss),H,D,C,true); \
              cimg_for(_raw,p,unsigned char) *p = (unsigned char)*(stream++); \
            } \
            if (endian!=cimg::endianness()) cimg::invert_endianness(raw._data,raw.size()); \
            raw.move_to(img); \
          } \
        } \
        loaded = true; \
      }

		if (buffer.is_empty())
			throw CImgArgumentException("CImgList<%s>::get_unserialize(): Specified serialized buffer is (null).",
				pixel_type());
		CImgList<T> res;
		const t *stream = buffer._data, *const estream = buffer._data + buffer.size();
		bool loaded = false, endian = cimg::endianness(), is_bytef = false;
		CImg<charT> tmp(256), str_pixeltype(256), str_endian(256);
		*tmp = *str_pixeltype = *str_endian = 0;
		unsigned int j, N = 0, W, H, D, C;
		uint64T csiz;
		int i, err;
		cimg::unused(is_bytef);
		do {
			j = 0; while ((i = (int)*stream) != '\n' && stream<estream && j<255) { ++stream; tmp[j++] = (char)i; }
			++stream; tmp[j] = 0;
		} while (*tmp == '#' && stream<estream);
		err = cimg_sscanf(tmp, "%u%*c%255[A-Za-z64_]%*c%255[sA-Za-z_ ]",
			&N, str_pixeltype._data, str_endian._data);
		if (err<2)
			throw CImgArgumentException("CImgList<%s>::get_unserialize(): CImg header not found in serialized buffer.",
				pixel_type());
		if (!cimg::strncasecmp("little", str_endian, 6)) endian = false;
		else if (!cimg::strncasecmp("big", str_endian, 3)) endian = true;
		res.assign(N);
		_cimg_unserialize_case("bool", bool);
		_cimg_unserialize_case("unsigned_char", unsigned char);
		_cimg_unserialize_case("uchar", unsigned char);
		_cimg_unserialize_case("char", char);
		_cimg_unserialize_case("unsigned_short", unsigned short);
		_cimg_unserialize_case("ushort", unsigned short);
		_cimg_unserialize_case("short", short);
		_cimg_unserialize_case("unsigned_int", unsigned int);
		_cimg_unserialize_case("uint", unsigned int);
		_cimg_unserialize_case("int", int);
		_cimg_unserialize_case("unsigned_int64", uint64T);
		_cimg_unserialize_case("uint64", uint64T);
		_cimg_unserialize_case("int64", int64T);
		_cimg_unserialize_case("float", float);
		_cimg_unserialize_case("double", double);
		if (!loaded)
			throw CImgArgumentException("CImgList<%s>::get_unserialize(): Unsupported pixel type '%s' defined "
				"in serialized buffer.",
				pixel_type(), str_pixeltype._data);
		return res;
	}

	//@}
	//----------------------------------
	//
	//! \name Others
	//@{
	//----------------------------------

	//! Crop font along the X-axis.
	/**
	**/
	CImgList<T>& crop_font() {
		return get_crop_font().move_to(*this);
	}

	//! Crop font along the X-axis \newinstance.
	/**
	**/
	CImgList<T> get_crop_font() const {
		CImgList<T> res;
		cimglist_for(*this, l) {
			const CImg<T>& letter = (*this)[l];
			int xmin = letter.width(), xmax = 0;
			cimg_forXY(letter, x, y) if (letter(x, y)) { if (x<xmin) xmin = x; if (x>xmax) xmax = x; }
			if (xmin>xmax) CImg<T>(letter._width, letter._height, 1, letter._spectrum, 0).move_to(res);
			else letter.get_crop(xmin, 0, xmax, letter._height - 1).move_to(res);
		}
		res[' '].resize(res['f']._width, -100, -100, -100, 0);
		if (' ' + 256<res.size()) res[' ' + 256].resize(res['f']._width, -100, -100, -100, 0);
		return res;
	}

	//! Return a CImg pre-defined font with desired size.
	/**
	\param font_height Height of the desired font (exact match for 13,23,53,103).
	\param is_variable_width Decide if the font has a variable (\c true) or fixed (\c false) width.
	**/
	static const CImgList<ucharT>& font(const unsigned int font_height, const bool is_variable_width = true) {
		if (!font_height) return CImgList<ucharT>::const_empty();
		cimg::mutex(11);

		// Decompress nearest base font data if needed.
		static const char *data_fonts[] = { cimg::data_font12x13, cimg::data_font20x23, cimg::data_font47x53, 0 };
		static const unsigned int data_widths[] = { 12,20,47,90 }, data_heights[] = { 13,23,53,103 },
			data_Ms[] = { 86,79,57,47 };
		const unsigned int data_ind = font_height <= 13U ? 0U : font_height <= 23U ? 1U : font_height <= 53U ? 2U : 3U;
		static CImg<ucharT> base_fonts[4];
		CImg<ucharT> &base_font = base_fonts[data_ind];
		if (!base_font) {
			const unsigned int w = data_widths[data_ind], h = data_heights[data_ind], M = data_Ms[data_ind];
			base_font.assign(256 * w, h);
			const char *data_font = data_fonts[data_ind];
			unsigned char *ptrd = base_font;
			const unsigned char *const ptrde = base_font.end();

			// Special case needed for 90x103 to avoid MS compiler limit with big strings.
			CImg<char> data90x103;
			if (!data_font) {
				((CImg<char>(cimg::_data_font90x103[0],
					(unsigned int)std::strlen(cimg::_data_font90x103[0]), 1, 1, 1, true),
					CImg<char>(cimg::_data_font90x103[1],
					(unsigned int)std::strlen(cimg::_data_font90x103[1]) + 1, 1, 1, 1, true))>'x').
					move_to(data90x103);
				data_font = data90x103.data();
			}

			// Uncompress font data (decode RLE).
			for (const char *ptrs = data_font; *ptrs; ++ptrs) {
				const int c = (int)(*ptrs - M - 32), v = c >= 0 ? 255 : 0, n = c >= 0 ? c : -c;
				if (ptrd + n <= ptrde) { std::memset(ptrd, v, n); ptrd += n; }
				else { std::memset(ptrd, v, ptrde - ptrd); break; }
			}
		}

		// Find optimal font cache location to return.
		static CImgList<ucharT> fonts[16];
		static bool is_variable_widths[16] = { 0 };
		unsigned int ind = ~0U;
		for (int i = 0; i<16; ++i)
			if (!fonts[i] || (is_variable_widths[i] == is_variable_width && font_height == fonts[i][0]._height)) {
				ind = (unsigned int)i; break; // Found empty slot or cached font.
			}
		if (ind == ~0U) { // No empty slots nor existing font in cache.
			fonts->assign();
			std::memmove(fonts, fonts + 1, 15 * sizeof(CImgList<ucharT>));
			std::memmove(is_variable_widths, is_variable_widths + 1, 15 * sizeof(bool));
			std::memset(fonts + (ind = 15), 0, sizeof(CImgList<ucharT>)); // Free a slot in cache for new font.
		}
		CImgList<ucharT> &font = fonts[ind];

		// Render requested font.
		if (!font) {
			const unsigned int padding_x = font_height<33U ? 1U : font_height<53U ? 2U : font_height<103U ? 3U : 4U;
			is_variable_widths[ind] = is_variable_width;
			font = base_font.get_split('x', 256);
			if (font_height != font[0]._height)
				cimglist_for(font, l)
				font[l].resize(std::max(1U, font[l]._width*font_height / font[l]._height), font_height, -100, -100,
					font[0]._height>font_height ? 2 : 5);
			if (is_variable_width) font.crop_font();
			cimglist_for(font, l) font[l].resize(font[l]._width + padding_x, -100, 1, 1, 0, 0, 0.5);
			font.insert(256, 0);
			cimglist_for_in(font, 0, 255, l) font[l].assign(font[l + 256]._width, font[l + 256]._height, 1, 3, 1);
		}
		cimg::mutex(11, 0);
		return font;
	}

	//! Compute a 1d Fast Fourier Transform, along specified axis.
	/**
	\param axis Axis along which the Fourier transform is computed.
	\param invert Tells if the direct (\c false) or inverse transform (\c true) is computed.
	**/
	CImgList<T>& FFT(const char axis, const bool invert = false) {
		if (is_empty()) return *this;
		if (_width == 1) insert(1);
		if (_width>2)
			cimg::warn(_cimglist_instance
				"FFT(): Instance has more than 2 images",
				cimglist_instance);

		CImg<T>::FFT(_data[0], _data[1], axis, invert);
		return *this;
	}

	//! Compute a 1-D Fast Fourier Transform, along specified axis \newinstance.
	CImgList<Tfloat> get_FFT(const char axis, const bool invert = false) const {
		return CImgList<Tfloat>(*this, false).FFT(axis, invert);
	}

	//! Compute a n-d Fast Fourier Transform.
	/**
	\param invert Tells if the direct (\c false) or inverse transform (\c true) is computed.
	**/
	CImgList<T>& FFT(const bool invert = false) {
		if (is_empty()) return *this;
		if (_width == 1) insert(1);
		if (_width>2)
			cimg::warn(_cimglist_instance
				"FFT(): Instance has more than 2 images",
				cimglist_instance);

		CImg<T>::FFT(_data[0], _data[1], invert);
		return *this;
	}

	//! Compute a n-d Fast Fourier Transform \newinstance.
	CImgList<Tfloat> get_FFT(const bool invert = false) const {
		return CImgList<Tfloat>(*this, false).FFT(invert);
	}

	//! Reverse primitives orientations of a 3d object.
	/**
	**/
	CImgList<T>& reverse_object3d() {
		cimglist_for(*this, l) {
			CImg<T>& p = _data[l];
			switch (p.size()) {
			case 2: case 3: cimg::swap(p[0], p[1]); break;
			case 6: cimg::swap(p[0], p[1], p[2], p[4], p[3], p[5]); break;
			case 9: cimg::swap(p[0], p[1], p[3], p[5], p[4], p[6]); break;
			case 4: cimg::swap(p[0], p[1], p[2], p[3]); break;
			case 12: cimg::swap(p[0], p[1], p[2], p[3], p[4], p[6], p[5], p[7], p[8], p[10], p[9], p[11]); break;
			}
		}
		return *this;
	}

	//! Reverse primitives orientations of a 3d object \newinstance.
	CImgList<T> get_reverse_object3d() const {
		return (+*this).reverse_object3d();
	}

	//@}
}; // struct CImgList<T> { ...

}

#endif // !_CIMG_LIST