#ifndef _CIMG_DISPLAY
#define _CIMG_DISPLAY


#include "CImgMain01.h"

namespace cimg_library_suffixed {
/*-----------------------------------
#
# Define the CImgDisplay structure
#
----------------------------------*/
//! Allow the creation of windows, display images on them and manage user events (keyboard, mouse and windows events).
/**
CImgDisplay methods rely on a low-level graphic library to perform: it can be either \b X-Window
(X11, for Unix-based systems) or \b GDI32 (for Windows-based systems).
If both libraries are missing, CImgDisplay will not be able to display images on screen, and will enter
a minimal mode where warning messages will be outputed each time the program is trying to call one of the
CImgDisplay method.

The configuration variable \c cimg_display tells about the graphic library used.
It is set automatically by \CImg when one of these graphic libraries has been detected.
But, you can override its value if necessary. Valid choices are:
- 0: Disable display capabilities.
- 1: Use \b X-Window (X11) library.
- 2: Use \b GDI32 library.

Remember to link your program against \b X11 or \b GDI32 libraries if you use CImgDisplay.
**/
struct CImgDisplay {
	cimg_ulong _timer, _fps_frames, _fps_timer;
	unsigned int _width, _height, _normalization;
	float _fps_fps, _min, _max;
	bool _is_fullscreen;
	char *_title;
	unsigned int _window_width, _window_height, _button, *_keys, *_released_keys;
	int _window_x, _window_y, _mouse_x, _mouse_y, _wheel;
	bool _is_closed, _is_resized, _is_moved, _is_event,
		_is_keyESC, _is_keyF1, _is_keyF2, _is_keyF3, _is_keyF4, _is_keyF5, _is_keyF6, _is_keyF7,
		_is_keyF8, _is_keyF9, _is_keyF10, _is_keyF11, _is_keyF12, _is_keyPAUSE, _is_key1, _is_key2,
		_is_key3, _is_key4, _is_key5, _is_key6, _is_key7, _is_key8, _is_key9, _is_key0,
		_is_keyBACKSPACE, _is_keyINSERT, _is_keyHOME, _is_keyPAGEUP, _is_keyTAB, _is_keyQ, _is_keyW, _is_keyE,
		_is_keyR, _is_keyT, _is_keyY, _is_keyU, _is_keyI, _is_keyO, _is_keyP, _is_keyDELETE,
		_is_keyEND, _is_keyPAGEDOWN, _is_keyCAPSLOCK, _is_keyA, _is_keyS, _is_keyD, _is_keyF, _is_keyG,
		_is_keyH, _is_keyJ, _is_keyK, _is_keyL, _is_keyENTER, _is_keySHIFTLEFT, _is_keyZ, _is_keyX,
		_is_keyC, _is_keyV, _is_keyB, _is_keyN, _is_keyM, _is_keySHIFTRIGHT, _is_keyARROWUP, _is_keyCTRLLEFT,
		_is_keyAPPLEFT, _is_keyALT, _is_keySPACE, _is_keyALTGR, _is_keyAPPRIGHT, _is_keyMENU, _is_keyCTRLRIGHT,
		_is_keyARROWLEFT, _is_keyARROWDOWN, _is_keyARROWRIGHT, _is_keyPAD0, _is_keyPAD1, _is_keyPAD2, _is_keyPAD3,
		_is_keyPAD4, _is_keyPAD5, _is_keyPAD6, _is_keyPAD7, _is_keyPAD8, _is_keyPAD9, _is_keyPADADD, _is_keyPADSUB,
		_is_keyPADMUL, _is_keyPADDIV;

	//@}
	//---------------------------
	//
	//! \name Plugins
	//@{
	//---------------------------

#ifdef cimgdisplay_plugin
#include cimgdisplay_plugin
#endif
#ifdef cimgdisplay_plugin1
#include cimgdisplay_plugin1
#endif
#ifdef cimgdisplay_plugin2
#include cimgdisplay_plugin2
#endif
#ifdef cimgdisplay_plugin3
#include cimgdisplay_plugin3
#endif
#ifdef cimgdisplay_plugin4
#include cimgdisplay_plugin4
#endif
#ifdef cimgdisplay_plugin5
#include cimgdisplay_plugin5
#endif
#ifdef cimgdisplay_plugin6
#include cimgdisplay_plugin6
#endif
#ifdef cimgdisplay_plugin7
#include cimgdisplay_plugin7
#endif
#ifdef cimgdisplay_plugin8
#include cimgdisplay_plugin8
#endif

	//@}
	//--------------------------------------------------------
	//
	//! \name Constructors / Destructor / Instance Management
	//@{
	//--------------------------------------------------------

	//! Destructor.
	/**
	\note If the associated window is visible on the screen, it is closed by the call to the destructor.
	**/
	~CImgDisplay() {
		assign();
		delete[] _keys;
		delete[] _released_keys;
	}

	//! Construct an empty display.
	/**
	\note Constructing an empty CImgDisplay instance does not make a window appearing on the screen, until
	display of valid data is performed.
	\par Example
	\code
	CImgDisplay disp;  // Does actually nothing.
	...
	disp.display(img); // Construct new window and display image in it.
	\endcode
	**/
	CImgDisplay() :
		_width(0), _height(0), _normalization(0),
		_min(0), _max(0),
		_is_fullscreen(false),
		_title(0),
		_window_width(0), _window_height(0), _button(0),
		_keys(new unsigned int[128]), _released_keys(new unsigned int[128]),
		_window_x(0), _window_y(0), _mouse_x(-1), _mouse_y(-1), _wheel(0),
		_is_closed(true), _is_resized(false), _is_moved(false), _is_event(false) {
		assign();
	}

	//! Construct a display with specified dimensions.
	/** \param width Window width.
	\param height Window height.
	\param title Window title.
	\param normalization Normalization type
	(<tt>0</tt>=none, <tt>1</tt>=always, <tt>2</tt>=once, <tt>3</tt>=pixel type-dependent, see normalization()).
	\param is_fullscreen Tells if fullscreen mode is enabled.
	\param is_closed Tells if associated window is initially visible or not.
	\note A black background is initially displayed on the associated window.
	**/
	CImgDisplay(const unsigned int width, const unsigned int height,
		const char *const title = 0, const unsigned int normalization = 3,
		const bool is_fullscreen = false, const bool is_closed = false) :
		_width(0), _height(0), _normalization(0),
		_min(0), _max(0),
		_is_fullscreen(false),
		_title(0),
		_window_width(0), _window_height(0), _button(0),
		_keys(new unsigned int[128]), _released_keys(new unsigned int[128]),
		_window_x(0), _window_y(0), _mouse_x(-1), _mouse_y(-1), _wheel(0),
		_is_closed(true), _is_resized(false), _is_moved(false), _is_event(false) {
		assign(width, height, title, normalization, is_fullscreen, is_closed);
	}

	//! Construct a display from an image.
	/** \param img Image used as a model to create the window.
	\param title Window title.
	\param normalization Normalization type
	(<tt>0</tt>=none, <tt>1</tt>=always, <tt>2</tt>=once, <tt>3</tt>=pixel type-dependent, see normalization()).
	\param is_fullscreen Tells if fullscreen mode is enabled.
	\param is_closed Tells if associated window is initially visible or not.
	\note The pixels of the input image are initially displayed on the associated window.
	**/
	template<typename T>
	explicit CImgDisplay(const CImg<T>& img,
		const char *const title = 0, const unsigned int normalization = 3,
		const bool is_fullscreen = false, const bool is_closed = false) :
		_width(0), _height(0), _normalization(0),
		_min(0), _max(0),
		_is_fullscreen(false),
		_title(0),
		_window_width(0), _window_height(0), _button(0),
		_keys(new unsigned int[128]), _released_keys(new unsigned int[128]),
		_window_x(0), _window_y(0), _mouse_x(-1), _mouse_y(-1), _wheel(0),
		_is_closed(true), _is_resized(false), _is_moved(false), _is_event(false) {
		assign(img, title, normalization, is_fullscreen, is_closed);
	}

	//! Construct a display from an image list.
	/** \param list The images list to display.
	\param title Window title.
	\param normalization Normalization type
	(<tt>0</tt>=none, <tt>1</tt>=always, <tt>2</tt>=once, <tt>3</tt>=pixel type-dependent, see normalization()).
	\param is_fullscreen Tells if fullscreen mode is enabled.
	\param is_closed Tells if associated window is initially visible or not.
	\note All images of the list, appended along the X-axis, are initially displayed on the associated window.
	**/
	template<typename T>
	explicit CImgDisplay(const CImgList<T>& list,
		const char *const title = 0, const unsigned int normalization = 3,
		const bool is_fullscreen = false, const bool is_closed = false) :
		_width(0), _height(0), _normalization(0),
		_min(0), _max(0),
		_is_fullscreen(false),
		_title(0),
		_window_width(0), _window_height(0), _button(0),
		_keys(new unsigned int[128]), _released_keys(new unsigned int[128]),
		_window_x(0), _window_y(0), _mouse_x(-1), _mouse_y(-1), _wheel(0),
		_is_closed(true), _is_resized(false), _is_moved(false), _is_event(false) {
		assign(list, title, normalization, is_fullscreen, is_closed);
	}

	//! Construct a display as a copy of an existing one.
	/**
	\param disp Display instance to copy.
	\note The pixel buffer of the input window is initially displayed on the associated window.
	**/
	CImgDisplay(const CImgDisplay& disp) :
		_width(0), _height(0), _normalization(0),
		_min(0), _max(0),
		_is_fullscreen(false),
		_title(0),
		_window_width(0), _window_height(0), _button(0),
		_keys(new unsigned int[128]), _released_keys(new unsigned int[128]),
		_window_x(0), _window_y(0), _mouse_x(-1), _mouse_y(-1), _wheel(0),
		_is_closed(true), _is_resized(false), _is_moved(false), _is_event(false) {
		assign(disp);
	}

	//! Take a screenshot.
	/**
	\param[out] img Output screenshot. Can be empty on input
	**/
	template<typename T>
	static void screenshot(CImg<T>& img) {
		return screenshot(0, 0, cimg::type<int>::max(), cimg::type<int>::max(), img);
	}

#if cimg_display==0

	static void _no_display_exception() {
		throw CImgDisplayException("CImgDisplay(): No display available.");
	}

	//! Destructor - Empty constructor \inplace.
	/**
	\note Replace the current instance by an empty display.
	**/
	CImgDisplay& assign() {
		return flush();
	}

	//! Construct a display with specified dimensions \inplace.
	/**
	**/
	CImgDisplay& assign(const unsigned int width, const unsigned int height,
		const char *const title = 0, const unsigned int normalization = 3,
		const bool is_fullscreen = false, const bool is_closed = false) {
		cimg::unused(width, height, title, normalization, is_fullscreen, is_closed);
		_no_display_exception();
		return assign();
	}

	//! Construct a display from an image \inplace.
	/**
	**/
	template<typename T>
	CImgDisplay& assign(const CImg<T>& img,
		const char *const title = 0, const unsigned int normalization = 3,
		const bool is_fullscreen = false, const bool is_closed = false) {
		_no_display_exception();
		return assign(img._width, img._height, title, normalization, is_fullscreen, is_closed);
	}

	//! Construct a display from an image list \inplace.
	/**
	**/
	template<typename T>
	CImgDisplay& assign(const CImgList<T>& list,
		const char *const title = 0, const unsigned int normalization = 3,
		const bool is_fullscreen = false, const bool is_closed = false) {
		_no_display_exception();
		return assign(list._width, list._width, title, normalization, is_fullscreen, is_closed);
	}

	//! Construct a display as a copy of another one \inplace.
	/**
	**/
	CImgDisplay& assign(const CImgDisplay &disp) {
		_no_display_exception();
		return assign(disp._width, disp._height);
	}

#endif

	//! Return a reference to an empty display.
	/**
	\note Can be useful for writing function prototypes where one of the argument (of type CImgDisplay&)
	must have a default value.
	\par Example
	\code
	void foo(CImgDisplay& disp=CImgDisplay::empty());
	\endcode
	**/
	static CImgDisplay& empty() {
		static CImgDisplay _empty;
		return _empty.assign();
	}

	//! Return a reference to an empty display \const.
	static const CImgDisplay& const_empty() {
		static const CImgDisplay _empty;
		return _empty;
	}

#define cimg_fitscreen(dx,dy,dz) CImgDisplay::_fitscreen(dx,dy,dz,128,-85,false), \
                                 CImgDisplay::_fitscreen(dx,dy,dz,128,-85,true)
	static unsigned int _fitscreen(const unsigned int dx, const unsigned int dy, const unsigned int dz,
		const int dmin, const int dmax, const bool return_y) {
		const unsigned int _nw = dx + (dz>1 ? dz : 0), _nh = dy + (dz>1 ? dz : 0);
		unsigned int nw = _nw ? _nw : 1, nh = _nh ? _nh : 1;
		const unsigned int
			sw = (unsigned int)CImgDisplay::screen_width(),
			sh = (unsigned int)CImgDisplay::screen_height(),
			mw = dmin<0 ? (unsigned int)(sw*-dmin / 100) : (unsigned int)dmin,
			mh = dmin<0 ? (unsigned int)(sh*-dmin / 100) : (unsigned int)dmin,
			Mw = dmax<0 ? (unsigned int)(sw*-dmax / 100) : (unsigned int)dmax,
			Mh = dmax<0 ? (unsigned int)(sh*-dmax / 100) : (unsigned int)dmax;
		if (nw<mw) { nh = nh*mw / nw; nh += (nh == 0); nw = mw; }
		if (nh<mh) { nw = nw*mh / nh; nw += (nw == 0); nh = mh; }
		if (nw>Mw) { nh = nh*Mw / nw; nh += (nh == 0); nw = Mw; }
		if (nh>Mh) { nw = nw*Mh / nh; nw += (nw == 0); nh = Mh; }
		if (nw<mw) nw = mw;
		if (nh<mh) nh = mh;
		return return_y ? nh : nw;
	}

	//@}
	//------------------------------------------
	//
	//! \name Overloaded Operators
	//@{
	//------------------------------------------

	//! Display image on associated window.
	/**
	\note <tt>disp = img</tt> is equivalent to <tt>disp.display(img)</tt>.
	**/
	template<typename t>
	CImgDisplay& operator=(const CImg<t>& img) {
		return display(img);
	}

	//! Display list of images on associated window.
	/**
	\note <tt>disp = list</tt> is equivalent to <tt>disp.display(list)</tt>.
	**/
	template<typename t>
	CImgDisplay& operator=(const CImgList<t>& list) {
		return display(list);
	}

	//! Construct a display as a copy of another one \inplace.
	/**
	\note Equivalent to assign(const CImgDisplay&).
	**/
	CImgDisplay& operator=(const CImgDisplay& disp) {
		return assign(disp);
	}

	//! Return \c false if display is empty, \c true otherwise.
	/**
	\note <tt>if (disp) { ... }</tt> is equivalent to <tt>if (!disp.is_empty()) { ... }</tt>.
	**/
	operator bool() const {
		return !is_empty();
	}

	//@}
	//------------------------------------------
	//
	//! \name Instance Checking
	//@{
	//------------------------------------------

	//! Return \c true if display is empty, \c false otherwise.
	/**
	**/
	bool is_empty() const {
		return !(_width && _height);
	}

	//! Return \c true if display is closed (i.e. not visible on the screen), \c false otherwise.
	/**
	\note
	- When a user physically closes the associated window, the display is set to closed.
	- A closed display is not destroyed. Its associated window can be show again on the screen using show().
	**/
	bool is_closed() const {
		return _is_closed;
	}

	//! Return \c true if associated window has been resized on the screen, \c false otherwise.
	/**
	**/
	bool is_resized() const {
		return _is_resized;
	}

	//! Return \c true if associated window has been moved on the screen, \c false otherwise.
	/**
	**/
	bool is_moved() const {
		return _is_moved;
	}

	//! Return \c true if any event has occured on the associated window, \c false otherwise.
	/**
	**/
	bool is_event() const {
		return _is_event;
	}

	//! Return \c true if current display is in fullscreen mode, \c false otherwise.
	/**
	**/
	bool is_fullscreen() const {
		return _is_fullscreen;
	}

	//! Return \c true if any key is being pressed on the associated window, \c false otherwise.
	/**
	\note The methods below do the same only for specific keys.
	**/
	bool is_key() const {
		return _is_keyESC || _is_keyF1 || _is_keyF2 || _is_keyF3 ||
			_is_keyF4 || _is_keyF5 || _is_keyF6 || _is_keyF7 ||
			_is_keyF8 || _is_keyF9 || _is_keyF10 || _is_keyF11 ||
			_is_keyF12 || _is_keyPAUSE || _is_key1 || _is_key2 ||
			_is_key3 || _is_key4 || _is_key5 || _is_key6 ||
			_is_key7 || _is_key8 || _is_key9 || _is_key0 ||
			_is_keyBACKSPACE || _is_keyINSERT || _is_keyHOME ||
			_is_keyPAGEUP || _is_keyTAB || _is_keyQ || _is_keyW ||
			_is_keyE || _is_keyR || _is_keyT || _is_keyY ||
			_is_keyU || _is_keyI || _is_keyO || _is_keyP ||
			_is_keyDELETE || _is_keyEND || _is_keyPAGEDOWN ||
			_is_keyCAPSLOCK || _is_keyA || _is_keyS || _is_keyD ||
			_is_keyF || _is_keyG || _is_keyH || _is_keyJ ||
			_is_keyK || _is_keyL || _is_keyENTER ||
			_is_keySHIFTLEFT || _is_keyZ || _is_keyX || _is_keyC ||
			_is_keyV || _is_keyB || _is_keyN || _is_keyM ||
			_is_keySHIFTRIGHT || _is_keyARROWUP || _is_keyCTRLLEFT ||
			_is_keyAPPLEFT || _is_keyALT || _is_keySPACE || _is_keyALTGR ||
			_is_keyAPPRIGHT || _is_keyMENU || _is_keyCTRLRIGHT ||
			_is_keyARROWLEFT || _is_keyARROWDOWN || _is_keyARROWRIGHT ||
			_is_keyPAD0 || _is_keyPAD1 || _is_keyPAD2 ||
			_is_keyPAD3 || _is_keyPAD4 || _is_keyPAD5 ||
			_is_keyPAD6 || _is_keyPAD7 || _is_keyPAD8 ||
			_is_keyPAD9 || _is_keyPADADD || _is_keyPADSUB ||
			_is_keyPADMUL || _is_keyPADDIV;
	}

	//! Return \c true if key specified by given keycode is being pressed on the associated window, \c false otherwise.
	/**
	\param keycode Keycode to test.
	\note Keycode constants are defined in the cimg namespace and are architecture-dependent. Use them to ensure
	your code stay portable (see cimg::keyESC).
	\par Example
	\code
	CImgDisplay disp(400,400);
	while (!disp.is_closed()) {
	if (disp.key(cimg::keyTAB)) { ... }  // Equivalent to 'if (disp.is_keyTAB())'.
	disp.wait();
	}
	\endcode
	**/
	bool is_key(const unsigned int keycode) const {
#define _cimg_iskey_test(k) if (keycode==cimg::key##k) return _is_key##k;
		_cimg_iskey_test(ESC); _cimg_iskey_test(F1); _cimg_iskey_test(F2); _cimg_iskey_test(F3);
		_cimg_iskey_test(F4); _cimg_iskey_test(F5); _cimg_iskey_test(F6); _cimg_iskey_test(F7);
		_cimg_iskey_test(F8); _cimg_iskey_test(F9); _cimg_iskey_test(F10); _cimg_iskey_test(F11);
		_cimg_iskey_test(F12); _cimg_iskey_test(PAUSE); _cimg_iskey_test(1); _cimg_iskey_test(2);
		_cimg_iskey_test(3); _cimg_iskey_test(4); _cimg_iskey_test(5); _cimg_iskey_test(6);
		_cimg_iskey_test(7); _cimg_iskey_test(8); _cimg_iskey_test(9); _cimg_iskey_test(0);
		_cimg_iskey_test(BACKSPACE); _cimg_iskey_test(INSERT); _cimg_iskey_test(HOME);
		_cimg_iskey_test(PAGEUP); _cimg_iskey_test(TAB); _cimg_iskey_test(Q); _cimg_iskey_test(W);
		_cimg_iskey_test(E); _cimg_iskey_test(R); _cimg_iskey_test(T); _cimg_iskey_test(Y);
		_cimg_iskey_test(U); _cimg_iskey_test(I); _cimg_iskey_test(O); _cimg_iskey_test(P);
		_cimg_iskey_test(DELETE); _cimg_iskey_test(END); _cimg_iskey_test(PAGEDOWN);
		_cimg_iskey_test(CAPSLOCK); _cimg_iskey_test(A); _cimg_iskey_test(S); _cimg_iskey_test(D);
		_cimg_iskey_test(F); _cimg_iskey_test(G); _cimg_iskey_test(H); _cimg_iskey_test(J);
		_cimg_iskey_test(K); _cimg_iskey_test(L); _cimg_iskey_test(ENTER);
		_cimg_iskey_test(SHIFTLEFT); _cimg_iskey_test(Z); _cimg_iskey_test(X); _cimg_iskey_test(C);
		_cimg_iskey_test(V); _cimg_iskey_test(B); _cimg_iskey_test(N); _cimg_iskey_test(M);
		_cimg_iskey_test(SHIFTRIGHT); _cimg_iskey_test(ARROWUP); _cimg_iskey_test(CTRLLEFT);
		_cimg_iskey_test(APPLEFT); _cimg_iskey_test(ALT); _cimg_iskey_test(SPACE); _cimg_iskey_test(ALTGR);
		_cimg_iskey_test(APPRIGHT); _cimg_iskey_test(MENU); _cimg_iskey_test(CTRLRIGHT);
		_cimg_iskey_test(ARROWLEFT); _cimg_iskey_test(ARROWDOWN); _cimg_iskey_test(ARROWRIGHT);
		_cimg_iskey_test(PAD0); _cimg_iskey_test(PAD1); _cimg_iskey_test(PAD2);
		_cimg_iskey_test(PAD3); _cimg_iskey_test(PAD4); _cimg_iskey_test(PAD5);
		_cimg_iskey_test(PAD6); _cimg_iskey_test(PAD7); _cimg_iskey_test(PAD8);
		_cimg_iskey_test(PAD9); _cimg_iskey_test(PADADD); _cimg_iskey_test(PADSUB);
		_cimg_iskey_test(PADMUL); _cimg_iskey_test(PADDIV);
		return false;
	}

	//! Return \c true if key specified by given keycode is being pressed on the associated window, \c false otherwise.
	/**
	\param keycode C-string containing the keycode label of the key to test.
	\note Use it when the key you want to test can be dynamically set by the user.
	\par Example
	\code
	CImgDisplay disp(400,400);
	const char *const keycode = "TAB";
	while (!disp.is_closed()) {
	if (disp.is_key(keycode)) { ... }  // Equivalent to 'if (disp.is_keyTAB())'.
	disp.wait();
	}
	\endcode
	**/
	bool& is_key(const char *const keycode) {
		static bool f = false;
		f = false;
#define _cimg_iskey_test2(k) if (!cimg::strcasecmp(keycode,#k)) return _is_key##k;
		_cimg_iskey_test2(ESC); _cimg_iskey_test2(F1); _cimg_iskey_test2(F2); _cimg_iskey_test2(F3);
		_cimg_iskey_test2(F4); _cimg_iskey_test2(F5); _cimg_iskey_test2(F6); _cimg_iskey_test2(F7);
		_cimg_iskey_test2(F8); _cimg_iskey_test2(F9); _cimg_iskey_test2(F10); _cimg_iskey_test2(F11);
		_cimg_iskey_test2(F12); _cimg_iskey_test2(PAUSE); _cimg_iskey_test2(1); _cimg_iskey_test2(2);
		_cimg_iskey_test2(3); _cimg_iskey_test2(4); _cimg_iskey_test2(5); _cimg_iskey_test2(6);
		_cimg_iskey_test2(7); _cimg_iskey_test2(8); _cimg_iskey_test2(9); _cimg_iskey_test2(0);
		_cimg_iskey_test2(BACKSPACE); _cimg_iskey_test2(INSERT); _cimg_iskey_test2(HOME);
		_cimg_iskey_test2(PAGEUP); _cimg_iskey_test2(TAB); _cimg_iskey_test2(Q); _cimg_iskey_test2(W);
		_cimg_iskey_test2(E); _cimg_iskey_test2(R); _cimg_iskey_test2(T); _cimg_iskey_test2(Y);
		_cimg_iskey_test2(U); _cimg_iskey_test2(I); _cimg_iskey_test2(O); _cimg_iskey_test2(P);
		_cimg_iskey_test2(DELETE); _cimg_iskey_test2(END); _cimg_iskey_test2(PAGEDOWN);
		_cimg_iskey_test2(CAPSLOCK); _cimg_iskey_test2(A); _cimg_iskey_test2(S); _cimg_iskey_test2(D);
		_cimg_iskey_test2(F); _cimg_iskey_test2(G); _cimg_iskey_test2(H); _cimg_iskey_test2(J);
		_cimg_iskey_test2(K); _cimg_iskey_test2(L); _cimg_iskey_test2(ENTER);
		_cimg_iskey_test2(SHIFTLEFT); _cimg_iskey_test2(Z); _cimg_iskey_test2(X); _cimg_iskey_test2(C);
		_cimg_iskey_test2(V); _cimg_iskey_test2(B); _cimg_iskey_test2(N); _cimg_iskey_test2(M);
		_cimg_iskey_test2(SHIFTRIGHT); _cimg_iskey_test2(ARROWUP); _cimg_iskey_test2(CTRLLEFT);
		_cimg_iskey_test2(APPLEFT); _cimg_iskey_test2(ALT); _cimg_iskey_test2(SPACE); _cimg_iskey_test2(ALTGR);
		_cimg_iskey_test2(APPRIGHT); _cimg_iskey_test2(MENU); _cimg_iskey_test2(CTRLRIGHT);
		_cimg_iskey_test2(ARROWLEFT); _cimg_iskey_test2(ARROWDOWN); _cimg_iskey_test2(ARROWRIGHT);
		_cimg_iskey_test2(PAD0); _cimg_iskey_test2(PAD1); _cimg_iskey_test2(PAD2);
		_cimg_iskey_test2(PAD3); _cimg_iskey_test2(PAD4); _cimg_iskey_test2(PAD5);
		_cimg_iskey_test2(PAD6); _cimg_iskey_test2(PAD7); _cimg_iskey_test2(PAD8);
		_cimg_iskey_test2(PAD9); _cimg_iskey_test2(PADADD); _cimg_iskey_test2(PADSUB);
		_cimg_iskey_test2(PADMUL); _cimg_iskey_test2(PADDIV);
		return f;
	}

	//! Return \c true if specified key sequence has been typed on the associated window, \c false otherwise.
	/**
	\param keycodes_sequence Buffer of keycodes to test.
	\param length Number of keys in the \c keycodes_sequence buffer.
	\param remove_sequence Tells if the key sequence must be removed from the key history, if found.
	\note Keycode constants are defined in the cimg namespace and are architecture-dependent. Use them to ensure
	your code stay portable (see cimg::keyESC).
	\par Example
	\code
	CImgDisplay disp(400,400);
	const unsigned int key_seq[] = { cimg::keyCTRLLEFT, cimg::keyD };
	while (!disp.is_closed()) {
	if (disp.is_key_sequence(key_seq,2)) { ... }  // Test for the 'CTRL+D' keyboard event.
	disp.wait();
	}
	\endcode
	**/
	bool is_key_sequence(const unsigned int *const keycodes_sequence, const unsigned int length,
		const bool remove_sequence = false) {
		if (keycodes_sequence && length) {
			const unsigned int
				*const ps_end = keycodes_sequence + length - 1,
				*const pk_end = (unsigned int*)_keys + 1 + 128 - length,
				k = *ps_end;
			for (unsigned int *pk = (unsigned int*)_keys; pk<pk_end; ) {
				if (*(pk++) == k) {
					bool res = true;
					const unsigned int *ps = ps_end, *pk2 = pk;
					for (unsigned int i = 1; i<length; ++i) res = (*(--ps) == *(pk2++));
					if (res) {
						if (remove_sequence) std::memset((void*)(pk - 1), 0, sizeof(unsigned int)*length);
						return true;
					}
				}
			}
		}
		return false;
	}

#define _cimg_iskey_def(k) \
    bool is_key##k() const { \
      return _is_key##k; \
    }

	//! Return \c true if the \c ESC key is being pressed on the associated window, \c false otherwise.
	/**
	\note Similar methods exist for all keys managed by \CImg (see cimg::keyESC).
	**/
	_cimg_iskey_def(ESC); _cimg_iskey_def(F1); _cimg_iskey_def(F2); _cimg_iskey_def(F3);
	_cimg_iskey_def(F4); _cimg_iskey_def(F5); _cimg_iskey_def(F6); _cimg_iskey_def(F7);
	_cimg_iskey_def(F8); _cimg_iskey_def(F9); _cimg_iskey_def(F10); _cimg_iskey_def(F11);
	_cimg_iskey_def(F12); _cimg_iskey_def(PAUSE); _cimg_iskey_def(1); _cimg_iskey_def(2);
	_cimg_iskey_def(3); _cimg_iskey_def(4); _cimg_iskey_def(5); _cimg_iskey_def(6);
	_cimg_iskey_def(7); _cimg_iskey_def(8); _cimg_iskey_def(9); _cimg_iskey_def(0);
	_cimg_iskey_def(BACKSPACE); _cimg_iskey_def(INSERT); _cimg_iskey_def(HOME);
	_cimg_iskey_def(PAGEUP); _cimg_iskey_def(TAB); _cimg_iskey_def(Q); _cimg_iskey_def(W);
	_cimg_iskey_def(E); _cimg_iskey_def(R); _cimg_iskey_def(T); _cimg_iskey_def(Y);
	_cimg_iskey_def(U); _cimg_iskey_def(I); _cimg_iskey_def(O); _cimg_iskey_def(P);
	_cimg_iskey_def(DELETE); _cimg_iskey_def(END); _cimg_iskey_def(PAGEDOWN);
	_cimg_iskey_def(CAPSLOCK); _cimg_iskey_def(A); _cimg_iskey_def(S); _cimg_iskey_def(D);
	_cimg_iskey_def(F); _cimg_iskey_def(G); _cimg_iskey_def(H); _cimg_iskey_def(J);
	_cimg_iskey_def(K); _cimg_iskey_def(L); _cimg_iskey_def(ENTER);
	_cimg_iskey_def(SHIFTLEFT); _cimg_iskey_def(Z); _cimg_iskey_def(X); _cimg_iskey_def(C);
	_cimg_iskey_def(V); _cimg_iskey_def(B); _cimg_iskey_def(N); _cimg_iskey_def(M);
	_cimg_iskey_def(SHIFTRIGHT); _cimg_iskey_def(ARROWUP); _cimg_iskey_def(CTRLLEFT);
	_cimg_iskey_def(APPLEFT); _cimg_iskey_def(ALT); _cimg_iskey_def(SPACE); _cimg_iskey_def(ALTGR);
	_cimg_iskey_def(APPRIGHT); _cimg_iskey_def(MENU); _cimg_iskey_def(CTRLRIGHT);
	_cimg_iskey_def(ARROWLEFT); _cimg_iskey_def(ARROWDOWN); _cimg_iskey_def(ARROWRIGHT);
	_cimg_iskey_def(PAD0); _cimg_iskey_def(PAD1); _cimg_iskey_def(PAD2);
	_cimg_iskey_def(PAD3); _cimg_iskey_def(PAD4); _cimg_iskey_def(PAD5);
	_cimg_iskey_def(PAD6); _cimg_iskey_def(PAD7); _cimg_iskey_def(PAD8);
	_cimg_iskey_def(PAD9); _cimg_iskey_def(PADADD); _cimg_iskey_def(PADSUB);
	_cimg_iskey_def(PADMUL); _cimg_iskey_def(PADDIV);

	//@}
	//------------------------------------------
	//
	//! \name Instance Characteristics
	//@{
	//------------------------------------------

#if cimg_display==0

	//! Return width of the screen (current resolution along the X-axis).
	/**
	**/
	static int screen_width() {
		_no_display_exception();
		return 0;
	}

	//! Return height of the screen (current resolution along the Y-axis).
	/**
	**/
	static int screen_height() {
		_no_display_exception();
		return 0;
	}

#endif

	//! Return display width.
	/**
	\note The width of the display (i.e. the width of the pixel data buffer associated to the CImgDisplay instance)
	may be different from the actual width of the associated window.
	**/
	int width() const {
		return (int)_width;
	}

	//! Return display height.
	/**
	\note The height of the display (i.e. the height of the pixel data buffer associated to the CImgDisplay instance)
	may be different from the actual height of the associated window.
	**/
	int height() const {
		return (int)_height;
	}

	//! Return normalization type of the display.
	/**
	The normalization type tells about how the values of an input image are normalized by the CImgDisplay to be
	correctly displayed. The range of values for pixels displayed on screen is <tt>[0,255]</tt>.
	If the range of values of the data to display is different, a normalization may be required for displaying
	the data in a correct way. The normalization type can be one of:
	- \c 0: Value normalization is disabled. It is then assumed that all input data to be displayed by the
	CImgDisplay instance have values in range <tt>[0,255]</tt>.
	- \c 1: Value normalization is always performed (this is the default behavior).
	Before displaying an input image, its values will be (virtually) stretched
	in range <tt>[0,255]</tt>, so that the contrast of the displayed pixels will be maximum.
	Use this mode for images whose minimum and maximum values are not prescribed to known values
	(e.g. float-valued images).
	Note that when normalized versions of images are computed for display purposes, the actual values of these
	images are not modified.
	- \c 2: Value normalization is performed once (on the first image display), then the same normalization
	coefficients are kept for next displayed frames.
	- \c 3: Value normalization depends on the pixel type of the data to display. For integer pixel types,
	the normalization is done regarding the minimum/maximum values of the type (no normalization occurs then
	for <tt>unsigned char</tt>).
	For float-valued pixel types, the normalization is done regarding the minimum/maximum value of the image
	data instead.
	**/
	unsigned int normalization() const {
		return _normalization;
	}

	//! Return title of the associated window as a C-string.
	/**
	\note Window title may be not visible, depending on the used window manager or if the current display is
	in fullscreen mode.
	**/
	const char *title() const {
		return _title ? _title : "";
	}

	//! Return width of the associated window.
	/**
	\note The width of the display (i.e. the width of the pixel data buffer associated to the CImgDisplay instance)
	may be different from the actual width of the associated window.
	**/
	int window_width() const {
		return (int)_window_width;
	}

	//! Return height of the associated window.
	/**
	\note The height of the display (i.e. the height of the pixel data buffer associated to the CImgDisplay instance)
	may be different from the actual height of the associated window.
	**/
	int window_height() const {
		return (int)_window_height;
	}

	//! Return X-coordinate of the associated window.
	/**
	\note The returned coordinate corresponds to the location of the upper-left corner of the associated window.
	**/
	int window_x() const {
		return _window_x;
	}

	//! Return Y-coordinate of the associated window.
	/**
	\note The returned coordinate corresponds to the location of the upper-left corner of the associated window.
	**/
	int window_y() const {
		return _window_y;
	}

	//! Return X-coordinate of the mouse pointer.
	/**
	\note
	- If the mouse pointer is outside window area, \c -1 is returned.
	- Otherwise, the returned value is in the range [0,width()-1].
	**/
	int mouse_x() const {
		return _mouse_x;
	}

	//! Return Y-coordinate of the mouse pointer.
	/**
	\note
	- If the mouse pointer is outside window area, \c -1 is returned.
	- Otherwise, the returned value is in the range [0,height()-1].
	**/
	int mouse_y() const {
		return _mouse_y;
	}

	//! Return current state of the mouse buttons.
	/**
	\note Three mouse buttons can be managed. If one button is pressed, its corresponding bit in the returned
	value is set:
	- bit \c 0 (value \c 0x1): State of the left mouse button.
	- bit \c 1 (value \c 0x2): State of the right mouse button.
	- bit \c 2 (value \c 0x4): State of the middle mouse button.

	Several bits can be activated if more than one button are pressed at the same time.
	\par Example
	\code
	CImgDisplay disp(400,400);
	while (!disp.is_closed()) {
	if (disp.button()&1) { // Left button clicked.
	...
	}
	if (disp.button()&2) { // Right button clicked.
	...
	}
	if (disp.button()&4) { // Middle button clicked.
	...
	}
	disp.wait();
	}
	\endcode
	**/
	unsigned int button() const {
		return _button;
	}

	//! Return current state of the mouse wheel.
	/**
	\note
	- The returned value can be positive or negative depending on whether the mouse wheel has been scrolled
	forward or backward.
	- Scrolling the wheel forward add \c 1 to the wheel value.
	- Scrolling the wheel backward substract \c 1 to the wheel value.
	- The returned value cumulates the number of forward of backward scrolls since the creation of the display,
	or since the last reset of the wheel value (using set_wheel()). It is strongly recommended to quickly reset
	the wheel counter when an action has been performed regarding the current wheel value.
	Otherwise, the returned wheel value may be for instance \c 0 despite the fact that many scrolls have been done
	(as many in forward as in backward directions).
	\par Example
	\code
	CImgDisplay disp(400,400);
	while (!disp.is_closed()) {
	if (disp.wheel()) {
	int counter = disp.wheel();  // Read the state of the mouse wheel.
	...                          // Do what you want with 'counter'.
	disp.set_wheel();            // Reset the wheel value to 0.
	}
	disp.wait();
	}
	\endcode
	**/
	int wheel() const {
		return _wheel;
	}

	//! Return one entry from the pressed keys history.
	/**
	\param pos Indice to read from the pressed keys history (indice \c 0 corresponds to latest entry).
	\return Keycode of a pressed key or \c 0 for a released key.
	\note
	- Each CImgDisplay stores a history of the pressed keys in a buffer of size \c 128. When a new key is pressed,
	its keycode is stored in the pressed keys history. When a key is released, \c 0 is put instead.
	This means that up to the 64 last pressed keys may be read from the pressed keys history.
	When a new value is stored, the pressed keys history is shifted so that the latest entry is always
	stored at position \c 0.
	- Keycode constants are defined in the cimg namespace and are architecture-dependent. Use them to ensure
	your code stay portable (see cimg::keyESC).
	**/
	unsigned int key(const unsigned int pos = 0) const {
		return pos<128 ? _keys[pos] : 0;
	}

	//! Return one entry from the released keys history.
	/**
	\param pos Indice to read from the released keys history (indice \c 0 corresponds to latest entry).
	\return Keycode of a released key or \c 0 for a pressed key.
	\note
	- Each CImgDisplay stores a history of the released keys in a buffer of size \c 128. When a new key is released,
	its keycode is stored in the pressed keys history. When a key is pressed, \c 0 is put instead.
	This means that up to the 64 last released keys may be read from the released keys history.
	When a new value is stored, the released keys history is shifted so that the latest entry is always
	stored at position \c 0.
	- Keycode constants are defined in the cimg namespace and are architecture-dependent. Use them to ensure
	your code stay portable (see cimg::keyESC).
	**/
	unsigned int released_key(const unsigned int pos = 0) const {
		return pos<128 ? _released_keys[pos] : 0;
	}

	//! Return keycode corresponding to the specified string.
	/**
	\note Keycode constants are defined in the cimg namespace and are architecture-dependent. Use them to ensure
	your code stay portable (see cimg::keyESC).
	\par Example
	\code
	const unsigned int keyTAB = CImgDisplay::keycode("TAB");  // Return cimg::keyTAB.
	\endcode
	**/
	static unsigned int keycode(const char *const keycode) {
#define _cimg_keycode(k) if (!cimg::strcasecmp(keycode,#k)) return cimg::key##k;
		_cimg_keycode(ESC); _cimg_keycode(F1); _cimg_keycode(F2); _cimg_keycode(F3);
		_cimg_keycode(F4); _cimg_keycode(F5); _cimg_keycode(F6); _cimg_keycode(F7);
		_cimg_keycode(F8); _cimg_keycode(F9); _cimg_keycode(F10); _cimg_keycode(F11);
		_cimg_keycode(F12); _cimg_keycode(PAUSE); _cimg_keycode(1); _cimg_keycode(2);
		_cimg_keycode(3); _cimg_keycode(4); _cimg_keycode(5); _cimg_keycode(6);
		_cimg_keycode(7); _cimg_keycode(8); _cimg_keycode(9); _cimg_keycode(0);
		_cimg_keycode(BACKSPACE); _cimg_keycode(INSERT); _cimg_keycode(HOME);
		_cimg_keycode(PAGEUP); _cimg_keycode(TAB); _cimg_keycode(Q); _cimg_keycode(W);
		_cimg_keycode(E); _cimg_keycode(R); _cimg_keycode(T); _cimg_keycode(Y);
		_cimg_keycode(U); _cimg_keycode(I); _cimg_keycode(O); _cimg_keycode(P);
		_cimg_keycode(DELETE); _cimg_keycode(END); _cimg_keycode(PAGEDOWN);
		_cimg_keycode(CAPSLOCK); _cimg_keycode(A); _cimg_keycode(S); _cimg_keycode(D);
		_cimg_keycode(F); _cimg_keycode(G); _cimg_keycode(H); _cimg_keycode(J);
		_cimg_keycode(K); _cimg_keycode(L); _cimg_keycode(ENTER);
		_cimg_keycode(SHIFTLEFT); _cimg_keycode(Z); _cimg_keycode(X); _cimg_keycode(C);
		_cimg_keycode(V); _cimg_keycode(B); _cimg_keycode(N); _cimg_keycode(M);
		_cimg_keycode(SHIFTRIGHT); _cimg_keycode(ARROWUP); _cimg_keycode(CTRLLEFT);
		_cimg_keycode(APPLEFT); _cimg_keycode(ALT); _cimg_keycode(SPACE); _cimg_keycode(ALTGR);
		_cimg_keycode(APPRIGHT); _cimg_keycode(MENU); _cimg_keycode(CTRLRIGHT);
		_cimg_keycode(ARROWLEFT); _cimg_keycode(ARROWDOWN); _cimg_keycode(ARROWRIGHT);
		_cimg_keycode(PAD0); _cimg_keycode(PAD1); _cimg_keycode(PAD2);
		_cimg_keycode(PAD3); _cimg_keycode(PAD4); _cimg_keycode(PAD5);
		_cimg_keycode(PAD6); _cimg_keycode(PAD7); _cimg_keycode(PAD8);
		_cimg_keycode(PAD9); _cimg_keycode(PADADD); _cimg_keycode(PADSUB);
		_cimg_keycode(PADMUL); _cimg_keycode(PADDIV);
		return 0;
	}

	//! Return the current refresh rate, in frames per second.
	/**
	\note Returns a significant value when the current instance is used to display successive frames.
	It measures the delay between successive calls to frames_per_second().
	**/
	float frames_per_second() {
		if (!_fps_timer) _fps_timer = cimg::time();
		const float delta = (cimg::time() - _fps_timer) / 1000.0f;
		++_fps_frames;
		if (delta >= 1) {
			_fps_fps = _fps_frames / delta;
			_fps_frames = 0;
			_fps_timer = cimg::time();
		}
		return _fps_fps;
	}

	//@}
	//---------------------------------------
	//
	//! \name Window Manipulation
	//@{
	//---------------------------------------

#if cimg_display==0

	//! Display image on associated window.
	/**
	\param img Input image to display.
	\note This method returns immediately.
	**/
	template<typename T>
	CImgDisplay& display(const CImg<T>& img) {
		return assign(img);
	}

#endif

	//! Display list of images on associated window.
	/**
	\param list List of images to display.
	\param axis Axis used to append the images along, for the visualization (can be \c x, \c y, \c z or \c c).
	\param align Relative position of aligned images when displaying lists with images of different sizes
	(\c 0 for upper-left, \c 0.5 for centering and \c 1 for lower-right).
	\note This method returns immediately.
	**/
	template<typename T>
	CImgDisplay& display(const CImgList<T>& list, const char axis = 'x', const float align = 0) {
		if (list._width == 1) {
			const CImg<T>& img = list[0];
			if (img._depth == 1 && (img._spectrum == 1 || img._spectrum >= 3) && _normalization != 1) return display(img);
		}
		CImgList<typename CImg<T>::ucharT> visu(list._width);
		unsigned int dims = 0;
		cimglist_for(list, l) {
			const CImg<T>& img = list._data[l];
			img.__get_select(*this, _normalization, (img._width - 1) / 2, (img._height - 1) / 2,
				(img._depth - 1) / 2).move_to(visu[l]);
			dims = std::max(dims, visu[l]._spectrum);
		}
		cimglist_for(list, l) if (visu[l]._spectrum<dims) visu[l].resize(-100, -100, -100, dims, 1);
		visu.get_append(axis, align).display(*this);
		return *this;
	}

#if cimg_display==0

	//! Show (closed) associated window on the screen.
	/**
	\note
	- Force the associated window of a display to be visible on the screen, even if it has been closed before.
	- Using show() on a visible display does nothing.
	**/
	CImgDisplay& show() {
		return assign();
	}

	//! Close (visible) associated window and make it disappear from the screen.
	/**
	\note
	- A closed display only means the associated window is not visible anymore. This does not mean the display has
	been destroyed.
	Use show() to make the associated window reappear.
	- Using close() on a closed display does nothing.
	**/
	CImgDisplay& close() {
		return assign();
	}

	//! Move associated window to a new location.
	/**
	\param pos_x X-coordinate of the new window location.
	\param pos_y Y-coordinate of the new window location.
	\note Depending on the window manager behavior, this method may not succeed (no exceptions are thrown
	nevertheless).
	**/
	CImgDisplay& move(const int pos_x, const int pos_y) {
		return assign(pos_x, pos_y);
	}

#endif

	//! Resize display to the size of the associated window.
	/**
	\param force_redraw Tells if the previous window content must be updated and refreshed as well.
	\note
	- Calling this method ensures that width() and window_width() become equal, as well as height() and
	window_height().
	- The associated window is also resized to specified dimensions.
	**/
	CImgDisplay& resize(const bool force_redraw = true) {
		resize(window_width(), window_height(), force_redraw);
		return *this;
	}

#if cimg_display==0

	//! Resize display to the specified size.
	/**
	\param width Requested display width.
	\param height Requested display height.
	\param force_redraw Tells if the previous window content must be updated and refreshed as well.
	\note The associated window is also resized to specified dimensions.
	**/
	CImgDisplay& resize(const int width, const int height, const bool force_redraw = true) {
		return assign(width, height, 0, 3, force_redraw);
	}

#endif

	//! Resize display to the size of an input image.
	/**
	\param img Input image to take size from.
	\param force_redraw Tells if the previous window content must be resized and updated as well.
	\note
	- Calling this method ensures that width() and <tt>img.width()</tt> become equal, as well as height() and
	<tt>img.height()</tt>.
	- The associated window is also resized to specified dimensions.
	**/
	template<typename T>
	CImgDisplay& resize(const CImg<T>& img, const bool force_redraw = true) {
		return resize(img._width, img._height, force_redraw);
	}

	//! Resize display to the size of another CImgDisplay instance.
	/**
	\param disp Input display to take size from.
	\param force_redraw Tells if the previous window content must be resized and updated as well.
	\note
	- Calling this method ensures that width() and <tt>disp.width()</tt> become equal, as well as height() and
	<tt>disp.height()</tt>.
	- The associated window is also resized to specified dimensions.
	**/
	CImgDisplay& resize(const CImgDisplay& disp, const bool force_redraw = true) {
		return resize(disp.width(), disp.height(), force_redraw);
	}

	// [internal] Render pixel buffer with size (wd,hd) from source buffer of size (ws,hs).
	template<typename t, typename T>
	static void _render_resize(const T *ptrs, const unsigned int ws, const unsigned int hs,
		t *ptrd, const unsigned int wd, const unsigned int hd) {
		unsigned int *const offx = new unsigned int[wd], *const offy = new unsigned int[hd + 1], *poffx, *poffy;
		float s, curr, old;
		s = (float)ws / wd;
		poffx = offx; curr = 0; for (unsigned int x = 0; x<wd; ++x) {
			old = curr; curr += s; *(poffx++) = (unsigned int)curr - (unsigned int)old;
		}
		s = (float)hs / hd;
		poffy = offy; curr = 0; for (unsigned int y = 0; y<hd; ++y) {
			old = curr; curr += s; *(poffy++) = ws*((unsigned int)curr - (unsigned int)old);
		}
		*poffy = 0;
		poffy = offy;
		for (unsigned int y = 0; y<hd; ) {
			const T *ptr = ptrs;
			poffx = offx;
			for (unsigned int x = 0; x<wd; ++x) { *(ptrd++) = *ptr; ptr += *(poffx++); }
			++y;
			unsigned int dy = *(poffy++);
			for (; !dy && y<hd; std::memcpy(ptrd, ptrd - wd, sizeof(t)*wd), ++y, ptrd += wd, dy = *(poffy++)) {}
			ptrs += dy;
		}
		delete[] offx; delete[] offy;
	}

	//! Set normalization type.
	/**
	\param normalization New normalization mode.
	**/
	CImgDisplay& set_normalization(const unsigned int normalization) {
		_normalization = normalization;
		_min = _max = 0;
		return *this;
	}

#if cimg_display==0

	//! Set title of the associated window.
	/**
	\param format C-string containing the format of the title, as with <tt>std::printf()</tt>.
	\warning As the first argument is a format string, it is highly recommended to write
	\code
	disp.set_title("%s",window_title);
	\endcode
	instead of
	\code
	disp.set_title(window_title);
	\endcode
	if \c window_title can be arbitrary, to prevent nasty memory access.
	**/
	CImgDisplay& set_title(const char *const format, ...) {
		return assign(0, 0, format);
	}

#endif

	//! Enable or disable fullscreen mode.
	/**
	\param is_fullscreen Tells is the fullscreen mode must be activated or not.
	\param force_redraw Tells if the previous window content must be displayed as well.
	\note
	- When the fullscreen mode is enabled, the associated window fills the entire screen but the size of the
	current display is not modified.
	- The screen resolution may be switched to fit the associated window size and ensure it appears the largest
	as possible.
	For X-Window (X11) users, the configuration flag \c cimg_use_xrandr has to be set to allow the screen
	resolution change (requires the X11 extensions to be enabled).
	**/
	CImgDisplay& set_fullscreen(const bool is_fullscreen, const bool force_redraw = true) {
		if (is_empty() || _is_fullscreen == is_fullscreen) return *this;
		return toggle_fullscreen(force_redraw);
	}

#if cimg_display==0

	//! Toggle fullscreen mode.
	/**
	\param force_redraw Tells if the previous window content must be displayed as well.
	\note Enable fullscreen mode if it was not enabled, and disable it otherwise.
	**/
	CImgDisplay& toggle_fullscreen(const bool force_redraw = true) {
		return assign(_width, _height, 0, 3, force_redraw);
	}

	//! Show mouse pointer.
	/**
	\note Depending on the window manager behavior, this method may not succeed
	(no exceptions are thrown nevertheless).
	**/
	CImgDisplay& show_mouse() {
		return assign();
	}

	//! Hide mouse pointer.
	/**
	\note Depending on the window manager behavior, this method may not succeed
	(no exceptions are thrown nevertheless).
	**/
	CImgDisplay& hide_mouse() {
		return assign();
	}

	//! Move mouse pointer to a specified location.
	/**
	\note Depending on the window manager behavior, this method may not succeed
	(no exceptions are thrown nevertheless).
	**/
	CImgDisplay& set_mouse(const int pos_x, const int pos_y) {
		return assign(pos_x, pos_y);
	}

#endif

	//! Simulate a mouse button release event.
	/**
	\note All mouse buttons are considered released at the same time.
	**/
	CImgDisplay& set_button() {
		_button = 0;
		_is_event = true;
#if cimg_display==1
		pthread_cond_broadcast(&cimg::X11_attr().wait_event);
#elif cimg_display==2
		SetEvent(cimg::Win32_attr().wait_event);
#endif
		return *this;
	}

	//! Simulate a mouse button press or release event.
	/**
	\param button Buttons event code, where each button is associated to a single bit.
	\param is_pressed Tells if the mouse button is considered as pressed or released.
	**/
	CImgDisplay& set_button(const unsigned int button, const bool is_pressed = true) {
		const unsigned int buttoncode = button == 1U ? 1U : button == 2U ? 2U : button == 3U ? 4U : 0U;
		if (is_pressed) _button |= buttoncode; else _button &= ~buttoncode;
		_is_event = buttoncode ? true : false;
		if (buttoncode) {
#if cimg_display==1
			pthread_cond_broadcast(&cimg::X11_attr().wait_event);
#elif cimg_display==2
			SetEvent(cimg::Win32_attr().wait_event);
#endif
		}
		return *this;
	}

	//! Flush all mouse wheel events.
	/**
	\note Make wheel() to return \c 0, if called afterwards.
	**/
	CImgDisplay& set_wheel() {
		_wheel = 0;
		_is_event = true;
#if cimg_display==1
		pthread_cond_broadcast(&cimg::X11_attr().wait_event);
#elif cimg_display==2
		SetEvent(cimg::Win32_attr().wait_event);
#endif
		return *this;
	}

	//! Simulate a wheel event.
	/**
	\param amplitude Amplitude of the wheel scrolling to simulate.
	\note Make wheel() to return \c amplitude, if called afterwards.
	**/
	CImgDisplay& set_wheel(const int amplitude) {
		_wheel += amplitude;
		_is_event = amplitude ? true : false;
		if (amplitude) {
#if cimg_display==1
			pthread_cond_broadcast(&cimg::X11_attr().wait_event);
#elif cimg_display==2
			SetEvent(cimg::Win32_attr().wait_event);
#endif
		}
		return *this;
	}

	//! Flush all key events.
	/**
	\note Make key() to return \c 0, if called afterwards.
	**/
	CImgDisplay& set_key() {
		std::memset((void*)_keys, 0, 128 * sizeof(unsigned int));
		std::memset((void*)_released_keys, 0, 128 * sizeof(unsigned int));
		_is_keyESC = _is_keyF1 = _is_keyF2 = _is_keyF3 = _is_keyF4 = _is_keyF5 = _is_keyF6 = _is_keyF7 = _is_keyF8 =
			_is_keyF9 = _is_keyF10 = _is_keyF11 = _is_keyF12 = _is_keyPAUSE = _is_key1 = _is_key2 = _is_key3 = _is_key4 =
			_is_key5 = _is_key6 = _is_key7 = _is_key8 = _is_key9 = _is_key0 = _is_keyBACKSPACE = _is_keyINSERT =
			_is_keyHOME = _is_keyPAGEUP = _is_keyTAB = _is_keyQ = _is_keyW = _is_keyE = _is_keyR = _is_keyT = _is_keyY =
			_is_keyU = _is_keyI = _is_keyO = _is_keyP = _is_keyDELETE = _is_keyEND = _is_keyPAGEDOWN = _is_keyCAPSLOCK =
			_is_keyA = _is_keyS = _is_keyD = _is_keyF = _is_keyG = _is_keyH = _is_keyJ = _is_keyK = _is_keyL =
			_is_keyENTER = _is_keySHIFTLEFT = _is_keyZ = _is_keyX = _is_keyC = _is_keyV = _is_keyB = _is_keyN =
			_is_keyM = _is_keySHIFTRIGHT = _is_keyARROWUP = _is_keyCTRLLEFT = _is_keyAPPLEFT = _is_keyALT = _is_keySPACE =
			_is_keyALTGR = _is_keyAPPRIGHT = _is_keyMENU = _is_keyCTRLRIGHT = _is_keyARROWLEFT = _is_keyARROWDOWN =
			_is_keyARROWRIGHT = _is_keyPAD0 = _is_keyPAD1 = _is_keyPAD2 = _is_keyPAD3 = _is_keyPAD4 = _is_keyPAD5 =
			_is_keyPAD6 = _is_keyPAD7 = _is_keyPAD8 = _is_keyPAD9 = _is_keyPADADD = _is_keyPADSUB = _is_keyPADMUL =
			_is_keyPADDIV = false;
		_is_event = true;
#if cimg_display==1
		pthread_cond_broadcast(&cimg::X11_attr().wait_event);
#elif cimg_display==2
		SetEvent(cimg::Win32_attr().wait_event);
#endif
		return *this;
	}

	//! Simulate a keyboard press/release event.
	/**
	\param keycode Keycode of the associated key.
	\param is_pressed Tells if the key is considered as pressed or released.
	\note Keycode constants are defined in the cimg namespace and are architecture-dependent. Use them to ensure
	your code stay portable (see cimg::keyESC).
	**/
	CImgDisplay& set_key(const unsigned int keycode, const bool is_pressed = true) {
#define _cimg_set_key(k) if (keycode==cimg::key##k) _is_key##k = is_pressed;
		_cimg_set_key(ESC); _cimg_set_key(F1); _cimg_set_key(F2); _cimg_set_key(F3);
		_cimg_set_key(F4); _cimg_set_key(F5); _cimg_set_key(F6); _cimg_set_key(F7);
		_cimg_set_key(F8); _cimg_set_key(F9); _cimg_set_key(F10); _cimg_set_key(F11);
		_cimg_set_key(F12); _cimg_set_key(PAUSE); _cimg_set_key(1); _cimg_set_key(2);
		_cimg_set_key(3); _cimg_set_key(4); _cimg_set_key(5); _cimg_set_key(6);
		_cimg_set_key(7); _cimg_set_key(8); _cimg_set_key(9); _cimg_set_key(0);
		_cimg_set_key(BACKSPACE); _cimg_set_key(INSERT); _cimg_set_key(HOME);
		_cimg_set_key(PAGEUP); _cimg_set_key(TAB); _cimg_set_key(Q); _cimg_set_key(W);
		_cimg_set_key(E); _cimg_set_key(R); _cimg_set_key(T); _cimg_set_key(Y);
		_cimg_set_key(U); _cimg_set_key(I); _cimg_set_key(O); _cimg_set_key(P);
		_cimg_set_key(DELETE); _cimg_set_key(END); _cimg_set_key(PAGEDOWN);
		_cimg_set_key(CAPSLOCK); _cimg_set_key(A); _cimg_set_key(S); _cimg_set_key(D);
		_cimg_set_key(F); _cimg_set_key(G); _cimg_set_key(H); _cimg_set_key(J);
		_cimg_set_key(K); _cimg_set_key(L); _cimg_set_key(ENTER);
		_cimg_set_key(SHIFTLEFT); _cimg_set_key(Z); _cimg_set_key(X); _cimg_set_key(C);
		_cimg_set_key(V); _cimg_set_key(B); _cimg_set_key(N); _cimg_set_key(M);
		_cimg_set_key(SHIFTRIGHT); _cimg_set_key(ARROWUP); _cimg_set_key(CTRLLEFT);
		_cimg_set_key(APPLEFT); _cimg_set_key(ALT); _cimg_set_key(SPACE); _cimg_set_key(ALTGR);
		_cimg_set_key(APPRIGHT); _cimg_set_key(MENU); _cimg_set_key(CTRLRIGHT);
		_cimg_set_key(ARROWLEFT); _cimg_set_key(ARROWDOWN); _cimg_set_key(ARROWRIGHT);
		_cimg_set_key(PAD0); _cimg_set_key(PAD1); _cimg_set_key(PAD2);
		_cimg_set_key(PAD3); _cimg_set_key(PAD4); _cimg_set_key(PAD5);
		_cimg_set_key(PAD6); _cimg_set_key(PAD7); _cimg_set_key(PAD8);
		_cimg_set_key(PAD9); _cimg_set_key(PADADD); _cimg_set_key(PADSUB);
		_cimg_set_key(PADMUL); _cimg_set_key(PADDIV);
		if (is_pressed) {
			if (*_keys)
				std::memmove((void*)(_keys + 1), (void*)_keys, 127 * sizeof(unsigned int));
			*_keys = keycode;
			if (*_released_keys) {
				std::memmove((void*)(_released_keys + 1), (void*)_released_keys, 127 * sizeof(unsigned int));
				*_released_keys = 0;
			}
		}
		else {
			if (*_keys) {
				std::memmove((void*)(_keys + 1), (void*)_keys, 127 * sizeof(unsigned int));
				*_keys = 0;
			}
			if (*_released_keys)
				std::memmove((void*)(_released_keys + 1), (void*)_released_keys, 127 * sizeof(unsigned int));
			*_released_keys = keycode;
		}
		_is_event = keycode ? true : false;
		if (keycode) {
#if cimg_display==1
			pthread_cond_broadcast(&cimg::X11_attr().wait_event);
#elif cimg_display==2
			SetEvent(cimg::Win32_attr().wait_event);
#endif
		}
		return *this;
	}

	//! Flush all display events.
	/**
	\note Remove all passed events from the current display.
	**/
	CImgDisplay& flush() {
		set_key().set_button().set_wheel();
		_is_resized = _is_moved = _is_event = false;
		_fps_timer = _fps_frames = _timer = 0;
		_fps_fps = 0;
		return *this;
	}

	//! Wait for any user event occuring on the current display.
	CImgDisplay& wait() {
		wait(*this);
		return *this;
	}

	//! Wait for a given number of milliseconds since the last call to wait().
	/**
	\param milliseconds Number of milliseconds to wait for.
	\note Similar to cimg::wait().
	**/
	CImgDisplay& wait(const unsigned int milliseconds) {
		cimg::_wait(milliseconds, _timer);
		return *this;
	}

	//! Wait for any event occuring on the display \c disp1.
	static void wait(CImgDisplay& disp1) {
		disp1._is_event = false;
		while (!disp1._is_closed && !disp1._is_event) wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1 or \c disp2.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2) {
		disp1._is_event = disp2._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed) &&
			!disp1._is_event && !disp2._is_event) wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1, \c disp2 or \c disp3.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2, CImgDisplay& disp3) {
		disp1._is_event = disp2._is_event = disp3._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed || !disp3._is_closed) &&
			!disp1._is_event && !disp2._is_event && !disp3._is_event) wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1, \c disp2, \c disp3 or \c disp4.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2, CImgDisplay& disp3, CImgDisplay& disp4) {
		disp1._is_event = disp2._is_event = disp3._is_event = disp4._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed || !disp3._is_closed || !disp4._is_closed) &&
			!disp1._is_event && !disp2._is_event && !disp3._is_event && !disp4._is_event) wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1, \c disp2, \c disp3, \c disp4 or \c disp5.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2, CImgDisplay& disp3, CImgDisplay& disp4,
		CImgDisplay& disp5) {
		disp1._is_event = disp2._is_event = disp3._is_event = disp4._is_event = disp5._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed || !disp3._is_closed || !disp4._is_closed || !disp5._is_closed) &&
			!disp1._is_event && !disp2._is_event && !disp3._is_event && !disp4._is_event && !disp5._is_event)
			wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1, \c disp2, \c disp3, \c disp4, ... \c disp6.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2, CImgDisplay& disp3, CImgDisplay& disp4, CImgDisplay& disp5,
		CImgDisplay& disp6) {
		disp1._is_event = disp2._is_event = disp3._is_event = disp4._is_event = disp5._is_event =
			disp6._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed || !disp3._is_closed || !disp4._is_closed || !disp5._is_closed ||
			!disp6._is_closed) &&
			!disp1._is_event && !disp2._is_event && !disp3._is_event && !disp4._is_event && !disp5._is_event &&
			!disp6._is_event) wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1, \c disp2, \c disp3, \c disp4, ... \c disp7.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2, CImgDisplay& disp3, CImgDisplay& disp4, CImgDisplay& disp5,
		CImgDisplay& disp6, CImgDisplay& disp7) {
		disp1._is_event = disp2._is_event = disp3._is_event = disp4._is_event = disp5._is_event =
			disp6._is_event = disp7._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed || !disp3._is_closed || !disp4._is_closed || !disp5._is_closed ||
			!disp6._is_closed || !disp7._is_closed) &&
			!disp1._is_event && !disp2._is_event && !disp3._is_event && !disp4._is_event && !disp5._is_event &&
			!disp6._is_event && !disp7._is_event) wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1, \c disp2, \c disp3, \c disp4, ... \c disp8.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2, CImgDisplay& disp3, CImgDisplay& disp4, CImgDisplay& disp5,
		CImgDisplay& disp6, CImgDisplay& disp7, CImgDisplay& disp8) {
		disp1._is_event = disp2._is_event = disp3._is_event = disp4._is_event = disp5._is_event =
			disp6._is_event = disp7._is_event = disp8._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed || !disp3._is_closed || !disp4._is_closed || !disp5._is_closed ||
			!disp6._is_closed || !disp7._is_closed || !disp8._is_closed) &&
			!disp1._is_event && !disp2._is_event && !disp3._is_event && !disp4._is_event && !disp5._is_event &&
			!disp6._is_event && !disp7._is_event && !disp8._is_event) wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1, \c disp2, \c disp3, \c disp4, ... \c disp9.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2, CImgDisplay& disp3, CImgDisplay& disp4, CImgDisplay& disp5,
		CImgDisplay& disp6, CImgDisplay& disp7, CImgDisplay& disp8, CImgDisplay& disp9) {
		disp1._is_event = disp2._is_event = disp3._is_event = disp4._is_event = disp5._is_event =
			disp6._is_event = disp7._is_event = disp8._is_event = disp9._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed || !disp3._is_closed || !disp4._is_closed || !disp5._is_closed ||
			!disp6._is_closed || !disp7._is_closed || !disp8._is_closed || !disp9._is_closed) &&
			!disp1._is_event && !disp2._is_event && !disp3._is_event && !disp4._is_event && !disp5._is_event &&
			!disp6._is_event && !disp7._is_event && !disp8._is_event && !disp9._is_event) wait_all();
	}

	//! Wait for any event occuring either on the display \c disp1, \c disp2, \c disp3, \c disp4, ... \c disp10.
	static void wait(CImgDisplay& disp1, CImgDisplay& disp2, CImgDisplay& disp3, CImgDisplay& disp4, CImgDisplay& disp5,
		CImgDisplay& disp6, CImgDisplay& disp7, CImgDisplay& disp8, CImgDisplay& disp9,
		CImgDisplay& disp10) {
		disp1._is_event = disp2._is_event = disp3._is_event = disp4._is_event = disp5._is_event =
			disp6._is_event = disp7._is_event = disp8._is_event = disp9._is_event = disp10._is_event = false;
		while ((!disp1._is_closed || !disp2._is_closed || !disp3._is_closed || !disp4._is_closed || !disp5._is_closed ||
			!disp6._is_closed || !disp7._is_closed || !disp8._is_closed || !disp9._is_closed || !disp10._is_closed) &&
			!disp1._is_event && !disp2._is_event && !disp3._is_event && !disp4._is_event && !disp5._is_event &&
			!disp6._is_event && !disp7._is_event && !disp8._is_event && !disp9._is_event && !disp10._is_event)
			wait_all();
	}

#if cimg_display==0

	//! Wait for any window event occuring in any opened CImgDisplay.
	static void wait_all() {
		return _no_display_exception();
	}

	//! Render image into internal display buffer.
	/**
	\param img Input image data to render.
	\note
	- Convert image data representation into the internal display buffer (architecture-dependent structure).
	- The content of the associated window is not modified, until paint() is called.
	- Should not be used for common CImgDisplay uses, since display() is more useful.
	**/
	template<typename T>
	CImgDisplay& render(const CImg<T>& img) {
		return assign(img);
	}

	//! Paint internal display buffer on associated window.
	/**
	\note
	- Update the content of the associated window with the internal display buffer, e.g. after a render() call.
	- Should not be used for common CImgDisplay uses, since display() is more useful.
	**/
	CImgDisplay& paint() {
		return assign();
	}


	//! Take a snapshot of the current screen content.
	/**
	\param x0 X-coordinate of the upper left corner.
	\param y0 Y-coordinate of the upper left corner.
	\param x1 X-coordinate of the lower right corner.
	\param y1 Y-coordinate of the lower right corner.
	\param[out] img Output screenshot. Can be empty on input
	**/
	template<typename T>
	static void screenshot(const int x0, const int y0, const int x1, const int y1, CImg<T>& img) {
		cimg::unused(x0, y0, x1, y1, &img);
		_no_display_exception();
	}

	//! Take a snapshot of the associated window content.
	/**
	\param[out] img Output snapshot. Can be empty on input.
	**/
	template<typename T>
	const CImgDisplay& snapshot(CImg<T>& img) const {
		cimg::unused(img);
		_no_display_exception();
		return *this;
	}
#endif

	// X11-based implementation
	//--------------------------
#if cimg_display==1

	Atom _wm_window_atom, _wm_protocol_atom;
	Window _window, _background_window;
	Colormap _colormap;
	XImage *_image;
	void *_data;
#ifdef cimg_use_xshm
	XShmSegmentInfo *_shminfo;
#endif

	static int screen_width() {
		Display *const dpy = cimg::X11_attr().display;
		int res = 0;
		if (!dpy) {
			Display *const _dpy = XOpenDisplay(0);
			if (!_dpy)
				throw CImgDisplayException("CImgDisplay::screen_width(): Failed to open X11 display.");
			res = DisplayWidth(_dpy, DefaultScreen(_dpy));
			XCloseDisplay(_dpy);
		}
		else {
#ifdef cimg_use_xrandr
			if (cimg::X11_attr().resolutions && cimg::X11_attr().curr_resolution)
				res = cimg::X11_attr().resolutions[cimg::X11_attr().curr_resolution].width;
			else res = DisplayWidth(dpy, DefaultScreen(dpy));
#else
			res = DisplayWidth(dpy, DefaultScreen(dpy));
#endif
		}
		return res;
	}

	static int screen_height() {
		Display *const dpy = cimg::X11_attr().display;
		int res = 0;
		if (!dpy) {
			Display *const _dpy = XOpenDisplay(0);
			if (!_dpy)
				throw CImgDisplayException("CImgDisplay::screen_height(): Failed to open X11 display.");
			res = DisplayHeight(_dpy, DefaultScreen(_dpy));
			XCloseDisplay(_dpy);
		}
		else {
#ifdef cimg_use_xrandr
			if (cimg::X11_attr().resolutions && cimg::X11_attr().curr_resolution)
				res = cimg::X11_attr().resolutions[cimg::X11_attr().curr_resolution].height;
			else res = DisplayHeight(dpy, DefaultScreen(dpy));
#else
			res = DisplayHeight(dpy, DefaultScreen(dpy));
#endif
		}
		return res;
	}

	static void wait_all() {
		if (!cimg::X11_attr().display) return;
		pthread_mutex_lock(&cimg::X11_attr().wait_event_mutex);
		pthread_cond_wait(&cimg::X11_attr().wait_event, &cimg::X11_attr().wait_event_mutex);
		pthread_mutex_unlock(&cimg::X11_attr().wait_event_mutex);
	}

	void _handle_events(const XEvent *const pevent) {
		Display *const dpy = cimg::X11_attr().display;
		XEvent event = *pevent;
		switch (event.type) {
		case ClientMessage: {
			if ((int)event.xclient.message_type == (int)_wm_protocol_atom &&
				(int)event.xclient.data.l[0] == (int)_wm_window_atom) {
				XUnmapWindow(cimg::X11_attr().display, _window);
				_is_closed = _is_event = true;
				pthread_cond_broadcast(&cimg::X11_attr().wait_event);
			}
		} break;
		case ConfigureNotify: {
			while (XCheckWindowEvent(dpy, _window, StructureNotifyMask, &event)) {}
			const unsigned int nw = event.xconfigure.width, nh = event.xconfigure.height;
			const int nx = event.xconfigure.x, ny = event.xconfigure.y;
			if (nw && nh && (nw != _window_width || nh != _window_height)) {
				_window_width = nw; _window_height = nh; _mouse_x = _mouse_y = -1;
				XResizeWindow(dpy, _window, _window_width, _window_height);
				_is_resized = _is_event = true;
				pthread_cond_broadcast(&cimg::X11_attr().wait_event);
			}
			if (nx != _window_x || ny != _window_y) {
				_window_x = nx; _window_y = ny; _is_moved = _is_event = true;
				pthread_cond_broadcast(&cimg::X11_attr().wait_event);
			}
		} break;
		case Expose: {
			while (XCheckWindowEvent(dpy, _window, ExposureMask, &event)) {}
			_paint(false);
			if (_is_fullscreen) {
				XWindowAttributes attr;
				XGetWindowAttributes(dpy, _window, &attr);
				while (attr.map_state != IsViewable) XSync(dpy, 0);
				XSetInputFocus(dpy, _window, RevertToParent, CurrentTime);
			}
		} break;
		case ButtonPress: {
			do {
				_mouse_x = event.xmotion.x; _mouse_y = event.xmotion.y;
				if (_mouse_x<0 || _mouse_y<0 || _mouse_x >= width() || _mouse_y >= height()) _mouse_x = _mouse_y = -1;
				switch (event.xbutton.button) {
				case 1: set_button(1); break;
				case 3: set_button(2); break;
				case 2: set_button(3); break;
				}
			} while (XCheckWindowEvent(dpy, _window, ButtonPressMask, &event));
		} break;
		case ButtonRelease: {
			do {
				_mouse_x = event.xmotion.x; _mouse_y = event.xmotion.y;
				if (_mouse_x<0 || _mouse_y<0 || _mouse_x >= width() || _mouse_y >= height()) _mouse_x = _mouse_y = -1;
				switch (event.xbutton.button) {
				case 1: set_button(1, false); break;
				case 3: set_button(2, false); break;
				case 2: set_button(3, false); break;
				case 4: set_wheel(1); break;
				case 5: set_wheel(-1); break;
				}
			} while (XCheckWindowEvent(dpy, _window, ButtonReleaseMask, &event));
		} break;
		case KeyPress: {
			char tmp = 0; KeySym ksym;
			XLookupString(&event.xkey, &tmp, 1, &ksym, 0);
			set_key((unsigned int)ksym, true);
		} break;
		case KeyRelease: {
			char keys_return[32];  // Check that the key has been physically unpressed.
			XQueryKeymap(dpy, keys_return);
			const unsigned int kc = event.xkey.keycode, kc1 = kc / 8, kc2 = kc % 8;
			const bool is_key_pressed = kc1 >= 32 ? false : (keys_return[kc1] >> kc2) & 1;
			if (!is_key_pressed) {
				char tmp = 0; KeySym ksym;
				XLookupString(&event.xkey, &tmp, 1, &ksym, 0);
				set_key((unsigned int)ksym, false);
			}
		} break;
		case EnterNotify: {
			while (XCheckWindowEvent(dpy, _window, EnterWindowMask, &event)) {}
			_mouse_x = event.xmotion.x;
			_mouse_y = event.xmotion.y;
			if (_mouse_x<0 || _mouse_y<0 || _mouse_x >= width() || _mouse_y >= height()) _mouse_x = _mouse_y = -1;
		} break;
		case LeaveNotify: {
			while (XCheckWindowEvent(dpy, _window, LeaveWindowMask, &event)) {}
			_mouse_x = _mouse_y = -1; _is_event = true;
			pthread_cond_broadcast(&cimg::X11_attr().wait_event);
		} break;
		case MotionNotify: {
			while (XCheckWindowEvent(dpy, _window, PointerMotionMask, &event)) {}
			_mouse_x = event.xmotion.x;
			_mouse_y = event.xmotion.y;
			if (_mouse_x<0 || _mouse_y<0 || _mouse_x >= width() || _mouse_y >= height()) _mouse_x = _mouse_y = -1;
			_is_event = true;
			pthread_cond_broadcast(&cimg::X11_attr().wait_event);
		} break;
		}
	}

	static void* _events_thread(void *arg) { // Thread to manage events for all opened display windows.
		Display *const dpy = cimg::X11_attr().display;
		XEvent event;
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
		if (!arg) for (; ; ) {
			cimg_lock_display();
			bool event_flag = XCheckTypedEvent(dpy, ClientMessage, &event);
			if (!event_flag) event_flag = XCheckMaskEvent(dpy,
				ExposureMask | StructureNotifyMask | ButtonPressMask |
				KeyPressMask | PointerMotionMask | EnterWindowMask |
				LeaveWindowMask | ButtonReleaseMask | KeyReleaseMask, &event);
			if (event_flag)
				for (unsigned int i = 0; i<cimg::X11_attr().nb_wins; ++i)
					if (!cimg::X11_attr().wins[i]->_is_closed && event.xany.window == cimg::X11_attr().wins[i]->_window)
						cimg::X11_attr().wins[i]->_handle_events(&event);
			cimg_unlock_display();
			pthread_testcancel();
			cimg::sleep(8);
		}
		return 0;
	}

	void _set_colormap(Colormap& _colormap, const unsigned int dim) {
		XColor *const colormap = new XColor[256];
		switch (dim) {
		case 1: { // colormap for greyscale images
			for (unsigned int index = 0; index<256; ++index) {
				colormap[index].pixel = index;
				colormap[index].red = colormap[index].green = colormap[index].blue = (unsigned short)(index << 8);
				colormap[index].flags = DoRed | DoGreen | DoBlue;
			}
		} break;
		case 2: { // colormap for RG images
			for (unsigned int index = 0, r = 8; r<256; r += 16)
				for (unsigned int g = 8; g<256; g += 16) {
					colormap[index].pixel = index;
					colormap[index].red = colormap[index].blue = (unsigned short)(r << 8);
					colormap[index].green = (unsigned short)(g << 8);
					colormap[index++].flags = DoRed | DoGreen | DoBlue;
				}
		} break;
		default: { // colormap for RGB images
			for (unsigned int index = 0, r = 16; r<256; r += 32)
				for (unsigned int g = 16; g<256; g += 32)
					for (unsigned int b = 32; b<256; b += 64) {
						colormap[index].pixel = index;
						colormap[index].red = (unsigned short)(r << 8);
						colormap[index].green = (unsigned short)(g << 8);
						colormap[index].blue = (unsigned short)(b << 8);
						colormap[index++].flags = DoRed | DoGreen | DoBlue;
					}
		}
		}
		XStoreColors(cimg::X11_attr().display, _colormap, colormap, 256);
		delete[] colormap;
	}

	void _map_window() {
		Display *const dpy = cimg::X11_attr().display;
		bool is_exposed = false, is_mapped = false;
		XWindowAttributes attr;
		XEvent event;
		XMapRaised(dpy, _window);
		do { // Wait for the window to be mapped.
			XWindowEvent(dpy, _window, StructureNotifyMask | ExposureMask, &event);
			switch (event.type) {
			case MapNotify: is_mapped = true; break;
			case Expose: is_exposed = true; break;
			}
		} while (!is_exposed || !is_mapped);
		do { // Wait for the window to be visible.
			XGetWindowAttributes(dpy, _window, &attr);
			if (attr.map_state != IsViewable) { XSync(dpy, 0); cimg::sleep(10); }
		} while (attr.map_state != IsViewable);
		_window_x = attr.x;
		_window_y = attr.y;
	}

	void _paint(const bool wait_expose = true) {
		if (_is_closed || !_image) return;
		Display *const dpy = cimg::X11_attr().display;
		if (wait_expose) { // Send an expose event sticked to display window to force repaint.
			XEvent event;
			event.xexpose.type = Expose;
			event.xexpose.serial = 0;
			event.xexpose.send_event = 1;
			event.xexpose.display = dpy;
			event.xexpose.window = _window;
			event.xexpose.x = 0;
			event.xexpose.y = 0;
			event.xexpose.width = width();
			event.xexpose.height = height();
			event.xexpose.count = 0;
			XSendEvent(dpy, _window, 0, 0, &event);
		}
		else { // Repaint directly (may be called from the expose event).
			GC gc = DefaultGC(dpy, DefaultScreen(dpy));
#ifdef cimg_use_xshm
			if (_shminfo) XShmPutImage(dpy, _window, gc, _image, 0, 0, 0, 0, _width, _height, 1);
			else XPutImage(dpy, _window, gc, _image, 0, 0, 0, 0, _width, _height);
#else
			XPutImage(dpy, _window, gc, _image, 0, 0, 0, 0, _width, _height);
#endif
		}
	}

	template<typename T>
	void _resize(T pixel_type, const unsigned int ndimx, const unsigned int ndimy, const bool force_redraw) {
		Display *const dpy = cimg::X11_attr().display;
		cimg::unused(pixel_type);

#ifdef cimg_use_xshm
		if (_shminfo) {
			XShmSegmentInfo *const nshminfo = new XShmSegmentInfo;
			XImage *const nimage = XShmCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
				cimg::X11_attr().nb_bits, ZPixmap, 0, nshminfo, ndimx, ndimy);
			if (!nimage) { delete nshminfo; return; }
			else {
				nshminfo->shmid = shmget(IPC_PRIVATE, ndimx*ndimy * sizeof(T), IPC_CREAT | 0777);
				if (nshminfo->shmid == -1) { XDestroyImage(nimage); delete nshminfo; return; }
				else {
					nshminfo->shmaddr = nimage->data = (char*)shmat(nshminfo->shmid, 0, 0);
					if (nshminfo->shmaddr == (char*)-1) {
						shmctl(nshminfo->shmid, IPC_RMID, 0); XDestroyImage(nimage); delete nshminfo; return;
					}
					else {
						nshminfo->readOnly = 0;
						cimg::X11_attr().is_shm_enabled = true;
						XErrorHandler oldXErrorHandler = XSetErrorHandler(_assign_xshm);
						XShmAttach(dpy, nshminfo);
						XFlush(dpy);
						XSetErrorHandler(oldXErrorHandler);
						if (!cimg::X11_attr().is_shm_enabled) {
							shmdt(nshminfo->shmaddr);
							shmctl(nshminfo->shmid, IPC_RMID, 0);
							XDestroyImage(nimage);
							delete nshminfo;
							return;
						}
						else {
							T *const ndata = (T*)nimage->data;
							if (force_redraw) _render_resize((T*)_data, _width, _height, ndata, ndimx, ndimy);
							else std::memset(ndata, 0, sizeof(T)*ndimx*ndimy);
							XShmDetach(dpy, _shminfo);
							XDestroyImage(_image);
							shmdt(_shminfo->shmaddr);
							shmctl(_shminfo->shmid, IPC_RMID, 0);
							delete _shminfo;
							_shminfo = nshminfo;
							_image = nimage;
							_data = (void*)ndata;
						}
					}
				}
			}
		}
		else
#endif
		{
			T *ndata = (T*)std::malloc(ndimx*ndimy * sizeof(T));
			if (force_redraw) _render_resize((T*)_data, _width, _height, ndata, ndimx, ndimy);
			else std::memset(ndata, 0, sizeof(T)*ndimx*ndimy);
			_data = (void*)ndata;
			XDestroyImage(_image);
			_image = XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
				cimg::X11_attr().nb_bits, ZPixmap, 0, (char*)_data, ndimx, ndimy, 8, 0);
		}
	}

	void _init_fullscreen() {
		if (!_is_fullscreen || _is_closed) return;
		Display *const dpy = cimg::X11_attr().display;
		_background_window = 0;

#ifdef cimg_use_xrandr
		int foo;
		if (XRRQueryExtension(dpy, &foo, &foo)) {
			XRRRotations(dpy, DefaultScreen(dpy), &cimg::X11_attr().curr_rotation);
			if (!cimg::X11_attr().resolutions) {
				cimg::X11_attr().resolutions = XRRSizes(dpy, DefaultScreen(dpy), &foo);
				cimg::X11_attr().nb_resolutions = (unsigned int)foo;
			}
			if (cimg::X11_attr().resolutions) {
				cimg::X11_attr().curr_resolution = 0;
				for (unsigned int i = 0; i<cimg::X11_attr().nb_resolutions; ++i) {
					const unsigned int
						nw = (unsigned int)(cimg::X11_attr().resolutions[i].width),
						nh = (unsigned int)(cimg::X11_attr().resolutions[i].height);
					if (nw >= _width && nh >= _height &&
						nw <= (unsigned int)(cimg::X11_attr().resolutions[cimg::X11_attr().curr_resolution].width) &&
						nh <= (unsigned int)(cimg::X11_attr().resolutions[cimg::X11_attr().curr_resolution].height))
						cimg::X11_attr().curr_resolution = i;
				}
				if (cimg::X11_attr().curr_resolution>0) {
					XRRScreenConfiguration *config = XRRGetScreenInfo(dpy, DefaultRootWindow(dpy));
					XRRSetScreenConfig(dpy, config, DefaultRootWindow(dpy),
						cimg::X11_attr().curr_resolution, cimg::X11_attr().curr_rotation, CurrentTime);
					XRRFreeScreenConfigInfo(config);
					XSync(dpy, 0);
				}
			}
		}
		if (!cimg::X11_attr().resolutions)
			cimg::warn(_cimgdisplay_instance
				"init_fullscreen(): Xrandr extension not supported by the X server.",
				cimgdisplay_instance);
#endif

		const unsigned int sx = screen_width(), sy = screen_height();
		if (sx == _width && sy == _height) return;
		XSetWindowAttributes winattr;
		winattr.override_redirect = 1;
		_background_window = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, sx, sy, 0, 0,
			InputOutput, CopyFromParent, CWOverrideRedirect, &winattr);
		const cimg_ulong buf_size = (cimg_ulong)sx*sy*(cimg::X11_attr().nb_bits == 8 ? 1 :
			(cimg::X11_attr().nb_bits == 16 ? 2 : 4));
		void *background_data = std::malloc(buf_size);
		std::memset(background_data, 0, buf_size);
		XImage *background_image = XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), cimg::X11_attr().nb_bits,
			ZPixmap, 0, (char*)background_data, sx, sy, 8, 0);
		XEvent event;
		XSelectInput(dpy, _background_window, StructureNotifyMask);
		XMapRaised(dpy, _background_window);
		do XWindowEvent(dpy, _background_window, StructureNotifyMask, &event);
		while (event.type != MapNotify);
		GC gc = DefaultGC(dpy, DefaultScreen(dpy));
#ifdef cimg_use_xshm
		if (_shminfo) XShmPutImage(dpy, _background_window, gc, background_image, 0, 0, 0, 0, sx, sy, 0);
		else XPutImage(dpy, _background_window, gc, background_image, 0, 0, 0, 0, sx, sy);
#else
		XPutImage(dpy, _background_window, gc, background_image, 0, 0, 0, 0, sx, sy);
#endif
		XWindowAttributes attr;
		XGetWindowAttributes(dpy, _background_window, &attr);
		while (attr.map_state != IsViewable) XSync(dpy, 0);
		XDestroyImage(background_image);
	}

	void _desinit_fullscreen() {
		if (!_is_fullscreen) return;
		Display *const dpy = cimg::X11_attr().display;
		XUngrabKeyboard(dpy, CurrentTime);
#ifdef cimg_use_xrandr
		if (cimg::X11_attr().resolutions && cimg::X11_attr().curr_resolution) {
			XRRScreenConfiguration *config = XRRGetScreenInfo(dpy, DefaultRootWindow(dpy));
			XRRSetScreenConfig(dpy, config, DefaultRootWindow(dpy), 0, cimg::X11_attr().curr_rotation, CurrentTime);
			XRRFreeScreenConfigInfo(config);
			XSync(dpy, 0);
			cimg::X11_attr().curr_resolution = 0;
		}
#endif
		if (_background_window) XDestroyWindow(dpy, _background_window);
		_background_window = 0;
		_is_fullscreen = false;
	}

	static int _assign_xshm(Display *dpy, XErrorEvent *error) {
		cimg::unused(dpy, error);
		cimg::X11_attr().is_shm_enabled = false;
		return 0;
	}

	void _assign(const unsigned int dimw, const unsigned int dimh, const char *const ptitle = 0,
		const unsigned int normalization_type = 3,
		const bool fullscreen_flag = false, const bool closed_flag = false) {
		cimg::mutex(14);

		// Allocate space for window title
		const char *const nptitle = ptitle ? ptitle : "";
		const unsigned int s = (unsigned int)std::strlen(nptitle) + 1;
		char *const tmp_title = s ? new char[s] : 0;
		if (s) std::memcpy(tmp_title, nptitle, s * sizeof(char));

		// Destroy previous display window if existing
		if (!is_empty()) assign();

		// Open X11 display and retrieve graphical properties.
		Display* &dpy = cimg::X11_attr().display;
		if (!dpy) {
			dpy = XOpenDisplay(0);
			if (!dpy)
				throw CImgDisplayException(_cimgdisplay_instance
					"assign(): Failed to open X11 display.",
					cimgdisplay_instance);

			cimg::X11_attr().nb_bits = DefaultDepth(dpy, DefaultScreen(dpy));
			if (cimg::X11_attr().nb_bits != 8 && cimg::X11_attr().nb_bits != 16 &&
				cimg::X11_attr().nb_bits != 24 && cimg::X11_attr().nb_bits != 32)
				throw CImgDisplayException(_cimgdisplay_instance
					"assign(): Invalid %u bits screen mode detected "
					"(only 8, 16, 24 and 32 bits modes are managed).",
					cimgdisplay_instance,
					cimg::X11_attr().nb_bits);
			XVisualInfo vtemplate;
			vtemplate.visualid = XVisualIDFromVisual(DefaultVisual(dpy, DefaultScreen(dpy)));
			int nb_visuals;
			XVisualInfo *vinfo = XGetVisualInfo(dpy, VisualIDMask, &vtemplate, &nb_visuals);
			if (vinfo && vinfo->red_mask<vinfo->blue_mask) cimg::X11_attr().is_blue_first = true;
			cimg::X11_attr().byte_order = ImageByteOrder(dpy);
			XFree(vinfo);

			cimg_lock_display();
			cimg::X11_attr().events_thread = new pthread_t;
			pthread_create(cimg::X11_attr().events_thread, 0, _events_thread, 0);
		}
		else cimg_lock_display();

		// Set display variables.
		_width = std::min(dimw, (unsigned int)screen_width());
		_height = std::min(dimh, (unsigned int)screen_height());
		_normalization = normalization_type<4 ? normalization_type : 3;
		_is_fullscreen = fullscreen_flag;
		_window_x = _window_y = 0;
		_is_closed = closed_flag;
		_title = tmp_title;
		flush();

		// Create X11 window (and LUT, if 8bits display)
		if (_is_fullscreen) {
			if (!_is_closed) _init_fullscreen();
			const unsigned int sx = screen_width(), sy = screen_height();
			XSetWindowAttributes winattr;
			winattr.override_redirect = 1;
			_window = XCreateWindow(dpy, DefaultRootWindow(dpy), (sx - _width) / 2, (sy - _height) / 2, _width, _height, 0, 0,
				InputOutput, CopyFromParent, CWOverrideRedirect, &winattr);
		}
		else
			_window = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, _width, _height, 0, 0L, 0L);

		XSelectInput(dpy, _window,
			ExposureMask | StructureNotifyMask | ButtonPressMask | KeyPressMask | PointerMotionMask |
			EnterWindowMask | LeaveWindowMask | ButtonReleaseMask | KeyReleaseMask);

		XStoreName(dpy, _window, _title ? _title : " ");
		if (cimg::X11_attr().nb_bits == 8) {
			_colormap = XCreateColormap(dpy, _window, DefaultVisual(dpy, DefaultScreen(dpy)), AllocAll);
			_set_colormap(_colormap, 3);
			XSetWindowColormap(dpy, _window, _colormap);
		}

		static const char *const _window_class = cimg_appname;
		XClassHint *const window_class = XAllocClassHint();
		window_class->res_name = (char*)_window_class;
		window_class->res_class = (char*)_window_class;
		XSetClassHint(dpy, _window, window_class);
		XFree(window_class);

		_window_width = _width;
		_window_height = _height;

		// Create XImage
#ifdef cimg_use_xshm
		_shminfo = 0;
		if (XShmQueryExtension(dpy)) {
			_shminfo = new XShmSegmentInfo;
			_image = XShmCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), cimg::X11_attr().nb_bits,
				ZPixmap, 0, _shminfo, _width, _height);
			if (!_image) { delete _shminfo; _shminfo = 0; }
			else {
				_shminfo->shmid = shmget(IPC_PRIVATE, _image->bytes_per_line*_image->height, IPC_CREAT | 0777);
				if (_shminfo->shmid == -1) { XDestroyImage(_image); delete _shminfo; _shminfo = 0; }
				else {
					_shminfo->shmaddr = _image->data = (char*)(_data = shmat(_shminfo->shmid, 0, 0));
					if (_shminfo->shmaddr == (char*)-1) {
						shmctl(_shminfo->shmid, IPC_RMID, 0); XDestroyImage(_image); delete _shminfo; _shminfo = 0;
					}
					else {
						_shminfo->readOnly = 0;
						cimg::X11_attr().is_shm_enabled = true;
						XErrorHandler oldXErrorHandler = XSetErrorHandler(_assign_xshm);
						XShmAttach(dpy, _shminfo);
						XSync(dpy, 0);
						XSetErrorHandler(oldXErrorHandler);
						if (!cimg::X11_attr().is_shm_enabled) {
							shmdt(_shminfo->shmaddr); shmctl(_shminfo->shmid, IPC_RMID, 0); XDestroyImage(_image);
							delete _shminfo; _shminfo = 0;
						}
					}
				}
			}
		}
		if (!_shminfo)
#endif
		{
			const cimg_ulong buf_size = (cimg_ulong)_width*_height*(cimg::X11_attr().nb_bits == 8 ? 1 :
				(cimg::X11_attr().nb_bits == 16 ? 2 : 4));
			_data = std::malloc(buf_size);
			_image = XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), cimg::X11_attr().nb_bits,
				ZPixmap, 0, (char*)_data, _width, _height, 8, 0);
		}

		_wm_window_atom = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);
		_wm_protocol_atom = XInternAtom(dpy, "WM_PROTOCOLS", 0);
		XSetWMProtocols(dpy, _window, &_wm_window_atom, 1);

		if (_is_fullscreen) XGrabKeyboard(dpy, _window, 1, GrabModeAsync, GrabModeAsync, CurrentTime);
		cimg::X11_attr().wins[cimg::X11_attr().nb_wins++] = this;
		if (!_is_closed) _map_window(); else { _window_x = _window_y = cimg::type<int>::min(); }
		cimg_unlock_display();
		cimg::mutex(14, 0);
	}

	CImgDisplay& assign() {
		if (is_empty()) return flush();
		Display *const dpy = cimg::X11_attr().display;
		cimg_lock_display();

		// Remove display window from event thread list.
		unsigned int i;
		for (i = 0; i<cimg::X11_attr().nb_wins && cimg::X11_attr().wins[i] != this; ++i) {}
		for (; i<cimg::X11_attr().nb_wins - 1; ++i) cimg::X11_attr().wins[i] = cimg::X11_attr().wins[i + 1];
		--cimg::X11_attr().nb_wins;

		// Destroy window, image, colormap and title.
		if (_is_fullscreen && !_is_closed) _desinit_fullscreen();
		XDestroyWindow(dpy, _window);
		_window = 0;
#ifdef cimg_use_xshm
		if (_shminfo) {
			XShmDetach(dpy, _shminfo);
			XDestroyImage(_image);
			shmdt(_shminfo->shmaddr);
			shmctl(_shminfo->shmid, IPC_RMID, 0);
			delete _shminfo;
			_shminfo = 0;
		}
		else
#endif
			XDestroyImage(_image);
		_data = 0; _image = 0;
		if (cimg::X11_attr().nb_bits == 8) XFreeColormap(dpy, _colormap);
		_colormap = 0;
		XSync(dpy, 0);

		// Reset display variables.
		delete[] _title;
		_width = _height = _normalization = _window_width = _window_height = 0;
		_window_x = _window_y = 0;
		_is_fullscreen = false;
		_is_closed = true;
		_min = _max = 0;
		_title = 0;
		flush();

		cimg_unlock_display();
		return *this;
	}

	CImgDisplay& assign(const unsigned int dimw, const unsigned int dimh, const char *const title = 0,
		const unsigned int normalization_type = 3,
		const bool fullscreen_flag = false, const bool closed_flag = false) {
		if (!dimw || !dimh) return assign();
		_assign(dimw, dimh, title, normalization_type, fullscreen_flag, closed_flag);
		_min = _max = 0;
		std::memset(_data, 0, (cimg::X11_attr().nb_bits == 8 ? sizeof(unsigned char) :
			(cimg::X11_attr().nb_bits == 16 ? sizeof(unsigned short) : sizeof(unsigned int)))*
			(size_t)_width*_height);
		return paint();
	}

	template<typename T>
	CImgDisplay& assign(const CImg<T>& img, const char *const title = 0,
		const unsigned int normalization_type = 3,
		const bool fullscreen_flag = false, const bool closed_flag = false) {
		if (!img) return assign();
		CImg<T> tmp;
		const CImg<T>& nimg = (img._depth == 1) ? img : (tmp = img.get_projections2d((img._width - 1) / 2,
			(img._height - 1) / 2,
			(img._depth - 1) / 2));
		_assign(nimg._width, nimg._height, title, normalization_type, fullscreen_flag, closed_flag);
		if (_normalization == 2) _min = (float)nimg.min_max(_max);
		return render(nimg).paint();
	}

	template<typename T>
	CImgDisplay& assign(const CImgList<T>& list, const char *const title = 0,
		const unsigned int normalization_type = 3,
		const bool fullscreen_flag = false, const bool closed_flag = false) {
		if (!list) return assign();
		CImg<T> tmp;
		const CImg<T> img = list>'x', &nimg = (img._depth == 1) ? img : (tmp = img.get_projections2d((img._width - 1) / 2,
			(img._height - 1) / 2,
			(img._depth - 1) / 2));
		_assign(nimg._width, nimg._height, title, normalization_type, fullscreen_flag, closed_flag);
		if (_normalization == 2) _min = (float)nimg.min_max(_max);
		return render(nimg).paint();
	}

	CImgDisplay& assign(const CImgDisplay& disp) {
		if (!disp) return assign();
		_assign(disp._width, disp._height, disp._title, disp._normalization, disp._is_fullscreen, disp._is_closed);
		std::memcpy(_data, disp._data, (cimg::X11_attr().nb_bits == 8 ? sizeof(unsigned char) :
			cimg::X11_attr().nb_bits == 16 ? sizeof(unsigned short) :
			sizeof(unsigned int))*(size_t)_width*_height);
		return paint();
	}

	CImgDisplay& resize(const int nwidth, const int nheight, const bool force_redraw = true) {
		if (!nwidth || !nheight || (is_empty() && (nwidth<0 || nheight<0))) return assign();
		if (is_empty()) return assign(nwidth, nheight);
		Display *const dpy = cimg::X11_attr().display;
		const unsigned int
			tmpdimx = (nwidth>0) ? nwidth : (-nwidth*width() / 100),
			tmpdimy = (nheight>0) ? nheight : (-nheight*height() / 100),
			dimx = tmpdimx ? tmpdimx : 1,
			dimy = tmpdimy ? tmpdimy : 1;
		if (_width != dimx || _height != dimy || _window_width != dimx || _window_height != dimy) {
			show();
			cimg_lock_display();
			if (_window_width != dimx || _window_height != dimy) {
				XWindowAttributes attr;
				for (unsigned int i = 0; i<10; ++i) {
					XResizeWindow(dpy, _window, dimx, dimy);
					XGetWindowAttributes(dpy, _window, &attr);
					if (attr.width == (int)dimx && attr.height == (int)dimy) break;
					cimg::wait(5);
				}
			}
			if (_width != dimx || _height != dimy) switch (cimg::X11_attr().nb_bits) {
			case 8: { unsigned char pixel_type = 0; _resize(pixel_type, dimx, dimy, force_redraw); } break;
			case 16: { unsigned short pixel_type = 0; _resize(pixel_type, dimx, dimy, force_redraw); } break;
			default: { unsigned int pixel_type = 0; _resize(pixel_type, dimx, dimy, force_redraw); }
			}
			_window_width = _width = dimx; _window_height = _height = dimy;
			cimg_unlock_display();
		}
		_is_resized = false;
		if (_is_fullscreen) move((screen_width() - _width) / 2, (screen_height() - _height) / 2);
		if (force_redraw) return paint();
		return *this;
	}

	CImgDisplay& toggle_fullscreen(const bool force_redraw = true) {
		if (is_empty()) return *this;
		if (force_redraw) {
			const cimg_ulong buf_size = (cimg_ulong)_width*_height*
				(cimg::X11_attr().nb_bits == 8 ? 1 : (cimg::X11_attr().nb_bits == 16 ? 2 : 4));
			void *image_data = std::malloc(buf_size);
			std::memcpy(image_data, _data, buf_size);
			assign(_width, _height, _title, _normalization, !_is_fullscreen, false);
			std::memcpy(_data, image_data, buf_size);
			std::free(image_data);
			return paint();
		}
		return assign(_width, _height, _title, _normalization, !_is_fullscreen, false);
	}

	CImgDisplay& show() {
		if (is_empty() || !_is_closed) return *this;
		cimg_lock_display();
		if (_is_fullscreen) _init_fullscreen();
		_map_window();
		_is_closed = false;
		cimg_unlock_display();
		return paint();
	}

	CImgDisplay& close() {
		if (is_empty() || _is_closed) return *this;
		Display *const dpy = cimg::X11_attr().display;
		cimg_lock_display();
		if (_is_fullscreen) _desinit_fullscreen();
		XUnmapWindow(dpy, _window);
		_window_x = _window_y = -1;
		_is_closed = true;
		cimg_unlock_display();
		return *this;
	}

	CImgDisplay& move(const int posx, const int posy) {
		if (is_empty()) return *this;
		if (_window_x != posx || _window_y != posy) {
			show();
			Display *const dpy = cimg::X11_attr().display;
			cimg_lock_display();
			XMoveWindow(dpy, _window, posx, posy);
			_window_x = posx; _window_y = posy;
			cimg_unlock_display();
		}
		_is_moved = false;
		return paint();
	}

	CImgDisplay& show_mouse() {
		if (is_empty()) return *this;
		Display *const dpy = cimg::X11_attr().display;
		cimg_lock_display();
		XUndefineCursor(dpy, _window);
		cimg_unlock_display();
		return *this;
	}

	CImgDisplay& hide_mouse() {
		if (is_empty()) return *this;
		Display *const dpy = cimg::X11_attr().display;
		cimg_lock_display();
		static const char pix_data[8] = { 0 };
		XColor col;
		col.red = col.green = col.blue = 0;
		Pixmap pix = XCreateBitmapFromData(dpy, _window, pix_data, 8, 8);
		Cursor cur = XCreatePixmapCursor(dpy, pix, pix, &col, &col, 0, 0);
		XFreePixmap(dpy, pix);
		XDefineCursor(dpy, _window, cur);
		cimg_unlock_display();
		return *this;
	}

	CImgDisplay& set_mouse(const int posx, const int posy) {
		if (is_empty() || _is_closed) return *this;
		Display *const dpy = cimg::X11_attr().display;
		cimg_lock_display();
		XWarpPointer(dpy, 0L, _window, 0, 0, 0, 0, posx, posy);
		_mouse_x = posx; _mouse_y = posy;
		_is_moved = false;
		XSync(dpy, 0);
		cimg_unlock_display();
		return *this;
	}

	CImgDisplay& set_title(const char *const format, ...) {
		if (is_empty()) return *this;
		char *const tmp = new char[1024];
		va_list ap;
		va_start(ap, format);
		cimg_vsnprintf(tmp, 1024, format, ap);
		va_end(ap);
		if (!std::strcmp(_title, tmp)) { delete[] tmp; return *this; }
		delete[] _title;
		const unsigned int s = (unsigned int)std::strlen(tmp) + 1;
		_title = new char[s];
		std::memcpy(_title, tmp, s * sizeof(char));
		Display *const dpy = cimg::X11_attr().display;
		cimg_lock_display();
		XStoreName(dpy, _window, tmp);
		cimg_unlock_display();
		delete[] tmp;
		return *this;
	}

	template<typename T>
	CImgDisplay& display(const CImg<T>& img) {
		if (!img)
			throw CImgArgumentException(_cimgdisplay_instance
				"display(): Empty specified image.",
				cimgdisplay_instance);
		if (is_empty()) return assign(img);
		return render(img).paint(false);
	}

	CImgDisplay& paint(const bool wait_expose = true) {
		if (is_empty()) return *this;
		cimg_lock_display();
		_paint(wait_expose);
		cimg_unlock_display();
		return *this;
	}

	template<typename T>
	CImgDisplay& render(const CImg<T>& img, const bool flag8 = false) {
		if (!img)
			throw CImgArgumentException(_cimgdisplay_instance
				"render(): Empty specified image.",
				cimgdisplay_instance);
		if (is_empty()) return *this;
		if (img._depth != 1) return render(img.get_projections2d((img._width - 1) / 2, (img._height - 1) / 2,
			(img._depth - 1) / 2));
		if (cimg::X11_attr().nb_bits == 8 && (img._width != _width || img._height != _height))
			return render(img.get_resize(_width, _height, 1, -100, 1));
		if (cimg::X11_attr().nb_bits == 8 && !flag8 && img._spectrum == 3) {
			static const CImg<typename CImg<T>::ucharT> default_colormap = CImg<typename CImg<T>::ucharT>::default_LUT256();
			return render(img.get_index(default_colormap, 1, false));
		}

		const T
			*data1 = img._data,
			*data2 = (img._spectrum>1) ? img.data(0, 0, 0, 1) : data1,
			*data3 = (img._spectrum>2) ? img.data(0, 0, 0, 2) : data1;

		if (cimg::X11_attr().is_blue_first) cimg::swap(data1, data3);
		cimg_lock_display();

		if (!_normalization || (_normalization == 3 && cimg::type<T>::string() == cimg::type<unsigned char>::string())) {
			_min = _max = 0;
			switch (cimg::X11_attr().nb_bits) {
			case 8: { // 256 colormap, no normalization
				_set_colormap(_colormap, img._spectrum);
				unsigned char
					*const ndata = (img._width == _width && img._height == _height) ? (unsigned char*)_data :
					new unsigned char[(size_t)img._width*img._height],
					*ptrd = (unsigned char*)ndata;
				switch (img._spectrum) {
				case 1:
					for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
						(*ptrd++) = (unsigned char)*(data1++);
					break;
				case 2: for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char
						R = (unsigned char)*(data1++),
						G = (unsigned char)*(data2++);
					(*ptrd++) = (R & 0xf0) | (G >> 4);
				} break;
				default: for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char
						R = (unsigned char)*(data1++),
						G = (unsigned char)*(data2++),
						B = (unsigned char)*(data3++);
					(*ptrd++) = (R & 0xe0) | ((G >> 5) << 2) | (B >> 6);
				}
				}
				if (ndata != _data) {
					_render_resize(ndata, img._width, img._height, (unsigned char*)_data, _width, _height);
					delete[] ndata;
				}
			} break;
			case 16: { // 16 bits colors, no normalization
				unsigned short *const ndata = (img._width == _width && img._height == _height) ? (unsigned short*)_data :
					new unsigned short[(size_t)img._width*img._height];
				unsigned char *ptrd = (unsigned char*)ndata;
				const unsigned int M = 248;
				switch (img._spectrum) {
				case 1:
					if (cimg::X11_attr().byte_order)
						for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							const unsigned char val = (unsigned char)*(data1++), G = val >> 2;
							ptrd[0] = (val&M) | (G >> 3);
							ptrd[1] = (G << 5) | (G >> 1);
							ptrd += 2;
						}
					else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
						const unsigned char val = (unsigned char)*(data1++), G = val >> 2;
						ptrd[0] = (G << 5) | (G >> 1);
						ptrd[1] = (val&M) | (G >> 3);
						ptrd += 2;
					}
					break;
				case 2:
					if (cimg::X11_attr().byte_order)
						for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							const unsigned char G = (unsigned char)*(data2++) >> 2;
							ptrd[0] = ((unsigned char)*(data1++)&M) | (G >> 3);
							ptrd[1] = (G << 5);
							ptrd += 2;
						}
					else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
						const unsigned char G = (unsigned char)*(data2++) >> 2;
						ptrd[0] = (G << 5);
						ptrd[1] = ((unsigned char)*(data1++)&M) | (G >> 3);
						ptrd += 2;
					}
					break;
				default:
					if (cimg::X11_attr().byte_order)
						for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							const unsigned char G = (unsigned char)*(data2++) >> 2;
							ptrd[0] = ((unsigned char)*(data1++)&M) | (G >> 3);
							ptrd[1] = (G << 5) | ((unsigned char)*(data3++) >> 3);
							ptrd += 2;
						}
					else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
						const unsigned char G = (unsigned char)*(data2++) >> 2;
						ptrd[0] = (G << 5) | ((unsigned char)*(data3++) >> 3);
						ptrd[1] = ((unsigned char)*(data1++)&M) | (G >> 3);
						ptrd += 2;
					}
				}
				if (ndata != _data) {
					_render_resize(ndata, img._width, img._height, (unsigned short*)_data, _width, _height);
					delete[] ndata;
				}
			} break;
			default: { // 24 bits colors, no normalization
				unsigned int *const ndata = (img._width == _width && img._height == _height) ? (unsigned int*)_data :
					new unsigned int[(size_t)img._width*img._height];
				if (sizeof(int) == 4) { // 32 bits int uses optimized version
					unsigned int *ptrd = ndata;
					switch (img._spectrum) {
					case 1:
						if (cimg::X11_attr().byte_order == cimg::endianness())
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
								const unsigned char val = (unsigned char)*(data1++);
								*(ptrd++) = (val << 16) | (val << 8) | val;
							}
						else
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
								const unsigned char val = (unsigned char)*(data1++);
								*(ptrd++) = (val << 16) | (val << 8) | val;
							}
						break;
					case 2:
						if (cimg::X11_attr().byte_order == cimg::endianness())
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
								*(ptrd++) = ((unsigned char)*(data1++) << 16) | ((unsigned char)*(data2++) << 8);
						else
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
								*(ptrd++) = ((unsigned char)*(data2++) << 16) | ((unsigned char)*(data1++) << 8);
						break;
					default:
						if (cimg::X11_attr().byte_order == cimg::endianness())
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
								*(ptrd++) = ((unsigned char)*(data1++) << 16) | ((unsigned char)*(data2++) << 8) |
								(unsigned char)*(data3++);
						else
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
								*(ptrd++) = ((unsigned char)*(data3++) << 24) | ((unsigned char)*(data2++) << 16) |
								((unsigned char)*(data1++) << 8);
					}
				}
				else {
					unsigned char *ptrd = (unsigned char*)ndata;
					switch (img._spectrum) {
					case 1:
						if (cimg::X11_attr().byte_order)
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
								ptrd[0] = 0;
								ptrd[1] = (unsigned char)*(data1++);
								ptrd[2] = 0;
								ptrd[3] = 0;
								ptrd += 4;
							}
						else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							ptrd[0] = 0;
							ptrd[1] = 0;
							ptrd[2] = (unsigned char)*(data1++);
							ptrd[3] = 0;
							ptrd += 4;
						}
						break;
					case 2:
						if (cimg::X11_attr().byte_order) cimg::swap(data1, data2);
						for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							ptrd[0] = 0;
							ptrd[1] = (unsigned char)*(data2++);
							ptrd[2] = (unsigned char)*(data1++);
							ptrd[3] = 0;
							ptrd += 4;
						}
						break;
					default:
						if (cimg::X11_attr().byte_order)
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
								ptrd[0] = 0;
								ptrd[1] = (unsigned char)*(data1++);
								ptrd[2] = (unsigned char)*(data2++);
								ptrd[3] = (unsigned char)*(data3++);
								ptrd += 4;
							}
						else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							ptrd[0] = (unsigned char)*(data3++);
							ptrd[1] = (unsigned char)*(data2++);
							ptrd[2] = (unsigned char)*(data1++);
							ptrd[3] = 0;
							ptrd += 4;
						}
					}
				}
				if (ndata != _data) {
					_render_resize(ndata, img._width, img._height, (unsigned int*)_data, _width, _height);
					delete[] ndata;
				}
			}
			}
		}
		else {
			if (_normalization == 3) {
				if (cimg::type<T>::is_float()) _min = (float)img.min_max(_max);
				else { _min = (float)cimg::type<T>::min(); _max = (float)cimg::type<T>::max(); }
			}
			else if ((_min>_max) || _normalization == 1) _min = (float)img.min_max(_max);
			const float delta = _max - _min, mm = 255 / (delta ? delta : 1.0f);
			switch (cimg::X11_attr().nb_bits) {
			case 8: { // 256 colormap, with normalization
				_set_colormap(_colormap, img._spectrum);
				unsigned char *const ndata = (img._width == _width && img._height == _height) ? (unsigned char*)_data :
					new unsigned char[(size_t)img._width*img._height];
				unsigned char *ptrd = (unsigned char*)ndata;
				switch (img._spectrum) {
				case 1: for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char R = (unsigned char)((*(data1++) - _min)*mm);
					*(ptrd++) = R;
				} break;
				case 2: for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char
						R = (unsigned char)((*(data1++) - _min)*mm),
						G = (unsigned char)((*(data2++) - _min)*mm);
					(*ptrd++) = (R & 0xf0) | (G >> 4);
				} break;
				default:
					for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
						const unsigned char
							R = (unsigned char)((*(data1++) - _min)*mm),
							G = (unsigned char)((*(data2++) - _min)*mm),
							B = (unsigned char)((*(data3++) - _min)*mm);
						*(ptrd++) = (R & 0xe0) | ((G >> 5) << 2) | (B >> 6);
					}
				}
				if (ndata != _data) {
					_render_resize(ndata, img._width, img._height, (unsigned char*)_data, _width, _height);
					delete[] ndata;
				}
			} break;
			case 16: { // 16 bits colors, with normalization
				unsigned short *const ndata = (img._width == _width && img._height == _height) ? (unsigned short*)_data :
					new unsigned short[(size_t)img._width*img._height];
				unsigned char *ptrd = (unsigned char*)ndata;
				const unsigned int M = 248;
				switch (img._spectrum) {
				case 1:
					if (cimg::X11_attr().byte_order)
						for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							const unsigned char val = (unsigned char)((*(data1++) - _min)*mm), G = val >> 2;
							ptrd[0] = (val&M) | (G >> 3);
							ptrd[1] = (G << 5) | (val >> 3);
							ptrd += 2;
						}
					else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
						const unsigned char val = (unsigned char)((*(data1++) - _min)*mm), G = val >> 2;
						ptrd[0] = (G << 5) | (val >> 3);
						ptrd[1] = (val&M) | (G >> 3);
						ptrd += 2;
					}
					break;
				case 2:
					if (cimg::X11_attr().byte_order)
						for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							const unsigned char G = (unsigned char)((*(data2++) - _min)*mm) >> 2;
							ptrd[0] = ((unsigned char)((*(data1++) - _min)*mm)&M) | (G >> 3);
							ptrd[1] = (G << 5);
							ptrd += 2;
						}
					else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
						const unsigned char G = (unsigned char)((*(data2++) - _min)*mm) >> 2;
						ptrd[0] = (G << 5);
						ptrd[1] = ((unsigned char)((*(data1++) - _min)*mm)&M) | (G >> 3);
						ptrd += 2;
					}
					break;
				default:
					if (cimg::X11_attr().byte_order)
						for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							const unsigned char G = (unsigned char)((*(data2++) - _min)*mm) >> 2;
							ptrd[0] = ((unsigned char)((*(data1++) - _min)*mm)&M) | (G >> 3);
							ptrd[1] = (G << 5) | ((unsigned char)((*(data3++) - _min)*mm) >> 3);
							ptrd += 2;
						}
					else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
						const unsigned char G = (unsigned char)((*(data2++) - _min)*mm) >> 2;
						ptrd[0] = (G << 5) | ((unsigned char)((*(data3++) - _min)*mm) >> 3);
						ptrd[1] = ((unsigned char)((*(data1++) - _min)*mm)&M) | (G >> 3);
						ptrd += 2;
					}
				}
				if (ndata != _data) {
					_render_resize(ndata, img._width, img._height, (unsigned short*)_data, _width, _height);
					delete[] ndata;
				}
			} break;
			default: { // 24 bits colors, with normalization
				unsigned int *const ndata = (img._width == _width && img._height == _height) ? (unsigned int*)_data :
					new unsigned int[(size_t)img._width*img._height];
				if (sizeof(int) == 4) { // 32 bits int uses optimized version
					unsigned int *ptrd = ndata;
					switch (img._spectrum) {
					case 1:
						if (cimg::X11_attr().byte_order == cimg::endianness())
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
								const unsigned char val = (unsigned char)((*(data1++) - _min)*mm);
								*(ptrd++) = (val << 16) | (val << 8) | val;
							}
						else
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
								const unsigned char val = (unsigned char)((*(data1++) - _min)*mm);
								*(ptrd++) = (val << 24) | (val << 16) | (val << 8);
							}
						break;
					case 2:
						if (cimg::X11_attr().byte_order == cimg::endianness())
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
								*(ptrd++) =
								((unsigned char)((*(data1++) - _min)*mm) << 16) |
								((unsigned char)((*(data2++) - _min)*mm) << 8);
						else
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
								*(ptrd++) =
								((unsigned char)((*(data2++) - _min)*mm) << 16) |
								((unsigned char)((*(data1++) - _min)*mm) << 8);
						break;
					default:
						if (cimg::X11_attr().byte_order == cimg::endianness())
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
								*(ptrd++) =
								((unsigned char)((*(data1++) - _min)*mm) << 16) |
								((unsigned char)((*(data2++) - _min)*mm) << 8) |
								(unsigned char)((*(data3++) - _min)*mm);
						else
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy)
								*(ptrd++) =
								((unsigned char)((*(data3++) - _min)*mm) << 24) |
								((unsigned char)((*(data2++) - _min)*mm) << 16) |
								((unsigned char)((*(data1++) - _min)*mm) << 8);
					}
				}
				else {
					unsigned char *ptrd = (unsigned char*)ndata;
					switch (img._spectrum) {
					case 1:
						if (cimg::X11_attr().byte_order)
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
								const unsigned char val = (unsigned char)((*(data1++) - _min)*mm);
								ptrd[0] = 0;
								ptrd[1] = val;
								ptrd[2] = val;
								ptrd[3] = val;
								ptrd += 4;
							}
						else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							const unsigned char val = (unsigned char)((*(data1++) - _min)*mm);
							ptrd[0] = val;
							ptrd[1] = val;
							ptrd[2] = val;
							ptrd[3] = 0;
							ptrd += 4;
						}
						break;
					case 2:
						if (cimg::X11_attr().byte_order) cimg::swap(data1, data2);
						for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							ptrd[0] = 0;
							ptrd[1] = (unsigned char)((*(data2++) - _min)*mm);
							ptrd[2] = (unsigned char)((*(data1++) - _min)*mm);
							ptrd[3] = 0;
							ptrd += 4;
						}
						break;
					default:
						if (cimg::X11_attr().byte_order)
							for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
								ptrd[0] = 0;
								ptrd[1] = (unsigned char)((*(data1++) - _min)*mm);
								ptrd[2] = (unsigned char)((*(data2++) - _min)*mm);
								ptrd[3] = (unsigned char)((*(data3++) - _min)*mm);
								ptrd += 4;
							}
						else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
							ptrd[0] = (unsigned char)((*(data3++) - _min)*mm);
							ptrd[1] = (unsigned char)((*(data2++) - _min)*mm);
							ptrd[2] = (unsigned char)((*(data1++) - _min)*mm);
							ptrd[3] = 0;
							ptrd += 4;
						}
					}
				}
				if (ndata != _data) {
					_render_resize(ndata, img._width, img._height, (unsigned int*)_data, _width, _height);
					delete[] ndata;
				}
			}
			}
		}
		cimg_unlock_display();
		return *this;
	}

	template<typename T>
	static void screenshot(const int x0, const int y0, const int x1, const int y1, CImg<T>& img) {
		img.assign();
		Display *dpy = cimg::X11_attr().display;
		cimg_lock_display();
		if (!dpy) {
			dpy = XOpenDisplay(0);
			if (!dpy)
				throw CImgDisplayException("CImgDisplay::screenshot(): Failed to open X11 display.");
		}
		Window root = DefaultRootWindow(dpy);
		XWindowAttributes gwa;
		XGetWindowAttributes(dpy, root, &gwa);
		const int width = gwa.width, height = gwa.height;
		int _x0 = x0, _y0 = y0, _x1 = x1, _y1 = y1;
		if (_x0>_x1) cimg::swap(_x0, _x1);
		if (_y0>_y1) cimg::swap(_y0, _y1);

		XImage *image = 0;
		if (_x1 >= 0 && _x0<width && _y1 >= 0 && _y0<height) {
			_x0 = std::max(_x0, 0);
			_y0 = std::max(_y0, 0);
			_x1 = std::min(_x1, width - 1);
			_y1 = std::min(_y1, height - 1);
			image = XGetImage(dpy, root, _x0, _y0, _x1 - _x0 + 1, _y1 - _y0 + 1, AllPlanes, ZPixmap);

			if (image) {
				const unsigned long
					red_mask = image->red_mask,
					green_mask = image->green_mask,
					blue_mask = image->blue_mask;
				img.assign(image->width, image->height, 1, 3);
				T *pR = img.data(0, 0, 0, 0), *pG = img.data(0, 0, 0, 1), *pB = img.data(0, 0, 0, 2);
				cimg_forXY(img, x, y) {
					const unsigned long pixel = XGetPixel(image, x, y);
					*(pR++) = (T)((pixel & red_mask) >> 16);
					*(pG++) = (T)((pixel & green_mask) >> 8);
					*(pB++) = (T)(pixel & blue_mask);
				}
				XDestroyImage(image);
			}
		}
		if (!cimg::X11_attr().display) XCloseDisplay(dpy);
		cimg_unlock_display();
		if (img.is_empty())
			throw CImgDisplayException("CImgDisplay::screenshot(): Failed to take screenshot "
				"with coordinates (%d,%d)-(%d,%d).",
				x0, y0, x1, y1);
	}

	template<typename T>
	const CImgDisplay& snapshot(CImg<T>& img) const {
		if (is_empty()) { img.assign(); return *this; }
		const unsigned char *ptrs = (unsigned char*)_data;
		img.assign(_width, _height, 1, 3);
		T
			*data1 = img.data(0, 0, 0, 0),
			*data2 = img.data(0, 0, 0, 1),
			*data3 = img.data(0, 0, 0, 2);
		if (cimg::X11_attr().is_blue_first) cimg::swap(data1, data3);
		switch (cimg::X11_attr().nb_bits) {
		case 8: {
			for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
				const unsigned char val = *(ptrs++);
				*(data1++) = (T)(val & 0xe0);
				*(data2++) = (T)((val & 0x1c) << 3);
				*(data3++) = (T)(val << 6);
			}
		} break;
		case 16: {
			if (cimg::X11_attr().byte_order) for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
				const unsigned char
					val0 = ptrs[0],
					val1 = ptrs[1];
				ptrs += 2;
				*(data1++) = (T)(val0 & 0xf8);
				*(data2++) = (T)((val0 << 5) | ((val1 & 0xe0) >> 5));
				*(data3++) = (T)(val1 << 3);
			}
			else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
				const unsigned short
					val0 = ptrs[0],
					val1 = ptrs[1];
				ptrs += 2;
				*(data1++) = (T)(val1 & 0xf8);
				*(data2++) = (T)((val1 << 5) | ((val0 & 0xe0) >> 5));
				*(data3++) = (T)(val0 << 3);
			}
		} break;
		default: {
			if (cimg::X11_attr().byte_order) for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
				++ptrs;
				*(data1++) = (T)ptrs[0];
				*(data2++) = (T)ptrs[1];
				*(data3++) = (T)ptrs[2];
				ptrs += 3;
			}
			else for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
				*(data3++) = (T)ptrs[0];
				*(data2++) = (T)ptrs[1];
				*(data1++) = (T)ptrs[2];
				ptrs += 3;
				++ptrs;
			}
		}
		}
		return *this;
	}

	// Windows-based implementation.
	//-------------------------------
#elif cimg_display==2

	bool _is_mouse_tracked, _is_cursor_visible;
	HANDLE _thread, _is_created, _mutex;
	HWND _window, _background_window;
	CLIENTCREATESTRUCT _ccs;
	unsigned int *_data;
	DEVMODE _curr_mode;
	BITMAPINFO _bmi;
	HDC _hdc;

	static int screen_width() {
		DEVMODE mode;
		mode.dmSize = sizeof(DEVMODE);
		mode.dmDriverExtra = 0;
		EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &mode);
		return (int)mode.dmPelsWidth;
	}

	static int screen_height() {
		DEVMODE mode;
		mode.dmSize = sizeof(DEVMODE);
		mode.dmDriverExtra = 0;
		EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &mode);
		return (int)mode.dmPelsHeight;
	}

	static void wait_all() {
		WaitForSingleObject(cimg::Win32_attr().wait_event, INFINITE);
	}

	static LRESULT APIENTRY _handle_events(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
#ifdef _WIN64
		CImgDisplay *const disp = (CImgDisplay*)GetWindowLongPtr(window, GWLP_USERDATA);
#else
		CImgDisplay *const disp = (CImgDisplay*)GetWindowLong(window, GWL_USERDATA);
#endif
		MSG st_msg;
		switch (msg) {
		case WM_CLOSE:
			disp->_mouse_x = disp->_mouse_y = -1;
			disp->_window_x = disp->_window_y = 0;
			disp->set_button().set_key(0).set_key(0, false)._is_closed = true;
			ReleaseMutex(disp->_mutex);
			ShowWindow(disp->_window, SW_HIDE);
			disp->_is_event = true;
			SetEvent(cimg::Win32_attr().wait_event);
			return 0;
		case WM_SIZE: {
			while (PeekMessage(&st_msg, window, WM_SIZE, WM_SIZE, PM_REMOVE)) {}
			WaitForSingleObject(disp->_mutex, INFINITE);
			const unsigned int nw = LOWORD(lParam), nh = HIWORD(lParam);
			if (nw && nh && (nw != disp->_width || nh != disp->_height)) {
				disp->_window_width = nw;
				disp->_window_height = nh;
				disp->_mouse_x = disp->_mouse_y = -1;
				disp->_is_resized = disp->_is_event = true;
				SetEvent(cimg::Win32_attr().wait_event);
			}
			ReleaseMutex(disp->_mutex);
		} break;
		case WM_MOVE: {
			while (PeekMessage(&st_msg, window, WM_SIZE, WM_SIZE, PM_REMOVE)) {}
			WaitForSingleObject(disp->_mutex, INFINITE);
			const int nx = (int)(short)(LOWORD(lParam)), ny = (int)(short)(HIWORD(lParam));
			if (nx != disp->_window_x || ny != disp->_window_y) {
				disp->_window_x = nx;
				disp->_window_y = ny;
				disp->_is_moved = disp->_is_event = true;
				SetEvent(cimg::Win32_attr().wait_event);
			}
			ReleaseMutex(disp->_mutex);
		} break;
		case WM_PAINT:
			disp->paint();
			cimg::mutex(15);
			if (disp->_is_cursor_visible) while (ShowCursor(TRUE)<0); else while (ShowCursor(FALSE) >= 0);
			cimg::mutex(15, 0);
			break;
		case WM_ERASEBKGND:
			//        return 0;
			break;
		case WM_KEYDOWN:
			disp->set_key((unsigned int)wParam);
			SetEvent(cimg::Win32_attr().wait_event);
			break;
		case WM_KEYUP:
			disp->set_key((unsigned int)wParam, false);
			SetEvent(cimg::Win32_attr().wait_event);
			break;
		case WM_MOUSEMOVE: {
			while (PeekMessage(&st_msg, window, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE)) {}
			disp->_mouse_x = LOWORD(lParam);
			disp->_mouse_y = HIWORD(lParam);
#if (_WIN32_WINNT>=0x0400) && !defined(NOTRACKMOUSEEVENT)
			if (!disp->_is_mouse_tracked) {
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = disp->_window;
				if (TrackMouseEvent(&tme)) disp->_is_mouse_tracked = true;
			}
#endif
			if (disp->_mouse_x<0 || disp->_mouse_y<0 || disp->_mouse_x >= disp->width() || disp->_mouse_y >= disp->height())
				disp->_mouse_x = disp->_mouse_y = -1;
			disp->_is_event = true;
			SetEvent(cimg::Win32_attr().wait_event);
			cimg::mutex(15);
			if (disp->_is_cursor_visible) while (ShowCursor(TRUE)<0); else while (ShowCursor(FALSE) >= 0);
			cimg::mutex(15, 0);
		}	break;
		case WM_MOUSELEAVE: {
			disp->_mouse_x = disp->_mouse_y = -1;
			disp->_is_mouse_tracked = false;
			cimg::mutex(15);
			while (ShowCursor(TRUE)<0) {}
			cimg::mutex(15, 0);
		} break;
		case WM_LBUTTONDOWN:
			disp->set_button(1);
			SetEvent(cimg::Win32_attr().wait_event);
			break;
		case WM_RBUTTONDOWN:
			disp->set_button(2);
			SetEvent(cimg::Win32_attr().wait_event);
			break;
		case WM_MBUTTONDOWN:
			disp->set_button(3);
			SetEvent(cimg::Win32_attr().wait_event);
			break;
		case WM_LBUTTONUP:
			disp->set_button(1, false);
			SetEvent(cimg::Win32_attr().wait_event);
			break;
		case WM_RBUTTONUP:
			disp->set_button(2, false);
			SetEvent(cimg::Win32_attr().wait_event);
			break;
		case WM_MBUTTONUP:
			disp->set_button(3, false);
			SetEvent(cimg::Win32_attr().wait_event);
			break;
		case 0x020A: // WM_MOUSEWHEEL:
			disp->set_wheel((int)((short)HIWORD(wParam)) / 120);
			SetEvent(cimg::Win32_attr().wait_event);
		}
		return DefWindowProc(window, msg, wParam, lParam);
	}

	static DWORD WINAPI _events_thread(void* arg) {
		CImgDisplay *const disp = (CImgDisplay*)(((void**)arg)[0]);
		const char *const title = (const char*)(((void**)arg)[1]);
		MSG msg;
		delete[](void**)arg;
		disp->_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		disp->_bmi.bmiHeader.biWidth = disp->width();
		disp->_bmi.bmiHeader.biHeight = -disp->height();
		disp->_bmi.bmiHeader.biPlanes = 1;
		disp->_bmi.bmiHeader.biBitCount = 32;
		disp->_bmi.bmiHeader.biCompression = BI_RGB;
		disp->_bmi.bmiHeader.biSizeImage = 0;
		disp->_bmi.bmiHeader.biXPelsPerMeter = 1;
		disp->_bmi.bmiHeader.biYPelsPerMeter = 1;
		disp->_bmi.bmiHeader.biClrUsed = 0;
		disp->_bmi.bmiHeader.biClrImportant = 0;
		disp->_data = new unsigned int[(size_t)disp->_width*disp->_height];
		if (!disp->_is_fullscreen) { // Normal window
			RECT rect;
			rect.left = rect.top = 0; rect.right = (LONG)disp->_width - 1; rect.bottom = (LONG)disp->_height - 1;
			AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, false);
			const int
				border1 = (int)((rect.right - rect.left + 1 - disp->_width) / 2),
				border2 = (int)(rect.bottom - rect.top + 1 - disp->_height - border1);
			disp->_window = CreateWindowA("MDICLIENT", title ? title : " ",
				WS_OVERLAPPEDWINDOW | (disp->_is_closed ? 0 : WS_VISIBLE), CW_USEDEFAULT, CW_USEDEFAULT,
				disp->_width + 2 * border1, disp->_height + border1 + border2,
				0, 0, 0, &(disp->_ccs));
			if (!disp->_is_closed) {
				GetWindowRect(disp->_window, &rect);
				disp->_window_x = rect.left + border1;
				disp->_window_y = rect.top + border2;
			}
			else disp->_window_x = disp->_window_y = 0;
		}
		else { // Fullscreen window
			const unsigned int
				sx = (unsigned int)screen_width(),
				sy = (unsigned int)screen_height();
			disp->_window = CreateWindowA("MDICLIENT", title ? title : " ",
				WS_POPUP | (disp->_is_closed ? 0 : WS_VISIBLE),
				(sx - disp->_width) / 2,
				(sy - disp->_height) / 2,
				disp->_width, disp->_height, 0, 0, 0, &(disp->_ccs));
			disp->_window_x = disp->_window_y = 0;
		}
		SetForegroundWindow(disp->_window);
		disp->_hdc = GetDC(disp->_window);
		disp->_window_width = disp->_width;
		disp->_window_height = disp->_height;
		disp->flush();
#ifdef _WIN64
		SetWindowLongPtr(disp->_window, GWLP_USERDATA, (LONG_PTR)disp);
		SetWindowLongPtr(disp->_window, GWLP_WNDPROC, (LONG_PTR)_handle_events);
#else
		SetWindowLong(disp->_window, GWL_USERDATA, (LONG)disp);
		SetWindowLong(disp->_window, GWL_WNDPROC, (LONG)_handle_events);
#endif
		SetEvent(disp->_is_created);
		while (GetMessage(&msg, 0, 0, 0)) DispatchMessage(&msg);
		return 0;
	}

	CImgDisplay& _update_window_pos() {
		if (_is_closed) _window_x = _window_y = -1;
		else {
			RECT rect;
			rect.left = rect.top = 0; rect.right = (LONG)_width - 1; rect.bottom = (LONG)_height - 1;
			AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, false);
			const int
				border1 = (int)((rect.right - rect.left + 1 - _width) / 2),
				border2 = (int)(rect.bottom - rect.top + 1 - _height - border1);
			GetWindowRect(_window, &rect);
			_window_x = rect.left + border1;
			_window_y = rect.top + border2;
		}
		return *this;
	}

	void _init_fullscreen() {
		_background_window = 0;
		if (!_is_fullscreen || _is_closed) _curr_mode.dmSize = 0;
		else {
			DEVMODE mode;
			unsigned int imode = 0, ibest = 0, bestbpp = 0, bw = ~0U, bh = ~0U;
			for (mode.dmSize = sizeof(DEVMODE), mode.dmDriverExtra = 0; EnumDisplaySettings(0, imode, &mode); ++imode) {
				const unsigned int nw = mode.dmPelsWidth, nh = mode.dmPelsHeight;
				if (nw >= _width && nh >= _height && mode.dmBitsPerPel >= bestbpp && nw <= bw && nh <= bh) {
					bestbpp = mode.dmBitsPerPel;
					ibest = imode;
					bw = nw; bh = nh;
				}
			}
			if (bestbpp) {
				_curr_mode.dmSize = sizeof(DEVMODE); _curr_mode.dmDriverExtra = 0;
				EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &_curr_mode);
				EnumDisplaySettings(0, ibest, &mode);
				ChangeDisplaySettings(&mode, 0);
			}
			else _curr_mode.dmSize = 0;

			const unsigned int
				sx = (unsigned int)screen_width(),
				sy = (unsigned int)screen_height();
			if (sx != _width || sy != _height) {
				CLIENTCREATESTRUCT background_ccs;
				_background_window = CreateWindowA("MDICLIENT", "", WS_POPUP | WS_VISIBLE, 0, 0, sx, sy, 0, 0, 0, &background_ccs);
				SetForegroundWindow(_background_window);
			}
		}
	}

	void _desinit_fullscreen() {
		if (!_is_fullscreen) return;
		if (_background_window) DestroyWindow(_background_window);
		_background_window = 0;
		if (_curr_mode.dmSize) ChangeDisplaySettings(&_curr_mode, 0);
		_is_fullscreen = false;
	}

	CImgDisplay& _assign(const unsigned int dimw, const unsigned int dimh, const char *const ptitle = 0,
		const unsigned int normalization_type = 3,
		const bool fullscreen_flag = false, const bool closed_flag = false) {

		// Allocate space for window title
		const char *const nptitle = ptitle ? ptitle : "";
		const unsigned int s = (unsigned int)std::strlen(nptitle) + 1;
		char *const tmp_title = s ? new char[s] : 0;
		if (s) std::memcpy(tmp_title, nptitle, s * sizeof(char));

		// Destroy previous window if existing
		if (!is_empty()) assign();

		// Set display variables
		_width = std::min(dimw, (unsigned int)screen_width());
		_height = std::min(dimh, (unsigned int)screen_height());
		_normalization = normalization_type<4 ? normalization_type : 3;
		_is_fullscreen = fullscreen_flag;
		_window_x = _window_y = 0;
		_is_closed = closed_flag;
		_is_cursor_visible = true;
		_is_mouse_tracked = false;
		_title = tmp_title;
		flush();
		if (_is_fullscreen) _init_fullscreen();

		// Create event thread
		void *const arg = (void*)(new void*[2]);
		((void**)arg)[0] = (void*)this;
		((void**)arg)[1] = (void*)_title;
		_mutex = CreateMutex(0, FALSE, 0);
		_is_created = CreateEvent(0, FALSE, FALSE, 0);
		_thread = CreateThread(0, 0, _events_thread, arg, 0, 0);
		WaitForSingleObject(_is_created, INFINITE);
		return *this;
	}

	CImgDisplay& assign() {
		if (is_empty()) return flush();
		DestroyWindow(_window);
		TerminateThread(_thread, 0);
		delete[] _data;
		delete[] _title;
		_data = 0;
		_title = 0;
		if (_is_fullscreen) _desinit_fullscreen();
		_width = _height = _normalization = _window_width = _window_height = 0;
		_window_x = _window_y = 0;
		_is_fullscreen = false;
		_is_closed = true;
		_min = _max = 0;
		_title = 0;
		flush();
		return *this;
	}

	CImgDisplay& assign(const unsigned int dimw, const unsigned int dimh, const char *const title = 0,
		const unsigned int normalization_type = 3,
		const bool fullscreen_flag = false, const bool closed_flag = false) {
		if (!dimw || !dimh) return assign();
		_assign(dimw, dimh, title, normalization_type, fullscreen_flag, closed_flag);
		_min = _max = 0;
		std::memset(_data, 0, sizeof(unsigned int)*_width*_height);
		return paint();
	}

	template<typename T>
	CImgDisplay& assign(const CImg<T>& img, const char *const title = 0,
		const unsigned int normalization_type = 3,
		const bool fullscreen_flag = false, const bool closed_flag = false) {
		if (!img) return assign();
		CImg<T> tmp;
		const CImg<T>& nimg = (img._depth == 1) ? img : (tmp = img.get_projections2d((img._width - 1) / 2,
			(img._height - 1) / 2,
			(img._depth - 1) / 2));
		_assign(nimg._width, nimg._height, title, normalization_type, fullscreen_flag, closed_flag);
		if (_normalization == 2) _min = (float)nimg.min_max(_max);
		return display(nimg);
	}

	template<typename T>
	CImgDisplay& assign(const CImgList<T>& list, const char *const title = 0,
		const unsigned int normalization_type = 3,
		const bool fullscreen_flag = false, const bool closed_flag = false) {
		if (!list) return assign();
		CImg<T> tmp;
		const CImg<T> img = list>'x', &nimg = (img._depth == 1) ? img : (tmp = img.get_projections2d((img._width - 1) / 2,
			(img._height - 1) / 2,
			(img._depth - 1) / 2));
		_assign(nimg._width, nimg._height, title, normalization_type, fullscreen_flag, closed_flag);
		if (_normalization == 2) _min = (float)nimg.min_max(_max);
		return display(nimg);
	}

	CImgDisplay& assign(const CImgDisplay& disp) {
		if (!disp) return assign();
		_assign(disp._width, disp._height, disp._title, disp._normalization, disp._is_fullscreen, disp._is_closed);
		std::memcpy(_data, disp._data, sizeof(unsigned int)*_width*_height);
		return paint();
	}

	CImgDisplay& resize(const int nwidth, const int nheight, const bool force_redraw = true) {
		if (!nwidth || !nheight || (is_empty() && (nwidth<0 || nheight<0))) return assign();
		if (is_empty()) return assign(nwidth, nheight);
		const unsigned int
			tmpdimx = (nwidth>0) ? nwidth : (-nwidth*_width / 100),
			tmpdimy = (nheight>0) ? nheight : (-nheight*_height / 100),
			dimx = tmpdimx ? tmpdimx : 1,
			dimy = tmpdimy ? tmpdimy : 1;
		if (_width != dimx || _height != dimy || _window_width != dimx || _window_height != dimy) {
			if (_window_width != dimx || _window_height != dimy) {
				RECT rect; rect.left = rect.top = 0; rect.right = (LONG)dimx - 1; rect.bottom = (LONG)dimy - 1;
				AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, false);
				const int cwidth = rect.right - rect.left + 1, cheight = rect.bottom - rect.top + 1;
				SetWindowPos(_window, 0, 0, 0, cwidth, cheight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOCOPYBITS);
			}
			if (_width != dimx || _height != dimy) {
				unsigned int *const ndata = new unsigned int[dimx*dimy];
				if (force_redraw) _render_resize(_data, _width, _height, ndata, dimx, dimy);
				else std::memset(ndata, 0x80, sizeof(unsigned int)*dimx*dimy);
				delete[] _data;
				_data = ndata;
				_bmi.bmiHeader.biWidth = (LONG)dimx;
				_bmi.bmiHeader.biHeight = -(int)dimy;
				_width = dimx;
				_height = dimy;
			}
			_window_width = dimx; _window_height = dimy;
			show();
		}
		_is_resized = false;
		if (_is_fullscreen) move((screen_width() - width()) / 2, (screen_height() - height()) / 2);
		if (force_redraw) return paint();
		return *this;
	}

	CImgDisplay& toggle_fullscreen(const bool force_redraw = true) {
		if (is_empty()) return *this;
		if (force_redraw) {
			const cimg_ulong buf_size = (cimg_ulong)_width*_height * 4;
			void *odata = std::malloc(buf_size);
			if (odata) {
				std::memcpy(odata, _data, buf_size);
				assign(_width, _height, _title, _normalization, !_is_fullscreen, false);
				std::memcpy(_data, odata, buf_size);
				std::free(odata);
			}
			return paint();
		}
		return assign(_width, _height, _title, _normalization, !_is_fullscreen, false);
	}

	CImgDisplay& show() {
		if (is_empty() || !_is_closed) return *this;
		_is_closed = false;
		if (_is_fullscreen) _init_fullscreen();
		ShowWindow(_window, SW_SHOW);
		_update_window_pos();
		return paint();
	}

	CImgDisplay& close() {
		if (is_empty() || _is_closed) return *this;
		_is_closed = true;
		if (_is_fullscreen) _desinit_fullscreen();
		ShowWindow(_window, SW_HIDE);
		_window_x = _window_y = 0;
		return *this;
	}

	CImgDisplay& move(const int posx, const int posy) {
		if (is_empty()) return *this;
		if (_window_x != posx || _window_y != posy) {
			if (!_is_fullscreen) {
				RECT rect;
				rect.left = rect.top = 0; rect.right = (LONG)_window_width - 1; rect.bottom = (LONG)_window_height - 1;
				AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, false);
				const int
					border1 = (int)((rect.right - rect.left + 1 - _width) / 2),
					border2 = (int)(rect.bottom - rect.top + 1 - _height - border1);
				SetWindowPos(_window, 0, posx - border1, posy - border2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}
			else SetWindowPos(_window, 0, posx, posy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			_window_x = posx;
			_window_y = posy;
			show();
		}
		_is_moved = false;
		return *this;
	}

	CImgDisplay& show_mouse() {
		if (is_empty()) return *this;
		_is_cursor_visible = true;
		return *this;
	}

	CImgDisplay& hide_mouse() {
		if (is_empty()) return *this;
		_is_cursor_visible = false;
		return *this;
	}

	CImgDisplay& set_mouse(const int posx, const int posy) {
		if (is_empty() || _is_closed || posx<0 || posy<0) return *this;
		_update_window_pos();
		const int res = (int)SetCursorPos(_window_x + posx, _window_y + posy);
		if (res) { _mouse_x = posx; _mouse_y = posy; }
		return *this;
	}

	CImgDisplay& set_title(const char *const format, ...) {
		if (is_empty()) return *this;
		char *const tmp = new char[1024];
		va_list ap;
		va_start(ap, format);
		cimg_vsnprintf(tmp, 1024, format, ap);
		va_end(ap);
		if (!std::strcmp(_title, tmp)) { delete[] tmp; return *this; }
		delete[] _title;
		const unsigned int s = (unsigned int)std::strlen(tmp) + 1;
		_title = new char[s];
		std::memcpy(_title, tmp, s * sizeof(char));
		SetWindowTextA(_window, tmp);
		delete[] tmp;
		return *this;
	}

	template<typename T>
	CImgDisplay& display(const CImg<T>& img) {
		if (!img)
			throw CImgArgumentException(_cimgdisplay_instance
				"display(): Empty specified image.",
				cimgdisplay_instance);
		if (is_empty()) return assign(img);
		return render(img).paint();
	}

	CImgDisplay& paint() {
		if (_is_closed) return *this;
		WaitForSingleObject(_mutex, INFINITE);
		SetDIBitsToDevice(_hdc, 0, 0, _width, _height, 0, 0, 0, _height, _data, &_bmi, DIB_RGB_COLORS);
		ReleaseMutex(_mutex);
		return *this;
	}

	template<typename T>
	CImgDisplay& render(const CImg<T>& img) {
		if (!img)
			throw CImgArgumentException(_cimgdisplay_instance
				"render(): Empty specified image.",
				cimgdisplay_instance);

		if (is_empty()) return *this;
		if (img._depth != 1) return render(img.get_projections2d((img._width - 1) / 2, (img._height - 1) / 2,
			(img._depth - 1) / 2));

		const T
			*data1 = img._data,
			*data2 = (img._spectrum >= 2) ? img.data(0, 0, 0, 1) : data1,
			*data3 = (img._spectrum >= 3) ? img.data(0, 0, 0, 2) : data1;

		WaitForSingleObject(_mutex, INFINITE);
		unsigned int
			*const ndata = (img._width == _width && img._height == _height) ? _data :
			new unsigned int[(size_t)img._width*img._height],
			*ptrd = ndata;

		if (!_normalization || (_normalization == 3 && cimg::type<T>::string() == cimg::type<unsigned char>::string())) {
			_min = _max = 0;
			switch (img._spectrum) {
			case 1: {
				for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char val = (unsigned char)*(data1++);
					*(ptrd++) = (unsigned int)((val << 16) | (val << 8) | val);
				}
			} break;
			case 2: {
				for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char
						R = (unsigned char)*(data1++),
						G = (unsigned char)*(data2++);
					*(ptrd++) = (unsigned int)((R << 16) | (G << 8));
				}
			} break;
			default: {
				for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char
						R = (unsigned char)*(data1++),
						G = (unsigned char)*(data2++),
						B = (unsigned char)*(data3++);
					*(ptrd++) = (unsigned int)((R << 16) | (G << 8) | B);
				}
			}
			}
		}
		else {
			if (_normalization == 3) {
				if (cimg::type<T>::is_float()) _min = (float)img.min_max(_max);
				else { _min = (float)cimg::type<T>::min(); _max = (float)cimg::type<T>::max(); }
			}
			else if ((_min>_max) || _normalization == 1) _min = (float)img.min_max(_max);
			const float delta = _max - _min, mm = 255 / (delta ? delta : 1.0f);
			switch (img._spectrum) {
			case 1: {
				for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char val = (unsigned char)((*(data1++) - _min)*mm);
					*(ptrd++) = (unsigned int)((val << 16) | (val << 8) | val);
				}
			} break;
			case 2: {
				for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char
						R = (unsigned char)((*(data1++) - _min)*mm),
						G = (unsigned char)((*(data2++) - _min)*mm);
					*(ptrd++) = (unsigned int)((R << 16) | (G << 8));
				}
			} break;
			default: {
				for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
					const unsigned char
						R = (unsigned char)((*(data1++) - _min)*mm),
						G = (unsigned char)((*(data2++) - _min)*mm),
						B = (unsigned char)((*(data3++) - _min)*mm);
					*(ptrd++) = (unsigned int)((R << 16) | (G << 8) | B);
				}
			}
			}
		}
		if (ndata != _data) { _render_resize(ndata, img._width, img._height, _data, _width, _height); delete[] ndata; }
		ReleaseMutex(_mutex);
		return *this;
	}

	template<typename T>
	static void screenshot(const int x0, const int y0, const int x1, const int y1, CImg<T>& img) {
		img.assign();
		HDC hScreen = GetDC(GetDesktopWindow());
		if (hScreen) {
			const int
				width = GetDeviceCaps(hScreen, HORZRES),
				height = GetDeviceCaps(hScreen, VERTRES);
			int _x0 = x0, _y0 = y0, _x1 = x1, _y1 = y1;
			if (_x0>_x1) cimg::swap(_x0, _x1);
			if (_y0>_y1) cimg::swap(_y0, _y1);
			if (_x1 >= 0 && _x0<width && _y1 >= 0 && _y0<height) {
				_x0 = std::max(_x0, 0);
				_y0 = std::max(_y0, 0);
				_x1 = std::min(_x1, width - 1);
				_y1 = std::min(_y1, height - 1);
				const int bw = _x1 - _x0 + 1, bh = _y1 - _y0 + 1;
				HDC hdcMem = CreateCompatibleDC(hScreen);
				if (hdcMem) {
					HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, bw, bh);
					if (hBitmap) {
						HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
						if (hOld && BitBlt(hdcMem, 0, 0, bw, bh, hScreen, _x0, _y0, SRCCOPY) && SelectObject(hdcMem, hOld)) {
							BITMAPINFOHEADER bmi;
							bmi.biSize = sizeof(BITMAPINFOHEADER);
							bmi.biWidth = bw;
							bmi.biHeight = -bh;
							bmi.biPlanes = 1;
							bmi.biBitCount = 32;
							bmi.biCompression = BI_RGB;
							bmi.biSizeImage = 0;
							bmi.biXPelsPerMeter = bmi.biYPelsPerMeter = 0;
							bmi.biClrUsed = bmi.biClrImportant = 0;
							unsigned char *buf = new unsigned char[4 * bw*bh];
							if (GetDIBits(hdcMem, hBitmap, 0, bh, buf, (BITMAPINFO*)&bmi, DIB_RGB_COLORS)) {
								img.assign(bw, bh, 1, 3);
								const unsigned char *ptrs = buf;
								T *pR = img.data(0, 0, 0, 0), *pG = img.data(0, 0, 0, 1), *pB = img.data(0, 0, 0, 2);
								cimg_forXY(img, x, y) {
									*(pR++) = (T)ptrs[2];
									*(pG++) = (T)ptrs[1];
									*(pB++) = (T)ptrs[0];
									ptrs += 4;
								}
							}
							delete[] buf;
						}
						DeleteObject(hBitmap);
					}
					DeleteDC(hdcMem);
				}
			}
			ReleaseDC(GetDesktopWindow(), hScreen);
		}
		if (img.is_empty())
			throw CImgDisplayException("CImgDisplay::screenshot(): Failed to take screenshot "
				"with coordinates (%d,%d)-(%d,%d).",
				x0, y0, x1, y1);
	}

	template<typename T>
	const CImgDisplay& snapshot(CImg<T>& img) const {
		if (is_empty()) { img.assign(); return *this; }
		const unsigned int *ptrs = _data;
		img.assign(_width, _height, 1, 3);
		T
			*data1 = img.data(0, 0, 0, 0),
			*data2 = img.data(0, 0, 0, 1),
			*data3 = img.data(0, 0, 0, 2);
		for (cimg_ulong xy = (cimg_ulong)img._width*img._height; xy>0; --xy) {
			const unsigned int val = *(ptrs++);
			*(data1++) = (T)(unsigned char)(val >> 16);
			*(data2++) = (T)(unsigned char)((val >> 8) & 0xFF);
			*(data3++) = (T)(unsigned char)(val & 0xFF);
		}
		return *this;
	}
#endif

	//@}
};

}

#endif // !_CIMG_DISPLAY