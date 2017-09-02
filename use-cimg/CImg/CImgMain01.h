
#ifndef _CIMG_MAIN
#define _CIMG_MAIN

#include "CImgConfigure.h"

/*------------------------------------------------
#
#
#  Define cimg_library:: namespace
#
#
-------------------------------------------------*/
//! Contains <i>all classes and functions</i> of the \CImg library.
/**
This namespace is defined to avoid functions and class names collisions
that could happen with the inclusion of other C++ header files.
Anyway, it should not happen often and you should reasonnably start most of your
\CImg-based programs with
\code
#include "CImg.h"
using namespace cimg_library;
\endcode
to simplify the declaration of \CImg Library objects afterwards.
**/
namespace cimg_library_suffixed {

	// Declare the four classes of the CImg Library.
	template<typename T = float> struct CImg;
	template<typename T = float> struct CImgList;
	struct CImgDisplay;
	struct CImgException;

	// Declare cimg:: namespace.
	// This is an uncomplete namespace definition here. It only contains some
	// necessary stuff to ensure a correct declaration order of the classes and functions
	// defined afterwards.
	namespace cimg {

		// Define ascii sequences for colored terminal output.
#ifdef cimg_use_vt100
		static const char t_normal[] = { 0x1b, '[', '0', ';', '0', ';', '0', 'm', 0 };
		static const char t_black[] = { 0x1b, '[', '0', ';', '3', '0', ';', '5', '9', 'm', 0 };
		static const char t_red[] = { 0x1b, '[', '0', ';', '3', '1', ';', '5', '9', 'm', 0 };
		static const char t_green[] = { 0x1b, '[', '0', ';', '3', '2', ';', '5', '9', 'm', 0 };
		static const char t_yellow[] = { 0x1b, '[', '0', ';', '3', '3', ';', '5', '9', 'm', 0 };
		static const char t_blue[] = { 0x1b, '[', '0', ';', '3', '4', ';', '5', '9', 'm', 0 };
		static const char t_magenta[] = { 0x1b, '[', '0', ';', '3', '5', ';', '5', '9', 'm', 0 };
		static const char t_cyan[] = { 0x1b, '[', '0', ';', '3', '6', ';', '5', '9', 'm', 0 };
		static const char t_white[] = { 0x1b, '[', '0', ';', '3', '7', ';', '5', '9', 'm', 0 };
		static const char t_bold[] = { 0x1b, '[', '1', 'm', 0 };
		static const char t_underscore[] = { 0x1b, '[', '4', 'm', 0 };
#else
		static const char t_normal[] = { 0 };
		static const char *const t_black = cimg::t_normal,
			*const t_red = cimg::t_normal,
			*const t_green = cimg::t_normal,
			*const t_yellow = cimg::t_normal,
			*const t_blue = cimg::t_normal,
			*const t_magenta = cimg::t_normal,
			*const t_cyan = cimg::t_normal,
			*const t_white = cimg::t_normal,
			*const t_bold = cimg::t_normal,
			*const t_underscore = cimg::t_normal;
#endif

		inline std::FILE* output(std::FILE *file = 0);
		inline void info();

		//! Avoid warning messages due to unused parameters. Do nothing actually.
		template<typename T>
		inline void unused(const T&, ...) {}

		// [internal] Lock/unlock a mutex for managing concurrent threads.
		// 'lock_mode' can be { 0=unlock | 1=lock | 2=trylock }.
		// 'n' can be in [0,31] but mutex range [0,15] is reserved by CImg.
		inline int mutex(const unsigned int n, const int lock_mode = 1);

		inline unsigned int& _exception_mode(const unsigned int value, const bool is_set) {
			static unsigned int mode = cimg_verbosity;
			if (is_set) { cimg::mutex(0); mode = value<4 ? value : 4; cimg::mutex(0, 0); }
			return mode;
		}

		// Functions to return standard streams 'stdin', 'stdout' and 'stderr'.
		inline FILE* _stdin(const bool throw_exception = true);
		inline FILE* _stdout(const bool throw_exception = true);
		inline FILE* _stderr(const bool throw_exception = true);

		// Mandatory because Microsoft's _snprintf() and _vsnprintf() do not add the '\0' character
		// at the end of the string.
#if cimg_OS==2 && defined(_MSC_VER)
		inline int _snprintf(char *const s, const size_t size, const char *const format, ...) {
			va_list ap;
			va_start(ap, format);
			const int result = _vsnprintf(s, size, format, ap);
			va_end(ap);
			return result;
		}

		inline int _vsnprintf(char *const s, const size_t size, const char *const format, va_list ap) {
			int result = -1;
			cimg::mutex(6);
			if (size) result = _vsnprintf_s(s, size, _TRUNCATE, format, ap);
			if (result == -1) result = _vscprintf(format, ap);
			cimg::mutex(6, 0);
			return result;
		}

		// Mutex-protected version of sscanf, sprintf and snprintf.
		// Used only MacOSX, as it seems those functions are not re-entrant on MacOSX.
#elif defined(__MACOSX__) || defined(__APPLE__)
		inline int _sscanf(const char *const s, const char *const format, ...) {
			cimg::mutex(6);
			va_list args;
			va_start(args, format);
			const int result = std::vsscanf(s, format, args);
			va_end(args);
			cimg::mutex(6, 0);
			return result;
		}

		inline int _sprintf(char *const s, const char *const format, ...) {
			cimg::mutex(6);
			va_list args;
			va_start(args, format);
			const int result = std::vsprintf(s, format, args);
			va_end(args);
			cimg::mutex(6, 0);
			return result;
		}

		inline int _snprintf(char *const s, const size_t n, const char *const format, ...) {
			cimg::mutex(6);
			va_list args;
			va_start(args, format);
			const int result = std::vsnprintf(s, n, format, args);
			va_end(args);
			cimg::mutex(6, 0);
			return result;
		}

		inline int _vsnprintf(char *const s, const size_t size, const char* format, va_list ap) {
			cimg::mutex(6);
			const int result = std::vsnprintf(s, size, format, ap);
			cimg::mutex(6, 0);
			return result;
		}
#endif

		//! Set current \CImg exception mode.
		/**
		The way error messages are handled by \CImg can be changed dynamically, using this function.
		\param mode Desired exception mode. Possible values are:
		- \c 0: Hide library messages (quiet mode).
		- \c 1: Print library messages on the console.
		- \c 2: Display library messages on a dialog window.
		- \c 3: Do as \c 1 + add extra debug warnings (slow down the code!).
		- \c 4: Do as \c 2 + add extra debug warnings (slow down the code!).
		**/
		inline unsigned int& exception_mode(const unsigned int mode) {
			return _exception_mode(mode, true);
		}

		//! Return current \CImg exception mode.
		/**
		\note By default, return the value of configuration macro \c cimg_verbosity
		**/
		inline unsigned int& exception_mode() {
			return _exception_mode(0, false);
		}

		//! Set current \CImg openmp mode.
		/**
		The way openmp-based methods are handled by \CImg can be changed dynamically, using this function.
		\param mode Desired openmp mode. Possible values are:
		- \c 0: Never parallelize.
		- \c 1: Always parallelize.
		- \c 2: Adaptive parallelization mode (default behavior).
		**/
		inline unsigned int& _openmp_mode(const unsigned int value, const bool is_set) {
			static unsigned int mode = 2;
			if (is_set) { cimg::mutex(0); mode = value<2 ? value : 2; cimg::mutex(0, 0); }
			return mode;
		}

		inline unsigned int& openmp_mode(const unsigned int mode) {
			return _openmp_mode(mode, true);
		}

		//! Return current \CImg openmp mode.
		inline unsigned int& openmp_mode() {
			return _openmp_mode(0, false);
		}

#define cimg_openmp_if(cond) if (cimg::openmp_mode()==1 || (cimg::openmp_mode()>1 && (cond)))

		// Display a simple dialog box, and wait for the user's response.
		inline int dialog(const char *const title, const char *const msg, const char *const button1_label = "OK",
			const char *const button2_label = 0, const char *const button3_label = 0,
			const char *const button4_label = 0, const char *const button5_label = 0,
			const char *const button6_label = 0, const bool centering = false);

		// Evaluate math expression.
		inline double eval(const char *const expression,
			const double x = 0, const double y = 0, const double z = 0, const double c = 0);

	}

	/*---------------------------------------
	#
	# Define the CImgException structures
	#
	--------------------------------------*/
	//! Instances of \c CImgException are thrown when errors are encountered in a \CImg function call.
	/**
	\par Overview

	CImgException is the base class of all exceptions thrown by \CImg (except \b CImgAbortException).
	CImgException is never thrown itself. Derived classes that specify the type of errord are thrown instead.
	These classes can be:

	- \b CImgAbortException: Thrown when a computationally-intensive function is aborted by an external signal.
	This is the only \c non-derived exception class.

	- \b CImgArgumentException: Thrown when one argument of a called \CImg function is invalid.
	This is probably one of the most thrown exception by \CImg.
	For instance, the following example throws a \c CImgArgumentException:
	\code
	CImg<float> img(100,100,1,3); // Define a 100x100 color image with float-valued pixels.
	img.mirror('e');              // Try to mirror image along the (non-existing) 'e'-axis.
	\endcode

	- \b CImgDisplayException: Thrown when something went wrong during the display of images in CImgDisplay instances.

	- \b CImgInstanceException: Thrown when an instance associated to a called \CImg method does not fit
	the function requirements. For instance, the following example throws a \c CImgInstanceException:
	\code
	const CImg<float> img;           // Define an empty image.
	const float value = img.at(0);   // Try to read first pixel value (does not exist).
	\endcode

	- \b CImgIOException: Thrown when an error occured when trying to load or save image files.
	This happens when trying to read files that do not exist or with invalid formats.
	For instance, the following example throws a \c CImgIOException:
	\code
	const CImg<float> img("missing_file.jpg");  // Try to load a file that does not exist.
	\endcode

	- \b CImgWarningException: Thrown only if configuration macro \c cimg_strict_warnings is set, and
	when a \CImg function has to display a warning message (see cimg::warn()).

	It is not recommended to throw CImgException instances by yourself,
	since they are expected to be thrown only by \CImg.
	When an error occurs in a library function call, \CImg may display error messages on the screen or on the
	standard output, depending on the current \CImg exception mode.
	The \CImg exception mode can be get and set by functions cimg::exception_mode() and
	cimg::exception_mode(unsigned int).

	\par Exceptions handling

	In all cases, when an error occurs in \CImg, an instance of the corresponding exception class is thrown.
	This may lead the program to break (this is the default behavior), but you can bypass this behavior by
	handling the exceptions by yourself,
	using a usual <tt>try { ... } catch () { ... }</tt> bloc, as in the following example:
	\code
	#define "CImg.h"
	using namespace cimg_library;
	int main() {
	cimg::exception_mode(0);                                    // Enable quiet exception mode.
	try {
	...                                                       // Here, do what you want to stress CImg.
	} catch (CImgException& e) {                                // You succeeded: something went wrong!
	std::fprintf(stderr,"CImg Library Error: %s",e.what());   // Display your custom error message.
	...                                                       // Do what you want now to save the ship!
	}
	}
	\endcode
	**/
	struct CImgException : public std::exception {
#define _cimg_exception_err(etype,disp_flag) \
  std::va_list ap, ap2; \
  va_start(ap,format); va_start(ap2,format); \
  int size = cimg_vsnprintf(0,0,format,ap2); \
  if (size++>=0) { \
    delete[] _message; \
    _message = new char[size]; \
    cimg_vsnprintf(_message,size,format,ap); \
    if (cimg::exception_mode()) { \
      std::fprintf(cimg::output(),"\n%s[CImg] *** %s ***%s %s\n",cimg::t_red,etype,cimg::t_normal,_message); \
      if (cimg_display && disp_flag && !(cimg::exception_mode()%2)) try { cimg::dialog(etype,_message,"Abort"); } \
      catch (CImgException&) {} \
      if (cimg::exception_mode()>=3) cimg_library_suffixed::cimg::info(); \
    } \
  } \
  va_end(ap); va_end(ap2); \

		char *_message;
		CImgException() { _message = new char[1]; *_message = 0; }
		CImgException(const char *const format, ...) :_message(0) { _cimg_exception_err("CImgException", true); }
		CImgException(const CImgException& e) :std::exception(e) {
			const size_t size = std::strlen(e._message);
			_message = new char[size + 1];
			std::strncpy(_message, e._message, size);
			_message[size] = 0;
		}
		~CImgException() throw() { delete[] _message; }
		CImgException& operator=(const CImgException& e) {
			const size_t size = std::strlen(e._message);
			_message = new char[size + 1];
			std::strncpy(_message, e._message, size);
			_message[size] = 0;
			return *this;
		}
		//! Return a C-string containing the error message associated to the thrown exception.
		const char *what() const throw() { return _message; }
	};

	// The CImgAbortException class is used to throw an exception when
	// a computationally-intensive function has been aborted by an external signal.
	struct CImgAbortException : public std::exception {
		char *_message;
		CImgAbortException() { _message = new char[1]; *_message = 0; }
		CImgAbortException(const char *const format, ...) :_message(0) { _cimg_exception_err("CImgAbortException", true); }
		CImgAbortException(const CImgAbortException& e) :std::exception(e) {
			const size_t size = std::strlen(e._message);
			_message = new char[size + 1];
			std::strncpy(_message, e._message, size);
			_message[size] = 0;
		}
		~CImgAbortException() throw() { delete[] _message; }
		CImgAbortException& operator=(const CImgAbortException& e) {
			const size_t size = std::strlen(e._message);
			_message = new char[size + 1];
			std::strncpy(_message, e._message, size);
			_message[size] = 0;
			return *this;
		}
		//! Return a C-string containing the error message associated to the thrown exception.
		const char *what() const throw() { return _message; }
	};

	// The CImgArgumentException class is used to throw an exception related
	// to invalid arguments encountered in a library function call.
	struct CImgArgumentException : public CImgException {
		CImgArgumentException(const char *const format, ...) { _cimg_exception_err("CImgArgumentException", true); }
	};

	// The CImgDisplayException class is used to throw an exception related
	// to display problems encountered in a library function call.
	struct CImgDisplayException : public CImgException {
		CImgDisplayException(const char *const format, ...) { _cimg_exception_err("CImgDisplayException", false); }
	};

	// The CImgInstanceException class is used to throw an exception related
	// to an invalid instance encountered in a library function call.
	struct CImgInstanceException : public CImgException {
		CImgInstanceException(const char *const format, ...) { _cimg_exception_err("CImgInstanceException", true); }
	};

	// The CImgIOException class is used to throw an exception related
	// to input/output file problems encountered in a library function call.
	struct CImgIOException : public CImgException {
		CImgIOException(const char *const format, ...) { _cimg_exception_err("CImgIOException", true); }
	};

	// The CImgWarningException class is used to throw an exception for warnings
	// encountered in a library function call.
	struct CImgWarningException : public CImgException {
		CImgWarningException(const char *const format, ...) { _cimg_exception_err("CImgWarningException", false); }
	};

	/*-------------------------------------
	#
	# Define cimg:: namespace
	#
	-----------------------------------*/
	//! Contains \a low-level functions and variables of the \CImg Library.
	/**
	Most of the functions and variables within this namespace are used by the \CImg library for low-level operations.
	You may use them to access specific const values or environment variables internally used by \CImg.
	\warning Never write <tt>using namespace cimg_library::cimg;</tt> in your source code. Lot of functions in the
	<tt>cimg:: namespace</tt> have the same names as standard C functions that may be defined in the global
	namespace <tt>::</tt>.
	**/
	namespace cimg {

		// Define traits that will be used to determine the best data type to work in CImg functions.
		//
		template<typename T> struct type {
			static const char* string() {
				static const char* s[] = { "unknown",   "unknown8",   "unknown16",  "unknown24",
					"unknown32", "unknown40",  "unknown48",  "unknown56",
					"unknown64", "unknown72",  "unknown80",  "unknown88",
					"unknown96", "unknown104", "unknown112", "unknown120",
					"unknown128" };
				return s[(sizeof(T)<17) ? sizeof(T) : 0];
			}
			static bool is_float() { return false; }
			static bool is_inf(const T) { return false; }
			static bool is_nan(const T) { return false; }
			static T min() { return ~max(); }
			static T max() { return (T)1 << (8 * sizeof(T) - 1); }
			static T inf() { return max(); }
			static T cut(const double val) { return val<(double)min() ? min() : val>(double)max() ? max() : (T)val; }
			static const char* format() { return "%s"; }
			static const char* format_s() { return "%s"; }
			static const char* format(const T& val) { static const char *const s = "unknown"; cimg::unused(val); return s; }
		};

		template<> struct type<bool> {
			static const char* string() { static const char *const s = "bool"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const bool) { return false; }
			static bool is_nan(const bool) { return false; }
			static bool min() { return false; }
			static bool max() { return true; }
			static bool inf() { return max(); }
			static bool is_inf() { return false; }
			static bool cut(const double val) { return val<(double)min() ? min() : val>(double)max() ? max() : (bool)val; }
			static const char* format() { return "%s"; }
			static const char* format_s() { return "%s"; }
			static const char* format(const bool val) { static const char* s[] = { "false", "true" }; return s[val ? 1 : 0]; }
		};

		template<> struct type<unsigned char> {
			static const char* string() { static const char *const s = "unsigned char"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const unsigned char) { return false; }
			static bool is_nan(const unsigned char) { return false; }
			static unsigned char min() { return 0; }
			static unsigned char max() { return (unsigned char)-1; }
			static unsigned char inf() { return max(); }
			static unsigned char cut(const double val) {
				return val<(double)min() ? min() : val>(double)max() ? max() : (unsigned char)val;
			}
			static const char* format() { return "%u"; }
			static const char* format_s() { return "%u"; }
			static unsigned int format(const unsigned char val) { return (unsigned int)val; }
		};

#if defined(CHAR_MAX) && CHAR_MAX==255
		template<> struct type<char> {
			static const char* string() { static const char *const s = "char"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const char) { return false; }
			static bool is_nan(const char) { return false; }
			static char min() { return 0; }
			static char max() { return (char)-1; }
			static char inf() { return max(); }
			static char cut(const double val) {
				return val<(double)min() ? min() : val>(double)max() ? max() : (unsigned char)val;
			}
			static const char* format() { return "%u"; }
			static const char* format_s() { return "%u"; }
			static unsigned int format(const char val) { return (unsigned int)val; }
		};
#else
		template<> struct type<char> {
			static const char* string() { static const char *const s = "char"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const char) { return false; }
			static bool is_nan(const char) { return false; }
			static char min() { return ~max(); }
			static char max() { return (char)((unsigned char)-1 >> 1); }
			static char inf() { return max(); }
			static char cut(const double val) { return val<(double)min() ? min() : val>(double)max() ? max() : (char)val; }
			static const char* format() { return "%d"; }
			static const char* format_s() { return "%d"; }
			static int format(const char val) { return (int)val; }
		};
#endif

		template<> struct type<signed char> {
			static const char* string() { static const char *const s = "signed char"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const signed char) { return false; }
			static bool is_nan(const signed char) { return false; }
			static signed char min() { return ~max(); }
			static signed char max() { return (signed char)((unsigned char)-1 >> 1); }
			static signed char inf() { return max(); }
			static signed char cut(const double val) {
				return val<(double)min() ? min() : val>(double)max() ? max() : (signed char)val;
			}
			static const char* format() { return "%d"; }
			static const char* format_s() { return "%d"; }
			static int format(const signed char val) { return (int)val; }
		};

		template<> struct type<unsigned short> {
			static const char* string() { static const char *const s = "unsigned short"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const unsigned short) { return false; }
			static bool is_nan(const unsigned short) { return false; }
			static unsigned short min() { return 0; }
			static unsigned short max() { return (unsigned short)-1; }
			static unsigned short inf() { return max(); }
			static unsigned short cut(const double val) {
				return val<(double)min() ? min() : val>(double)max() ? max() : (unsigned short)val;
			}
			static const char* format() { return "%u"; }
			static const char* format_s() { return "%u"; }
			static unsigned int format(const unsigned short val) { return (unsigned int)val; }
		};

		template<> struct type<short> {
			static const char* string() { static const char *const s = "short"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const short) { return false; }
			static bool is_nan(const short) { return false; }
			static short min() { return ~max(); }
			static short max() { return (short)((unsigned short)-1 >> 1); }
			static short inf() { return max(); }
			static short cut(const double val) { return val<(double)min() ? min() : val>(double)max() ? max() : (short)val; }
			static const char* format() { return "%d"; }
			static const char* format_s() { return "%d"; }
			static int format(const short val) { return (int)val; }
		};

		template<> struct type<unsigned int> {
			static const char* string() { static const char *const s = "unsigned int"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const unsigned int) { return false; }
			static bool is_nan(const unsigned int) { return false; }
			static unsigned int min() { return 0; }
			static unsigned int max() { return (unsigned int)-1; }
			static unsigned int inf() { return max(); }
			static unsigned int cut(const double val) {
				return val<(double)min() ? min() : val>(double)max() ? max() : (unsigned int)val;
			}
			static const char* format() { return "%u"; }
			static const char* format_s() { return "%u"; }
			static unsigned int format(const unsigned int val) { return val; }
		};

		template<> struct type<int> {
			static const char* string() { static const char *const s = "int"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const int) { return false; }
			static bool is_nan(const int) { return false; }
			static int min() { return ~max(); }
			static int max() { return (int)((unsigned int)-1 >> 1); }
			static int inf() { return max(); }
			static int cut(const double val) { return val<(double)min() ? min() : val>(double)max() ? max() : (int)val; }
			static const char* format() { return "%d"; }
			static const char* format_s() { return "%d"; }
			static int format(const int val) { return val; }
		};

		template<> struct type<cimg_uint64> {
			static const char* string() { static const char *const s = "unsigned int64"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const cimg_uint64) { return false; }
			static bool is_nan(const cimg_uint64) { return false; }
			static cimg_uint64 min() { return 0; }
			static cimg_uint64 max() { return (cimg_uint64)-1; }
			static cimg_uint64 inf() { return max(); }
			static cimg_uint64 cut(const double val) {
				return val<(double)min() ? min() : val>(double)max() ? max() : (cimg_uint64)val;
			}
			static const char* format() { return cimg_fuint64; }
			static const char* format_s() { return cimg_fuint64; }
			static unsigned long format(const cimg_uint64 val) { return (unsigned long)val; }
		};

		template<> struct type<cimg_int64> {
			static const char* string() { static const char *const s = "int64"; return s; }
			static bool is_float() { return false; }
			static bool is_inf(const cimg_int64) { return false; }
			static bool is_nan(const cimg_int64) { return false; }
			static cimg_int64 min() { return ~max(); }
			static cimg_int64 max() { return (cimg_int64)((cimg_uint64)-1 >> 1); }
			static cimg_int64 inf() { return max(); }
			static cimg_int64 cut(const double val) {
				return val<(double)min() ? min() : val>(double)max() ? max() : (cimg_int64)val;
			}
			static const char* format() { return cimg_fint64; }
			static const char* format_s() { return cimg_fint64; }
			static long format(const long val) { return (long)val; }
		};

		template<> struct type<double> {
			static const char* string() { static const char *const s = "double"; return s; }
			static bool is_float() { return true; }
			static bool is_inf(const double val) {
#ifdef isinf
				return (bool)isinf(val);
#else
				return !is_nan(val) && (val<cimg::type<double>::min() || val>cimg::type<double>::max());
#endif
			}
			static bool is_nan(const double val) {
#ifdef isnan
				return (bool)isnan(val);
#else
				return !(val == val);
#endif
			}
			static double min() { return -DBL_MAX; }
			static double max() { return DBL_MAX; }
			static double inf() {
#ifdef INFINITY
				return (double)INFINITY;
#else
				return max()*max();
#endif
			}
			static double nan() {
#ifdef NAN
				return (double)NAN;
#else
				const double val_nan = -std::sqrt(-1.0); return val_nan;
#endif
			}
			static double cut(const double val) { return val; }
			static const char* format() { return "%.17g"; }
			static const char* format_s() { return "%g"; }
			static double format(const double val) { return val; }
		};

		template<> struct type<float> {
			static const char* string() { static const char *const s = "float"; return s; }
			static bool is_float() { return true; }
			static bool is_inf(const float val) {
#ifdef isinf
				return (bool)isinf(val);
#else
				return !is_nan(val) && (val<cimg::type<float>::min() || val>cimg::type<float>::max());
#endif
			}
			static bool is_nan(const float val) {
#ifdef isnan
				return (bool)isnan(val);
#else
				return !(val == val);
#endif
			}
			static float min() { return -FLT_MAX; }
			static float max() { return FLT_MAX; }
			static float inf() { return (float)cimg::type<double>::inf(); }
			static float nan() { return (float)cimg::type<double>::nan(); }
			static float cut(const double val) { return (float)val; }
			static float cut(const float val) { return (float)val; }
			static const char* format() { return "%.9g"; }
			static const char* format_s() { return "%g"; }
			static double format(const float val) { return (double)val; }
		};

		template<> struct type<long double> {
			static const char* string() { static const char *const s = "long double"; return s; }
			static bool is_float() { return true; }
			static bool is_inf(const long double val) {
#ifdef isinf
				return (bool)isinf(val);
#else
				return !is_nan(val) && (val<cimg::type<long double>::min() || val>cimg::type<long double>::max());
#endif
			}
			static bool is_nan(const long double val) {
#ifdef isnan
				return (bool)isnan(val);
#else
				return !(val == val);
#endif
			}
			static long double min() { return -LDBL_MAX; }
			static long double max() { return LDBL_MAX; }
			static long double inf() { return max()*max(); }
			static long double nan() { const long double val_nan = -std::sqrt(-1.0L); return val_nan; }
			static long double cut(const long double val) { return val; }
			static const char* format() { return "%.17g"; }
			static const char* format_s() { return "%g"; }
			static double format(const long double val) { return (double)val; }
		};

#ifdef cimg_use_half
		template<> struct type<half> {
			static const char* string() { static const char *const s = "half"; return s; }
			static bool is_float() { return true; }
			static bool is_inf(const long double val) {
#ifdef isinf
				return (bool)isinf(val);
#else
				return !is_nan(val) && (val<cimg::type<half>::min() || val>cimg::type<half>::max());
#endif
			}
			static bool is_nan(const long double val) {
#ifdef isnan
				return (bool)isnan(val);
#else
				return !(val == val);
#endif
			}
			static half min() { return (half)-65504; }
			static half max() { return (half)65504; }
			static half inf() { return max()*max(); }
			static half nan() { const half val_nan = (half)-std::sqrt(-1.0); return val_nan; }
			static half cut(const double val) { return (half)val; }
			static const char* format() { return "%.9g"; }
			static const char* format_s() { return "%g"; }
			static double format(const half val) { return (double)val; }
		};
#endif

		template<typename T, typename t> struct superset { typedef T type; };
		template<> struct superset<bool, unsigned char> { typedef unsigned char type; };
		template<> struct superset<bool, char> { typedef char type; };
		template<> struct superset<bool, signed char> { typedef signed char type; };
		template<> struct superset<bool, unsigned short> { typedef unsigned short type; };
		template<> struct superset<bool, short> { typedef short type; };
		template<> struct superset<bool, unsigned int> { typedef unsigned int type; };
		template<> struct superset<bool, int> { typedef int type; };
		template<> struct superset<bool, cimg_uint64> { typedef cimg_uint64 type; };
		template<> struct superset<bool, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<bool, float> { typedef float type; };
		template<> struct superset<bool, double> { typedef double type; };
		template<> struct superset<unsigned char, char> { typedef short type; };
		template<> struct superset<unsigned char, signed char> { typedef short type; };
		template<> struct superset<unsigned char, unsigned short> { typedef unsigned short type; };
		template<> struct superset<unsigned char, short> { typedef short type; };
		template<> struct superset<unsigned char, unsigned int> { typedef unsigned int type; };
		template<> struct superset<unsigned char, int> { typedef int type; };
		template<> struct superset<unsigned char, cimg_uint64> { typedef cimg_uint64 type; };
		template<> struct superset<unsigned char, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<unsigned char, float> { typedef float type; };
		template<> struct superset<unsigned char, double> { typedef double type; };
		template<> struct superset<signed char, unsigned char> { typedef short type; };
		template<> struct superset<signed char, char> { typedef short type; };
		template<> struct superset<signed char, unsigned short> { typedef int type; };
		template<> struct superset<signed char, short> { typedef short type; };
		template<> struct superset<signed char, unsigned int> { typedef cimg_int64 type; };
		template<> struct superset<signed char, int> { typedef int type; };
		template<> struct superset<signed char, cimg_uint64> { typedef cimg_int64 type; };
		template<> struct superset<signed char, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<signed char, float> { typedef float type; };
		template<> struct superset<signed char, double> { typedef double type; };
		template<> struct superset<char, unsigned char> { typedef short type; };
		template<> struct superset<char, signed char> { typedef short type; };
		template<> struct superset<char, unsigned short> { typedef int type; };
		template<> struct superset<char, short> { typedef short type; };
		template<> struct superset<char, unsigned int> { typedef cimg_int64 type; };
		template<> struct superset<char, int> { typedef int type; };
		template<> struct superset<char, cimg_uint64> { typedef cimg_int64 type; };
		template<> struct superset<char, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<char, float> { typedef float type; };
		template<> struct superset<char, double> { typedef double type; };
		template<> struct superset<unsigned short, char> { typedef int type; };
		template<> struct superset<unsigned short, signed char> { typedef int type; };
		template<> struct superset<unsigned short, short> { typedef int type; };
		template<> struct superset<unsigned short, unsigned int> { typedef unsigned int type; };
		template<> struct superset<unsigned short, int> { typedef int type; };
		template<> struct superset<unsigned short, cimg_uint64> { typedef cimg_uint64 type; };
		template<> struct superset<unsigned short, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<unsigned short, float> { typedef float type; };
		template<> struct superset<unsigned short, double> { typedef double type; };
		template<> struct superset<short, unsigned short> { typedef int type; };
		template<> struct superset<short, unsigned int> { typedef cimg_int64 type; };
		template<> struct superset<short, int> { typedef int type; };
		template<> struct superset<short, cimg_uint64> { typedef cimg_int64 type; };
		template<> struct superset<short, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<short, float> { typedef float type; };
		template<> struct superset<short, double> { typedef double type; };
		template<> struct superset<unsigned int, char> { typedef cimg_int64 type; };
		template<> struct superset<unsigned int, signed char> { typedef cimg_int64 type; };
		template<> struct superset<unsigned int, short> { typedef cimg_int64 type; };
		template<> struct superset<unsigned int, int> { typedef cimg_int64 type; };
		template<> struct superset<unsigned int, cimg_uint64> { typedef cimg_uint64 type; };
		template<> struct superset<unsigned int, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<unsigned int, float> { typedef float type; };
		template<> struct superset<unsigned int, double> { typedef double type; };
		template<> struct superset<int, unsigned int> { typedef cimg_int64 type; };
		template<> struct superset<int, cimg_uint64> { typedef cimg_int64 type; };
		template<> struct superset<int, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<int, float> { typedef float type; };
		template<> struct superset<int, double> { typedef double type; };
		template<> struct superset<cimg_uint64, char> { typedef cimg_int64 type; };
		template<> struct superset<cimg_uint64, signed char> { typedef cimg_int64 type; };
		template<> struct superset<cimg_uint64, short> { typedef cimg_int64 type; };
		template<> struct superset<cimg_uint64, int> { typedef cimg_int64 type; };
		template<> struct superset<cimg_uint64, cimg_int64> { typedef cimg_int64 type; };
		template<> struct superset<cimg_uint64, float> { typedef double type; };
		template<> struct superset<cimg_uint64, double> { typedef double type; };
		template<> struct superset<cimg_int64, float> { typedef double type; };
		template<> struct superset<cimg_int64, double> { typedef double type; };
		template<> struct superset<float, double> { typedef double type; };
#ifdef cimg_use_half
		template<> struct superset<half, unsigned short> { typedef float type; };
		template<> struct superset<half, short> { typedef float type; };
		template<> struct superset<half, unsigned int> { typedef float type; };
		template<> struct superset<half, int> { typedef float type; };
		template<> struct superset<half, cimg_uint64> { typedef float type; };
		template<> struct superset<half, cimg_int64> { typedef float type; };
		template<> struct superset<half, float> { typedef float type; };
		template<> struct superset<half, double> { typedef double type; };
#endif

		template<typename t1, typename t2, typename t3> struct superset2 {
			typedef typename superset<t1, typename superset<t2, t3>::type>::type type;
		};

		template<typename t1, typename t2, typename t3, typename t4> struct superset3 {
			typedef typename superset<t1, typename superset2<t2, t3, t4>::type>::type type;
		};

		template<typename t1, typename t2> struct last { typedef t2 type; };

#define _cimg_Tt typename cimg::superset<T,t>::type
#define _cimg_Tfloat typename cimg::superset<T,float>::type
#define _cimg_Ttfloat typename cimg::superset2<T,t,float>::type
#define _cimg_Ttdouble typename cimg::superset2<T,t,double>::type

		// Define variables used internally by CImg.
#if cimg_display==1
		struct X11_info {
			unsigned int nb_wins;
			pthread_t *events_thread;
			pthread_cond_t wait_event;
			pthread_mutex_t wait_event_mutex;
			CImgDisplay **wins;
			Display *display;
			unsigned int nb_bits;
			bool is_blue_first;
			bool is_shm_enabled;
			bool byte_order;
#ifdef cimg_use_xrandr
			XRRScreenSize *resolutions;
			Rotation curr_rotation;
			unsigned int curr_resolution;
			unsigned int nb_resolutions;
#endif
			X11_info() :nb_wins(0), events_thread(0), display(0),
				nb_bits(0), is_blue_first(false), is_shm_enabled(false), byte_order(false) {
#ifdef __FreeBSD__
				XInitThreads();
#endif
				wins = new CImgDisplay*[1024];
				pthread_mutex_init(&wait_event_mutex, 0);
				pthread_cond_init(&wait_event, 0);
#ifdef cimg_use_xrandr
				resolutions = 0;
				curr_rotation = 0;
				curr_resolution = nb_resolutions = 0;
#endif
			}

			~X11_info() {
				delete[] wins;
				/*
				if (events_thread) {
				pthread_cancel(*events_thread);
				delete events_thread;
				}
				if (display) { } // XCloseDisplay(display); }
				pthread_cond_destroy(&wait_event);
				pthread_mutex_unlock(&wait_event_mutex);
				pthread_mutex_destroy(&wait_event_mutex);
				*/
			}
		};
#if defined(cimg_module)
		X11_info& X11_attr();
#elif defined(cimg_main)
		X11_info& X11_attr() { static X11_info val; return val; }
#else
		inline X11_info& X11_attr() { static X11_info val; return val; }
#endif
#define cimg_lock_display() cimg::mutex(15)
#define cimg_unlock_display() cimg::mutex(15,0)

#elif cimg_display==2
		struct Win32_info {
			HANDLE wait_event;
			Win32_info() { wait_event = CreateEvent(0, FALSE, FALSE, 0); }
		};
#if defined(cimg_module)
		Win32_info& Win32_attr();
#elif defined(cimg_main)
		Win32_info& Win32_attr() { static Win32_info val; return val; }
#else
		inline Win32_info& Win32_attr() { static Win32_info val; return val; }
#endif
#endif

		struct Mutex_info {
#if cimg_OS==2
			HANDLE mutex[32];
			Mutex_info() { for (unsigned int i = 0; i<32; ++i) mutex[i] = CreateMutex(0, FALSE, 0); }
			void lock(const unsigned int n) { WaitForSingleObject(mutex[n], INFINITE); }
			void unlock(const unsigned int n) { ReleaseMutex(mutex[n]); }
			int trylock(const unsigned int) { return 0; }
#elif defined(_PTHREAD_H)
			pthread_mutex_t mutex[32];
			Mutex_info() { for (unsigned int i = 0; i<32; ++i) pthread_mutex_init(&mutex[i], 0); }
			void lock(const unsigned int n) { pthread_mutex_lock(&mutex[n]); }
			void unlock(const unsigned int n) { pthread_mutex_unlock(&mutex[n]); }
			int trylock(const unsigned int n) { return pthread_mutex_trylock(&mutex[n]); }
#else
			Mutex_info() {}
			void lock(const unsigned int) {}
			void unlock(const unsigned int) {}
			int trylock(const unsigned int) { return 0; }
#endif
		};
#if defined(cimg_module)
		Mutex_info& Mutex_attr();
#elif defined(cimg_main)
		Mutex_info& Mutex_attr() { static Mutex_info val; return val; }
#else
		inline Mutex_info& Mutex_attr() { static Mutex_info val; return val; }
#endif

#if defined(cimg_use_magick)
		static struct Magick_info {
			Magick_info() {
				Magick::InitializeMagick("");
			}
		} _Magick_info;
#endif

#if cimg_display==1
		// Define keycodes for X11-based graphical systems.
		const unsigned int keyESC = XK_Escape;
		const unsigned int keyF1 = XK_F1;
		const unsigned int keyF2 = XK_F2;
		const unsigned int keyF3 = XK_F3;
		const unsigned int keyF4 = XK_F4;
		const unsigned int keyF5 = XK_F5;
		const unsigned int keyF6 = XK_F6;
		const unsigned int keyF7 = XK_F7;
		const unsigned int keyF8 = XK_F8;
		const unsigned int keyF9 = XK_F9;
		const unsigned int keyF10 = XK_F10;
		const unsigned int keyF11 = XK_F11;
		const unsigned int keyF12 = XK_F12;
		const unsigned int keyPAUSE = XK_Pause;
		const unsigned int key1 = XK_1;
		const unsigned int key2 = XK_2;
		const unsigned int key3 = XK_3;
		const unsigned int key4 = XK_4;
		const unsigned int key5 = XK_5;
		const unsigned int key6 = XK_6;
		const unsigned int key7 = XK_7;
		const unsigned int key8 = XK_8;
		const unsigned int key9 = XK_9;
		const unsigned int key0 = XK_0;
		const unsigned int keyBACKSPACE = XK_BackSpace;
		const unsigned int keyINSERT = XK_Insert;
		const unsigned int keyHOME = XK_Home;
		const unsigned int keyPAGEUP = XK_Page_Up;
		const unsigned int keyTAB = XK_Tab;
		const unsigned int keyQ = XK_q;
		const unsigned int keyW = XK_w;
		const unsigned int keyE = XK_e;
		const unsigned int keyR = XK_r;
		const unsigned int keyT = XK_t;
		const unsigned int keyY = XK_y;
		const unsigned int keyU = XK_u;
		const unsigned int keyI = XK_i;
		const unsigned int keyO = XK_o;
		const unsigned int keyP = XK_p;
		const unsigned int keyDELETE = XK_Delete;
		const unsigned int keyEND = XK_End;
		const unsigned int keyPAGEDOWN = XK_Page_Down;
		const unsigned int keyCAPSLOCK = XK_Caps_Lock;
		const unsigned int keyA = XK_a;
		const unsigned int keyS = XK_s;
		const unsigned int keyD = XK_d;
		const unsigned int keyF = XK_f;
		const unsigned int keyG = XK_g;
		const unsigned int keyH = XK_h;
		const unsigned int keyJ = XK_j;
		const unsigned int keyK = XK_k;
		const unsigned int keyL = XK_l;
		const unsigned int keyENTER = XK_Return;
		const unsigned int keySHIFTLEFT = XK_Shift_L;
		const unsigned int keyZ = XK_z;
		const unsigned int keyX = XK_x;
		const unsigned int keyC = XK_c;
		const unsigned int keyV = XK_v;
		const unsigned int keyB = XK_b;
		const unsigned int keyN = XK_n;
		const unsigned int keyM = XK_m;
		const unsigned int keySHIFTRIGHT = XK_Shift_R;
		const unsigned int keyARROWUP = XK_Up;
		const unsigned int keyCTRLLEFT = XK_Control_L;
		const unsigned int keyAPPLEFT = XK_Super_L;
		const unsigned int keyALT = XK_Alt_L;
		const unsigned int keySPACE = XK_space;
		const unsigned int keyALTGR = XK_Alt_R;
		const unsigned int keyAPPRIGHT = XK_Super_R;
		const unsigned int keyMENU = XK_Menu;
		const unsigned int keyCTRLRIGHT = XK_Control_R;
		const unsigned int keyARROWLEFT = XK_Left;
		const unsigned int keyARROWDOWN = XK_Down;
		const unsigned int keyARROWRIGHT = XK_Right;
		const unsigned int keyPAD0 = XK_KP_0;
		const unsigned int keyPAD1 = XK_KP_1;
		const unsigned int keyPAD2 = XK_KP_2;
		const unsigned int keyPAD3 = XK_KP_3;
		const unsigned int keyPAD4 = XK_KP_4;
		const unsigned int keyPAD5 = XK_KP_5;
		const unsigned int keyPAD6 = XK_KP_6;
		const unsigned int keyPAD7 = XK_KP_7;
		const unsigned int keyPAD8 = XK_KP_8;
		const unsigned int keyPAD9 = XK_KP_9;
		const unsigned int keyPADADD = XK_KP_Add;
		const unsigned int keyPADSUB = XK_KP_Subtract;
		const unsigned int keyPADMUL = XK_KP_Multiply;
		const unsigned int keyPADDIV = XK_KP_Divide;

#elif cimg_display==2
		// Define keycodes for Windows.
		const unsigned int keyESC = VK_ESCAPE;
		const unsigned int keyF1 = VK_F1;
		const unsigned int keyF2 = VK_F2;
		const unsigned int keyF3 = VK_F3;
		const unsigned int keyF4 = VK_F4;
		const unsigned int keyF5 = VK_F5;
		const unsigned int keyF6 = VK_F6;
		const unsigned int keyF7 = VK_F7;
		const unsigned int keyF8 = VK_F8;
		const unsigned int keyF9 = VK_F9;
		const unsigned int keyF10 = VK_F10;
		const unsigned int keyF11 = VK_F11;
		const unsigned int keyF12 = VK_F12;
		const unsigned int keyPAUSE = VK_PAUSE;
		const unsigned int key1 = '1';
		const unsigned int key2 = '2';
		const unsigned int key3 = '3';
		const unsigned int key4 = '4';
		const unsigned int key5 = '5';
		const unsigned int key6 = '6';
		const unsigned int key7 = '7';
		const unsigned int key8 = '8';
		const unsigned int key9 = '9';
		const unsigned int key0 = '0';
		const unsigned int keyBACKSPACE = VK_BACK;
		const unsigned int keyINSERT = VK_INSERT;
		const unsigned int keyHOME = VK_HOME;
		const unsigned int keyPAGEUP = VK_PRIOR;
		const unsigned int keyTAB = VK_TAB;
		const unsigned int keyQ = 'Q';
		const unsigned int keyW = 'W';
		const unsigned int keyE = 'E';
		const unsigned int keyR = 'R';
		const unsigned int keyT = 'T';
		const unsigned int keyY = 'Y';
		const unsigned int keyU = 'U';
		const unsigned int keyI = 'I';
		const unsigned int keyO = 'O';
		const unsigned int keyP = 'P';
		const unsigned int keyDELETE = VK_DELETE;
		const unsigned int keyEND = VK_END;
		const unsigned int keyPAGEDOWN = VK_NEXT;
		const unsigned int keyCAPSLOCK = VK_CAPITAL;
		const unsigned int keyA = 'A';
		const unsigned int keyS = 'S';
		const unsigned int keyD = 'D';
		const unsigned int keyF = 'F';
		const unsigned int keyG = 'G';
		const unsigned int keyH = 'H';
		const unsigned int keyJ = 'J';
		const unsigned int keyK = 'K';
		const unsigned int keyL = 'L';
		const unsigned int keyENTER = VK_RETURN;
		const unsigned int keySHIFTLEFT = VK_SHIFT;
		const unsigned int keyZ = 'Z';
		const unsigned int keyX = 'X';
		const unsigned int keyC = 'C';
		const unsigned int keyV = 'V';
		const unsigned int keyB = 'B';
		const unsigned int keyN = 'N';
		const unsigned int keyM = 'M';
		const unsigned int keySHIFTRIGHT = VK_SHIFT;
		const unsigned int keyARROWUP = VK_UP;
		const unsigned int keyCTRLLEFT = VK_CONTROL;
		const unsigned int keyAPPLEFT = VK_LWIN;
		const unsigned int keyALT = VK_LMENU;
		const unsigned int keySPACE = VK_SPACE;
		const unsigned int keyALTGR = VK_CONTROL;
		const unsigned int keyAPPRIGHT = VK_RWIN;
		const unsigned int keyMENU = VK_APPS;
		const unsigned int keyCTRLRIGHT = VK_CONTROL;
		const unsigned int keyARROWLEFT = VK_LEFT;
		const unsigned int keyARROWDOWN = VK_DOWN;
		const unsigned int keyARROWRIGHT = VK_RIGHT;
		const unsigned int keyPAD0 = 0x60;
		const unsigned int keyPAD1 = 0x61;
		const unsigned int keyPAD2 = 0x62;
		const unsigned int keyPAD3 = 0x63;
		const unsigned int keyPAD4 = 0x64;
		const unsigned int keyPAD5 = 0x65;
		const unsigned int keyPAD6 = 0x66;
		const unsigned int keyPAD7 = 0x67;
		const unsigned int keyPAD8 = 0x68;
		const unsigned int keyPAD9 = 0x69;
		const unsigned int keyPADADD = VK_ADD;
		const unsigned int keyPADSUB = VK_SUBTRACT;
		const unsigned int keyPADMUL = VK_MULTIPLY;
		const unsigned int keyPADDIV = VK_DIVIDE;

#else
		// Define random keycodes when no display is available.
		// (should rarely be used then!).
		const unsigned int keyESC = 1U;   //!< Keycode for the \c ESC key (architecture-dependent).
		const unsigned int keyF1 = 2U;   //!< Keycode for the \c F1 key (architecture-dependent).
		const unsigned int keyF2 = 3U;   //!< Keycode for the \c F2 key (architecture-dependent).
		const unsigned int keyF3 = 4U;   //!< Keycode for the \c F3 key (architecture-dependent).
		const unsigned int keyF4 = 5U;   //!< Keycode for the \c F4 key (architecture-dependent).
		const unsigned int keyF5 = 6U;   //!< Keycode for the \c F5 key (architecture-dependent).
		const unsigned int keyF6 = 7U;   //!< Keycode for the \c F6 key (architecture-dependent).
		const unsigned int keyF7 = 8U;   //!< Keycode for the \c F7 key (architecture-dependent).
		const unsigned int keyF8 = 9U;   //!< Keycode for the \c F8 key (architecture-dependent).
		const unsigned int keyF9 = 10U;  //!< Keycode for the \c F9 key (architecture-dependent).
		const unsigned int keyF10 = 11U;  //!< Keycode for the \c F10 key (architecture-dependent).
		const unsigned int keyF11 = 12U;  //!< Keycode for the \c F11 key (architecture-dependent).
		const unsigned int keyF12 = 13U;  //!< Keycode for the \c F12 key (architecture-dependent).
		const unsigned int keyPAUSE = 14U;  //!< Keycode for the \c PAUSE key (architecture-dependent).
		const unsigned int key1 = 15U;  //!< Keycode for the \c 1 key (architecture-dependent).
		const unsigned int key2 = 16U;  //!< Keycode for the \c 2 key (architecture-dependent).
		const unsigned int key3 = 17U;  //!< Keycode for the \c 3 key (architecture-dependent).
		const unsigned int key4 = 18U;  //!< Keycode for the \c 4 key (architecture-dependent).
		const unsigned int key5 = 19U;  //!< Keycode for the \c 5 key (architecture-dependent).
		const unsigned int key6 = 20U;  //!< Keycode for the \c 6 key (architecture-dependent).
		const unsigned int key7 = 21U;  //!< Keycode for the \c 7 key (architecture-dependent).
		const unsigned int key8 = 22U;  //!< Keycode for the \c 8 key (architecture-dependent).
		const unsigned int key9 = 23U;  //!< Keycode for the \c 9 key (architecture-dependent).
		const unsigned int key0 = 24U;  //!< Keycode for the \c 0 key (architecture-dependent).
		const unsigned int keyBACKSPACE = 25U;  //!< Keycode for the \c BACKSPACE key (architecture-dependent).
		const unsigned int keyINSERT = 26U;  //!< Keycode for the \c INSERT key (architecture-dependent).
		const unsigned int keyHOME = 27U;  //!< Keycode for the \c HOME key (architecture-dependent).
		const unsigned int keyPAGEUP = 28U;  //!< Keycode for the \c PAGEUP key (architecture-dependent).
		const unsigned int keyTAB = 29U;  //!< Keycode for the \c TAB key (architecture-dependent).
		const unsigned int keyQ = 30U;  //!< Keycode for the \c Q key (architecture-dependent).
		const unsigned int keyW = 31U;  //!< Keycode for the \c W key (architecture-dependent).
		const unsigned int keyE = 32U;  //!< Keycode for the \c E key (architecture-dependent).
		const unsigned int keyR = 33U;  //!< Keycode for the \c R key (architecture-dependent).
		const unsigned int keyT = 34U;  //!< Keycode for the \c T key (architecture-dependent).
		const unsigned int keyY = 35U;  //!< Keycode for the \c Y key (architecture-dependent).
		const unsigned int keyU = 36U;  //!< Keycode for the \c U key (architecture-dependent).
		const unsigned int keyI = 37U;  //!< Keycode for the \c I key (architecture-dependent).
		const unsigned int keyO = 38U;  //!< Keycode for the \c O key (architecture-dependent).
		const unsigned int keyP = 39U;  //!< Keycode for the \c P key (architecture-dependent).
		const unsigned int keyDELETE = 40U;  //!< Keycode for the \c DELETE key (architecture-dependent).
		const unsigned int keyEND = 41U;  //!< Keycode for the \c END key (architecture-dependent).
		const unsigned int keyPAGEDOWN = 42U;  //!< Keycode for the \c PAGEDOWN key (architecture-dependent).
		const unsigned int keyCAPSLOCK = 43U;  //!< Keycode for the \c CAPSLOCK key (architecture-dependent).
		const unsigned int keyA = 44U;  //!< Keycode for the \c A key (architecture-dependent).
		const unsigned int keyS = 45U;  //!< Keycode for the \c S key (architecture-dependent).
		const unsigned int keyD = 46U;  //!< Keycode for the \c D key (architecture-dependent).
		const unsigned int keyF = 47U;  //!< Keycode for the \c F key (architecture-dependent).
		const unsigned int keyG = 48U;  //!< Keycode for the \c G key (architecture-dependent).
		const unsigned int keyH = 49U;  //!< Keycode for the \c H key (architecture-dependent).
		const unsigned int keyJ = 50U;  //!< Keycode for the \c J key (architecture-dependent).
		const unsigned int keyK = 51U;  //!< Keycode for the \c K key (architecture-dependent).
		const unsigned int keyL = 52U;  //!< Keycode for the \c L key (architecture-dependent).
		const unsigned int keyENTER = 53U;  //!< Keycode for the \c ENTER key (architecture-dependent).
		const unsigned int keySHIFTLEFT = 54U;  //!< Keycode for the \c SHIFTLEFT key (architecture-dependent).
		const unsigned int keyZ = 55U;  //!< Keycode for the \c Z key (architecture-dependent).
		const unsigned int keyX = 56U;  //!< Keycode for the \c X key (architecture-dependent).
		const unsigned int keyC = 57U;  //!< Keycode for the \c C key (architecture-dependent).
		const unsigned int keyV = 58U;  //!< Keycode for the \c V key (architecture-dependent).
		const unsigned int keyB = 59U;  //!< Keycode for the \c B key (architecture-dependent).
		const unsigned int keyN = 60U;  //!< Keycode for the \c N key (architecture-dependent).
		const unsigned int keyM = 61U;  //!< Keycode for the \c M key (architecture-dependent).
		const unsigned int keySHIFTRIGHT = 62U;  //!< Keycode for the \c SHIFTRIGHT key (architecture-dependent).
		const unsigned int keyARROWUP = 63U;  //!< Keycode for the \c ARROWUP key (architecture-dependent).
		const unsigned int keyCTRLLEFT = 64U;  //!< Keycode for the \c CTRLLEFT key (architecture-dependent).
		const unsigned int keyAPPLEFT = 65U;  //!< Keycode for the \c APPLEFT key (architecture-dependent).
		const unsigned int keyALT = 66U;  //!< Keycode for the \c ALT key (architecture-dependent).
		const unsigned int keySPACE = 67U;  //!< Keycode for the \c SPACE key (architecture-dependent).
		const unsigned int keyALTGR = 68U;  //!< Keycode for the \c ALTGR key (architecture-dependent).
		const unsigned int keyAPPRIGHT = 69U;  //!< Keycode for the \c APPRIGHT key (architecture-dependent).
		const unsigned int keyMENU = 70U;  //!< Keycode for the \c MENU key (architecture-dependent).
		const unsigned int keyCTRLRIGHT = 71U;  //!< Keycode for the \c CTRLRIGHT key (architecture-dependent).
		const unsigned int keyARROWLEFT = 72U;  //!< Keycode for the \c ARROWLEFT key (architecture-dependent).
		const unsigned int keyARROWDOWN = 73U;  //!< Keycode for the \c ARROWDOWN key (architecture-dependent).
		const unsigned int keyARROWRIGHT = 74U;  //!< Keycode for the \c ARROWRIGHT key (architecture-dependent).
		const unsigned int keyPAD0 = 75U;  //!< Keycode for the \c PAD0 key (architecture-dependent).
		const unsigned int keyPAD1 = 76U;  //!< Keycode for the \c PAD1 key (architecture-dependent).
		const unsigned int keyPAD2 = 77U;  //!< Keycode for the \c PAD2 key (architecture-dependent).
		const unsigned int keyPAD3 = 78U;  //!< Keycode for the \c PAD3 key (architecture-dependent).
		const unsigned int keyPAD4 = 79U;  //!< Keycode for the \c PAD4 key (architecture-dependent).
		const unsigned int keyPAD5 = 80U;  //!< Keycode for the \c PAD5 key (architecture-dependent).
		const unsigned int keyPAD6 = 81U;  //!< Keycode for the \c PAD6 key (architecture-dependent).
		const unsigned int keyPAD7 = 82U;  //!< Keycode for the \c PAD7 key (architecture-dependent).
		const unsigned int keyPAD8 = 83U;  //!< Keycode for the \c PAD8 key (architecture-dependent).
		const unsigned int keyPAD9 = 84U;  //!< Keycode for the \c PAD9 key (architecture-dependent).
		const unsigned int keyPADADD = 85U;  //!< Keycode for the \c PADADD key (architecture-dependent).
		const unsigned int keyPADSUB = 86U;  //!< Keycode for the \c PADSUB key (architecture-dependent).
		const unsigned int keyPADMUL = 87U;  //!< Keycode for the \c PADMUL key (architecture-dependent).
		const unsigned int keyPADDIV = 88U;  //!< Keycode for the \c PADDDIV key (architecture-dependent).
#endif

		const double PI = 3.14159265358979323846;   //!< Value of the mathematical constant PI

													// Define a 12x13 font (small size).
		static const char *const data_font12x13 =
			"                          .wjwlwmyuw>wjwkwbwjwkwRxuwmwjwkwmyuwJwjwlx`w      Fw                      "
			"   mwlwlwuwnwuynwuwmyTwlwkwuwmwuwnwlwkwuwmwuw_wuxlwlwkwuwnwuynwuwTwlwlwtwnwtwnw my     Qw   +wlw   b"
			"{ \\w  Wx`xTw_w[wbxawSwkw  nynwky<x1w `y    ,w  Xwuw   CxlwiwlwmyuwbwuwUwiwlwbwiwrwqw^wuwmxuwnwiwlwmy"
			"uwJwiwlw^wnwEymymymymy1w^wkxnxtxnw<| gybwkwuwjwtwowmxswnxnwkxlxkw:wlymxlymykwn{myo{nymy2ykwqwqwm{myo"
			"zn{o{mzpwrwpwkwkwswowkwqwqxswnyozlyozmzp}pwrwqwqwqwswswsxsxqwqwp}qwlwiwjybw`w[wcw_wkwkwkwkw mw\"wlwiw"
			"=wtw`xIw awuwlwm{o{mylwn|pwtwtwoy`w_w_wbwiwkxcwqwpwkznwuwjzpyGzqymyaxlylw_zWxkxaxrwqxrwqyswowkwkwkwk"
			"wkwkwk}qyo{o{o{o{owkwkwkwkznxswnymymymymyayuwqwrwpwrwpwrwpwrwqwqwpwkwtwlwkwlwuwnwuynwuwmyTwkwlwuwmwu"
			"wnwkwlwuwmwuwkxlwuxmwkwlwuwnwuynwuwTwkwlwuwmwuwlwmwkwtwUwuwuwowswowswowswowsw;wqwtw_ymzp~py>w bwswcw"
			"kwuwjwuwozpwtwuwnwtwowkwjwmwuwuwkwIxmxuxowuwmwswowswmxnwjwhwowswowsw0wmwowswuwnwrwowswpwswowkwjwrwqw"
			"rwpwkwkwtwnwkxsxqxswowswpwswnwswpwswowrwnwmwrwqwqwqwswswrwswowswjwpwlxjwkxuxLw[wcw_wSwkw mw\"wlwiw=wt"
			"wmxlwFw cwswnwuwnwkwjwswo{pwrwpwtwtwpwswby`w`yUwlwtwpwqwpwswowlw\\wrwrxuwHwrwfwuwjwlwlwTyuwVwlwtwawsw"
			"owswowswcwuwmwuwmwuwmwuwmwuwlwkwuwnwswpwkwkwkwkwkwkwkwkwswoxswowswowswowswowswowswowrwpwswpwrwpwrwpw"
			"rwpwrwpwswoznwtw  Ww (wGwtwtwqwqwqwuwuwuwqwswuwqwqw=wqxtw`{nzp~q{ozowrwnxmwtwow bzawkwuwl}rwuwnwtwuw"
			"nwtwowkwjwlyjwIwlwswmwiwkwnwuwnwkwhwnwswowswowkwewewixnwsytwswuwnwrwpwkwrwpwkwkwkwrwpwkwkwuwmwkxsxqw"
			"uwtwpwqwqwswowqwqwswowiwmwrwpwswowtwtwpwuwmwuwjwowkwjwlxsxXynzmymznyozlzoznwkwkwtwnwkzuyrzmynzmzowux"
			"myozmwswpwrwowtwtwrwrwpwrwp{mwlwiwHyuwpwtwkwmxlynzoxswmwmwswnwswowtxq|owtwtwpym{p{owswnwuwmwlwkwqwqx"
			"uwuxqwrwpwtwtwqwqwowlwuwuwkwmwlwtwowuwuwdwjznwl{nwuwnwkx_wtxtwswtwlwtwWwuytwgyjwmwjwawswoyuwVwlwtwnw"
			"twmwtwnwtwmwuwmwlwuwmwuwmwuwmwuwmwuwmwuwmxuwowkwkwkwkwkwkwkwkwkwrwpwuwtwpwqwqwqwqwqwqwqwqwqwowtwpwsw"
			"uwqwrwpwrwpwrwpwrwowuwnwswowuwlymymymymymymyuyqymymymymynwkwkwkwjynzmymymymymykwmzowswowswowswowswpw"
			"rwozowrwW}q}qwtwtwqwtwtwqwtwtwA}rwuw_{p~r~r}pwtwowrwnxmwtwow aw_w]wtwpwuwmxuwmybwjwlyjwIwlwswmwiwnyn"
			"wtwnznzkwmynwswTyp}pylwmwtwtwtwswuwn{owkwrwp{o{owk|pwkwkxlwkwuwuwuwqwuwtwpwqwqwswowqwqwswoykwmwrwpws"
			"wowuwuwuwowkwjwnwkwjwDwowswowkwswowswowkwswowswowkwkwuwmwkwswswswswowswowswowswoxlwswowkwswpwrwowtwt"
			"wqwtwowrwlwoxkwhxVxuxpwtypwuwjwnwtwnwkwswowtxnxmwswowqwqwtwuxqwtwnwtwtwqwswowswmwm{nwuwlxnwkwqwqwtwt"
			"wqwrwpwtwtwqwuyuwpwiwhwnwmwrwnwbwkwuwlwlwswoxuxowlwtw`wuwrwszmwtwo}dwuwtwuw[}qymx`wswoyuwow_ylxlwtwo"
			"yuwoyuwoyuwmwlwuwmwuwmwuwmwuwmwuwmwuwmwt{swk{o{o{o{owkwkwkwlztwpwuwtwpwqwqwqwqwqwqwqwqwqwnxowtwtwqwr"
			"wpwrwpwrwpwrwnwmwswowuwiwkwkwkwkwkwkwswswkwswowswowswowswowkwkwkwkwswowswowswowswowswowswowswcwtxows"
			"wowswowswowswpwrwowswpwrwWwtwtwqwqwqwuwuwuwqwuwswqwqw>wowuw`}q~q|q}qwrwpwrwowtwnwtwo~ izaw]wtwoykwux"
			"qwtwswfwjwmwuwuwn}eyaxlwswmwjwjwpwswjwowswmwmwswnzWy]ypwlwtwtwuwswswowrwpwkwrwpwkwkwsyqwrwpwkwkwuwmw"
			"kwuwuwuwqwtwuwpwqwqznwqwqzkynwmwrwowuwnwuwuwuwowkwjwnwkxkwGzowswowkwswo{owkwswowswowkwkxlwkwswswswsw"
			"owswowswowswowjxmwkwswowtwnwuwuwuwpxmwtwlwlwlwiwlytwewtwtwqwswowtxoznwswnxmwswnwuwmwuwnwswowtwtwqwtw"
			"twqwtwnwtwtwqwswowswmwmwswowswmwmwkwqwqwtwtwqwrwowuwuwpwuyuwq~own~own~owbwkwuwmznwswmwbwswawuwrwgwtw"
			"hwdwuytwXwJwswnxuw=wtwmwswowtxowswqxmwswowswowswowswowswowswnwtwowkwkwkwkwkwkwkwkwkwrwpwtwuwpwqwqwqw"
			"qwqwqwqwqwqwnxowtwtwqwrwpwrwpwrwpwrwnwmwswowtwmznznznznznzn~swk{o{o{o{owkwkwkwkwswowswowswowswowswow"
			"swowswo}qwuwuwowswowswowswowswowtwnwswowtwUwuwuwowswowswowswowsw@}qx`}q~pzo{pwrwpwrwowtwnwtwow aw_w_"
			"}owuwmwuwtwrwswuwewjwkwiwJwkwswmwkwiwp|kwowswmwmwswkwWym}mypwlwszr{owrwpwkwrwpwkwkwqwqwrwpwkwkwtwnwk"
			"wtwtwqwtwuwpwqwqwkwqwqwtwiwnwmwrwowuwnwuwuwuwpwuwlwkwmwjwkwHwswowswowkwswowkwkwswowswowkwkwuwmwkwsws"
			"wswswowswowswowswowhwnwkwswowtwnwuwuwuwpxmwtwmwkwlwiwmwtydwtwtwqwswowswowtwnwswowkwswnwuwnwtwnwswowt"
			"wtwqwtwtwqwtwnwtwtwqwswowswmwmwswowswnwlwkwqwqxuwuxqwrwnyowqwpwiwhwpwuwuwowrwpwuwuwdwkwuwlwlwswo{owk"
			"xuwawtxtwszmwtwiwdwuwtwuwXwJwswmwuwKzmwtwlwtxowrwpwtxrxl{o{o{o{o{o{o{owkwkwkwkwkwkwkwkwkwrwpwtwuwpwq"
			"wqwqwqwqwqwqwqwqwowtwpwuwswqwrwpwrwpwrwpwrwnwmznwswowswowswowswowswowswowswowswowkwkwkwkwkwkwkwkwkws"
			"wowswowswowswowswowswowswcwuwuwowswowswowswowswowtwnwswowtwTymymymymy=wmw^wuwuwmxlxmyowrwowtwnwtwmxm"
			"w bwswIwuwmwuwmwuwtwrxswdwjw]wJwkxuxmwlwlwswlwjwowswmwmwswlwSycyawlwswowrwowswpwswowkwjwrwqwrwpwkwkw"
			"swowkwqwqwsxowswpwjwswpwswowrwnwmxtxnwlwswpwswmwlwlwjwkwHwswowswowkwswowswowkwswowswowkwkwtwnwkwswsw"
			"swswowswowswowswowkwswowkwswnxlwswpwtwmxmwjwlwiwTxuxpwtxowswowtwnwswowkwswnynwtwnwswowtwtwqxuwuxqwtw"
			"nwtwtwqwswowswmwlwuwnwswowkwjwswo{pwrwmwmwswnwjwiwnymwtwnycwkwuwlwl{mwmwiw_wrwdwtwVwrw*wswmwuw?wtwlw"
			"tzqwrwpwtzswkwswowswowswowswowswowswowswnwswpwkwkwkwkwkwkwkwkwswowsxowswowswowswowswowswowrwpwswpxtx"
			"pxtxpxtxpxtxnwmwkwswowswowswowswowswowswowswowtxowkwswowswowswowswowkwkwkwkwswowswowswowswowswowswow"
			"swlwnxtwowswowswowswowswnxmwswnx >wlw\\wkx`wnwrwoznwtwmxl| gybw^wtwozmwsxpzuxfxlx]wnw_wlxjyn{o{nykwnz"
			"mymwkynymwkwewewjwjwrwswqwp{myozn{owizpwrwpwkwkwrwp{owqwqwsxnyowiyowrwozmwlzmwlwswqxsxnwm}qwjxlwGzoz"
			"mymznynwjzowswowkwkwswowkwswswswswnynzmzowjymxlznxlwswqwrwnwm{mwlwiwHxuxpzmxlymynwswmwnwrwozmxuxo{pw"
			"txn{pzmykwmyo}p{owkyuynwnwrwmwly`w_w_wbwjzo{pwqwnwmwhw_z>zY}M|nwuw2wqwqwryrwqwqyowqwqwqwqwqwqwqwqwqw"
			"qwqwqwr{qyo{o{o{o{owkwkwkwkznwsxnymymymymycwuynznznznzmwmwkwuynznznznznznznyuzrymymymymynwkwkwkwjynw"
			"swnymymymymybzmznznznznwlzmw     hwHwlwSwTw <w8z ]x tx Zxjwmx RwWw/wgw pw_ynwky=wCwmwaw\\w_wnw  1wIwl"
			"z 'wiwuwaw  mw    Pw   swlwjw     hw        f| pyWx/wgw rxSw/wCwmwaw\\w_wnw  1w  AwRx  nw    Pw   txk"
			"wlxm";

		// Define a 20x23 font (normal size).
		static const char *const data_font20x23 =
			"                                                9q\\q^r_rnp`qnq`plp7q\\q^q_qmqbq\\q^q_qmqHqmp_q\\q^r_rnp"
			"`qnq7q\\q^q_qmq_q \"r                                                       Mq^q^qnq`pnr`qnq`plp6q^q^p"
			"mp`qmqaq^q^pmp`qmqIpmq]q^q^qnq`pnr`qnq6q^q^pmp`qmq`q \"plp         'q     5qmq               Vq      "
			"               Xq    [plp      3qYq_p^rnpLplp8qYq_qNqYq_q4rmpaqYq_q_rmp%qYq^pGq  Irc|!pKp]raqjq`p   "
			"HtNq_qmq\\plqbp_shpdscq[q^q[p [q]s_r`uau]rbv`tcxbuat LsZucrav_udwcxdw`udqiqeq]q]qjreq]sksgrjqbtcv_tcv"
			"aud{eqiqgqfqgqjsjqlrjrhrirfzfs`q[sZqMqJqCqNsLq]q]q]q]q   .scq]s \\sKt%r  [s^raxdxat_qazgqlqlqctJqIqIq"
			"LqHsOqiqOtaqmq\\uft nufu`sLs`t\\qKv<r\\rLrepirepitgpeq]r^r^r^r^r^r^{gudxdxdxdxdq]q]q]q]wcrjqbt`t`t`t`tL"
			"tlpgqiqeqiqeqiqeqiqgrireq[s_q[q_pnp_pnr`qnq`plp7q[q_s`qmqcq[q_s`qmq]pkpbpmr`q[q_s`pmraqmq8q[q^pnp_qn"
			"q^qaq\\qnq !pnqd{!pJp^tdunucr _y  dvOq_qmq\\plpap_pmpipdudq[p\\p_plplp _q^ubtawcw^rbvavdxcwcw Ou]yerawb"
			"xeyexdwbxeqiqeq]q]qkrdq]sksgrjqdxewbxewcwe{eqiqfqhqfqjsjqkqjqfqiqezfs`q[s[sMpJqCqOtLq]q]q]q]q  q 1tc"
			"q]t ^vaq_w&r  \\u_raxdxcxcuczgqlqlqexMsJqJsMq[p^uPqiqdq]uaqmq]qkqcq!qkqguaqmqNpkp\\p]pKtmp:p]plpKpfpfp"
			"fpcpipdq]r^r^r^r^r^r^{ixexdxdxdxdq]q]q]q]yerjqdxdxdxdxdxPwnpfqiqeqiqeqiqeqiqfqiqdq\\u_p[p^pnpKqnq_r5p"
			"[p^pmp`qmqbp[p^pmp`qmq]tKp[p^pmpLqmq7p[p]pnp_qnq^p`q\\qnq5uauauauaucq`qhq4p]pKr_ueunucr `q  \\rkpOq_qm"
			"q\\plpctbqmqkqerlpdq\\q\\q_qnpnq\\q%q^qkqcqnqapjrdpjr`sbq]rkp^qcrkrerkq Oplr`sirgtbqkrdripeqjsfq]q]ripeq"
			"iqeq]q]qlrcq]sksgskqerjrfqkrdrjrfqkrerjp`q`qiqfqhqeqkskqiqlqdqkq\\qeq]qZq\\qmqNqKqCqOqIq5q]q  q 1q`qZq"
			" _rlqbtaqjp$q  ^qkqatbr^q]rjrewdqhqgqlqlqfrjrOuKqKu8p_rlpOqkqcq]qFpgpcp\"pgpTpkp\\q^p\\p^qLump:p^pjpLpg"
			"pepgpbpjpPt`t`t`t`t`qnq_qnqcripeq]q]q]q]q]q]q]q]qjsfskqerjrfrjrfrjrfrjrfrjrRrjrfqiqeqiqeqiqeqiqeqkqc"
			"vbrlq`q]q_plp Iq]q_qmqNq]q_qmqKtIq]q_qmq ^q]q^plpKq`q mqkqcqkqcqkqcqkqcqkqdq`qhq5q^qLt`ueunudtasbqip"
			"`q`pipcq  [qIq_qmq`{gvcqmqkpdq_q\\q\\q]rZq%q_rkraqZq]qaqnqbq]qXqcqiqeqiq1pSpXq`qfrhqnqbqjqdq]qhqfq]q]q"
			"]qiqeq]q]qmrbq]qnqmqnqgskqeqhqfqjqdqhqfqjqeqYq`qiqfrjreqkskqirnrdrmr]qdq]qZq]qkq)qCqOqIq5q]q  q 1q`q"
			"Zq _qkq_qaq mq  ^qkqaqnqar_q]qhqfrnqnreqhqgqlqlqfqhqPwLqLw9p_q_phqdqkqcq]qGplslpiu#pmtlpUpkp\\q_q_r8u"
			"mp:p^pjpLpgpepgperipcq^qnq`qnq`qnq`qnq`qnq`qnq`qmqcq]q]q]q]q]q]q]q]q]qhqfskqeqhqfqhqfqhqfqhqfqhqdphp"
			"fqirfqiqeqiqeqiqeqiqermrcwcqkq    [q 3qZp Oq nqmqmqeqiqeqiqeqiqeqiqeq_piq4q^pLvatd|evdvcqipasaqkqdq "
			" [qHq_qmq`{hrnpmpcqmqlpcq_q\\pZp]rZq%q_qiqaqZq]qapmqbq^qWqcqiqeqiqdq]qUsSs[qaqdqhqnqbqjqeq\\qgqgq]q^q\\"
			"qiqeq]q]qnraq]qnqmqnqgqnqlqfqfqgqjqeqfqgqjqeqYq`qiqeqjqdqlqmqlqhqnqbqmq]rdq]qZq^pgp=taqns`s`snqatdv_"
			"snqcqnsbq]q]qkqcq]qnsmshqns`saqnsasnqcqnr`tbvaqjqeqiqdqkqkqjrkreqiqdw`q`qZq#tnreqkq^qatauaqnsdqiq`ra"
			"qjqdqiqdpmrcxdqmqmqatbxfyeqiqbqnq`r`q^qfqhrmqmrfqhqgqlqlqgqfqep[pnqnp[p`q`pipbpnqnpNq]taq^qnqnqbqmqb"
			"q\\qIqmpkpmqkqkp$qmpkpmqVqmq\\q`q[pLqjqeump:p^pjpLphpdphpapkpbq^qnq`qnq`qnq`qnq`qnq`qnq`qmqdq\\q]q]q]q]"
			"q]q]q]q]qgqgqnqlqfqfqhqfqhqfqhqfqhqfqfrjrhqiqnqgqiqeqiqeqiqeqiqdqmqbqkrdqmsbt`t`t`t`t`t`tlsfs_t`t`t`"
			"tbq]q]q]q[tbqns`s_s_s_s_s\\q`smpdqjqdqjqdqjqdqjqeqiqdqnscqiq;qlqlqgqgqgqnqmqnqgqjqnqgqgqfq_qjq<{fpjpL"
			"vatd|fxeqkqdqipasaqkqdp  \\yNqGplqeqmp`qmqmqcrLqZq`qnpnq\\q%q_qiqaqZq^rbqmqbubqms^qaqkqdqiqdq]qXuf{fu_"
			"q`qlrnqlqjqlqcqkreq\\qgqgq]q^q\\qiqeq]q]t`q]qnqmqnqgqnqlqfqfqgqkreqfqgqkres[q`qiqeqjqdqlqmqlqhs`s]rcq]"
			"qZq#vbwcvbwcwev`wcwcq]q]qlqbq]vnthwcwcwcwcubwcvaqjqdqkqcqkqkqiqkqdqiqdw`q`qZq7smsfxdqlr^qavdvawdqkq_"
			"raqjqdpgpeqntdxdqmqmqcwdyfyeqiqcqlq`raq^qfqhqlqlqfqhqgqlqlqgqfqfrZqZraqarkraqLq^vbq^wbqmqbq]tKpmpfpk"
			"pjp_plp9plpkplpUs[qaqZpLqjqeump:p^pjpaplp_piqdpiqaplqbq_qlqbqlqbqlqbqlqbqlqbqlqbrmqdq\\q]q]q]q]q]q]q]"
			"q]qgqgqnqlqfqfqhqfqhqfqhqfqhqfqerlrgqjqmqgqiqeqiqeqiqeqiqcsaqjqdqnq`vbvbvbvbvbvbvnuivbwcwcwcwcq]q]q]"
			"q]wcwcwcwcwcwcwOwcqjqdqjqdqjqdqjqeqiqdwdqiq;pkqkpgpepgpmumpgpjrmpgpepfq_qkq;{hrkpLxdxf|fxepipdqipas`"
			"pkpcp  ZqHqGplpdt_pmplpmshsMqZqaplplp]q&q^qiqaq[qat`plqbvcx_q`ucrkr:uc{cucq`qlvlqjqlqcwdq\\qgqgxdvcqj"
			"tfyeq]q]s_q]qmsmqgqmqmqfqfqgwdqfqgwcv_q`qiqdqlqbqmqmqmqfr`s]qbq\\q[q#pjqcrlrdqkpcrlrcqkrdq^rlrcrlrdq]"
			"q]qmqaq]rlrlqirlrdqkqcrlrerlrcr_qjpbq]qjqdqkqcqlslqhqmqbqkq^q_q`qZq_tjpSqmsmpgrlsdqnsaqmqbqkqdq\\rlrd"
			"qlq_raqjqeqgqgrnqnrdqlqcqmqmqcqkqerkq`qaycqlq_rbq^qfqhqlqlqfqhqgqlqlqgqnvnqgrYqYrbqbrirbqLq_rnpmpdwa"
			"qmqcydq^qlqLpmpfpkpkq`plpa{RpltkpB{gpXpLqjqdtmpcqHp]plp_plp`pipjpipipmsfplpjphr_qlqbqlqbqlqbqlqbqlqb"
			"qlqbqlxkq\\xdxdxdxdq]q]q]q_vjqgqmqmqfqfqhqfqhqfqhqfqhqfqdrnrfqkqlqgqiqeqiqeqiqeqiqcsaqjqdqnq`pjqcpjqc"
			"pjqcpjqcpjqcpjqcpjrlrjqkpbqkrdqkrdqkrdqkrdq]q]q]q]qkrdrlrdqkqcqkqcqkqcqkqcqkqOqkqcqjqdqjqdqjqdqjqdqk"
			"qcrlrdqkq:pnwnpgpnwnpgplslpgpkrlpgpkqkpfq^qlq6qaqlpMzfzfzfzgqipdqipbqmp`qmqc|  fqHqHqlpcuasmplpmpiul"
			"qSqZq]p^{+q^qiqaq\\q`ubqlqbpkrdrkrarawcx<tEteq`qlqlqlqjqlqcwdq\\qgqgxdvcqjtfyeq]q]t`q]qmsmqgqmqmqfqfqg"
			"vcqfqgv_t`q`qiqdqlqbqmqmqmqgs_q]qaq\\q[q\"vcqjqeq]qjqdqiqdq^qjqcqjqdq]q]qnq`q]qkqkqiqjqeqiqdqjqeqjqcq^"
			"s^q]qjqdqkqbqmsmqgqmqbqkq_qas_qYsc{Spkqkphqkrcqntcvcqiqeq\\qjqdqmr`tbqjqeqgqgqmqmqdqlqcqmqmqdqiqfqiqa"
			"qaycqlq_qaq^qfqhqlqlqfqhqfqmqmqfqnvnqh}cqc}cqc}cqLq_qmpawbqkqasaq^qkqMpmpfpjsnpaplp`{RplpmqkpB{huatK"
			"qjqbrmpcqJt^r]plpctlpjqktlpmpkpltlpjqhq^qlqbqlqbqlqbqlqbqlqcrlrcqlxkq\\xdxdxdxdq]q]q]q_vjqgqmqmqfqfqh"
			"qfqhqfqhqfqhqfqcteqlqkqgqiqeqiqeqiqeqiqbq`qkrdqmravbvbvbvbvbvbvjqkq]qiqeqiqeqiqeqiqdq]q]q]q^qiqdqjqe"
			"qiqeqiqeqiqeqiqeqiqd{hqkpnqdqjqdqjqdqjqdqjqdqkqcqjqdqkq:pnwnpgpnwnpgplslpgplrkpgpkqkpfq^qlq6qaqmqMzg"
			"|fxdxfqipdqipbqmqaqmqcp  \\wLqK{dt]qmqmqkrmrnrSqZqK{TtKq^qiqaq]r\\rdqkq\\qdqiqaqarkrcsmq<tEtfq_qlqlqlqk"
			"qjqdqjqeq\\qgqgq]q^qgqfqiqeq]q]qnraq]qmsmqgqlqnqfqfqgq^qfqgqkq]raq`qiqdqlqbqnqkqnqgt`q^raq\\q[q#wcqjqe"
			"q]qjqdydq^qjqcqjqdq]q]s_q]qkqkqiqjqeqiqdqjqeqjqcq]uaq]qjqcqmqaqmpmpmqfs`qmq_ras_qYscpjtRpkqkphqkrcqk"
			"reqlrcqiqcr_qjqdqmq_qnqbqjqeqlqlqgqmqmqdqlqcqmqmqdqiqfqiqaqaqiqdqjqaq`q^qfqhqlqlqfqhqfrnqnrfqfqh}cqc"
			"}cqc}cqLq_qmp_q^qkq`qMrlqMpmpfpWplpUqRplplqlp=q&qjq`pmp _plp]qkpnpdqhpeqkpnpiq^qjqdqjqdqjqdqjqdqjqdq"
			"jqdqkqdq\\q]q]q]q]q]q]q]q]qgqgqlqnqfqfqhqfqhqfqhqfqhqfqbrdqmqjqgqiqeqiqeqiqeqiqbq`wcqlrcwcwcwcwcwcwc~"
			"kq]yeyeyeydq]q]q]q^qiqdqjqeqiqeqiqeqiqeqiqeqiqd{hqlpmqdqjqdqjqdqjqdqjqcqmqbqjqcqmq9pkqkpgpepgpmumpgp"
			"mrjpgpepfq]pmq:{epmpLzg|evbveqipdqipbqmqaqmpbq  [qHqK{cpmq^plqmqkqktRqZqFqOtKq^qiqaq^rZqdy^qdqiqaqaq"
			"iq]q:uc{cudq_qlqlqmqjxdqiqfq\\qgqgq]q^qgqfqiqeq]q]qmrbq]qlqlqgqlqnqfqfqgq^qfqgqkr]qaq`qiqcqnqaqnqkqnq"
			"hrnq`q_r`q\\q[q$qjqcqjqeq]qjqdydq^qjqcqjqdq]q]s_q]qkqkqiqjqeqiqdqjqeqjqcqZsbq]qjqcqmqaqnqmqnqfs`qmq`r"
			"^r`qZr9pkqkphqkrcqjqeqkqcqiqet_qjqcqnq`rnqbqjqeqlqlqgqmqmqdqlqcqmqmqdqiqfqiqaqaqiqdqjqbr`q]qhqgrmqmr"
			"fqhqeweqfqgrYqYrdpnqnpdrirdpnqnpNq_qmp_q]qmqcyPrmqMqmpkpmqkvaplpVqRqmpkpmq=q&qjq`pmp(v_plp\\pkpmpdphq"
			"epkpmpjq]xdxdxdxdxdxdwdq\\q]q]q]q]q]q]q]q]qgqgqlqnqfqfqhqfqhqfqhqfqhqfqcteqnqiqgqiqeqiqeqiqeqiqbq`vbq"
			"jqeqjqdqjqdqjqdqjqdqjqdqjqdqjxkq]yeyeyeydq]q]q]q^qiqdqjqeqiqeqiqeqiqeqiqeqiqQqmplqdqjqdqjqdqjqdqjqcq"
			"mqbqjqcqmq9qlqlqgqgqgqnqmqnqgqnqjqgqgqfq]qnq:{eqnpLzg|dt`tdqipcpipbpkp`sbq  Zq plq`pmq_pkqmqkqjrQqZq"
			"Fq'q]rkraq_rYqdy^qdqiqbq`qiq^q6uf{fuaq_qlyjzeqiqeq]qhqfq]q]qhqfqiqeq]q]qlrcq]qlqlqgqkseqhqfq]qhqfqjq"
			"]qaq`qiqcqnq`skshrmraq_q_q[q\\q$qjqcqjqeq]qjqdq\\q^qjqcqjqdq]q]qnq`q]qkqkqiqjqeqiqdqjqeqjqcqXqbq]qjqcq"
			"mqaqnqmqnqgqmq`s_q\\q`qZq7pmpnqmpgqkrcqjqeqkpbqiqeq\\qjqcs_qlqcqjqeqlqlqgqmqmqdqlqcqmqmqdqiqfqiqaq`qkq"
			"drjrdr_q]riqfrnqnreqhqducqhqerZqZrdwdrkrdwOq_qmp_q^w`q`q[sKplslpTplpWqQpmpkqnp<q&qjq`pmp aplp\\pkplpe"
			"phqepkplpjq^zfzfzfzfzfzfxcq]q]q]q]q]q]q]q]q]qhqfqkseqhqfqhqfqhqfqhqfqhqcrnreriqfqiqeqiqeqiqeqiqbq`q]"
			"qjqeqjqdqjqdqjqdqjqdqjqdqjqdqjqdq]q]q]q]q\\q]q]q]q^qiqdqjqeqiqeqiqeqiqeqiqeqiqQqnpkqdqjqdqjqdqjqdqjqb"
			"saqjqbs7qmqmqeqiqeqiqeqiqeqiqeq]qnp7q]rJrnpnresnpnsct_rcqipcqkqcqkqasaq  [rkp&plpcplpnr`qkqmqkrltRqZ"
			"qFq'q\\qkq`q`r_pjr^qcpjrcqkrbq`rkrdpkr3sSsLrlrnrhqhqeqjreripeqjsfq]q]riqfqiqeq]q]qkrdq]qgqgqkserjrfq]"
			"rjrfqjrfpiraq_qkqbt`skshqkqaq`q^q[q\\q$qkrcrlrdqkpcrlrcqipdq^rlrcqjqdq]q]qmqaq]qkqkqiqjqdqkqcrlrerlrc"
			"q^pjqbq]rlrbs_rkrfqmq`s`r\\q`qZq6qlrfrmscrlrepkqbrkqdqkpaqjqcs`rlqcrlrernsnrgrnqnrdqlqcrnqnrdrkqdqkra"
			"q`qkqdqhqer^q\\rkqdwdqhqbqarjrdpYqYpbubpipbuNq_rnpmpbq^qnqnq`q`qZqIpgpRplp7pgp;q&rlr`pmp bplp[pkufpiq"
			"dpkukrlpcqhqfqhqfqhqfqhqfqhqfqhqfqjqcripeq]q]q]q]q]q]q]q]qjsfqkserjrfrjrfrjrfrjrfrjrdrlrfrjreqkqcqkq"
			"cqkqcqkqaq`q]qnplqeqkrdqkrdqkrdqkrdqkrdqkrdqksjpjqkpbqipdqipdqipdqipdq]q]q]q]qkqcqjqdqkqcqkqcqkqcqkq"
			"cqkq^qbqkqcrlrdrlrdrlrdrlrbsarlrbs6qkqcqkqcqkqcqkqcqkqdq\\r7q\\qFp\\p]r^rcqipcvbqkqas`r  \\vOqIqlpcw_pip"
			"mpivnrRpZpEqbqIq^q[ubwdxdw]qcwbwaq_wcvbq]qRpSp[q^q^qhqexcxeyexdq\\xeqiqeq]q]qjrexdqgqgqjrdxeq\\xeqiqfx"
			"`q_war_ririqiqbqazfq[q\\q$xcwcvbwcxdq]wcqjqdq]q]qlqbq]qkqkqiqjqdwcwcwcq^wbu`wbs_rkrgqkq`q`w`q`qZq$yew"
			"dqmq`wdvaqjqbr`qkqcyeyewcqlsdwcxdw`sauczexdq^umteucqhqbq`xLqJsKsMq^vdxdpgpaq`qYqIqkq bqkq?{+yapmp Jp"
			"fpfpipcpfpiucqhqfqhqfqhqfqhqfqhqfqhqfqjxixexdxdxdxdq]q]q]q]yeqjrdxdxdxdxdxdrjrgpnwdwcwcwcwaq`q]qnuex"
			"dxdxdxdxdxdvnwjvbxdxdxdxdq]q]q]q]wcqjqdwcwcwcwcw^qbwbwcwcwcwaq`w`q4uauauauaucq\\r7p[qFp\\p\\p\\pbqipasap"
			"ip`q^y  ctNqIqmqbu_phsgslrSq\\qEqbqIq^qZsawdxcu\\qbt^taq]uataq]q q]qgpiqfqfw`udwcxdqZudqiqeq]q]qirfxdq"
			"gqgqjrbtcqZtcqirfv_q]s_r_rirjrircqazfq[q\\q#tnqcqns`s`snqaucq\\snqcqjqdq]q]qkqcq]qkqkqiqjqbsaqnsasnqcq"
			"]t_t_snqaq^rkrhrkraq`w`q`qZq#smrevbs^t`s`qjqbq`qiqdqnrmqdrmrcubqkrcubqntat^r`sc|fxdq^umtcqaqhqbq^tJq"
			"IqIqLq]tcxLq`qYqHu `u>{+qnrmqapmp Kpepgpiuhpephscqfqhqfqhqfqhqfqhqfqhqfqhqixgudxdxdxdxdq]q]q]q]wcqjr"
			"bt`t`t`t`taphpgplt`s_s_s_s_q`q]qmsctnqctnqctnqctnqctnqctnqbsktgs_uauauaucq]q]q]q[saqjqbs_s_s_s_sNpms"
			"_snqbsnqbsnqbsnqaq`qns_q !p Zp      jp#q\\q6q7q   lq [sjq  Qq -q  OqZq]q  Cq;q HqWq $rIq`qZq _q iqbqK"
			"qFqIq`q     hp$q]u   JqYpmpLp   .p        jp    ]p Xr`q[r !p       Tp\"p\\p6q6q   mq Yx  Qr -r  Ps\\q_s"
			"  Ipkq:q HqWq $qHq`qZq _q iqbqKqFqIq`q     hp$q]t   IqYpmpLq   /q        kq     Fq_q[q #s       Tp\"q"
			"^q6p   1p Vu  Rs    YsJsMy &v<s HqWq &sHtcq]t _q iqbqKqFqIq`q     hp$q   2q2q   /q        kq     Hs_"
			"q]s \"q                (r     Xy %t;r GqWq &rFscq]s ^q iqbqKqFqIq`q         ,q4r   0r        lr     G"
			"r^q                               *q                                                                "
			"                   kr               i";

		// Define a 47x53 font (extra-large size).
		static const char *const data_font47x53 =
			"                                                                                          "
			"                                                                    9])]2_2]T\\8^U^3]  E])]"
			"2`4^U^>])]2_4^U^ 6^T\\5])]1_2]T\\8^U^  K])]2`4^V^3]                                         "
			"                                                                                          "
			"                                                        U]*\\2a4`V\\8^U^5a  F]*\\1\\X\\4^U^=]*\\"
			"2a5^U^ 7aV\\4]*\\1a4`V\\8^U^  J]*\\1\\X\\4^V^3\\                                                 "
			"                                                                                          "
			"                                                S],\\1\\W\\5g8^U^6c  F],\\1\\V\\5^U^<],\\2]W]6^U^"
			" 8h3],\\0\\W\\5g8^U^  I],\\1\\V\\5^V^4\\      ;]                                                 "
			"                                                                                          "
			"                                         :\\-]2\\U\\6\\V`7^U^7]U]  F\\-]2\\T\\6^U^;\\-]3]U]7^U^ 8\\"
			"Va1\\-]1\\U\\6\\V`7^U^  H\\-]2\\T\\6^V^5]      =a                                J]              "
			"                                                                                          "
			"                                            N\\/]2\\S\\7\\T]6^U^7\\S\\  E\\/]2\\R\\7^U^:\\/]3]S]8^U^"
			" 8\\T^/\\/]1\\S\\7\\T]6^U^  G\\/]2\\R\\7^V^6]      =c                                L^           "
			"                                                         *^                            U` "
			"                                        O^             )\\S\\                     !^$^3\\  E]"
			"U\\  K^$^4^ G^$^4]   J^$^3\\   #^$^3\\ 4^            B[                                      "
			"                              &^                            Xe                            "
			"             S^             (\\S\\               )Z      Q^&^3^2]S\\ A\\S\\  K^&^3^ F^&^4_  >]S"
			"\\9^&^3^2]S\\   W^&^3^ 6^        Q]    M[               ?`   ![1^H]?` =]4](\\    %` >b4c  Bb "
			"?`2a    .a   Ib   Pb      Aa <a @b      Fb =b  F^ :] '] Da A].].].].]            <_:]._   "
			" Xh ?c   W^       @`   La   Pa        Sa   Va5^U^ @`   \"f4_ >`0`*^   $^.` <^F]F^F]G`G]    "
			" F\\S\\ ;b        %a2a2a2a2a <bR\\     D`4^(^3`4`U\\8^V^6\\S\\  J^(^3`4^U^@^(^3_4^U^/^/`U\\8^(^3`"
			"4`U\\8^V^  K^(^3`4^V^1^9]+^V^      ?`    O\\  D\\6]M]            We D]1]T] 9[3bJ\\@e<])]2])\\  "
			"  T]0d3_7h9i/_;k5f?n:f7e    3g :_8i3h@h9n?l5iB]H]C].].]J^B].`I`H_J]<g?g1g?g4hAuB]H]G]C]F]K"
			"_K]S^J^F^G^CrBb7]*b'_ D] :] '] Fc A].].].].]            >a:].a   !^T_ Bg   `       Dd2_8n?"
			"m7g3]:rD]P]P]@g <] 8] 8] B] 3e J^K^ If7^U^+b@d   Fb@f5a Ad4e-] :f  Ra0d AaF\\HaF\\HeJ\\?]._0_"
			"0_0_0_2\\U\\0tHh@n?n?n?n?].].].]-h:_J]<g8g8g8g8g BhV]G]H]C]H]C]H]C]H]G^G^B]*d5](]2\\X\\4aW]8^V"
			"^6\\S\\  I](]3]X]5^U^?](]3\\W\\5^U^.^R[9aW]7](]2\\X\\4aW]8^V^  J](]2\\X\\4^V^1]8]+^V^      ?a>w   "
			"P[ 9[/a:aQa7[    Wl      \"h E]1]T]+\\R\\;[4dL]Ag=])]2])\\    U^1f8c8k;j1`;k7h?n;h9g    5i*b:_"
			"8k6kBl=n?l7mD]H]C].].]L_A].`I`H`K]>kAj6kAj9kBuB]H]F]E]E^L_L^R^L^D^I^BrBb7^+b(a D] ;] '] Gd"
			" A].].].].]      ;]     (b:].b   #^Q] Dj  !a       Ff3_8n?m8i4]:rD]P]P]Bk ?_ 9] 9_ C]&[0f "
			"I]K]=]0g7^U^-fC\\S]   IfBf6c B[S]5[S].] <i  R\\W\\1]T] B\\W\\G]H\\W\\G]H[S]K]?]._0_0_0_0_2c1uIkBn"
			"?n?n?n?].].].]-l>`K]>k<k<k<k<k EoF]H]C]H]C]H]C]H]F^I^A],h6]*]2\\V\\6]Wa7^V^6\\S\\  H]*]2\\V]6^U"
			"^>]*]3]W]6^U^._V_;]Wa5]*]2\\V\\6]Wa7^V^  I]*]2\\V\\5^V^2]7]+^V^      @]W\\=v   P[ 9\\1c<cSd:]   "
			"\"o      #_S^ F]1]T],]S];[5^V^N]A_T]=]*]0]*\\    U]1^T^;e8`S_<^R_2`;k8^R]?n<_T_;^S^    6^S_."
			"i>_8m:`R`Cn?n?l9`QaE]H]C].].]M_@].aKaH`K]?`S`Bk8`S`Bk;_R_BuB]H]F]E]D]MaM]P]L]B^K^ArB]1]&])"
			"c D] <] '] G] :].].].].]      ;]     (^6]*^   #]P^ E^P\\   V^       H^T^4_8n?m:`S`6]:rD]P]P"
			"]C`S` Aa :] :a D]&[1^S\\ I^M^=]0^R[7^U^/^R^EZO\\   L^R^ N]U] :],\\0] <j  M\\2]R] >\\H]B\\H]=\\M]>"
			"]._0_0_0_0_0_/uK`R`Cn?n?n?n?].].].]-n@`K]?`S`>`S`>`S`>`S`>`S` H`ScE]H]C]H]C]H]C]H]E^K^@],^"
			"T^5],]1\\V\\6\\U`7^V^6]U\\  F],]2\\T\\6^U^=],]2\\U\\6^U^-e9\\U`4],]1\\V\\6\\U`7^V^  H],]1\\V\\5^V^3]6]+^"
			"V^  B`1`1`1`1`6]W]>u   P[ 9]2e>eUf;^   %q      $^O\\ F]1]T],]S];[5]T]N\\@]P[=]*]0]2ZR\\RZ   $"
			"]2]P]<_W]8]N]<ZL^4a;]+]MZ/]<^P^=^Q^    7\\O]1nAa9]N_<_M]C]NaA].]+_L^E]H]C].].]N_?].aKaHaL]@"
			"^M^C]P_:^M^C]P_=^M\\6]6]H]F^G^D]MaM]P^N^B^K^-^B]1]&]*e D] =] '] H] 9].].].].]      ;]     )"
			"^5])^   %^O]8^3]LZ   U]       I^R^6a9_0]+^M^7]:]H]D]P]P]D^M^ Cc ;] ;c E]&[2^PZ H]M]<]1^-^U"
			"^1]L];[   N]L] Q]S] :\\,\\1] <dU\\  M\\2\\P\\ >\\H\\A\\H\\<\\M\\=]/a2a2a2a2a1_/]V];_M]C].].].].].].].]"
			"-]ObBaL]@^M^@^M^@^M^@^M^@^M^ J^N`D]H]C]H]C]H]C]H]E^K^@]-^Q]5].]1\\T\\7\\S]6^V^5c  E].]2]S\\7^U"
			"^<].]2\\S\\7^U^,a6\\S]2].]1\\T\\7\\S]6^V^  G].]1\\T\\6^V^4]5]+^V^  De6e6e6e6e9\\U\\>u   P[ :_3f@gVf<"
			"_   &r      $]M[ F]1]T],\\R]>d<^T^P]A^OZ=]+].]4]T\\T]   &^3^P^=[S]8[K].]4\\X];],]!]<]N]>^O^  "
			"  8ZM^3`P`Ba9]M^=^J\\C]K_B].],^H\\E]H]C].].]O_>].aKaHaL]A^K^D]N^<^K^D]N^>]JZ6]6]H]E]G]C]MaM]"
			"O^P^@^M^-^A]1]&]+_W_ D] >] '] H] 9]  B].]      ;]     )]4](]   %]N]:c6]   G]       J^P^7a8"
			"_1],^K^;c=]H]D]P]P]E^K^ Ee <] <e F]&[2] =^O^<]1] 0\\H\\<\\   P\\H\\ R\\Q\\+]3\\,\\2] <eU\\  M\\3]P\\ >"
			"\\I]A\\I]<\\N]=]/a2a2a2a2a2a1]U]<^J\\C].].].].].].].]-]K_CaL]A^K^B^K^B^K^B^K^B^K^ K]K^D]H]C]H]"
			"C]H]C]H]D^M^?]-]P]4]0]1\\R\\  Ha  C]0]2]R] E]0]2\\Q\\ 9c 9]0]1\\R\\   !]0]1\\R\\ ?]4]   Di:i:i:i:i"
			";\\6]G]   P\\ :`5g@gWh>a   (_       J]KZ F]1]T],\\R\\?h>]R]P\\@]1]+].]3^V\\V^.]   T]2]N]5]8ZJ]-]"
			"6]X];]-]!^=]L]?]M]    *]5_J_Ec:]L^>]H[C]I^C].],]F[E]H]C].].]P_=].]X]M]X]HbM]A]I]D]M]<]I]D]"
			"M]?]%]6]H]E]G]C^NaN^N]Q^>^O^-^@]0]'],_U_  &] '] H] 9]  B].]      ;]     )]4](]   %]N]:d7] "
			"  F]       K]N]8c8^1],]I]>i@]H]D]P]P]E]I] Fg =] =g G]&[2] <]O];]1] 1\\F\\=\\   Q\\F\\ S\\Q\\+]3\\."
			"]  IeU\\  M\\3\\N\\ ?\\I\\@\\I\\=]M\\<]0c4c4c4c4c3a1]U]<]H[C].].].].].].].]-]J_DbM]A]I]B]I]B]I]B]I]"
			"B]I] L]J_E]H]C]H]C]H]C]H]C^O^>].]N]    .]        '`X_           I]   FbWa=bWa=bWa=bWa=bWa<"
			"\\6^I^  ?Z2[ :a5gAiXh?c   *^       H] 7]1]T]-]S]Aj>]R]Q]@]1],],\\1^X\\X^,]   T]3]L]6]'].]7]W]"
			";]-]!]<]L]?]M^    +]6^F^F]W]:]K]?]FZC]H^D].]-]DZE]H]C].].]Q_<].]X]M]X]H]X]M]B]G]E]M^>]G]E]"
			"M^@]%]6]H]E^I^B]O^X]O]M^R^=]O^-^@]0]']-_S_  '] '] H] 9]  B].]      ;]     )]4](]   %]N]:e8"
			"_   H]       L]M]8]W]7^2]-]G]AmB]H]D]P]P]F]G] Hi >] >i  J[3] ;^Q^;]1] 2\\RbT\\Ge   R\\VdR\\ T\\"
			"Q\\+]4\\2a  IfU\\  M\\3\\N\\ ?\\J\\?\\J\\AaM\\ G]W]4]W]4]W]4]W]4]W]4c3^U]=]FZC].].].].].].].]-]H]D]X]"
			"M]B]G]D]G]D]G]D]G]D]G]A[H[B]J`E]H]C]H]C]H]C]H]B]O^>g8]N]             1]T_      3[    9]   "
			"G_O^?_O^?_O^?_O^?_O^=\\5]I^  @\\3[ ;c6gAy?d7`8]L]7^7]L]>^       H] 6]1]T]-]S]B_W[U]>]R]R]?]1"
			"],],]0d*]   T]3]L]6]'].]7\\V];].] ]<]L]@]K]  7Z PZ X]7^D^G]W]:]K]?]/]G]D].]-]/]H]C].].]R_;]"
			".]X^O^X]H]X^N]B]G]E]L]>]G]E]L]@]%]6]H]D]I]A]O]W]O]L^T^<^Q^-^?]0]'].^O^  Sb7]U`2b4`U]8a8])`"
			"7]T_  M].]%_O_@_2`0`3`/_3c9]     )]4](]   N_6]N]3^7a/c0_ <^  D[U^  Ga  N]L]9]W]6^3]-]G]B`W"
			"]W`C]H]D]P]P]F]G] I_X]X_ ?] ?_X]X_  Nb7]2ZFZ=]Q]:]0] 3[SfU[Ig   R[UfS[ T\\Q\\+]5]2a  IfU\\  M"
			"\\3\\N\\ ?\\K]?\\K]AaN] G]W]4]W]4]W]4]W]4]W]4]W]3]T]=]/].].].].].].].]-]G]E]X^N]B]G]D]G]D]G]D]G"
			"]D]G]B]J]C]KbF]H]C]H]C]H]C]H]B^Q^=j;]P_9b3b3b3b3b3b3bN`Bb3a2a2a2a    V_2_2`1`1`1`1` ;aU]  "
			"  :]U`   S^T]U^A^L^A^L^A^L^A^L^?]5]I]  @^5\\ <e7gAy@f;e:]L]8`8^N^?^       G] 6]1]T]-\\R\\A]U["
			"RZ>]R]R\\>]1],],].`(]   U^3]L]6]'].]8]V];].]!^<]L]@]K]  :] P]#^8^A]I^W^;]K]@].]G^E].].].]H]"
			"C].].]S_:].]W]O]W]H]W]N]C]E]F]L]?]E]F]L]@]%]6]H]D]J^A]O]W]O]L^U^:^S^-^>]0^(]/^M^  Wh:]Wd6f"
			"8dW]:e>h2dW]?]Vd<].].]O_>].]WdScK]Vd8f;]Wd7dW]?]Wa6h>h6]L]B]I]A]P`P]K^L^B^K^@l4]4](]   PdU"
			"]A]N]2^8e5g;]Vd?^J^8]6]L] E]V`>pA]S]S]:e6kDo>]L]:^W^6^4].]E]D_U]U_D]H]D]P]P]G]E] K_W]W_ @]"
			" @_W]W_  Qf9]3\\H\\>^S^:]0_ 6[ThT[K]Q\\   S[T\\R]S[ U]S]+]6],] ?]L]@fU\\  M\\3\\N\\ ?\\K\\>\\K\\;]O\\ G"
			"^W^6^W^6^W^6^W^6^W^5]W]4^T]>].].].].].].].].]-]G^F]W]N]C]E]F]E]F]E]F]E]F]E]D_L_E]K]W]F]H]C"
			"]H]C]H]C]H]A^S^<k<]Ra<h9h9h9h9h9h9hTeFf7e6e6e6e;].].].]\"^;]Vd8f7f7f7f7f/^6eX]@]L]?]L]?]L]?"
			"]L]B^K^?]Wd>^K^  O]S]S]B]I]B]I]B]I]B]I]@]5^K^  @]4[ ;f8gAyAg<h<]L]8`7]N]>]       F] 6]1]T]"
			"-\\R\\B]T[6]R]S]>^2]-]*\\.`(]   U]2]L]6]'].]9]U];].]!];]L]@]K]  =` P`'^7]?\\I]U];]K]@].]F]E].]"
			".].]H]C].].]T_9].]W]O]W]H]W^O]C]E]F]L]?]E]F]L]@]%]6]H]C]K]@^P]W]P^K^V^9]S]-^=]/](]0^K^  Xi"
			";]Xf9h9fX]<h?h3fX]?]Xg=].].]P_=].]XfVfL]Xg:h<]Xf9fX]?]Xb7i>h6]L]A]K]@^Q`Q^J^N^@]K]?l4]4](]"
			"   QfW^A]O^1]6f9h;]Xg@_K]7]6]L]=]G]C^Wc@pA]S]S]<h9mDo>]L]:]U]5^5].]E]E^S]S^E]H]D]P]P]G]E]@"
			"Z+]V]V^-Z4]5ZKZ:]V]V^  Sh9]4^J^>]S]9]._ 8[U_Q[T[L]P\\   S[T\\Q]T[ T]U]*]7]*] @]L]@fU\\  M\\3\\N"
			"\\ ?\\L]>\\L]:]Q]:]1]U]6]U]6]U]6]U]6]U]6^W^5]S]>].].].].].].].].]-]F]F]W^O]C]E]F]E]F]E]F]E]F]"
			"E]C_N_D]L^W]F]H]C]H]C]H]C]H]@]S];]P_=]S^8i:i:i:i:i:i:iVgIh9h9h9h9h<].].].]'d<]Xg:h9h9h9h9h"
			"0^8k?]L]?]L]?]L]?]L]A]K]>]Xf>]K]  O]R]R]D]G]D]VZOZV]D]KZV]D]G]A]4]K]  @]3[ <g7fAyBi>j=]L]8"
			"`7]N]?]       F^ 6]1]T]5uI]T[6]R]S\\<^3]-]*]1d*]   U]3]J]7]'].]9\\T];].\\Ua-^;]L]@]K^?].] Uc "
			"Pc+_8]>]J]U];]K]@].]F]E].].].]H]C].].]U_8].]W^Q^W]H]V]O]C]E]F]L]?]E]F]L]@^&]6]H]C]K]?]Q^V]"
			"Q]I^X^8^U^.^<]/](]1^I^  ]R_<aT_;_R\\:^Tb=_S^@h4_Ub?bT^=].].]Q_<].aT_X]T^LbT^;_T_=aT_;^Tb?aT"
			"Z8_R]>h6]L]A]K]?]Q`Q]H^P^?]K]?l4]4](]   R^U^W]@]O]0^7g;_S];bT^@`L]8_7]L]>]E]E^W]V]@pA]S]S]"
			"=_T_<oDo?]K^;]U]5_6].\\D]E]R]R]E]H]D]P]P]G]E]A\\+[U]U\\,\\6]6\\L\\;[U]U\\  S_W[V\\9]3^V`V^=^U^9]/a"
			" :[T]G[M\\O\\1ZQZ  M[S\\P\\S[ Ud)]8](\\ @]L]@fU\\  M\\3\\N\\9ZQZ0\\L\\=\\L\\8\\Q\\9]1]U]6]U]6]U]6]U]6]U]6"
			"]U]5]S]>].].].].].].].].]-]F]F]V]O]C]E]F]E]F]E]F]E]F]E]B_P_C]L]V^G]H]C]H]C]H]C]H]@^U^;]N^>"
			"]T]6]R_;]R_;]R_;]R_;]R_;]R_;]R_X_T^K_R\\:_S^;_S^;_S^;_S^=].].].]*h=bT^;_T_;_T_;_T_;_T_;_T_1"
			"^9_T`>]L]?]L]?]L]?]L]A]K]>aT_?]K]  P]Q]R]E]F]E]V\\Q\\W]E]K\\W]E]F]A]4^L]  A^@ZN\\ =i8e@yCk?^R^"
			"=]L]9b8]O^?]       Im B]1]T]5uI]T[6]S^T]<^3]-]*]3^X\\X^,]   V^3]J]7](^/]9]T];e7]We/]9]N]?]K"
			"^?].] Wd Nd._8]O`U\\T\\K]S]<]L^A]-]F^F].]/]-]H]C].].]V_7].]V]Q]V]H]V^P]D]C]G]L]@]C]G]L]?^']6"
			"]H]C^M^?]Q]U]Q]Ic6^W^._<]/^)]2^G^ !ZM^=`Q^=^NZ;^Q`>^P^=].^Q`?`Q^>].].]R_;].`R^X\\R^M`Q^=^P^"
			">`Q^=^Q`?`1]MZ;].]L]A^M^?]Q`Q]G^R^>^M^1^4]4](]  D]P^A]R^X]@]P^/]9^Vb=^NZ;`Q^AaN^8_7]L]>]E]"
			"F^V]U]>]P]>]S]S]>^P^>`T`7]6]J]<]S]5^6]/]C]G]Q]Q]F]H]D]P]P]H]C]C^&]TZ,^7]7^N^6]TZ H]/^U[TZ9"
			"]2n;]U]8]0d <[U]F[M\\P]2[R[  M[S\\P\\S[ Tb(]9]'\\ @]L]@fU\\  M\\3]P]9[R[1\\M\\<\\M\\7\\R\\8]2]S]8]S]8]"
			"S]8]S]8]S]7]U]6]R]?]-].].].].].].].]-]F]F]V^P]D]C]H]C]H]C]H]C]H]C]B_R_C]L]T]G]H]C]H]C]H]C]"
			"H]?^W^:]M]>]U^6ZM^<ZM^<ZM^<ZM^<ZM^<ZM^<ZMbP]M^NZ;^P^=^P^=^P^=^P^>].].].]+i=`Q^=^P^=^P^=^P^"
			"=^P^=^P^2^:^P^>]L]?]L]?]L]?]L]A^M^>`Q^@^M^  P]Q]Q]F]E]F]W^S^W]F]L^W]F]E]B]3]M^  B^B^O[ =k8"
			"d?xClA^P^>]L]9]X]8^P]>\\       Hl A] 9uI]T[5]T]T]:^ =]*]5^V\\V^.]   V]2]J]7](]/^:]S];h:]Xg0]"
			"9^P^?]K^?].]!e Je2_7\\PdW\\S\\L]S]<]M^@]-]E]F].]/]-]H]C].].]X_5].]V]Q]V]H]U^Q]D]C]G]L]@]C]G]M"
			"^?`)]6]H]B]M]>]Q]U]Q]Hb5c-^;].])]   B]=_O]=].]O_>]N^>].]O_?_O]>].].]S_:]._P`P]M_O]=]N]>_O]"
			"=]O_?_1]-].]L]@]M]>]RbR]G^R^=]M]1^3]4](]  FaSaD^Qa?]R_.]9]R`>]._O]>^N]8`7]L]>]E]G^U]U^?]P]"
			">]S]S]>]N]>^P^7]6]J]<]S]4^7]/]C]G]Q]Q]F]H]D]P]P]H]C]D_&]&_8]8_N_7] B]/]T[3]1l:^W^8]1]W` >\\"
			"U\\E\\N\\P]3\\S\\  N\\S\\P\\S\\ S_']:]&\\ @]L]@fU\\  M\\2\\P\\8\\S\\2\\N]<\\N]7\\S]8]2]S]8]S]8]S]8]S]8]S]8]S]"
			"7]R]?]-].].].].].].].]-]E]G]U^Q]D]C]H]C]H]C]H]C]H]C]A_T_B]M]S]G]H]C]H]C]H]C]H]>c9]M^?]U]']"
			".].].].].].`O^N].]N^>]N^>]N^>]N^?].].].],_R^>_O]=]N]=]N]=]N]=]N]=]N]2^:]O_?]L]?]L]?]L]?]L]"
			"@]M]=_O]?]M]  O\\P]Q]F\\D]F\\U^U^V]F\\L^V]F\\D]B]3]M]  RuJ`O[ >m9c>wCmA]N]>]L]9]X]7]P]?]       "
			"Im A] 2\\R\\A]T[5^V^T\\:` ?](\\6]T\\T]/]   V]2]J]7])^1_9]S];i;bS^2^8^S_>]K^?].]$e@u@e6_7]QfX\\S\\"
			"M^S^=]N^?]-]E]F].]/]-]H]C].].c4].]U]S]U]H]T]Q]D]C]G]M^@]C]G]M]=c-]6]H]B]M]>^R]U]R^G`4c.^:]"
			".])]   B]=^M]?^/]M^?]L]>]/]M^?^N^?].].]T_9].^O_O^N^N^?]M^?^M]?]M^?^0]-].]L]@]M]>^S]X]S^F^T"
			"^<^O^2_3]4](]  GcUcE]Pa?]Vb-]:]O_?].^N^>]O^8a8]L]?]C]H]T]T]?]P]>]S]S]?]L]@^N^8]6]J]=^S^4^8"
			"]/]C]H^Q]Q^G]H]D]P]P]H]C]E_%]%_9]9_L_8] B]0^T[3]0_T_>cWc=]1]U_ ?[U\\C[N]R^4]T]  N[R\\Q]R[ 'u"
			"G]&] @]L]?eU\\  M\\2]R]8]T]3\\N\\;\\N\\7]S\\7]3^S^:^S^:^S^:^S^:^S^9]S]8^R]?]-].].].].].].].]-]E]G"
			"]T]Q]D]C]H]C]H]C]H]C]H]C]@_V_A]N]R]G]H]C]H]C]H]C]H]>c9]L]?]U]'].].].].].]._M]O^/]L]?]L]?]L"
			"]?]L]?].].].]-^O]>^N^?]M^?]M^?]M^?]M^?]M^ I]O`?]L]?]L]?]L]?]L]@^O^=^M]@^O^  P]P]P\\G]C\\G]T^"
			"W^T\\G]M^T\\G]C\\B]3^O^  RuJ[X]P[ >o=\\XaX]BwDoC]L\\>]L]:^X^8]P]?]       E] 5] 3]S]A^U[4dT];b @"
			"](]6ZR\\RZ.]   V]2]J]7]*^7d8]R];]R_<aQ^3]5f<^M_?].]'e=u=e:_6\\Q^S`S]N]Q]=l>]-]E]Fm>k=]-rC].]"
			".b3].]U]S]U]H]T^R]D]C]G]M]?]C]G]N^<f1]6]H]B^O^=]S^U^S]F_2a.^9].])]   A]>^M]?].]M^?]L]>]/]M"
			"^?^M]?].].]U_8].^N^N]N^M]?]L]?^M]?]M^?^0]-].]L]@^O^=]S]X]S]D^V^:]O]2_2]4](]  H\\U^W]U\\E]Pa?"
			"]Vb-];]M^?].^M]>^P]7a8]L]?]C]H]T]T]?]P]>]S]S]?]L]@]L]8]6p=]Q]3^9]/]C]H]P]P]G]H]C]Q]Q]G]ViV"
			"]F_$]$_:]:_J_9] B]0]S[3]0]P]>o=]2]S_ @[U\\C[M]T_5^U^;u O[R\\R]Q[ 'uH]/ZQ] ?]L]?eU\\  M\\1]T]7^"
			"U^4\\O]O]I\\O]T`MZQ]S]O]E]3]Q]:]Q]:]Q]:]Q]:]Q]:^S^9]QmO]-m>m>m>m>].].].]1hL]G]T^R]D]C]H]C]H]"
			"C]H]C]H]C]?_X_@]O]Q]G]H]C]H]C]H]C]H]=a8]L]?]U]&].].].].].].^M]O].]L]?]L]?]L]?]L]?].].].].^"
			"M]?^M]?]L]?]L]?]L]?]L]?]L] I]Pa?]L]?]L]?]L]?]L]?]O]<^M]?]O]  O]P]P\\G]C\\G]ScS\\G]N^S\\G]P]P\\B"
			"]2]O]  QuF]Q[ >oAqDuDqD]L]?]L]:^X^8^R^?\\       D] 5] 3]S]@`X[3bS\\R^G]W^N] P](].\\&]   W]1]J"
			"]7]*^7c8]Q];ZM^=`O^4]4d:]M_?].])d:u:d=_5\\R]O^R\\N]Q]=j<]-]E]Fm>k=]-rC].].a2].]U^U^U]H]S]R]D"
			"]C]G]N^?]C]G]P_:g3]6]H]A]O]<]S]S]S]E^1_.^8]-]*]   A]>^M]?]/^M^?]K]?]0^M^?]L]?].].]V_7].]M]"
			"M]N]L]@^L]?^M]@^M^?]/]-].]L]?]O]<]S]X]S]C^X^9]O]2^1]4](]0_IZ O[R\\X]S\\G^O_>]Vd9_U];]L]?].]L"
			"]=]P]8]X^9]L]?]C]I^T]S]@]P]>]S]S]?]L]@]L^9]6p=]Q]3^9]/]C]H]P]P]G]H]C]Q]Q]G]ViV]G_#]#_;];_H"
			"_:] B]0]S[3]0\\N\\>o=]2]Q^ A[U\\C[LcX\\6]T]9u O[RfP[ 'uIf7e >]L]>dU\\<] :f5d4]T]:fT\\O^NfT\\UdOeR"
			"\\O^F^3]Q]:]Q]:]Q]:]Q]:]Q]:]Q]:^QmO]-m>m>m>m>].].].]1hL]G]S]R]D]C]H]C]H]C]H]C]H]C]>d?]P^Q]G"
			"]H]C]H]C]H]C]H]<_7]L]?]U^'].].].].].].^L]P].]K]@]K]@]K]@]K]@].].].].]L]?]L]@^L]@^L]@^L]@^L"
			"]@^L] I]Q]X^@]L]?]L]?]L]?]L]?]O]<^M]?]O]  O\\WmX]H\\WmX]H\\QaR]H\\N^R]H\\O]P]C]2]O]  QuF]R\\ ?qC"
			"sDtDrE]L]?]L]:]V]7]R]>x      '] 5] 3\\R\\?e3^R\\SbJ^V^O] P](].\\&]   W]1]J]7]+^6e:]Q]-^>_M]5^6"
			"h<^O`  Qe8u8e@^5]R\\M]R\\O^Q^>m?]-]E]Fm>k=]KdFrC].].b3].]T]U]T]H]S^S]D]C]G]P_>]C]Gk6f5]6]H]A"
			"^Q^<]S]S]S]F_1_/_8]-]*]   A]>]K]A].]K]@]J]?]0]K]?]L]?].].]W_6].]M]M]N]L]@]J]@]K]A]K]?]/^.]"
			".]L]?]O]<]T^W]T]C^X^9^Q^3^1]3]']3dN\\ P\\R`Q[G]N_>]Q`;bW];\\K^?]/]L]=]Q^8]W]9]L]?]C]I]S]S]@]P"
			"]>]S]S]@]J]B^L^9]6p>^Q^4^9]/]C]H]P]P]G]H]C]Q]Q]G]ViV]H_\"]\"_<]<_F_;] B]1]R[3]1]N]8a6]2]P^ B"
			"[U\\C[K`V\\7]T]8u O[RdN[ 'uIf5a <]L]=cU\\<] :f3`1]T];fU\\N^NfU\\T[S]NaQ\\N^G^3^Q^<^Q^<^Q^<^Q^<^Q"
			"^;]Q]:]PmO]-m>m>m>m>].].].]1hL]G]S^S]D]C]H]C]H]C]H]C]H]C]=b>]P]P]G]H]C]H]C]H]C]H]<_7]L]?]U"
			"_(].].].].].].]K]Q].]J]A]J]A]J]A]J]@].].].].]L]?]L]@]J]A]J]A]J]A]J]A]J] K]P\\V]@]L]?]L]?]L]"
			"?]L]?^Q^<]K]@^Q^  O\\WmX]H\\WmX]H\\P_Q]H\\O^Q]H\\O]P]C]2^Q^  D^<]R[ >qDuEsCqD]L]?]L]:]V]7]R]>x "
			"     '] 5] 3\\R\\=f+]TdL^T^P] P](].\\2u  *]1]J]7],^-_=]P],]>_M]5]7_R^<^Qa  Sd .dC^4\\R]M]R\\O]O"
			"]>]N_@]-]E]F].]/]KdF]H]C].].]X^4].]T]U]T]H]R]S]D]C]Gk=]C]Gj1c6]6]H]@]Q];^T]S]T^Ga1].^7]-]*"
			"]   Lh>]K]A].]K]@]J]?]0]K]?]L]?].].]X_5].]M]M]N]L]@]J]@]K]A]K]?]._0].]L]>]Q];^U]V]U^Bb7]Q]"
			"3^1^3]'^6iS^ P[P^P[G]N_>]N^=dX]<]J]>^1]L]=^R]8^W]9]L]@]A]J]S]S]@]P]>]S]S]@]J]B]J]9]6]J]>]O"
			"]5^8]/]C]H]P]P]G]H]B]R]R]F]C]Iz<]<z=]=z<] B]1]R[7j:\\L\\7_5]2]P^ B[U\\C[ V]T]7u O[R\\U^O[  T] "
			"  ]L];aU\\<]   I]T],]O[X\\>]K]@]O[X\\I`3]O]<]O]<]O]<]O]<]O]<]O];]P]?]-].].].].].].].]-]E]G]R]"
			"S]D]C]H]C]H]C]H]C]H]C]<`=]Q]O]G]H]C]H]C]H]C]H];]6]L]?]T_4h9h9h9h9h9h9hK]Q].]J]A]J]A]J]A]J]"
			"@].].].]/]J]@]L]@]J]A]J]A]J]A]J]A]J]?tG]Q\\U]@]L]?]L]?]L]?]L]>]Q];]K]?]Q]  N\\WmX]H\\WmX]H\\P_"
			"Q]H\\P^P]H\\O]P]C]1]Q]  C]:]S[ ?sEvEqAoC]L]?]L];^V^8^T^>x      '] 5] 4]S]<g-\\T^V^M]S_Q\\ O](]"
			".\\2u Se =^1]J]7]-^*^?]O],^?^K]7^7]N]<^Sb  Sa (aC]3\\R\\K\\R\\P^O^?]L^A]-]E]F].]/]KdF]H]C].].]W"
			"^5].]T^W^T]H]R^T]D]C]Gj<]C]Gj-`7]6]H]@]Q]:]U^S^U]Fb2]/^6]-^+]   Nj>]K]A].]K]@p?]0]K]?]L]?]"
			".].b3].]M]M]N]L]@]J]@]K]A]K]?].c4].]L]>]Q]:]U]V]U]@`6^S^4^5b2]&b<u P[O]P\\H]N^=]M]>^Ua<]J]="
			"c7]L]<]S^8]V^:]L]@]A]J]S]S]@]P]>]S]S]@]J]B]J]9]6]J]?^O^7^7]/]C]H]P]P]G]H]B]R]R]F]C]Iz<]<z="
			"]=z<] B]1]R[7j:\\L\\7_ C^P] B[U\\C[ W]T] W] O[R\\T^P[  T]   ]L]7]U\\<]   H]T]-\\O\\X\\>\\I\\@\\O\\X\\J`"
			"3^O^>^O^>^O^>^O^>^O^=]O]<^P]?]-].].].].].].].]-]E]G]R^T]D]C]H]C]H]C]H]C]H]C];^<]R]N]G]H]C]"
			"H]C]H]C]H];]6]L]?]S`8j;j;j;j;j;j;|Q].pApApAp@].].].]/]J]@]L]@]J]A]J]A]J]A]J]A]J]?tG]R]U]@]"
			"L]?]L]?]L]?]L]>^S^;]K]?^S^  N\\WmX]H\\WmX]H\\QaR]H\\Q^O]H\\O]P]C]1^S^  D]9]T\\ ?sFwDo?nC]L]?]L];"
			"]T]7]T]=]       Hj ?] 4]S]8d/]T]T]N^R_R\\ O](] =u Se =]0]J]7].^(]?]O]+]?^K]7]7]L]<gX]  Sa ("
			"aC]3\\R\\K\\R\\P]M]?]K]A]-]E]F].]/]D]F]H]C].].]V^6].]S]W]S]H]Q]T]D]C]Gg9]C]G]Q_,^7]6]H]@^S^:]U"
			"]Q]U]G^X]2]0^5],]+]   Pl>]K]A].]K]@p?]0]K]?]L]?].].a2].]M]M]N]L]@]J]@]K]A]K]?]-f8].]L]>^S^"
			":]U]V]U]?^4]S]4^4`0]$`<^Si O[O\\O\\H]N^=]M^@^S`<]J]=c7]L]<]S]8^U]:]L]@]O]O]J]S]S]@]P]>]S]S]@"
			"]J]B]J]9]6]J]?]M]7]6]/^E^H]P]P]G]H]A]S]S]E]C]Iz<]<z=]=z<] B]1]R[7j:\\L\\6] A^Q] B[U\\C[Ni:]T]"
			" V] O[R\\S]P[  T]   ]L]6\\U\\<]  Dh2]T]/]P\\W\\?]I\\A]P\\W\\K`2]M]>]M]>]M]>]M]>]M]>^O^=]O]?]-].].]"
			".].].].].]-]E]G]Q]T]D]C]H]C]H]C]H]C]H]C]<`=]S]M]G]H]C]H]C]H]C]H];]6]M^?]R`;l=l=l=l=l=l=~Q]"
			".pApApAp@].].].]/]J]@]L]@]J]A]J]A]J]A]J]A]J]?tG]S]T]@]L]?]L]?]L]?]L]=]S]:]K]>]S]  M]P]P\\G]"
			"C\\G]ScS\\G]S^N\\G]P]P\\B]0]S]  D]7\\T[ >sFwCn?mB]L]?]L];]T]7]T]=]       Hi >] 4]S]7[Xa1]T^T^O]"
			"P_T] O](] =u Se =]0]J]7]/^'^A]N]+]?^K]7]8^L^<eW]  Sd .dC]3\\R\\K\\R\\P]M]?]K]A]-]E]F].]/]D]F]H"
			"]C].].]U^7].]ScS]H]Q^U]D]C]G]/]C]G]O^,^8]6]H]?]S]9]U]Q]U]H^W^3]1^4],]+]   Q`P]>]K]A].]K]@p"
			"?]0]K]?]L]?].].b3].]M]M]N]L]@]J]@]K]A]K]?]+e9].]L]=]S]9]V]T]V]@_4]S]5_4b2]&b<\\Nd M[O]P\\H]N"
			"^=]L]@]Q_<]J]?e7]L];]T]8]T]:]L]@]O]O]J]S]S]@]P]>]S]S]@]J]B]J]9]6]J]?]M]8^6].]E]G]P]Q^G]H]A"
			"^T]T^E]C]Iz<]<z=]=z<] B]1]R[3]1\\L\\6] A_R] B\\U\\E\\Ni:]T] V] O\\S\\R]R\\  T]   ]L]6\\U\\<]  Dh2]T]"
			"/\\O[V\\?\\H\\A\\O[V\\L`1]M]>]M]>]M]>]M]>]M]>]M]>^O]?]-].].].].].].].]-]E]G]Q^U]D]C]H]C]H]C]H]C]"
			"H]C]=b>]T]L]G]H]C]H]C]H]C]H];]6]M]>]Qa>`P]>`P]>`P]>`P]>`P]>`P]>`PoQ].pApApAp@].].].]/]J]@]"
			"L]@]J]A]J]A]J]A]J]A]J]?tG]T]S]@]L]?]L]?]L]?]L]=]S]:]K]>]S]  L\\P]P\\F\\C\\F\\T^W^T\\F\\T^M\\F\\C\\B]"
			"0]S]  E^7]U[ >sFwBl=kA]L]?]L]<^T^8^V^=]       Ij >] <u=[U^1\\S]R]O]O_U\\ N](] 1] Ge =]0]J]7]"
			"0_&]A]N]+]?^K]8^8]J]:aU\\  Pe 4eA]3\\R\\K\\R\\Qo@]J]A].]F^F].].]E]F]H]C].].]T^8].]RaR]H]P]U]C]E"
			"]F].]E]F]N^,]8]6]H]?]S]9^V]Q]V^H^V^4]2_4],]+]   Q]M]>]K]A].]K]@],]0]K]?]L]?].].c4].]M]M]N]"
			"L]@]J]@]K]A]K]?](d;].]L]=]S]9^W]T]W^@`5^U^5^/_3]'_8ZJ` K[O]P\\H]N^=]L]@]P];]J]@_0]L];]U^9^T"
			"^;]L]@]O]O]J]S]S]@]P]>]S]S]@]J]B]J]9]6]J]@^M^:^5].]E]F]Q]Q]F]H]@^U]U^C]E]G_\"]\"_BZT]TZB_F_;"
			"] B]1]R[3]1\\L\\?o I_S] A[U]F[ V]T] W] N[S\\R]R[  S]   ]L]6\\U\\   ']T]/\\O\\V\\@\\H\\A\\O\\V\\M_0o@o@o"
			"@o@o?m>l>].].].].].].].].]-]F^G]P]U]C]E]F]E]F]E]F]E]F]E]=d?^V]L]F]H]C]H]C]H]C]H];]6]N^>]O`"
			"?]M]>]M]>]M]>]M]>]M]>]M]>]M]?].].].].]-].].].]/]J]@]L]@]J]A]J]A]J]A]J]A]J] K]U]R]@]L]?]L]?"
			"]L]?]L]=^U^:]K]>^U^  L\\P]Q]F\\D]F\\U^U^V]F\\U^M]F\\D]B\\/^U^  OuD]V[ =sFwBk;i@]L]?]L]<]R]7]V];]"
			"       F^   Nu=[T^3]S]R]O]N_V\\ N](] 1]   ].]L]6]1_%]Aq0]>]K]8]7]J]/]  Md:u:d>]3\\R\\K\\S\\Po@]"
			"J]A].]F]E].].]E]F]H]C].].]S^9].]RaR]H]P^V]C]E]F].]E]F]M],]8]6]H]>]U^8]W^Q^W]H^U^4]2^3]+],]"
			"   R^M]>]K]A].]K]@],]0]K]?]L]?].].]X_5].]M]M]N]L]@]J]@]K]A]K]?]$`;].]L]=^U^8]W]T]W]@b5]U]5"
			"^,]3]']  J\\Q_Q[G]N^=]L]A]O];]J]@].]L];]U]8]R];]L]@]O]O]J]S]S]@]P]>]S]S]@]J]B]J]9]5]L]?]K];"
			"^4].^G^F]Q]Q]F]H]?_W]W_B]E]F_#]#_B\\U]U\\B_H_A\\U]U[ H]1]R[3]1]N]?o H`V] @[T]G[ U]T] X] N[S\\Q"
			"]S[  S]   ]L]6\\U\\   (]T]/]P\\U\\A]I]B]P\\U\\M^/o@o@o@o@o@o@m>].].].].].].].].]-]F]F]P^V]C]E]F]"
			"E]F]E]F]E]F]E]>_X_?]W^L]F]H]C]H]C]H]C]H];]6]P_=]M^@^M]?^M]?^M]?^M]?^M]?^M]?^M]?].].].].]-]"
			".].].]/]J]@]L]@]J]A]J]A]J]A]J]A]J] K]U\\Q]@]L]?]L]?]L]?]L]<]U]9]K]=]U]  K]Q]Q]F]E]F]W^S^W]F"
			"]W^L]F]E]B\\.]U]  NuC\\V[ =eXZXdFgXhAi9h@]L]?]L]<]R]7]V];]       E]   Nu=[S]3\\R]R]O]M_X\\ M]("
			"] 1]   ].]L]6]2_$]Aq0]>]K]8]7]J]/]  Ke=u=e<]3\\R\\K\\S\\Po@]J]A].]F]E].].]E]F]H]C].].]R^:].]Ra"
			"R]H]O^W]C]E]F].]E]F]M^-]8]6]H]>]U]7]W]O]W]I^S^5]3^2]+],]   R]L]>]K]A].]K]@],]0]K]?]L]?].]."
			"]W_6].]M]M]N]L]@]J]@]K]A]K]?]\"_<].]L]<]U]7]W]T]W]Ac5^W^6^+^4](]  H[R\\X]S\\G]N^=]L]A]O];]J]A"
			"^.]L]:]W^9^R];]L]@]O]O]J]S]S]@]P]>]S]S]@]J]B]J]9]5]L]?]K];^4]-]G]D]R]R]E]H]>kA]E]E_$]$_B^V"
			"]V^B_J_A^V]V] I]1]R[3]0\\N\\>o G`X] ?\\U_Q[T\\ T]T] ] N\\T\\Q]T\\  S]   ]L]6\\U\\   )]T].\\P\\T\\A\\I]A"
			"\\P\\T\\N^.o@o@o@o@o@o@m>].].].].].].].].]-]F]F]O^W]C]E]F]E]F]E]F]E]F]E]?_V_@]W]K]F]H]C]H]C]H"
			"]C]H];]6k<]L^A]L]?]L]?]L]?]L]?]L]?]L]?]L]?].].].].]-].].].]/]J]@]L]@]J]A]J]A]J]A]J]A]J] K]"
			"V\\P]@]L]?]L]?]L]?]L]<^W^9]K]=^W^  J]R]R]D]G]D]W\\Q\\W]D]W\\L]D]G]A\\.^V]  NuC]W[ <cWZXdEfXh@g8"
			"g?]L]?]L]=^R^8^X^:]       F]   G\\R\\5[S]4]R]R]O]Lb M](\\ 0]   ].]L]6]3_#]Aq0]>]K]9]6]J]/]  H"
			"e@u@e H\\R]M]T]Q^J]A]J]@]/]G^E].]-]F]F]H]C].].]Q^;].]Q_Q]H]N]W]B]G]E]-]G^F]L]-]8]6]I^>^W^7]"
			"W]O]W]I^R^6]4^1]+],]   R]M^>^M^@]/^M^?]-]0^M^?]L]?].].]V_7].]M]M]N]L]@^L]?^M^A^M^?] ]<].]L"
			"]<]U]7]X]R]X]B^W^5]W]6^)]4](]  H\\T]W]U\\F]O_=]L]A]P^;^L^A]-]L]:]W]8]P]<]L]@]O]O]J^T]T]?]P]>"
			"]S]S]@^L]A^L]8]5]L]@^J]=^3]-^I^D^S]S^E]H]<g>]G]C_%]%_A_W]W_A_L_@_W]W_ J]0]S[3]0]P]5]4],b ="
			"[ThT[ R]T]!] M[T\\P]U[  R]   ]L]6\\U\\   *]T].]P[S\\B]J]A]P[S\\N].^J]B^J]B^J]B^J]B^J]B^K^A]M]=]"
			"/].].].].].].].]-]G^F]N]W]B]G]D]G]D]G]D]G]D]G]?_T_AbK]E]I^C]I^C]I^C]I^;]6j;]K]A]M^?]M^?]M^"
			"?]M^?]M^?]M^?]M_?].].].].].].].].]/]J]@]L]@^L]@^L]@^L]@^L]@^L] J^X]Q]?]L]?]L]?]L]?]L];]W]8"
			"^M^<]W]  I]R]S]C]H]C]VZOZW]C]VZL]C]H]@\\-]W]  MuC]X[ ;cWZWbDeWZXe>e6e>]L]?]L]=]P]8^X^:]    "
			"   F^   H\\R\\5[S]5]Q]R]O^L` K]*] 0]  !^.]L]6]4_\"]2],^>^M]8]6]J]0]  DeCuCe E]R\\M]T\\P]I]A]J]@"
			"]/]G]D].]-]F]F]H]C].].]P^<].]Q_Q]H]N^X]B]G]E]-]G]E]L^.]8]5]J]<]W]6^X]O]X^J^Q^6]5^0]+^-]   "
			"R]M^>^M]?].]M^?]-]/]M^?]L]?].].]U_8].]M]M]N]L]?]L]?^M]?]M^?] ]<].]M^<^W^6aRbB^V^6]W]7^(]4]"
			"(]  GcUcE]P_=]L]A]P]9]L]@]-]L]:^X]9^P]<]M^@]P^O]I]T]T]?]P]>]S]S]@^L]@]L]8]5]M]?]I]>^2],]I]"
			"B_U]U_D]H]:c<]G]B_&]&_?_X]X_?_N_>_X]X_ I]0]S[3]0_T_5]4]+` ;[SfU[ P^U^#] L[U\\P]V[  Q]   ]M^"
			"6\\U\\   ,^U^-\\P\\S\\B\\J]@\\P\\S\\N].]I]B]I]B]I]B]I]B]I]B]I]B^M]=]/].].].].].].].]-]G]E]N^X]B]G]D"
			"]G]D]G]D]G]D]G]@_R_A`J]D]J]A]J]A]J]A]J]:]6g8]K]A]M^?]M^?]M^?]M^?]M^?]M^?]M_?].].].].].].]."
			"].].]L]?]L]?]L]?]L]?]L]?]L]?]L]3^;aP]?]M^?]M^?]M^?]M^;]W]8^M];]W]  H]S]T^B]J^B]J^B]J^B]J^@"
			"\\-]W]  G^1_ :aW[V`BcW[Wc<d5c=]L]>]N]<]P]7]X]8]       F]KZ   X]S]5[S]5\\P]R]N]K_ K]*] 0]  !]"
			",]N]5]5_\"]1],]<]M]9^6^L^0]  Ad Nd A\\R]O^U\\P^I^B]K^?]H[C]H^D].],]G]F]H]C].].]O^=].]P^Q]H]M]"
			"X]A]I]D],]I^E]K]AZH^8]5]J]<]W]5bObJ^O^7]6_0]*]-]   R]M^>^M]?^/]M^?^.]/]M^?]L]?].].]T_9].]M"
			"]M]N]L]?]L]?^M]?]M^?] ]<].]M^;]W]5aRaB^U^6c8_(]4](]  FaSaD]P_=]M]@]P]9]L]@]-]L]9b9]O^=^N^?"
			"\\P_Q]H]T]T]?]P]=]T]T]?^L]@]L]8]4]N]@^I^?]1],^K^A`W]W`C]H]7]8]I]@^&]&^=i=^N^<i H]0^T[3]1l6]"
			"4])_ <\\RbT\\ O]T]#] L\\V\\O]X\\     M^N^6\\U\\   ,]T]-\\OhF\\J]@\\OhQ]/^I^D^I^D^I^D^I^D^I^C]I]B]L]<"
			"]H[C].].].].].].].]-]H]D]M]X]A]I]B]I]B]I]B]I]B]I]@_P_B_J]C]J]A]J]A]J]A]J]:]6].]K]A]M^?]M^?"
			"]M^?]M^?]M^?]M^?]M_?^/^/^/^/^/].].].].]L]?]L]?]L]?]L]?]L]?]L]?]L]3^;`O]?]M^?]M^?]M^?]M^;c8"
			"^M];c  G^U]U^@^M^@^M^@^M^@^M^?\\-c  H^0_ 9^U[U^@aV[Va:b3a<]L]>^P^=^P]7]X]8_       H^M[ F] 6"
			"]S]>ZQ[T^6]P]S^N^K^ K]*] 0]:] 8]0],]O^5]6_2ZI]1]-^<^O^9]4]L]0]<].] Uc Pc1]2\\Q^S`W^P]G]B]K]"
			">^J\\C]I^C].],^H]F]H]C].].]N^>].]C]H]MbA^K^D],^K^D]K^B[I]7]5^L^<c5aMaJ^N]7]6^/]*]-]   R^O_>"
			"_O]=].]O_>].].]O_?]L]?].].]S_:].]M]M]N]L]>]N]>_O]=]O_?] ]<]-]O_;]X^5aRaC^S^6a8_']4](]  D]P"
			"^B^Ra>^N]@]Q]7]N]?^.]L]9a8]N]=^N^?]Q_Q]G]U]U]>]P]=]T]T]?_N]>]N]7]4^P^@]G]@^1]+^M^?mB]H]7]8"
			"^K^?\\%]%\\;g;\\L\\:g G]/]T[3]2n7]4]'^ <\\F\\ M\\S\\  J\\F\\     L^N^6\\U\\   ,\\S\\-]OhG]K]@]OhQ]LZ=]G]"
			"D]G]D]G]D]G]D]G]D]G]D^L]<^J\\C].].].].].].].]-]J_D]MbA^K^B^K^B^K^B^K^B^K^A_N_B^K]B^L^A^L^A^"
			"L^A^L^:]6].]K]A^O_?^O_?^O_?^O_?^O_?^O_?^Oa?].].].].]/].].].]-]N]>]L]>]N]=]N]=]N]=]N]=]N]2^"
			";_O]=]O_>]O_>]O_>]O_:a7_O]9a  E^P_>^P_>^P_>^P_>^P_>\\,a  H^.] /[5]T[S\\8a1`<]L]=^R^<]O^8b7_ "
			"      H^O\\ F] 6\\R\\=[R[U^5\\N]T]L^M` L]*] 0]:] 8]1^+]P]4]7_1[L_1]<ZL^:^Q^8]4^N^>ZM];].] R` P"
			"`.]2]QfXaN]G]B]L^=^L]C]K_B].]+_J]F]H]C].].]M^?].]C]H]La@^M^C]+^M^C]J]B]L^7]4^N^:a4aMaK^M^8"
			"]7^.]*^.]   Q]P`>`Q^=^NZ;^Q`>_LZ>].^Q`?]L]?].].]Q^;].]M]M]N]L]>^P^>`Q^=^Q`?]/ZL];]-^Q`:a4`"
			"P`D^Q^7a8^&]4](]   S]Sb>_P^@]R^7^P^>^MZ<]L]9a9]M]=_P`XZB]Q_Q]G^V]V^>]P]=^U]U^?`P^>^P^6]4]Q"
			"^?]G]A^0]*^O^<i@]H]7]7^M^=Z$]%Z8e9ZKZ7e F]/^U[TZ9]3^V`V^8]4]&^ <\\H\\ K[R[  I\\H\\     K_P`XZ9"
			"\\U\\   ,[R[,\\E\\D\\K]?\\E\\M]O\\=]G]D]G]D]G]D]G]D]G]D]G]D]K];^L]C].].].].].].].]-]K_C]La@^M^@^M^"
			"@^M^@^M^@^M^A_L_C`N^A^N^?^N^?^N^?^N^9]6].]L]?]P`>]P`>]P`>]P`>]P`>]P`>]P]X^LZN^NZ;_LZ>_LZ>_"
			"LZ>_LZ?].].].]-^P^>]L]>^P^=^P^=^P^=^P^=^P^2^:^P^=^Q`>^Q`>^Q`>^Q`:a7`Q^9a  Dk<k<k<k<k>],a  "
			"H]-] /[,[._0_;]L]=j<]N]7`5a       J_S^ F] 6\\R\\=^U[W_5]N^V^K_Rd L],] /]:] 8]1])^T^3]8_0^Q`0"
			"]<]Q_8^S^8^3_R_=]R^:].] O] P]+]1\\PdW`N^G^C]N_;`R`C]NaA].]*`O`F]H]C].].]L^@].]C]H]La?`S`B]*"
			"`S`B]J]B`Q_6]3_R_9a4aMaL^K^9]8^-])].]   Q_Tb>aS^;_R\\:^Sa=`Q]>]-^Sa?]L]?].].]P^<].]M]M]N]L]"
			"=_T_=aS^;^Sa?]/^R_:]-^Sa:a3_P_C^P^7_8^%]4](]   S_V^X^?aS^>]T^5_T_=`R]<]L]8_8]M^>`SdA]SaS]E"
			"^W]W^=]P^=_W]W_>]X]T_<_T_5^4^T^?^G^C^/])^Q^8c=]H]7]6`S` ?] ;c >c E]._W[V\\9]4^J^9]4]%] ;]L]"
			" IZQZ  H]L] !u  ,`Sd9\\U\\   ,ZQZ,]E\\E]L]?]E\\M_S^>^G^F^G^F^G^F^G^F^G^F^G^F^K]:`R`C].].].].]."
			"].].]-]ObB]La?`S`>`S`>`S`>`S`>`S`?]J]CcS`?_R_=_R_=_R_=_R_8]6].]V[R^?_Tb>_Tb>_Tb>_Tb>_Tb>_T"
			"b>_T^V_Q]M_R\\:`Q]=`Q]=`Q]=`Q]?].].].],_T_=]L]=_T_;_T_;_T_;_T_;_T_1^:`T_;^Sa=^Sa=^Sa=^Sa9_6"
			"aS^7_  Bi:i:i:i:i=]+`  I],] /[,[-].]:]L]<h;]N]7`3q      \"h E] 7]S]=k5]LdIjW^ M],] /]:] 8]1"
			"](f9k?n?l/]<j6g7]1j<h9].] LZ PZ(]1]O`U]K]E]Cm8kBn?n?](nE]H]C].].]K^Am>]C]H]K`>kA])kA]J^Cm5"
			"]2j7_2`M`K^J]9]8tC])].]   PgX]>]Xf9h9fX]<k>],fX]?]L]?].].]O^=].]M]M]N]L]<h<]Xf9fX]?]/j9d4g"
			"X]:a3_P_D^O^7_8m4]4](]   RfXaBk=^V^3h;j<]L]8_9^L]>qA^U]W]U^Di<]O`?k=]Xg:h3a7f>uCn?]/eSe;]:"
			"]H]7]5k >] :a <a D]-h>n?\\H\\8]4]%] 9^R^   *^R^  Xu  ,q9\\U\\    /]D\\F]LfH]D\\Li>]E]F]E]F]E]F]E"
			"]F]E]F]E]F]JnIkBn?n?n?n?].].].]-n@]K`>k<k<k<k<k=[H[Co<j;j;j;j7]6].]Vf=gX]=gX]=gX]=gX]=gX]="
			"gX]=gTjLh9k<k<k<k?].].].]+h<]L]<h9h9h9h9h Fk:gX]=gX]=gX]=gX]9_6]Xf6_  @e6e6e6e6e;]+_  G\\+["
			" /].]-[,[9]L];e:^N^8`2p       e D] 7]S]<i4\\JbGgT^ M\\,\\ .]:] 8]1]'d8k?n>i-]<i4e6]0h;g8].]  "
			" I]0]3]E]Cl6h@l=n?]&jC]H]C].].]J^Bm>]C]H]K`<g?]'g?]I]Bj3]1h6_2_K_L^I^:]8tC])].]   OdV]>]Wd"
			"6f8dW]:i>]+dW]?]L]?].].]N^>].]M]M]N]L];f;]Wd7dW]?]/i7c3dV]9_2_P_E^M^8_8m4]4](]   QdV`B]Xe;"
			"d1f8h<]L]8_9]K]>]XdW_@eWeBg;]O`=g;]Vd8f1`6d=uCn?]/eSe;]:]H]7]3g <] 9_ :_ C]+f>n>ZFZ7]4]%] "
			"7f   &f  Vu  ,]XdW_9\\U\\    /\\C\\F\\KfH\\C\\Kg=]E]F]E]F]E]F]E]F]E]F]E]F]JnHh@n?n?n?n?].].].]-l>"
			"]K`<g8g8g8g8g J]Vh:h9h9h9h6]6].]Ve;dV]<dV]<dV]<dV]<dV]<dV]<eRiJf7i:i:i:i?].].].]*f;]L];f7f"
			"7f7f7f F]Xe7dV]<dV]<dV]<dV]9_6]Wd5_  <\\-\\-\\-\\-\\6]+_  FZ*[ /].],Z+Z9]L]8`8]L]7^.m       W` "
			"A] 7\\R\\7b2]H^BaP_ O].] .]:\\ 7]2^%`6k?n:b*]9c/a5],b6b5].\\   H]/\\4]C]Di0b=h9n?]#c?]H]C].].]I"
			"_Dm>]C]H]J_9a<]$d?]I^?c0].b3_2_K_M^G^;]8tC](]/]   M`T]>]U`2b4`U]7c;])`U]?]L]?].].]M^?].]M]"
			"M]N]L]8`8]U`3`U]?],c2a0_T]9_2^N^F^K^8]7m4]4](]   O`R^B]Va8b-`3d:]L]7]9^J]?]V`T]>cUc?c9]N_:"
			"a8]T`3`-_4`<wDn?]/eSe;]:]H]7]0a 9] 8] 8] B])b<n @]4]&^ 5b   \"b  Tu  ,]V`T]8\\U\\    0].].]0b"
			";]C]H]C]H]C]H]C]H]C]H^E^H^JnEb=n?n?n?n?].].].]-h:]J_9a2a2a2a2a G\\Rb4b3b3b3b3]6].]Vc7`T]:`T"
			"]:`T]:`T]:`T]:`T]:aMcEb2c4c4c4c<].].].]'`8]L]8`1`1`1`1` D]Ua2_T]9_T]9_T]9_T]8]5]U`2]      "
			"=]                       &[   O].]  E]  E]         ']    S]        R]      ^       (](]/] "
			"       C]  S]    '] V]      F^ 7]4](]   %])[  4]7] @])_Q_:] 9]6]                6[   S]0[R"
			"^           H]%\\U\\ A\\            @\\             /Z            <\\             ,[    M^5](^ "
			"     =]                       &[   N]0]  D\\  D]         '\\    Q^DZ       1]      _       )"
			"](]/]        D^  S]    '] V]      F] 6]4](]   %]   ;]7] @] /] 9]6]                6[   S]0"
			"g           H]%\\U\\ @\\            @\\                          J\\                  X]4](]   "
			"   <]                       &[   N]0]  D\\  E^         '\\    P^G]       2]      X^       )]"
			"(^0]        D]  R]    '] V]      G^ 6]4](]   %]   ;]7] @] /] 9]6]                6[   S]0e"
			"           F]%\\U\\ ?[            ?[                          I[                  ^4])^     "
			" @ZV]                       &[   M]2]  D]  E]         ']    O_K_       3]      V^       *b"
			",]5b        E^  R]    '] V]      G^ 6^5])^   %]   ;]7] @] /] 9]6]                6[   S].a"
			"           D]%\\U\\ ?\\            @\\                          J\\                 !^4])^     "
			" B\\V]                       &[   M]2]  D\\            G\\    L`P`       2]      U^       +b "
			"=b        RZN^  R^    '] V]      H^ 4^6]*^   $]   ;]7] @] /] 9]6]                6[   S]  "
			"          J]  :\\            @\\                          J\\                 \"^3]*^      A\\V"
			"\\                       %[   L]4]                   Vm       2^      S^       ,b =b       "
			" R\\Q_  R]    &] V]      I^ 3b:].b   $]   ;]7] @] /] 9]6]                6[   S]           "
			" J]  @ZU]            FZU]                          PZU]                 #^2]+^      @b    "
			"                   %[                       Si       4b                       %i  Ua    &]"
			" V]      Mb 2a:].a   #]   ;]7] @] /] 9]6]                   .]            J]  @b          "
			"  Fb                          Pb                 'b2]       E`                            "
			"                   Qb       1a                       $g  S`    %] V]      Ma /_:]._   !]  "
			" ;]7] @] /] 9]6]                   .]            J]  @a            Ea                     "
			"     Oa                 &a1]       D^                                                     "
			"  X^                 Ip      Fc  Q^    #] V]      M_  A]    )]   ;]7] @] /] 9]6]          "
			"                      T]  @`            D`                          N`                 %_/"
			"]       BZ                                                                        Ap      "
			"                 6]                                                                       "
			"                                                                                          "
			"                          p                       6]                                      "
			"                                                                                          "
			"                                                                                          "
			"                                                F]']2]    +]']2^ D]']3_   E]']1]   \"]']2^ "
			"8]                             H";

		// Define a 90x103 font (huge size).
		static const char *const _data_font90x103[] = {
			// Defined as an array to avoid MS compiler limit about constant string (65Kb).
			// First string:
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"            HX     4V         >X       IX           *W             FW                     "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                         HX  W 4Z 3VCT   <Z     >X  W 4Z  "
			" HX  W 4Z     'VCT ;X  W 3Y 2UCT       KX  W 3Y   0W                                      "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                    @W !W 4\\ 5YET ?XHX 8]     >W !W 4\\ 7XGX KW !W 4\\ 7XHX "
			"  +YET :W !W 3[ 5ZFT ?XGX     EW !W 3[ 7XGX 5W                                            "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                              >W \"V 3\\ 7]HU ?XHX 9`     ?W \"V 3\\ 7XGX JW \"V 3\\ 7XHX   -]HU"
			" 9W \"V 3] 7]HT ?XGX     DW \"V 3] 8XGX 5V                                                  "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                        <W $V 3VNV 8_KV ?XHX 9`     >W $V 3VNV 8XGX IW $V 3VNV 8XHX   -_KV"
			" 8W $V 2] 7_KU ?XGX     CW $V 2] 8XGX 6V                                                  "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                        :W &W 4VLV :j >XHX :VJV     >W &W 4VLV 9XGX HW &W 4VLV 9XHX   .j 6"
			"W &W 3VMV 9i >XGX     BW &W 3VMV 9XGX 7W               MW                                 "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                          CV 'W 4VJV ;j >XHX ;UGV     >V 'W 4VJV :XGX GV 'W 4VJV :XHX   .j"
			" 5V 'W 3VKV :i >XGX     AV 'W 3VKV :XGX 8W               N[                               "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                            DV )W 4VHU <VK_ =XHX ;TEU     =V )W 4VHU :XGX FV )W 4VHU :XHX "
			"  /VK_ 3V )W 3VIV <UK_ =XGX     @V )W 3VIV ;XGX 9W               N]                       "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                    DV *V 3UFU =UH\\ <XHX <UDT     <V *V 3UFU ;XGX EV *V 3U"
			"FU ;XHX   /UH\\ 1V *V 2UGU <TH] =XGX     ?V *V 2UGU ;XGX 9V               a                "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                           EV ,V 3UDU >TEY ;XHX <TBT     <V ,V 3UDU <XGX D"
			"V ,V 3UDU <XHX   /TEY /V ,V 2UEU =TFZ <XGX     >V ,V 2UEU <XGX :V               Na        "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                   DU -V 3VDV ?TCV :XHX <TBT     ;U -V 3VD"
			"V =XGX CU -V 3VDV =XHX   /TCV -U -V 2UCU >TCU :XGX     =U -V 2UCU =XGX ;V               NV"
			"IV                                                                          \"W            "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                                              JU /V 3VBV     ETBT     :U /"
			"V 3VBV   FU /V 3VBV       (U /V 2UAU         DU /V 2UAU   @V               NVGV           "
			"                                                               $X                         "
			"                                                                                          "
			"                                            *X                                            "
			"                                                                                          "
			"                           JX                                GTBT                         "
			"                          MX  GX 7V     :UEU     DX  GX 7V   JX  GX 7W       4X  GX 6V    "
			"     GX  GX 5V   (X                            &X                                         "
			"                                                                                          "
			"                            )X                                                     8V     "
			"                                                                                          "
			"            ;X                                FTBT                                        "
			"           LX  IX 7X     <UCU     DX  IX 7X   JX  IX 6W       3X  IX 6X         GX  IX 5X "
			"  *X                            &Y                                                        "
			"                                                                                          "
			"             (X                                                     9V                    "
			"                                                                                       <X "
			"                               ETBT                                                   KX  "
			"KX 6X 1TBT   BTAT     CX  KX 6Y   JX  KX 6Y     (TBT BX  KX 5X 1TBT       LX  KX 4X   +X  "
			"                          %T                                                    #W 9W     "
			"                                                                                          "
			"3a   :a     <W   2W    >W   E\\   AW ,W ,W ,W ,W                             HY GV +Y      "
			"   4Z           NX                 @X                                                     "
			"             %W                                DUDU                                       "
			"          =Y 7W  KW 6Z 4XDT   BTAT     BW  KW 6Z   IW  KW 6[   ,Y )XDT AW  KW 5Z 4XDT     "
			"  KW  KW 4Z   ,W BW                 8V         (S                                         "
			"    <S       9V 7V                                                                        "
			"                       3a   :a     ;W   3W    >W   H_   AW ,W ,W ,W ,W                    "
			"         L] GV +]         ;a          #[                 F^                               "
			"            8XGX                      +W                                BTEU              "
			"                      *R            9a :W  MW 6\\ 6ZET ?XHX <TAT     AW  MW 6\\ 7XGX LW  MW "
			"5[ 7XGX .Y +ZET @W  MW 5\\ 6ZET ?XHX     DW  MW 4\\ 7XHX 0W AW &XHX               MZ        "
			" +T                                   $Y         BS 1W,V MY   8W 7W  T           9X   5Z /"
			"[     0Z   8Z /Y           GY       .\\       <\\               [   4[   :\\              -a "
			"  :a     :W   4W    >W   Ja   AW ,W ,W ,W ,W                             N_ GV +_         "
			"?e   8]       J]                 Jb       8[       <[                  $Y       FY 7XGX   "
			"=Z         Di 5W   8Z .Y !W         FW *Y   4W)V*W)V-Y(V            <UFU   3\\             "
			"       +[ 0[ 0[ 0[ 0[   4[=T            <e ;W  W 5\\ 7\\FT ?XHX <TAT     @W  W 6^ 8XGX KW  W"
			" 5] 8XGX .Z@R ?\\FT ?W  W 4\\ 7\\FT ?XHX     CW  W 3\\ 7XHX 1W @W &XHX               N\\       "
			"  ,T     :U :U5U                            `   EX 2VFV   .S 4]0W\"b DV  V 5V  T         7W"
			" .` 3[ 7c 8d )Z Dq 8b Hy Bb 7`           Na   /Z @k .d Kj ?x Mt 7f MX/X'X -X -X2Z&X -]0]0["
			"3X Dc Ii -c Ij 4f N~W$X/X.X&X.X4Y4XDY/Y/Y,Y'~S%a >W $a  MY   EW   5W    >W   Kb   AW ,W ,W"
			" ,W ,W                            !a GV +a         Ch   =f       ^                 Mf 2Z @"
			"x Mx <c 3X C~Q)X?X?X Kc   2T   .V   .T   CX   $a  !W.W   N` ;XGX ![ Lb       &Z Mi 7[   >a"
			" 5a &W   0g    #\\ -_   <\\*V.\\*V0a-V\"X )Z /Z /Z /Z /Z 4WJV 1~U+d Kx Mx Mx Mx MX -X -X -X ,j"
			" @[3X Dc 8c 8c 8c 8c   <cBV.X/X'X/X'X/X'X/X/Y,Y$X &h ;W \"W 5VNV 8]HU ?XHX <TAT     ?W \"W 5"
			"VNV 8XGX JW \"W 5VMV 9XGX -ZDV @]HU >W \"W 4VNV 8]HU ?XHX     BW \"W 3VNV 8XHX 2W ?W &XHX    "
			"           ^ K~\\       >S   3Q +[ @[;[ ;Q                          ;e   HX 2VFV #VBV FS 6`"
			"1V#g GV !V 3V !T         7W 0d :` ;j ?k -[ Dq :g Ky Df ;d          $f   1Z @o 5j Np Ex Mt "
			":m\"X/X'X -X -X3Z%X -]0]0\\4X Gi Lm 4i Ln ;m#~W$X/X-X(X-X4Y4XCY1Y-Y.Y&~S%a >W $a  N[   EV   "
			"5W    >W   Lc   AW ,W ,W ,W ,W                            \"b GV +a         Dk   Aj      \"_"
			"                 h 3Z @x Mx ?i 6X C~Q)X?X?X Ni   6V   /V   /V   DX   &f  #W0W   e >XGX %c#"
			"e       +b\"i 9_   Be 9d 'V   3k    %^ /c   @^*V0^*V2d.V\"X )Z /Z /Z /Z /Z 3b 1~U.j Nx Mx Mx"
			" Mx MX -X -X -X ,p F\\4X Gi >i >i >i >i   BiEV.X/X'X/X'X/X'X/X.Y.Y#X 'j ;V \"V 5VLV :_IT >XH"
			"X <TAT     >V \"V 5VLV 9XGX IV \"V 4VMV 9XGX ,ZHY A_IT <V \"V 4VLV :_IT >XHX     AV \"V 3VLV 9"
			"XHX 2V >W &XHX              !_ K~[       >T   4R -_ D_?_ >S         =t                Fh  "
			" IX 2VFV #VBV FS 7c4V#i HV \"W 3V !T         7V 0f @e >o Co 0\\ Dq <j Ly Fj ?h          (i  "
			"\\ ?Z @r :o\"s Hx Mt <q$X/X'X -X -X4Z$X -]0]0\\4X Im Np 9m Np ?q%~W$X/X-X(X,W5[6XAX1X+X.X%~S%"
			"a =V $a  ]   EV   6W    >W   Md   AW ,W ,W ,W ,W               HW             1b GV +b    "
			"     Fm   Dm      #`                \"j 4Z @x Mx Am 8X C~Q)X?X?X!m   9X   0V   0X   EX   'h"
			"  $W0W  \"h ?XGX 'g%g       0h%i :a   Cf :f *V   4m    %^ 0e   A^+V/^+V1f1V!X )Z /Z /Z /Z /"
			"Z 2` 1~V0o\"x Mx Mx Mx MX -X -X -X ,t J\\4X Im Bm Bm Bm Bm   FmHV-X/X'X/X'X/X'X/X-X.X\"X (l ;"
			"V $V 4UJU :ULXLU >XHX <UCU     =V $V 5VJV :XGX HV $V 4VKV :XGX +ZL\\ AULXLU ;V $V 3UJU :ULX"
			"LU >XHX     @V $V 2UJU 9XHX 3V =W &XHX              !` K~Z       >T   4S /a FaAa @T       "
			"  @w                Hl   KX 2VFV $WCV ES 8e5V$j HV \"V 1V \"T         7V 2j Eh ?q Dp 1\\ Dq >"
			"l Ly Hn Bj          +l %e E\\ At >s$v Kx Mt >u&X/X'X -X -X5Z#X -^2^0]5X Jo q ;o r Br%~W$X/X"
			"-X(X,X6[6XAY3Y+Y0Y%~S%W 3V  IW !_   FW   7W    >W   Md   AW ,W ,W ,W ,W               HW  "
			"           2[ ?V #[         Hn   En      #`                #l 6\\ Ax Mx Cp 9X C~Q)X?X?X\"o  "
			" ;Z   1V   1Z   FX  KS 0i  #W2W LV ,i ?XGX *l'h       3l'i ;c   Dg ;g ,W   6o    %^ 1g   B"
			"^,V.^,V0g3V X *\\ 1\\ 1\\ 1\\ 1\\ 2^ 0~V2s$x Mx Mx Mx MX -X -X -X ,v L]5X Jo Do Do Do Do   HpKW"
			"-X/X'X/X'X/X'X/X-Y0Y\"X )n <W &W 5VJV ;TI_ >XHX ;UEU     <W &W 5VIV ;XGX HW &W 5VIV ;XGX *g"
			" ?TI_ ;W &W 4VJV ;TI_ >XHX     @W &W 3VJV :XHX 4W =W &XHX     1\\ 1\\ 1\\ 1\\ 1\\ =XMV K~Y     "
			"  =S   4U 1c IdCc AU         Dz                In   LX 2VFV $VBV ES 9g7V$k HV #W 1W #T    "
			"     8W 3l Fh ?r Eq 3] Dq ?m Ly Ip Em          -n )k H\\ Au Av%x Mx Mt ?x(X/X'X -X -X6Z\"X -"
			"^2^0]5X Ls\"s ?s\"s Et%~W$X/X,X*X+X6[6X@Y5Y)Y2Y$~S%W 3W  JW \"a   FW   8W    >W   NZ   6W ,W "
			",W ,W ,W               HW             2X <V  X         H[G[   Go       KZ                %"
			"[H[ 7\\ Ax Mx Ds ;X C~Q)X?X?X$s   >\\   2V   2\\   GX  KS 1j  #W2W LV -j ?XGX +ZEZ)VGY       "
			"5ZDZ)i <e   EUFY <UFX -W   7q    %VMU 2YIY   CVMU,V.VMU,V0UFX3V X *\\ 1\\ 1\\ 1\\ 1\\ 1\\ 0~W4v%"
			"x Mx Mx Mx MX -X -X -X ,x N]5X Ls Hs Hs Hs Hs   LsMW,X/X'X/X'X/X'X/X,Y2Y!X *\\G[ <W (W 4UHU"
			" <UH] =XHX ;VGV     ;W (W 5VHV ;XGX GW (W 4UGU ;XGX )c =UH] 9W (W 3UHU <UH] =XHX     ?W (W"
			" 2UHU :XHX 5W <W &XHX     5c 8c 8c 8c 8c @WKU J~X       >T   5V 2e KfEe CW         G|     "
			"           Jp   MX 2VFV $VBV ES 9XIX8V$l HV #V /V #T         8V 3n Gh ?s Fr 5^ Dq @n Lx Ir"
			" Go          .o -q L^ Bv Cx&z x Mt A{)X/X'X -X -X7Z!X -^2^0^6X Mu#t Au#t Gu%~W$X/X,X*X+X6["
			"6X?X5X'X2X#~S%W 2V  JW #c   FW   9W    >W   NX   4W ,W ,W ,W ,W               HW          "
			"   2W ;V  NW         IZCY   Hp       JY                &ZDZ 9^ Bx Mx Eu <X C~Q)X?X?X%u   @"
			"^   3V   3^   HX  KS 2k  \"W4W KV -ZGW ?XGX -X=X+R@W       8X<X  .XIX   FQ@W <Q@W /W   7dGU"
			"    %QHU 3XEX   DQHU-V-QHU-V/Q@W5V NX +^ 3^ 3^ 3^ 3^ 2\\ 0~W5x&x Mx Mx Mx MX -X -X -X ,z!^6"
			"X Mu Ju Ju Ju Ju   N}+X/X'X/X'X/X'X/X+X2X X +ZBY ;W *W 4UFU =TF\\ =XHX :VIV     9W *W 5VFV "
			"<XGX FW *W 4VGV <XGX (_ :TF\\ 8W *W 3UFU =TF\\ =XHX     >W *W 2UFU ;XHX 6W ;W &XHX     7h =h"
			" =h =h =h DWJV K~X       >T   5W 4g MgFg EY         J~                K]FZ   MX 2VFV $VBV "
			"ES :XGX9V%\\GX HV $W /W 3PATAP         GV 3[H[ Gh ?]F] GZE^ 6^ Dq A]FX Lx I\\F\\ G\\G[        "
			"  /[H] 0u N^ Bw E_D^&{!x Mt B`C_)X/X'X -X -X8Z X -_4_0_7X N^E^$u C^E^$u H^E\\%~W$X/X,Y,Y*W7"
			"]8X>Y7Y'Y4Y#~S%W 2V  JW $e   FV   9W    >W   NW   3W ,W ,W ,W ,W               HW         "
			"    2W ;V  NW         IY@X >X 4[AV       IX                &X@X 9^ Bx Mx F^E^ =X C~Q)X?X?X"
			"&^E^   B`   4V   4`   IX  KS 3\\GW  \"W4W KV .YBT ?XGX .V7V,P=W       :W8W  /VEV   3V +V /V "
			"  7eGU     KU 3WCW   ;U-V$U-V LV5V NX +^ 3^ 3^ 3^ 3^ 3^ 1~W6_D^&x Mx Mx Mx MX -X -X -X ,{\""
			"_7X N^E^ L^E^ L^E^ L^E^ L^E^  !^Ed*X/X'X/X'X/X'X/X+Y4Y X +Y?X ;V *V 4UDU >TEZ <XHX 9a     "
			"7V *V 4UDV =XGX EV *V 4VEV =XGX )] 7TEZ 6V *V 3UDU >TEZ <XHX     =V *V 2UDU <XHX 6V :W &XH"
			"X     9k @k @k @k @k EWJV K~W       >T   5Y 5g MhHi G[         M~Q                L\\AW   M"
			"X 2VFV $VCV DS :WEW:V%ZAU HV $V -V 3RCTCR         HW 4ZDZ H\\LX ?Y?[ HV>\\ 8_ DX )[?T -Y J[B"
			"[ I[CZ          0WAZ 2x ^ BX>^ G]=Z&X=b#X -X '];[)X/X'X -X -X:[ NX -_4_0_7X \\?\\%X@^ E\\?\\%X"
			"?] J[=X =X <X/X+X,X)X8]8X=Y9Y%Y6Y )Y$W 2W  KW %ZMZ   FV   :W    >W   X   3W     4W ,W     "
			"          HW             3X ;V  NX         KY?X Ca 9Y:R       HX                (X>X :VNV "
			"BZ /X '\\?\\ A^ FX0X)X?X?X'\\?\\   Db   5V   5b   JX  KS 3ZBT  !W6W JV .X?R   4V4U HV       ;V"
			"4V  1VCV   4V *U 0V   7fGU     KU 4WAW   <U.V#U.V JU6V MX +^ 3^ 3^ 3^ 3^ 3^ 2XIX F]=Z&X -X"
			" -X -X -X -X -X -X ,X=b$_7X \\?\\ N\\?\\ N\\?\\ N\\?\\ N\\?\\  #\\?`)X/X'X/X'X/X'X/X*Y6Y NX ,Y=W :V ,"
			"V 3UDU >TDX   ;a     6V ,V 4UBU   GV ,V 3UCU   0` 6TDX 4V ,V 2UDU >TDX       >V ,V 1UDU   "
			":V 9W       (o Do Do Do Do GWIU J~V       >T   6Z 6i jIj I\\         N~R                M[="
			"U   MX 2VFV %VBV H] AWCW;V%Y=R HV %W -V 4UETEU         IV 4ZBZ IWGX ?V;[ IS9Z 9VNX DX *Z;R"
			" -X JZ>Y JZ?Y          1U>Z 5`C_#` CX;[ H[7W&X9_$X -X (\\6X)X/X'X -X -X;[ MX -_4_0`8X![;[&X"
			"=[ F[;[&X<[ LZ8U =X <X/X+X,X)X8]8X<X9X#X6X )Z$W 1V  KW &ZKZ   FV   ;W    >W   W   2W     4"
			"W ,W               HW             3W :V  MW         KX=W Cc ;X7P       HX                ("
			"W<W ;WNW BY /X ([;[ Gg JX0X)X?X?X([;[   Fd   6V   6d   KX  KS 4Y>R  !X8X JV /X<P   6V1U IV"
			"       <U0U  2UAU   3U *U 1V   6fGU     KU 4V?V   <U/V\"U/V IU7V LX ,` 5` 5` 5` 5` 5` 3XIX "
			"G[7W&X -X -X -X -X -X -X -X ,X9_%`8X![;[![;[![;[![;[![;[  %[;](X/X'X/X'X/X'X/X)X6X MX ,X;W"
			" :V .V 3UBU ?TBT   7]     3V .V 4VAU   GV .V 3UAU   4d 7TBT 1V .V 2UBU ?TBT       ;V .V 1U"
			"BU   <V 8W       )r Gr Gr Gr Gr IVHR GX+W       =S   5[ 7i!kJk I]        !^               "
			" )Y:T   MX 2VFV %VBV Le EVAV<V$X:P HV %W -W 6WFTFV         IV 4X?Y IRBX ?T7Y IP5Z :VNX DX "
			"+Z8P .Y JY<Y KY=X          1S;Y 6];\\$WNW CX9Z J[4U&X6]%X -X )[2V)X/X'X -X -X<[ LX -XNV6VNX"
			"0`8X\"Z7Z'X;Z HZ7Z'X;Z LY4R =X <X/X*X.X(X8]8X<Y;Y#Y8Y *Z#W 1V  KW 'ZIZ   FV   <W    >W   W "
			"  2W     4W ,W               HW             3W :V  MW         KW<X Dd <W       -W         "
			"       )W;X <WNW AY 0X )Z7Z Jl MX0X)X?X?X)Z7Z   Hf   7V   7f   LX  KS 4X;P   W8W IV /W   \""
			"V.U JV       >U.U  4VAV &V 5U *U 2V   6gGU     KU 5W?W   =U/V\"U/V IU7V LX ,WNW 5WNW 5WNW 5"
			"WNW 5WNW 5WNW 4XHX H[4U&X -X -X -X -X -X -X -X ,X6]&`8X\"Z7Z#Z7Z#Z7Z#Z7Z#Z7Z  'Z8['X/X'X/X'"
			"X/X'X/X)Y8Y MX ,W:W 9V 0V 3U@U     ?[     1V 0V 3U@V   GV 0V 3U?U   8h   1V 0V 2U@U       "
			"  CV 0V 1U@U   >V 7W       *`L` I`L` I`L` I`L` I`L` JV =X,X       >T   6] 9k\"lKl K_       "
			" #\\                'Y8S   MX 2VFV %VBV Nk IVAV=V$X 1V %V +V 6YHTHY -V       EW 5Y>Y :X ?R5"
			"Z .Y ;VMX DX +Y  DX IY<Y LY;X          2Q8Y 8[5[&WNW CX8Y KZ1T&X4\\&X -X *Z.T)X/X'X -X -X=["
			" KX -XNV6VNX0a9X#Z5Z(X:Y IZ5Z(X:Z NY1P =X <X/X*X.X'W9WNV:X:Y=Y!Y:Y *Z\"W 1W  LW (ZGZ      -"
			"W    >W   W   2W     4W ,W               HW             3W :V  MW         KW;W De =W      "
			" -X                *W:W <VLV @Y 1X *Z5Z Mp X0X)X?X?X*Z5Z   Jh   8V   8h   MX  KS 5Y   :X:X"
			" IV /W   #U+T JV       ?U+T  5U?U &V 5U +V     AgGU     KU 5V=V   =U0V!U0V IV8V KX ,WNW 5W"
			"NW 5WNW 5WNW 5WNW 5WNW 4XHX IZ1T&X -X -X -X -X -X -X -X ,X4\\'a9X#Z5Z%Z5Z%Z5Z%Z5Z%Z5Z  )Z5Z"
			"(X/X'X/X'X/X'X/X(Y:Y LX -X:W          !W                    2\\LZ                          "
			"EW       +[@[ K[@[ K[@[ K[@[ K[@[ KV <X-X     /P 0T   7^ 9k\"lLm La        %Z              "
			"  %Z6Q   MX 2VFV %VCV n KWAW>V$X 1V &W +W 5XITIX +V       EV 4X<X :X ?P2Y -X <WMX DX ,Y  C"
			"X JY:Y MX9W          2P7Y :Z0Z(WLW DX7X KY.R&X2Z&X -X *Y+R)X/X'X -X -X>[ JX -XNW8WNX0a9X#Y"
			"3Y(X9Y JY3Y(X9Y NX  LX <X/X*X.X'X:VMV:X9X=X NX:X *Z!W 0V  LW )ZEZ      .W    >W   W   2W  "
			"   4W ,W               HW             3W :V  MW         LX;W Df >W       ,W               "
			" +W8W >WLW @Y 2X +Z3Z!t\"X0X)X?X?X*Y3Y   Kj   9V   9j     AS 5X   8W:W HV /W   #T)T KV     "
			"  @T(T  6U?U &V 5T +V     AhGU     KU 5V=V   =U0V!U0V JV7V   WLW 7WLW 7WLW 7WLW 7WLW 7XNX "
			"6XGX IY.R&X -X -X -X -X -X -X -X ,X2Z'a9X#Y3Y%Y3Y%Y3Y%Y3Y%Y3Y  )Y3Z)X/X'X/X'X/X'X/X'X:X Ki"
			" >W8V                               *XHZ                          FW       ,Z<Z MZ<Z MZ<Z "
			"MZ<Z MZ<Z LV <X.X     .R 2S   7` :k#nMm Mb        &Z                $Y4P   MX 2VFV &VBV!o "
			"KV?V?V#W 0V &V )V 3XKTKX )V       EV 5X:X ;X  X -Y =VLX DX -Y  CY JY:Y NY9X           HX ;"
			"Z-Y)WLW DX7Y MY,Q&X1Z'X -X +Y)Q)X/X'X -X -X?[ IX -XMV8VMX0XNX:X$Y1Y)X9Y KY1Y)X8X NX  LX <X"
			"/X)X0X&X:VMV:X9Y?Y NY<Y *Y W 0V  LW *ZCZ      /W    >W   W   2W     4W ,W               HW"
			"             3W :V  MW         LW:W Dg ?W       ,X                ,W8W >WLW ?Y 3X +Y1Y\"v#X"
			"0X)X?X?X+Y1Y   MYNVNY   :V   :YNVNY     BS 5X   8X<X HV /W   $T?ZBT*c       AT&T  7U?U &V "
			"6U -W     @hGU     KU 6V;V   >U1V U1V KW7V   NWLW 7WLW 7WLW 7WLW 7WLW 7WLW 6XGX JY,Q&X -X "
			"-X -X -X -X -X -X ,X1Z(XNX:X$Y1Y'Y1Y'Y1Y'Y1Y'Y1Y P)P$Y3[)X/X'X/X'X/X'X/X'Y<Y Km BW8W      "
			"                         +UDZ               7P          1W       -Y8Y Y8Y Y8Y Y8Y Y8Y MV ;"
			"W.X     /T 4T   7a ;k#nMn Nc 6P :W4W ?Z ?X6X KY                #Y   0X 2VFV &VBV\"p KV?V?V#"
			"W 0V 'W )W 2XMTMX 'V       FW 5X:X ;X  Y -X >VKX DX -X  BX IX8X NX7W      KP  1P  =X <Y)X+"
			"XLX EX6X NY*P&X0Z(X -X ,Y'P)X/X'X -X -X@Z GX -XMV8VMX0XNX:X%Y/Y*X8X LY/Y*X8Y!X  KX <X/X)X0"
			"X&X:VMV:X8YAY LY>Y *Z W 0W  MW +ZAZ      0W    >W   W   2W     4W ,W               HW     "
			"        3W :V  MW         LW:W DSF[ @X       -X                -X8W ?WJW ?Y 4X ,Y/Y%z%X0X)"
			"X?X?X,Y/Y   YMVMY   ;V   ;YMVMY     CS 5X 5P*Q JW<W GV /W   %TBbET/g       BTGb?T  8U?U &V"
			" 7U 5_     ?hGU     KU 6V;V   >U2V NU2V$_7V   NXLX 9XLX 9XLX 9XLX 9XLX 8WLW 6XGX KY*P&X -X"
			" -X -X -X -X -X -X ,X0Z)XNX:X%Y/Y)Y/Y)Y/Y)Y/Y)Y/Y\"R+R&Y3]*X/X'X/X'X/X'X/X&Y>Y Jp EW:Y     "
			"                          +R@Y               7Q          2W       .XEVFY\"X5Y\"X5Y\"X5Y\"X5Y N"
			"V ;X/X     0V 5T   8c <k#nNo e >^ AW4W ?Z >W6W KY                \"Y   0X 2VFV &VCW#[LSKZ K"
			"V?V@V\"W 0V 'W )W 1XNTNX &V       FW 6Y:Y <X  NX -X ?WKX DX .Y  CY IX8X NX7W      NS  1S  @"
			"X =X&X,WJW EX6X NY /X/Y(X -X ,Y /X/X'X -X -XAZ FX -XMW:WMX0XMX;X%Y/Y*X8Y MY/Y*X8Y!X  KX <X"
			"/X)Y1X%W;WMW;W6XAX JX>X *Z NW 0W  MW ,Z?Z      1W    >W   W   2W     4W ,W               H"
			"W             3W :V  MW         LW:W DPAY ?Y       .W                -W6W @WJW >Y 5X ,X-X&"
			"_MXM_&X0X)X?X?X,Y/Y  !YLVLY   <V   <YLVLY     DS 6Y 6R,R JX>W FV /X   'TCfFT2i       CUGfB"
			"T  9U?U &V 7U 5]     >iGU     KU 6V;V   >U2V NU2V$]5V   NWJW 9WJW 9WJW 9WJW 9WJW 9WJW 8XFX"
			" KY /X -X -X -X -X -X -X -X ,X/Y)XMX;X%Y/Y)Y/Y)Y/Y)Y/Y)Y/Y#T-T'Y3]*X/X'X/X'X/X'X/X%X>X Ir "
			"GW=\\                                GY               9S          3W       /XDVDX$X2X$X2X$X"
			"2X$X2X V ;X0X     0X 7T   8d <k#~`!g Bd DW4W ?[ ?X7W LY                !X   /X 2VFV &VCV#Z"
			"JSGV KV?VAV!W 0V 'V 'V /d $V       FV 5X8X <X  NX -X ?VJX DX .X  BX HX8X Y7X     #V  1V  C"
			"X >X$X-WJW EX6X Y .X.Y)X -X -Y .X/X'X -X -XBZ EX -XLV:VLX0XMX;X&Y-Y+X7X NY-Y+X7X!X  KX <X/"
			"X(X2X$X<VKV<X6YCY JY@Y +Z MW /V  MW -Y;Y    \"Z ;WDX 0Z 2XDW >Z <W !X :WDY     IW ,W  HX8X "
			"MY 3Z *X 3X &X 7] <W             3W :V  MW       ;X :W:W 4Y @[ )\\ (Y   6X     8QEV     :[ "
			"    JW6W @VIW =Y 6X -Y-Y(]JXJ]'X0X)X?X?X-Y-Y  #YKVKY   =V   =YKVKY     IZ 9X 6T.T JW>W FV "
			".X   (TDgFT3j       CTFhDT  9U?U &V 8U 4\\     =iGU     KU 6V;V   >U3V MU3V#\\5V   MWJW 9WJW"
			" 9WJW 9WJW 9WJW 9WJW 8XFX LY .X -X -X -X -X -X -X -X ,X.Y*XMX;X&Y-Y+Y-Y+Y-Y+Y-Y+Y-Y%V/V)Y3"
			"_+X/X'X/X'X/X'X/X%Y@Y Is HW?^ ?Z /Z /Z /Z /Z /Z /Z6Y NZ 0Z /Z /Z /Z         8Y 1Y 3Z /Z /Z"
			" /Z /Z   3ZCV          5WDX       DXCVCW%X0W%X0W%X0W%X0W V :X1X     0X 7T   9f =k#~`\"h Cf "
			"EW4W @\\ ?X8X LX                !Y   /X 2VFV 'VBV#XHSET KV?VAV!W 0V (W 'W .` \"V       GW 5X"
			"8X <X  NX -X @VIX DX .X  BX HX8X X5W     &Y  1Y  FX >W\"W.XJX FX6X X -X.Y)X -X -X -X/X'X -X"
			" -XCZ DX -XLV:VLX0XLX<X&X+X+X7X NX+X+X7X!X  KX <X/X(X2X$X<VKV<X5YEY HYBY +Z LW /W  NW .Y9Y"
			"    'b ?WG^ 7b 9^GW A` Gl 2_GW MWG_ DW ,W ,W8Y MW ,WG^>^4WG_ 9` @WG^ 9^GW MWG\\ ;f Gm <W6W#"
			"X2X#W;X;W5Y7Y#W1X\"u 6W :V  MW       >^BV\"W:W 3X ?^ 0e AWG_ KV.X ?X <W6W   HTG[ K}!WCWCW Ca"
			" 7p&{ NW6W AWHW >Z 7X -X+X)\\HXH\\(X0X)X?X?X-X+X  $YJVJY   >V   >YJVJY     Ma =X 7V0V JW@W E"
			"V .Y   *TEiET5k       DTEiDT  :VAV &V 9U 3_   ;W6W NiGU     KU 6V;V   >U3V MU3V#_8V   NXJX"
			" ;XJX ;XJX ;XJX ;XJX ;XJX :XEX LX -X -X -X -X -X -X -X -X ,X.Y*XLX<X&X+X+X+X+X+X+X+X+X+X&X"
			"1X*X3`+X/X'X/X'X/X'X/X$YBY Ht IW@_ Cb 7b 7b 7b 7b 7b 7b>a'b 7` 5` 5` 5` AW ,W ,W ,W  DY EW"
			"G_ 9` 5` 5` 5` 5` (Z <`GV W6W MW6W MW6W MW6W#W1X NWG^ HW1X     NWBVBW&W.W&WJP:PJW&W4PJW&W."
			"W!V :X2X     0X 6S   8g >k#~`#j Fj GW4W @\\ >W8W LX                 X   .X 2VFV 'VBV$XGSCR "
			"KV?VBV X 1V (W 'W ,\\  V       GW 5X8X <X  NX -X AWIX DX /X  BY HX8X X5W     ([  1[  HX ?W "
			"W/WHW FX6X!Y -X-Y*X -X .Y -X/X'X -X -XDZ CX -XLW<WLX0XKW<X'Y+X+X7X Y+X+X7X!X  KX <X/X'X4X#"
			"X<VKV<X4XFY FXBX *Y KW /W  NW /Y7Y    +g AWIb ;f =bIW De Il 3bIW MWIc FW ,W ,W9Y LW ,WIbBb"
			"6WIc >f CWIb =bIW MWI^ =j Im <W6W\"W2W\"W<Z<W4X7X!W2W!u 6W :V  MW       @bEW\"W:W 2X @c 8j CW"
			"Ic MX0W =W <W6W IW/W\"VI^ L}!WCWCW Ee =t&{ W4W BWHW =Y 7X .X*Y*ZFXFZ(X0X)X?X?X.Y+X  #WIVIW "
			"  =V   =WIVIW     f ?X 8X2X KW@W EV .Z   +SE[GVDS6ZDV       DSDVDXDS  9UAU %V :U 2`   <W6W"
			" NiGU     KU 6V;V   >U4V LU4V\"`:V GX /WHW ;WHW ;WHW ;WHW ;WHW ;WHW :XEX MY -X -X -X -X -X "
			"-X -X -X ,X-Y+XKW<X'Y+X,Y+X,Y+X,Y+X,Y+X'Z3Z,Y4WNY,X/X'X/X'X/X'X/X#XBX Gu JWB\\ Ag <g <g <g "
			"<g <g <gBe+f <e :e :e :e CW ,W ,W ,W  Mc FWIc >f ;f ;f ;f ;f +Z >eJU NW6W MW6W MW6W MW6W\"W"
			"2W MWIb IW2W     NWAVAW(W,W(WJR<RJW(W4RJW(W,W\"V 9W2X     1X 6T   9i ?k#~`#k Hl HW4W @] ?X9"
			"W LW                 NX   .X 2VFV 'VCW$WFSAP KV?VBV NW 1V (V &W *X  MV       GV 5X6X =X  N"
			"X -X AVHX DX /X  BX GX8X X5X     ,^  1^  LX ?W MW0WHW FX6X!X ,X-Y*X -X .X ,X/X'X -X -XEZ B"
			"X -XKV<VKX0XKX=X'Y+Y,X7X Y+Y,X7X!X  KX <X/X'X4X\"W=WKV<W3YGY FYDY +Z KW .V  NW 0Y5Y    /l C"
			"WJe ?j AeJW Eh Kl 5eJW MWJe GW ,W ,W:Y KW ,WJdDd7WJe @h DWJe AeJW MWJ_ ?l Im <W6W\"W2W!W=Z="
			"W2X9X W2W!u 6W :V  MW       BeFV!W;X 1W ?f =k CWJe NY2X =X =W6W JW-W$WI` N}!WCWCW Gi Av&{ "
			"W4W BVGW <Y 8X .X)X+ZEXEZ)X0X)X?X?X.Y+Y  #UHVHU   <V   <UHVHU    !j AX 9Z4Z KWBW DV -Z   -"
			"TFY@RDT8XAV       ETDVBWET  :VCV %V ;V )X   =W6W NiGU     KU 6V;V   >U5V KU5V GX<V FX /WHW"
			" ;WHW ;WHW ;WHW ;WHW ;WHW :WDX MX ,X -X -X -X -X -X -X -X ,X-Y+XKX=X'Y+Y-Y+Y-Y+Y-Y+Y-Y+Y'Z"
			"5Z+Y5WMY,X/X'X/X'X/X'X/X#YDY GX@^ KWCZ Al Al Al Al Al Al AlFh.j ?h =h =h =h EW ,W ,W ,W !g"
			" GWJe @h =h =h =h =h ,Z @hLV NW6W MW6W MW6W MW6W\"W2W MWJe KW2W     W@VAW)W+W)WJT>TKW)W4TKW"
			")W+W\"V 9X3X     2X 5T   :k ?i\"~`$m Jn IW4W A^ ?X:X MW                 NY   .X 2VFV 7~X2XFS"
			" <V?VCV MX 2V )W %W +X  MV       GV 5X6X =X  NX -X BVGX DX /X  BX GX8X X5X LX -X  7a  1a  "
			"X @W KW2XHX GX6X!X ,X,X*X -X .X ,X/X'X -X -XFZ AX -XKV<VKX0XJW=X'X)X,X7X X)X,X7X!X  KX <X/"
			"X'X4X\"X>VIV>X2YIY DYFY +Z JW .V  NW 1Y3Y    1n DWLh Bm ChLW Gk Ll 6hLW MWKg HW ,W ,W;Y JW "
			",WKfGg8WKg Cl FWLh ChLW MWK` @m Im <W6W\"X4X!W=Z=W1X;X NW3X!u 6W :V  MW       CgGV!W;W 0X ?"
			"g Am CWKg [4X >Y =W6W JW-W&YJb }!WCWCW Hk Dx&{ W4W CWFW <Y 9X /Y)X,ZDXDZ*X0X)X?X?X.X)X P #"
			"SGVGS %P 7V 9P0P CSGVGS    !l BX 8ZGWFZ JWCX DV ,Z   .SEW<PCS8V?V .P>P     JSCVAVDS  :WEV "
			"$V <V &W   >W6W NiGU     KU 6V;V BP>P /U5V KU5V EW=V FX 0XHX =XHX =XHX =XHX =XHX =XHX <XDX"
			" MX ,X -X -X -X -X -X -X -X ,X,X+XJW=X'X)X-X)X-X)X-X)X-X)X&Z7Z*X5WKX,X/X'X/X'X/X'X/X\"YFY F"
			"X=[ KWDY @n Cn Cn Cn Cn Cn CnHj1m Bk @k @k @k FW ,W ,W ,W $j GWKg Cl Al Al Al Al .Z Bs MW6"
			"W MW6W MW6W MW6W\"W3X MWLh LW3X     V?V@W*V)W*VJV@VKW*V4VKW*V)W#V 9X4X     2X 4S   :l ?i\"~`"
			"%o Lp JW4W A^ >W:X MW                 NX   -X 2VFV 7~X2WES <V?VDV LX 2V )W %W -\\  V       "
			"HW 5X6X =X  NX .X BWGX DX 0X  BY FX:X NX5X LX -X  :d  1d $Y @V IV2WFW GX6X\"Y ,X,Y+X -X /Y "
			",X/X'X -X -XH[ @X -XKW>WKX0XJX>X(Y)X,X7X!Y)X,X7X!Y  LX <X/X&X6X!X>VIV>X1YKY BXFX +Z IW .W "
			" W 2Y1Y    2o EWMj Dn DjMW Hn Nl 7jMW MWLi IW ,W ,W<Y IW ,WLhIi9WLi En GWMj EjMW MWLa An I"
			"m <W6W!W4W W=Z=W1Y=Y MW4W u 6W :V  MW       DiIV W;W /W =g Cm CWLi![4W =Y =W6W KW+W(ZKd!}!"
			"WCWCW Im Fy&{ W4W CWFW ;Y :X /X'X-YCXCY*X0X)X?X?X/Y)X!R #QFVFQ $R 9V :R1R DQFVFQ    \"n BX "
			"7ZJ\\JZ HWDW CV +[   1TFW.T:W?V /Q?Q     KTCVAWET  :XIX $V =V #U   >W6W NiGU     KU 6V;V BQ"
			"?Q 0U6V JU6V BU>V EX 0WFW =WFW =WFW =WFW =WFW =WFW <XDX NY ,X -X -X -X -X -X -X -X ,X,Y,XJ"
			"X>X(Y)X.Y)X.Y)X.Y)X.Y)X%Z9Z*Y6WJX,X/X'X/X'X/X'X/X!XFX EX;Z LWDX ?o Do Do Do Do Do DoKn4n C"
			"n Cn Cn Cn HW ,W ,W ,W %l HWLi En Cn Cn Cn Cn /Z Cs LW6W MW6W MW6W MW6W!W4W LWMj LW4W     "
			"W?V?V+W(V+WKXBXKV+W5XKV+W(V$W 8W4X     2X 5T   ;n ?g!~_%p LZDZ JW4W A^ >W:W MW            "
			"     MX   -X 2VFV 7~X2WES <WAWDV KX 3V )W %W /` \"V       HV 4X6X =X  Y .X BVFX DX 0X  BX E"
			"X:X NX5X LX -X  <e  /e 'Y @V GV4XFX HX7X!X +X+X+X -X /X +X/X'X -X -XI[ ?X -XJV>VJX0XIW>X(X"
			"'X-X7X!X'X-X7X!Y  LX <X/X&X6X!X>VIV>X1YKY AXHX +Z HW -V  W 3Y/Y    3p FWMk Fo EkMW Io Nl 8"
			"kMW MWMk JW ,W ,W=Y HW ,WMjJj:WMk Gp HWMk GkMW MWMb Bo Im <W6W!W4W W>\\>W0X=X LW5X u 6W :V "
			" MW       EkJV W<X /W >j Fn CWMk\"\\6X =Z >W6W KW+W)[Ke\"}!WCWCW Jo Hz&{ W4W DWDW ;Y ;X /X'X."
			"YBXBY+X0X)X?X?X/X'X#T  HV  IT :V ;T3T :V   CV +o BX 6ZM`MZ GXFX CV *\\   3SFW,S:V>V 0R@R   "
			"  KSBV@VDS  9e #V ?W \"V   ?W6W NiGU     KU 6V;V BR@R 1U6V JU6V BV?V EX 1XFX ?XFX ?XFX ?XFX"
			" ?XFX ?XFW =XCX NX +X -X -X -X -X -X -X -X ,X+X,XIW>X(X'X/X'X/X'X/X'X/X'X%Z;Z)X5VHX-X/X'X/"
			"X'X/X'X/X XHX DX:Y LWEX >p Ep Ep Ep Ep Ep EpMp6o Do Do Do Do HW ,W ,W ,W 'o IWMk Gp Ep Ep "
			"Ep Ep 0Z Ds KW6W MW6W MW6W MW6W!W5X LWMk MW5X     V>V?W,V'W,VKZDYKW,V5YKW,V'W%W 8X5W     2"
			"X 4T   ;o @g ~^%q NY@Y KW4W B` ?X<X MV                 LX   -X 2VFV 7~X2WES ;VAVDV JY 4V )"
			"V $W 1d $V       HV 4X6X =X  X .Y CWFX DXLY =XEX 'Y EY<X MX5X LX -X  ?e  )e +Y ?V:X6V4WDW "
			"HX7X!X +X+X+X -X /X +X/X'X -X -XJ[ >X -XJW@WJX0XIX?X(X'X-X7X!X'X-X8Y Y  MX <X/X%W6X W?WIV>"
			"W/YMY @YJY +Y GW -V  W 4X+X    4YE\\ FWNXG\\ H]EX F\\GXNW J\\F[ GW ,\\GXNW MWNXG[ JW ,W ,W?Z GW"
			" ,WNXH[KXH[:WNXG[ H]H] IWNXG\\ I\\GXNW MWNXFQ C\\CW CW ,W6W!X6X NW?\\?W.X?X JW6W 1X 6W :V  MW "
			"    9X=X\"[IZKW W=Y /W @m H]DV CWNXG[\"\\6W =[ >W6W LW)W*ZJWKY\"}!WCWCW K\\H] J{&{ V3W DWDW :Y "
			"<X /X'X.XAXAX+X0X)X?X?X/X'X$V  IV  JV ;V <V5V ;V   CV ,^MSKW BX 5x EWFW BV ,_   5TFW,S:V?W"
			" 1SAS     LTBV@VDS  9d \"V @W  U   ?W6W NiGU     KU 5V=V ASAS 2U7V IU7V @U@V DX 1WDW ?WDW ?"
			"WDW ?WDW ?WDW ?XFX >XCX NX +X -X -X -X -X -X -X -X ,X+X,XIX?X(X'X/X'X/X'X/X'X/X'X$Z=Z(X6WH"
			"X-X/X'X/X'X/X'X/X YJY DX9Y MWEW =YE\\ EYE\\ EYE\\ EYE\\ EYE\\ EYE\\ EYE]N\\G[7]EX E\\F[ F\\F[ F\\F[ "
			"F\\F[ IW ,W ,W ,W (p IWNXG[ H]H] G]H] G]H] G]H] G]H] 1Z E]H^ JW6W MW6W MW6W MW6W W6W KWNXG\\"
			" MW6W     NV>V>V,V&V,VJZFYIV,V6YIV,V&V%W 7W6X     3X LR:T   ;q @e N~^&s!Y>Y LW4W B` >W<X N"
			"W                $x   FX 2VFV 7~X2WES ;VAVEW IY 5V *W #W 4XNTNX &V       IW 5X5X =X  X .X "
			"CWEX Di AXH_ +X CX<X MX5X LX -X  Be  #e /Z @V<^IUDV5WDW HX8Y!X +X+X+X -X /X +X/X'X -X -XK["
			" =X -XIV@VIX0XHW?X(X'X-X7X!X'X-X8X NZ  NX <X/X%X8X NX@VGV@X.c >XJX +Z GW -W !W 5X)X    5U>"
			"Z G_CZ I[>T FZC_ KZAZ HW -ZB_ M^BZ KW ,W ,W@Z FW ,^CZMVCZ;^BZ IZBZ I_CZ IZC_ M^ 5Y<S CW ,W"
			"6W W6W MW?\\?W.YAY JW6W 2Y 6W :V  MW     ;\\A\\%YDYLV NW>Y .W AXJa IZ<Q C^BZ MX8X =\\ ?W6W LW)"
			"W+YIXJY LW=W JWCWCW LZBZ K]F] ;W >W2W EWDW 9Y =X /X'X/YAXAY,X0X)X?X?X/X'X%X  JV  KX <V =X7"
			"X <V   CV -\\JSHT BX 4v DXHX BV -b   7SEV*S;V?W 2TBT     LSAV@VCS  9b !V AV  MU   ?W6W MhGU"
			"     KU 5V=V ATBT 3U8V HU8V ?UAV CX 1WDW ?WDW ?WDW ?WDW ?WDW ?WDW ?XBX NX +X -X -X -X -X -"
			"X -X -X ,X+X,XHW?X(X'X/X'X/X'X/X'X/X'X#Z?Z'X7WGX-X/X'X/X'X/X'X/X NXJX CX9Y MWFW <U>Z FU>Z "
			"FU>Z FU>Z FU>Z FU>Z FU>eBZ9[>T FZAZ HZAZ HZAZ HZAZ JW ,W ,W ,W )r J^BZ IZBZ GZBZ GZBZ GZBZ"
			" GZBZ 1Z EZB[ JW6W MW6W MW6W MW6W W6W K_CZ MW6W     V=V>V-V%V-VHZHYHV-V6YHV-V%V%W 7X7X    "
			" 4X NU:T   <s Ae N~^'u\"X<X LW4W BWNW >W<W MW                $w   EX   2~X2WES ;WCWEV GY   "
			"9W #W 5XMTMX 'V       IV 4X4X >X !Y 0Y BVDX Dk CXJc -X BX>X LX5Y MX -X  Ee   Le 3Z ?U=bKUC"
			"U6XDX IX9Y X +X+X+X -X /X +X/X'X -X -XL[ <X -XIV@VIX0XHX@X(X'X-X8Y!X'X-X8X N[  X <X/X%X8X "
			"NX@VGV@X.c =XLX +Z FW ,V !W       AR9Y H]?Y KZ:R GY?] LY=Y IW -Y?] M]@Y KW ,W ,WAY DW ,]@X"
			"NV@X;]@Y JY>Y J]?Y KY?] M] 4X8P CW ,W6W X8X MW?\\?W-XAX IW7X 3Y 5W :V  MW     =_C_(YBXLV NW"
			"?Z -W CXC\\ KY ,]@Y LW8X >] ?W6W LW)W,YHWHY MW=W JWCWCW MY>Y L[B[ ;W >W2W FWBW 9Y >X 0X%X0X"
			"@X@X,X0X)X?X?X/X'X&Y  JV  KY =V >Y7Y =V   CV .[HSFR BX 3t BWHW AV .WN\\   9SFV)S;V?W 3UCU  "
			"   LSAV@VCS  7_  V BV  LU   ?W6W MhGU     KU 5W?W AUCU 4U8V HU8V ?UAV CX 2XDX AXDX AXDX AX"
			"DX AXDX AXDX @XBX NX +X -X -X -X -X -X -X -X ,X+X,XHX@X(X'X/X'X/X'X/X'X/X'X\"ZAZ&X8WFX-X/X'"
			"X/X'X/X'X/X MXLX BX8X MWFW <R9Y GR9Y GR9Y GR9Y GR9Y GR9Y GR9a>Y;Z:R GY=Y JY=Y JY=Y JY=Y KW"
			" ,W ,W ,W *]E[ J]@Y JY>Y IY>Y IY>Y IY>Y IY>Y 2Z FY>Y JW6W MW6W MW6W MW6W W7X K]?Y NW7X    "
			" V=V=U-V$U-VGZJYFU-V7YFU-V$U%W 7X8X    &~X/X:T   =t @c L~\\'v\"W:W LW4W CXNX ?X>X MV        "
			"        $x   EX   2~X2WES :VDWEV FZ   :W #W 7XKTKX )V       IV 4X4X >X !X 0Y BWDX Dm FXKf "
			"/Y AYBY KX5Y MX -X  Gd ~X d 5Y ?V>dLUCU6WBW IX;Z Y +X+Y,X -X 0Y +X/X'X -X -XM[ ;X -XIWBWIX"
			"0XGW@X)Y'Y.X8X!Y'Y.X9Y M] #X <X/X$X:X MX@VGV@X-a <YNY ,Z EW ,V !W       AP6X H\\=Y LY7P HY="
			"\\ LX;X IW .Y=\\ M[=X KW ,W ,WBY CW ,[=]=W;[=X KY<Y K\\=Y MY=\\ M\\ 4X *W ,W6W NW8X MW@VNV@W,XC"
			"X GW8W 3Y 4W :V  MW     >aEa)X@XNW NWA[ ,W DW?[ LX +[=X KW:X =] ?W6W MW'W-XGWGX MW=W JWCWC"
			"W MX<Y NZ>Z <W >W2W FWBW 9Z ?X 0X%X0X@X@X,X0X(X@X@X/Y'Y(Y  IV  JY >V ?Y5Y >V   CV .YFSDP B"
			"X 2q @XJX AV /WK[   :SFV)S;V@X 4VDV     LSAV@VCS  6\\  MV CV  KU   ?W6W MhGU     KU 4V?V @V"
			"DV 5U9V GU9V >UBV BX 2WBW AWBW AWBW AWBW AWBW AXDX @XBX Y +X -X -X -X -X -X -X -X ,X+Y-XGW"
			"@X)Y'Y1Y'Y1Y'Y1Y'Y1Y'Y\"ZCZ&Y9WEY.X/X'X/X'X/X'X/X MYNY BX8Y NWFW <P6X GP6X GP6X GP6X GP6X G"
			"P6X GP6_<X;Y7P GX;X JX;X JX;X JX;X KW ,W ,W ,W *Z?Y K[=X KY<Y KY<Y KY<Y KY<Y KY<Y 3Z GY<Y "
			"KW6W MW6W MW6W MW6W NW8W J\\=Y NW8W     NV=V=V.V$V.VFZLYEV.V8YEV.V$V&W 6W8X    &~X2\\<T   =v"
			" Ab K~\\(x$W8W MW4W CXNX ?X>X NW                $w   DX   $VBV#XFS :WFXEV H]   ;W #W 9XITIX"
			" +V       JW 4X4X >X \"Y 3[ BWCX Dn GXLi 1X ?ZFZ JY7Z MX -X  Je M~X Me 9Y >U?gMUCV7WBW IX>\\"
			" NX *X*X,X -X 0X *X/X'X -X -XNZ 9X -XHVBVHX0XGXAX)X%X.X9Y!X%X.X:Y La 'X <X/X$X:X LWAWGV@W+"
			"_ :XNX ,Z DW ,W \"W       &W H[;X MY .X;[ MX9X JW .X;[ M[<X LW ,W ,WCY BW ,Z<\\<X<[<X LX:X K"
			"[;X MX;[ M[ 3W )W ,W6W NW8W KWAVNVAW*XEX FW9X 4Y 3W :V  MW     ?cGc+Y?WNV MWD] +W DV=Z LX "
			"+Z;X LW:X >_ @W6W MW'W.YGWFX NW=W JWCWCW NX:X NY<Y <W >W2W FWBW 8Z @X 0X%X0X@X@X,X0X(X@X@X"
			"/X%X)Y  HV  IY ?V @Y3Y ?V   CV /YES 6X 1\\H[ JcJc LV 0WI\\   =TFV)S;WAX 5WEW     MTAVAWCS  3"
			"W 4~W.W  KV   ?W6W LgGU     KU 4WAW @WEW 6U9V GU9V ?VBV BX 2WBW AWBW AWBW AWBW AWBW AWBW A"
			"XAX X *X -X -X -X -X -X -X -X ,X*X-XGXAX)X%X1X%X1X%X1X%X1X%X!ZEZ%X9WCX.X/X'X/X'X/X'X/X LXN"
			"X AX7X NWFW !W ,W ,W ,W ,W ,W ,]:X=Y .X9X LX9X LX9X LX9X LW ,W ,W ,W +Z=X K[<X LX:X KX:X K"
			"X:X KX:X KX:X 3Z GX<Z KW6W MW6W MW6W MW6W NW9X J[;X NW9X     NU<V=V.U#V.UDZNYDV.U8YDV.U#V&"
			"V 5X9W    %~X3]<T   >x A` J~\\(y%W8W MW4W CXMW >W>W MV                $x   DX   $VCV\"XFS 9X"
			"IXEV H_   <W #W ;YHTHY -V       JV 3X4X >X #Y ?g AVBX Do HXMk 3Y >l HX7Z MX -X  Me J~X Je "
			"=Y >V?hNUBU8XBX Ju MX *X*X,w Lq IX *~R'X -X -c 8X -XHVBVHX0XFWAX)X%X.X9Y!X%X.X;Z Ke ,X <X/"
			"X$X:X LXBVEVBX+_ 9` +Y CW +V \"W       %W IZ9X NX .X9Z MW7W JW /X9Z MZ;X LW ,W ,WDY AW ,Z;["
			";W<Z;X MY:Y LZ9X X9Z MZ 2W )W ,W6W NX:X KWAVNVAW*YGY EW:W 4Z 3W :V  MW     ?XMYIe,X>WNV MW"
			"Ib +W EW;Y MW *Z;X KV:W =_ @W6W NW%W/XFWFX NW=W JWCWCW NW8X!Y:Y =W >| GW@W 8Y @X 0X%X1Y@X@"
			"Y-X0X(X@X@X/XImIX*Y  GV  HY @V AY1Y @V   CV /XDS 6X 0YDY JdLd LV 1WF[   >SFV'S<WBY 6XFX   "
			"  MS@VAVAS    @~W/W  JU   >W6W LgGU     KU 3WCW ?XFX 7U:V FU:V >UBV AX 3XBX CXBX CXBX CXBX"
			" CXBX CXBX BXAw?X *w Lw Lw Lw LX -X -X -X ,X*X-XFWAX)X%X1X%X1X%X1X%X1X%X ZGZ$X:WBX.X/X'X/X"
			"'X/X'X/X K` @X7X NWFW  W ,W ,W ,W ,W ,W ,[8W=X -W7W LW7W LW7W LW7W LW ,W ,W ,W ,Y:X LZ;X M"
			"Y:Y MY:Y MY:Y MY:Y MY:Y  \"Y=\\ LW6W MW6W MW6W MW6W MW:W IZ9X NW:W     NV<V=V/V#V/VCcCV/V9YC"
			"V/V=X>V&V 4W:X    %~X2TNV<S   =y KWM^LW$~Z({&W7V MW4W CWLX ?X?W MV                 KX   ,X"
			"   %VBV!XGS 9gFV Ha   >W \"W ;WFTFW -V       JV 3X4X >X #Y ?f AWBX Dp IXNm 4X <j GX7Z MX -X"
			" !e G~X Ge AY =U?ZH^BU8W@W Jt LX *X*X,w Lq IX *~R'X -X -b 7X -XHWDWHX0XFXBX)X%X.X:Y X%X.X<"
			"Z Ih 0X <X/X#X<X KXBVEVBX*] 8` ,Z CW +V \"W       %W IZ9X X -X9Z NX7X KW /X9Z MY9W LW ,W ,W"
			"EY @W ,Y:Z:W<Y9W MX8X LZ9X X9Z MY 1W )W ,W6W MW:W JWAVNVAW)XGX DW:W 4Y 3X :V  MW     @VHXK"
			"WGV,W<^ MWIa *W FW9Y NW *Y9W KW<X >` @W6W NW%W/WEWEW NW=W JWCWCW X8X!X8X =W >| GW@W 7Y AX "
			"0X%X1X?X?X-X0X(X@X@X/XImIX+Y  FV  GY AV BY/Y AV   DX 1XCS 6X 0W@X KdLd LV 1VCZ   ?SFV'S;WE"
			"[ 7XFX G~X  .S@VBWAS    @~W0W .P>W   >W6W KfGU     KU 3XEX >XFX 8U;V:W3U;VCZ9P>WCV:W/Y 3W@"
			"W CW@W CW@W CW@W CW@W CXBX CX@w?X *w Lw Lw Lw LX -X -X -X 5p9X-XFXBX)X%X1X%X1X%X1X%X1X%X N"
			"ZIZ#X:VAX.X/X'X/X'X/X'X/X K` @X7X NWFW  W ,W ,W ,W ,W ,W ,[8X?X -X7X NX7X NX7X NX7X MW ,W "
			",W ,W ,X9X LY9W MX8X MX8X MX8X MX8X MX8X  \"X=] LW6W MW6W MW6W MW6W MW:W IZ9X NW:W     NVLu"
			"KU/VLuKU/VBaAU/V:YAU/V=X=U&V 4X;X    %~X2RLW>T   >{!z'~Z)}(W6W NW4W DXLX ?X@X MV          "
			"       KX   ,X   %VBV!YHS 8eEV Ic   ?W !W ;UETEU ,V       KW 3X4X >X $Y >c ?WAX DWD^ JbG] "
			"5X 9d DY9[ MX -X #d D~X Dd DY <U@YD\\BU9X@X Kq IX *X*X,w Lq IX *~R'X -X -a 6X -XGVDVGX0XEWB"
			"X)X%X.X;Z X%X.X?\\ Gk 4X <X/X#X<X KXBVEVBX)[ 6^ ,Z BW +W #W       %W IY7W X -W7Y NW5W KW 0X"
			"7Y MY9W LW ,W ,WFY ?W ,Y:Z:W<Y9W MW6W LY7W W7Y MY 1W )W ,W6W MW:W JWBVLVBW(XIX CW;X 5Y 2X "
			":V  MX     BUDVKVDU.X<] LWI_ :WEW FV7X NW *Y9W JV<X >a AW6W NW%W0XEWEX W=W JWCWCW W6W!X8X "
			"=W >| HX@X 7Y BX 0X%X1X?X?X-X0X(X@X@X/XImIX,Y  EV  FY BV CY-Y BV   DX 1XCS 6X 1W>W KeNe LV"
			" 1VB[   ASFV'S;YI] 9YGY F~X  .S@VDX@S    @~W1V ,TEZ   >W6W JeGU IX   +U 2YIY <YGY :U;V:W3U"
			";VGa<TEZCV:W/X 3X@X EX@X EX@X EX@X EX@X EX@X DX@w?X *w Lw Lw Lw LX -X -X -X 5p9X-XEWBX)X%X"
			"1X%X1X%X1X%X1X%X MZKZ\"X;WAX.X/X'X/X'X/X'X/X J^ ?X7X NWFX !W ,W ,W ,W ,W ,W ,Z6W?X -W5W NW5"
			"W NW5W NW5W MW ,W ,W ,W -X7W LY9W MW6W MW6W MW6W MW6W MW6W  \"W=^ LW6W MW6W MW6W MW6W MW;X "
			"IY7W NW;X     NVLuKU/VLuKU/VA_@U/V;Y@U/V=X=U&V 4X<X    $~X,W>T   ?|\"}(~X)~(W6W NW4W DXKW >"
			"W@X MV                 KX   ,X   %VBV!ZIS 7cEV IYNZ8W  0W !W :RCTCR +V       KW 3X4X >X %Y"
			" =b >V@X DS=\\ K`C[ 6Y 8b BX9[     Nd A~X Ad HY <VAX@ZBV:X?W Kq IX *X*X,w Lq IX *~R'X -X -a"
			" 6X -XGVDVGX0XEXCX)X%X.X=[ NX%X.u Fl 6X <X/X\"W<W IWCWEVBW([ 5\\ ,Z AW +W #W       $V IY7X\"X"
			" -X7Y NW5W KW 0X7Y MX8X MW ,W ,WHZ >W ,X8X8W=X8X X6X MY7X\"X7Y MX 0W )W ,W6W MX<X IWCVLVCW&"
			"XKX AW<W 5Y 1W 9V  LW  4P  /TBVMVBT.X;\\ LWI` =\\HW GW7X NW *X8X KV=X >XMW AW6W NW%W0XEWDW W"
			"=W JWCWCW!X6X#X6X >W >| HW>W 6Y CX 0X%X1X?X?X-X0X'XAXAX.XImIX-Y  DV  EY CV DY+Y CV   DX 2X"
			"BS 6X 1V<V KeNe LV 2V?Y   ASFV'S:dNV :XFY E~X  .S@i?S    @~W2i >h   =W6W JeGU IX   4g :g :"
			"YFX DgEV:X<gEVHe>hCV:X/X 3X?W EX?W EX?W EX?W EX?W EX@X EX?w?X *w Lw Lw Lw LX -X -X -X 5p9X"
			"-XEXCX)X%X1X%X1X%X1X%X1X%X LZMZ!X<W@X.X/X'X/X'X/X'X/X I\\ >X7X NWFY !V +V +V +V +V +V +Y6W@"
			"X ,W5W NW5W NW5W NW5W MW ,W ,W ,W -X7X MX8X X6X X6X X6X X6X X6X  $X=_ MW6W MW6W MW6W MW6W "
			"LW<W HY7X NW<W     MVLuKU/VLuKU/V@]?U/V<Y?U/V=X=U&V 3W<X    $~X+V>S   >}%~R)~V(~P)W6W NW4W"
			" DWJX ?XAW L~^               $X   ,X   %VCV N\\LS 6aDVAW0XLZ9W  0W !W :PATAP +V       KV 2X"
			"4X >X &Z =e BW@X DP8[ L^?Z 7X :h EY;\\    \"d >~X ?e LY ;U@W>YAU:W>W Ks KX *X*X,w Lq IX6f+~R"
			"'X -X -b 7X -XGWFWGX0XDWCX)X%X.X@^ NX%X.s Bl 8X <X/X\"X>X IXDVCVDX)[ 4\\ -Z @W *V #W       $"
			"W JX5W\"X -W5X W4W KW 0W5X MX7W MW ,W ,WIZ =W ,X8X8W=X7W W4W MX5W\"W5X MX 0X *W ,W6W LW<W HW"
			"CVLVCW&YMY AW=X 6Y 1X 9V  LX 1X.Q  /TA]AU/W:\\ LWIb A`JW GV5X NW +X7W KW>X >XMX BW6W W#W1WD"
			"WDW W=W JWCWCW!W4W#X6X >W >| HW>W 7Y BX 0X%X1X?X?X-X0X'XAXAX.XImIX.Y  CV  DY DV EY)Y DV   "
			"DX 2XBS 6X 2W<W =^ =V 2V>Y   BSFV'S9bMV ;XFY D~X  .S@h>S    @~W2i >g   <W6W HcGU IX   4g 9"
			"e 8YFX EgEV;Y<gEVHf?gBV;Y0Y 3W>W EW>W EW>W EW>W EW>W EW>W EX?w?X *w Lw Lw Lw LX -X -X -X 5"
			"p9X-XDWCX)X%X1X%X1X%X1X%X1X%X Ke X=W?X.X/X'X/X'X/X'X/X I\\ >X7X NWEY \"W ,W ,W ,W ,W ,W ,X5W"
			"@X -W4W W4W W4W W4W MW ,W ,W ,W -W6X MX7W W4W W4W W4W W4W W4W  $W=VMW MW6W MW6W MW6W MW6W "
			"LW=X HX5W NW=X     MVLuKU/VLuKU/V?[>U/V=Y>U/V=X=U&V 3X=W     7X FW@T   ?~&~T*~V)~R*W5V NW4"
			"W EXJX ?XBX L~^               $X   ,X   &VBV Mb 4]CVC]4XJZ:W  0W !W +T  KV       KV 2X4X >"
			"X 'Z <g EW?X +Z L]=Z 9Y <l GZ=]    %e    e!Y :UAW<XAU;X>X Lu MX *X*X,w Lq IX6f+~R'X -X -c "
			"8X -XFVFVFX0XDXDX)X%X.u MX%X.r ?l :X <X/X\"X>X IXDVCVDX)\\ 4Z ,Y ?W *V #W       $W JX5W\"W ,W"
			"5X W3W LW 0W5X MX7W MW ,W ,WJY ;W ,X8X8W=X7W W4W MX5W\"W5X MX 0X *W ,W6W LW<W HWCVKUCW%XMX "
			"?W>W 6Y 0X 9V  LX 5`3R  0T?[?T/W:[ KWId DbKW HW5X NW +X7W JV>W =WLX BW6W W#W1WDWDW W=W JWC"
			"WCW!W4W#W4W >W >| IX>X 9Y AX 0X%X1X?X?X-X0X'XAXAX.XImIX/Y  BV  CY EV FY'Y EV   DX 2WAS ?r "
			"CV:V =^ =V 2V=Y   CSFV'S8`LV <XFX B~X  .S@e;S    @~W2i >e   :W6W GbGU IX   4g 8c 5XFX FgFV"
			":Y<gFVGg@eAV:Y1Y 3X>X GX>X GX>X GX>X GX>X GX>X FX?w?X *w Lw Lw Lw LX -X -X -X 5p9X-XDXDX)X"
			"%X1X%X1X%X1X%X1X%X Jc NX>W>X.X/X'X/X'X/X'X/X HZ =X7X NWEZ #W ,W ,W ,W ,W ,W ,X4WAW ,W3W!W3"
			"W!W3W!W3W NW ,W ,W ,W .X5W MX7W W4W W4W W4W W4W W4W  $W>VLW MW6W MW6W MW6W MW6W KW>W GX5W "
			"MW>W     LVLuKU/VLuKU/V>Z>U/V>Y=U/V=X=U&V 2W>X     8Y FW@T   ?~P(~V*~T(~Q)V4V NW4W EXJX >W"
			"BX L~^               $X   ,X   &VBV Ld 4WAVD`6XHZ;W  0W !W +T  KV       LW 2X4X >X 'Y ;i G"
			"V>X *Z M\\;Y 9X =p HZ?^    'd    Id$Y 9UAW<XAU;W<W Lw X *X*X,w Lq IX6f+~R'X -X -d 9X -XFVFV"
			"FX0XCWDX)X%X.t LX%X.p ;k ;X <X/X!X@X HXDVCVDX*^ 4X ,Z ?W *W $W       $W JX5W\"W ,W5X W3W LW"
			" 0W5X MW6W MW ,W ,WKY :W ,W7W7W=W6W W4W MX5W\"W5X MX /Y ,W ,W6W LX>X GWEVJVEW#a >W>W 7Y 1Y "
			"8V  KY 9e8T  0T?Z>T0X:[ KWIf GdLW HW4W MW ,W6W JV?X >XKW BW6W W#W2XDWDX!W=W JWCWCW!W4W#W4W"
			" >W >| IW<W :Y @X 0X%X1X?X?X-X0X&XBXBX-XImIX0Y  AV  BY FV GY%Y FV   DX 2WAS ?r DW:W =\\ <V "
			"2V;W   CSFV'S7]JV =XFX A~X  .S@d:S    (V Ii <a   8W6W FaGU IX   4g 6_ 2XFX GgGV:Z<gGVFUFY?"
			"a@V:Z2Y 2W<W GW<W GW<W GW<W GW<W GX>X GX>w?X *w Lw Lw Lw LX -X -X -X 5p9X-XCWDX)X%X1X%X1X%",

			// Second string:
			"X1X%X1X%X Ia MX?W=X.X/X'X/X'X/X'X/X GX <X7X NWDZ $W ,W ,W ,W ,W ,W ,X4WAW ,W3W!W3W!W3W!W3W"
			" NW ,W ,W ,W .W4W MW6W W4W W4W W4W W4W W4W  $W?VKW MW6W MW6W MW6W MW6W KW>W GX5W MW>W     "
			"LVLuKU/VLuKU/V?\\?U/V?Y<U/V=X=U&V 2W>X     8X DWBT   ?~Q)~W)~R&~(V4V NW4W EWHW >WBW K~^    "
			"           $X   ,X   &VBV Kg \"VEc8WFZ=W  /W !W +T 4~W      5V 1X4X >X (Y -] IW>X )Y M[9X 9"
			"X >\\F\\ H[C`    'a    Ca$Y 9UAV:WAU;W<W LX<\\!X *X*X,X -X 0X6f+X/X'X -X -XN[ :X -XEVHVEX0XCX"
			"EX)X%X.s KX%X.o 6h <X <X/X!X@X GWDVCVDW*_ 4X -Z >W )V $W       6i JX5X$X -X5X V2W LW 1W3W "
			"MW6W MW ,W ,WLY 9W ,W7W7W=W6W!X4X NX5X$X5X MW .[ .W ,W6W KW>W FWEVJVEW#a >W?X 8Z 4\\ 8V  K["
			" =i<V  0S=Y=S0X:[ KW@^ IfMW HW4W MY .W6W JW@W =XKX CW6W W#W2WCWCW!W=W JWCWCW\"X4X%X4X ?W >W"
			"2W IW<W :Y @X 0X%X1X?X?X-X0X&XBXBX-X%X1~` GV H~` GV H~` GV   DX 3XAS ?r DV8V =\\ <V 2V;X   "
			"DSFV'S4W /XFX @~X  .S@VIX;S    (V Ii 8Z   5W6W D_GU IX   4g 3Y .XFX HgGV;TNU<gGVFQ@W;Z=V;T"
			"NU3Y 1W<W GW<W GW<W GW<W GW<W GW<W GX>X X *X -X -X -X -X -X -X -X ,X*X-XCXEX)X%X1X%X1X%X1X"
			"%X1X%X H_ LX@W<X.X/X'X/X'X/X'X/X GX <X7X NWD\\ 8i >i >i >i >i >i >i3WBX ,V2W!V2W!V2W!V2W NW"
			" ,W ,W ,W .W4W MW6W!X4X\"X4X\"X4X\"X4X\"X4X M~Y2X@VIW NW6W MW6W MW6W MW6W KW?X GX5X NW?X     L"
			"VLuKU/VLuKU/V@^@U/V@Y;U/V=X=U&V 2X?W     8X CWBT   ?~R*~X)~Q%}(V4W W4W FXHX ?XDX K~^      "
			"         $X   ,X   'WCV Ii &VEe:XEZ>W  /W !W +T 4~W      5V 1X4X >X )Y )[ KW=X (Y N[9Y ;Y "
			"?Z@Z I]Gb    '^    =^$X 9U@V:WAU<X<X MX9Z\"X *X*X,X -X 0X6f+X/X'X -X -XM[ ;X -XEVHVEX0XBWEX"
			")X%X.r JX%X.q 4e =X <X/X!X@X GXFVAVFX*` 5X .Z =W )V $W       :m JW3W$W ,W3W!W2W LW 1W3W MW"
			"6W MW ,W ,WMY 8W ,W7W7W=W6W!W2W NW3W$W3W MW -^ 2W ,W6W KX@X FWEVJVEW\"_ <W@W 7Y :b 7V  Jb F"
			"mAX  0S<W<S0W8Y JW<[ KYHVMV GV3X MZ 0W6W IVAX >XIW CW6W!W!W3WCWCW!W=W JWCWCW\"W2W%W3X ?W >W"
			"2W JW;X <Y ?X 0X&Y1X?X?X-X0X&YCXCY-X%X2~a GV H~a HV I~b HV   DX 3W@S ?r DV8V <Z ;V 2W;W   "
			"DSFV'S  <XFX  =V  .S@VGW<S    (V      \"W6W A\\GU IX       2XFX *V;TMU LV2V V;TMU4Z 2X<X IX<"
			"X IX<X IX<X IX<X IX<X IX=X X *X -X -X -X -X -X -X -X ,X*X-XBWEX)X%X1X%X1X%X1X%X1X%X G] KX@"
			"V;X.X/X'X/X'X/X'X/X GX <X8Y NWC\\ =m Bm Bm Bm Bm Bm Bm3WBW ,W2W\"W2W\"W2W\"W2W NW ,W ,W ,W /X4"
			"X NW6W!W2W\"W2W\"W2W\"W2W\"W2W M~Y2W@VHW NW6W MW6W MW6W MW6W JW@W FW3W MW@W     KVLuKU/VLuKU/V"
			"A`AU/VAY:U/V=X=U&V 1W@X     9X BWBS   >~R+~Z*~P#{'V4W W4W FXHX ?XDX K~^               $X  "
			" ,X   'VBV Gi (VFg;WCZ?W  /W !W +T 4~W      6W 1X4X >X *Y &Z LW=X (Y NZ7X ;X ?Z>Z ImNX    "
			"'[    8\\%Y 9UAW:WAU<W:W MX7Y#X *X*X,X -X 0X6f+X/X'X -X -XL[ <X -XEWJWEX0XBXFX)X%X.p HX%X.r"
			" 0a >X <X/X XBX FXFVAVFX+b 6X /Z <W )W %W       =p JW3W$W ,W3W!| LW 1W3W MW6W MW ,W ,WNY 7"
			"W ,W7W7W=W6W!W2W NW3W$W3W MW -b 6W ,W6W JW@W EWFVHVFW!] ;WAX 8Y 9` 5V  H` HrG[  0S<W<S0W8Y"
			" JW:Y KXF^ HW2W Kc ;W6W IVAX >XIW CW6W!W!W3WCWCW!W=W JWCWCW\"W2W%W2W ?W >W2W JW:W =Y >X 0Y'"
			"X0X?X?X-X0X%XCXCX,X%X2~a GV H~a HV I~b HV   DX 3W@S ?r DV8V <Z   FW;W   DSFV'S  =XFX  <V  "
			".S@VFW=S    (V      \"W6W <WGU IX       1XFX +V;SLU LV2V V;SLU5Z 1W:W IW:W IW:W IW:W IW:W I"
			"X<X IX=X X *X -X -X -X -X -X -X -X ,X*X-XBXFX)X%X1X%X1X%X1X%X1X%X F[ JXAW;X.X/X'X/X'X/X'X/"
			"X GX <X8X MWB] Bp Ep Ep Ep Ep Ep E~eBW ,|\"|\"|\"| NW ,W ,W ,W /W2W NW6W!W2W\"W2W\"W2W\"W2W\"W2W "
			"M~Y2WAWHW NW6W MW6W MW6W MW6W JWAX FW3W MWAX     KV<V=V/V#V/VBbCV/VBY:V/V=X>V&V 1XAW     9"
			"X @WDT   ?~S+~Z)}!y'W4W W4W FWFW >WDW J~^               *r   ?V   &VBV Eh *VEXIX<XBZ@W  /W"
			" !W +T 4~W  5f   8V 0X4X >X +Y $Z NW<X 'X NZ7X ;X ?X:X HkMX    '[    7[%X 8UAV8VAU=X:X NX6"
			"X#X *X*X,X -X 0X6f+X/X'X -X -XK[ =X -XDVJVDX0XAWFX)X%X.m EX%X.XA\\ -^ ?X <X/X XBX FXFVAVFX,"
			"c 6X /Y ;W (V %W       ?r JW3W$W ,W3W!| LW 1W3W MW6W MW ,W ,a 6W ,W7W7W=W6W!W2W NW3W$W3W M"
			"W ,e :W ,W6W JW@W DWGVHVGW N[ 9WBW 8Y 8^ 3V  F^ I~X  0S;U;T1W8Y JW8X MXC\\ HW2W Ia ;W6W IWB"
			"W >XHX DW6W!W<W<W3WCWCW!W=W JWCWCW\"W2W%W2W ?W >W2W KX:X ?Y =X /X'X0Y@X@Y-X0X%YDXDY,X%X2~a "
			"GV H~a HV I~b HV   DX 3W@S ?r DV8V ;X   DW;V   DSFV'S  >XFX  ;V  .S@VFW=S    (V      \"W6W "
			":UGU IX       0XFX -V;TLU MV0U!V;TLU6Y 0X:X KX:X KX:X KX:X KX:X KX:X JW<X X *X -X -X -X -X"
			" -X -X -X ,X*X-XAWFX)X%X1X%X1X%X1X%X1X%X F[ JXBW:X.X/X'X/X'X/X'X/X GX <X9Y MWA] Er Gr Gr G"
			"r Gr Gr G~gBW ,|\"|\"|\"| NW ,W ,W ,W /W2W NW6W!W2W\"W2W\"W2W\"W2W\"W2W M~Y2WBWGW NW6W MW6W MW6W "
			"MW6W IWBW EW3W LWBW     IU<V=V.U#V.UCdDV.UCY9V.U=X>V&V 1XBX     :X ?WDT   ?~S,~[({ x&W4W W"
			"4W FWFX ?XFX JV                \"q   >V   &VBV Af -VEXGX=W@ZBW  .W !W +T 4~W  5f   8V 0X4X "
			">X ,Y \"Y W;X 'X NZ7X <Y @Y:Y HiLX    '^    =^%X 8UAV8VAU=X:X NX5X$X *X*X,X -X 0X(X+X/X'X -"
			"X -XJ[ >X -XDVJVDX0XAXGX)X%X.i AX%X.X>Z ,\\ ?X <X/X NWBW DWFVAVFW+XMY 7X 0Z ;W (V %W       "
			"@s JW3W$W ,W3W!| LW 1W3W MW6W MW ,W ,` 5W ,W7W7W=W6W!W2W NW3W$W3W MW +g =W ,W6W JXBX DWGVH"
			"VGW N[ 9WBW 9Y 7^ 3V  F^ I[Gr  /S;U;T1W8X IW7X NWA[ HW2W F^ ;W6W HVCX >XGW DW6W!W<W<W3WCWC"
			"W!W=W JWCWCW\"W2W%W2W ?W >W2W KW9X ?Y =X /X'X/X@X@X,X0X$YEXEY+X%X2~a GV H~a HV I~b HV   DX "
			"3W@S 6X 3V8V ;X   DX<V   DTFV)T  >WEW  :V  .TAVEW?T    (V      \"W6W :UGU IX       /WEW .V;"
			"TKU NV/U\"V;TKU7Y /X:X KX:X KX:X KX:X KX:X KX:X KX<X X *X -X -X -X -X -X -X -X ,X*X-XAXGX)X"
			"%X1X%X1X%X1X%X1X%X G] KXCW9X.X/X'X/X'X/X'X/X GX <X9Y MW?] Hs Hs Hs Hs Hs Hs H~hBW ,|\"|\"|\"|"
			" NW ,W ,W ,W /W2W NW6W!W2W\"W2W\"W2W\"W2W\"W2W M~Y2WBVFW NW6W MW6W MW6W MW6W IWBW EW3W LWBW   "
			"  IU<V=V.U#V.UDYMZEV.UDY8V.U#V&V 0WBX     ;X >WDS   >~T-~\\(y Mw&W4W W4W GXFX ?XFX JV      "
			"          #r   >V   'WCV <c .VEWEW=W?ZCW  .W !W   :~W  5f   9W 0X4X >X -Y  Y!W;X 'Y Y5X =X"
			" @Y8Y HgKX    'a    Ca%X 8UAV8VAU=W8W NX4X%X *X+Y,X -X 0X(X+X/X'X -X -XI[ ?X -XDWLWDX0X@WG"
			"X)X&Y.X 0X&Y.X=Y *[ @X <X/X NXDX DXHW@VHX,YMZ 8X 1Z :W (W &W       At JW3W$W ,W3W!| LW 1W3"
			"W MW6W MW ,W ,` 5W ,W7W7W=W6W!W2W NW3W$W3W MW )g ?W ,W6W IWBW CWGVHVGW MY 8WCX :Y 6` 5V  H"
			"` IW@m  -S;V<T1W8X IW7X W@[ HW2W Ia ;W6W HVCW >XFX EW6W!W<W<W3WCWCW!W=W JWCWCW\"W2W%W2W ?W "
			">W2W KW8W @Y <X /X'X/X@X@X,X0X#YFXFY*X&Y2~a GV H~a HV I~b HV   DX 3W@S 6X 3V8V ;X   CX=V  "
			" CSFV)S  =WEW  :V  -SAVDW@S    'V      \"W6W :UGU IX       /WEW .V<TJU NV/U\"V<TJU8Z /W8W KW"
			"8W KW8W KW8W KW8W KX:X KX<X X *X -X -X -X -X -X -X -X ,X+Y-X@WGX)X&Y1X&Y1X&Y1X&Y1X&Y H_ LX"
			"DW9Y.X/X'X/X'X/X'X/X GX <X:Y LW>] Jt It It It It It I~iBW ,|\"|\"|\"| NW ,W ,W ,W /W2W NW6W!W"
			"2W\"W2W\"W2W\"W2W\"W2W M~Y2WCVEW NW6W MW6W MW6W MW6W IWCX EW3W LWCX     IV=V=V.V$V.VFYKZFV.VFY"
			"7V.V$V&V 0XCW     ;Y =WFT   >~T-~\\'w Ku%W4W W4W GXEW >WFW IV                #q   =V   6~X "
			"JSN^ /VEWCW?W=ZDW  .W !W   :~W  5f   9V /X4X >X .Y  MX\"W:X &X Y5X >Y @X6X FcJX    &d    Id"
			"%X 8UAV8VAU>X8X X4X$X +X+X+X -X /X)X+X/X'X -X -XH[ @X -XCVLVCX0X@XHX(X'X-X /X'X-X<Y *Z @X "
			"<X/X NXDX DXHV?VHX-YKY 8X 2Z 9W 'V &W       B]?W JW3W$W ,W3W!| LW 1W3W MW6W MW ,W ,a 6W ,W"
			"7W7W=W6W!W2W NW3W$W3W MW 'g AW ,W6W IWBW CWHVFVHW NZ 7WDW :Z 6a 6V  Jb IU;i  ,S;V<S0W7W IW"
			"6W W?Z HW2W Kc ;W6W HWEX >XFX EW6W!W<W<W3WCWCW!W=W JWCWCW\"W2W%W2W ?W =V2V KX8X BY ;X /Y)Y/"
			"X@X@X,X0X#YFXGZ)X'X0~` GV H~` GV H~` GV   DX 3W@S 6X 3V8V M|  &Z?V   CSFV)S:m AXFX  ;V  -S"
			"AVDW@S    'V      \"W6W :UGU      *m 5XFX /V;SIU V.T\"V;SIU9Z /X8X MX8X MX8X MX8X MX8X MX8X "
			"MX;X NX +X -X -X -X -X -X -X -X ,X+X,X@XHX(X'X/X'X/X'X/X'X/X'X Ha LXFW8X-X/X'X/X'X/X'X/X G"
			"X <X;Z LW<\\ L]?W J]?W J]?W J]?W J]?W J]?W J]?{BW ,|\"|\"|\"| NW ,W ,W ,W /W2W NW6W!W2W\"W2W\"W2"
			"W\"W2W\"W2W M~Y2WDVDW NW6W MW6W MW6W MW6W HWDW DW3W KWDW     HV=V>V-V%V-VGYIZHV-VGY7V-V%V%V "
			"/WDX     ;X <WFT   >~T-~\\'v Is$W4W W4W GWDX ?XGW HV                %r   =V   6~X JSJ[ 0VEV"
			"AV?W<ZFW  -W !W   \"V   Lf   9V /X5X =X /Z  MX\"V9X &X NX5X >X ?X6X D`IX    $d    Ne#X 8UAV8"
			"VBU=x X4X$X +X+X+X -X /X)X+X/X'X -X -XG[ AX -XCVLVCX0X?WHX(X'X-X /X'X-X;Y *Y @X <X/X MXFX "
			"CXHV?VHX-XIY 9X 3Z 8W 'V &W       CZ;W JW3W$W ,W3W!| LW 1W3W MW6W MW ,W ,b 7W ,W7W7W=W6W!W"
			"2W NW3W$W3W MW %f BW ,W6W IXDX BWIVFVIW N\\ 8WEX :Y .[ 7V  K\\ BT8e  *S<X=S0W7V HW6X\"W=X GW2"
			"W Me ;W6W GVEX >WDW EW6W!W<W<W3WCWCW!W=W JWCWCW\"W2W%W2W ?W =W4W KW6W CY :X .X)X.YAXAY,X0X\""
			"ZHXHZ(X'X/Y  AV  BY FV GY%Y FV   DX 3W@S 6X 2V:V L|  %ZAV   BSEV*S:m @XFX  <V  -SAVCWAS   "
			" 'V      \"W6W :UGU      *m 6XFX .V<TIU V/U\"V<TIU9Y .x Mx Mx Mx Mx Mx Mu NX +X -X -X -X -X "
			"-X -X -X ,X+X,X?WHX(X'X/X'X/X'X/X'X/X'X Ic MXGW7X-X/X'X/X'X/X'X/X GX <X=[ KW:[ NZ;W KZ;W K"
			"Z;W KZ;W KZ;W KZ;W KZ;{BW ,|\"|\"|\"| NW ,W ,W ,W /W2W NW6W!W2W\"W2W\"W2W\"W2W\"W2W  &WEVCW NW6W "
			"MW6W MW6W MW6W HWEX DW3W KWEX     GV>V>V,V&V,VIYGZIV,VIY6V,V&V&W /XEW     N~X'VGT   =~T-~\\"
			"&u Ir#W4W NV4W HXDX ?XHX HV                 KX   ,V   6~X JSHZ 2VDVAV?W;ZGW  -W !W   \"V   "
			"Lf   :W .X6X =X 0Z  LY#~ /X NX5X >X @X5Y AYFX    !d >~X >d X 8UAV8VBU>z!X3X%X +X+X+X -X /X"
			")X+X/X'X -X -XF[ BX -XCWNWCX0X?XIX(X'X-X /X'X-X:X )Y AX <X/X MXFX BWHV?VHW-YIY 9X 3Y 7W 'W"
			" 'W       CX9W JW3W$W ,W3W!W 'W 1W3W MW6W MW ,W ,WNZ 8W ,W7W7W=W6W!W2W NW3W$W3W MW !c CW ,"
			"W6W HWDW AWIVFVIW N] 8WFW :Y *Y 8V  KY ?R3`  (S<X=S0W7V HW5W\"W=X GW2W N[ 0W6W GWFW >XDX FW"
			"6W!W<W<W3WCWCW!W=W JWCWCW\"W2W%W2W ?W =W4W LX6X DY :X .X)X-XAXAX+X0X!ZIXIZ'X'X.Y  BV  CY EV"
			" FY'Y EV   DX 3W@S 6X 2V:V L|  $[CV   BTFW,T:m ?XFX  =V  -TBVBVBT    'V      \"W6W :UGU    "
			"  *m 7XFX .V<THU!V/U\"V<THU:Y .z z z z z Nx Nv NX +X -X -X -X -X -X -X -X ,X+X,X?XIX(X'X/X'"
			"X/X'X/X'X/X'X Je NXGV6X-X/X'X/X'X/X'X/X GX <X@^ KW9[ X9W KX9W KX9W KX9W KX9W KX9W KX9W MW "
			",W ,W ,W ,W )W ,W ,W ,W /W2W NW6W!W2W\"W2W\"W2W\"W2W\"W2W  &WFVBW NW6W MW6W MW6W MW6W GWFW CW3"
			"W JWFW     FV>V?W,V'W,VJYEZKW,VJY6W,V'W&W /XFX     N~X'WHT   =~T-~\\%s Gp\"W4W NV4V GXCW >WH"
			"X HW                 LX   ,V   6~X JSGY 3VDWAW@W:ZIW  ,W !W   \"V   Lf   :W .X6X =X 1Z  JX#"
			"~ /X NX5X ?Y @X4X .X     Md A~X Ad LX 8UAV8VBU>z!X3X%X +X+X+X -X /X)X+X/X'X -X -XE[ CX -XB"
			"VNVBX0X>WIX(X'X-X /X'X-X9X *Y AX <X/X MXFX BXJW?WJX.YGY :X 4Z 7W 'W 'W       DX8W JW3W$W ,"
			"W3W!W 'W 1W3W MW6W MW ,W ,WLY 9W ,W7W7W=W6W!W2W NW3W$W3W MW  K_ DW ,W6W HXFX AWIVFVIW ^ 8W"
			"FW ;Y (Y 9V  LY >Q.X  $T>Z?T0W8W HW5W\"W<W GW2W Y -W6W GWGX >WCX FW6W!W<W<W3WCWCW!W=W JWCWC"
			"W\"W2W%W2W ?W =W4W LX6X EY 9X .Y+Y-YBXBY+X0X ZJXJZ&X'X-Y  CV  DY DV EY)Y DV   DX 3W@S 6X 2W"
			"<W L|  #\\FW   ASFW,S9m >XFX  >V  ,SBVBWCS    &V      \"W6W :UGU      *m 8XFX .V<TGU\"V.U#V<T"
			"GU;Y -z z z z z z v NX +X -X -X -X -X -X -X -X ,X+X,X>WIX(X'X/X'X/X'X/X'X/X'X KZMZ XHW6X-X"
			"/X'X/X'X/X'X/X GX <u JW7Y!X8W LX8W LX8W LX8W LX8W LX8W LX8W MW ,W ,W ,W ,W )W ,W ,W ,W /W2"
			"W NW6W!W2W\"W2W\"W2W\"W2W\"W2W  &WGWBW NW6W MW6W MW6W MW6W GWFW CW3W JWFW     FW?V?V+W(V+WKXCY"
			"KV+WKX5V+W(V%W .WFX     N~X'WHT   =~T-~\\$q Eo\"W4W NV4V GWBW >XIW GW                 LX    "
			"   ;~X JSFX 3VDV?V@W9ZJW  +V \"W   !V       V -X6X =X 2Z  IX#~ /X NX5X ?X ?X4X .X     Jd D~"
			"X Dd IX 8UAV8VCV>z!X3X%Y ,X,Y+X -X /Y*X+X/X'X -X -XD[ DX -XBVNVBX0X>XJX(Y)X,X /Y)X,X9Y *X "
			"AX <X/X LXHX AXJV=VJX.XEY ;X 5Z 6W &V 'W       DW7W JW3W$W ,W3W!W 'W 1W3W MW6W MW ,W ,WKY "
			":W ,W7W7W=W6W!W2W NW3W$W3W MW  H\\ DW ,W6W GWFW @WJVDVJW!` 9WGX <Y &X 9V  LX =P   (T?\\@T0W8"
			"X IW5W\"W<W GW2W X ,W6W FVGW >XBW FW6W!W<W<W3WCWCW!W=W JWCWCW\"W2W%W2W ?W =W4W LW4W FY 8X -X"
			"+X+YCXCY*X0X N\\MXM\\%Y)X+Y  DV  EY NQFVFQ Y+Y CV   DX 3W@S 6X 1V<V K|  ![HW   @TFW.T9m =XFX"
			"  ?V  ,TCVAVDT    &V      \"W6W :UGU      *m 9XFX -V<SFU\"V/U\"V<SFU;X ,z z z z z z v NY ,X -"
			"X -X -X -X -X -X -X ,X,Y,X>XJX(Y)X.Y)X.Y)X.Y)X.Y)X KZKZ!YJW6X,X/X'X/X'X/X'X/X GX <t IW6Y\"W"
			"7W LW7W LW7W LW7W LW7W LW7W LW7W MW ,W ,W ,W ,W )W ,W ,W ,W /W2W NW6W!W2W\"W2W\"W2W\"W2W\"W2W "
			" &WHWAW NW6W MW6W MW6W MW6W GWGX CW3W JWGX     EV?V@W*V)W*VJVAWKW*VJV5W*V)W%W .XGW     M~X"
			"&WJT   <~S,kNn#o Cm!W4W NV4V HXBX ?XJX FW                 MY       <~X JSEX 5VCV?V@W8ZLW  "
			"*W #W   !V       V -X6X =X 3Z  HX#~ /X NX5X @Y ?X4X /X     Ge G~X Ge GX 8UAV9WCU>|\"X3X$X ,"
			"X,X*X -X .X*X+X/X'X -X -XC[ EX -XA\\AX0X=WJX'X)X,X .X)X,X8X *X AX <X/X LXHX AXJV=VJX/YEY ;X"
			" 6Z 5W &V 'W       DW7W JW3W$W ,W3W!W 'W 1W3W MW6W MW ,W ,WJY ;W ,W7W7W=W6W!W2W NW3W$W3W M"
			"W  EZ EW ,W6W GWFW ?WKVDVKW!b 9WHW <Y $W 9V  LW     BTAVNUAT/W8X IW5W#W;V FW2W!X +W6W FWIX"
			" >XBX GW6W!W<W<W3WCWCW!W=W JWCWCW\"W2W%W2W ?W =W4W MX4X HY 7X -Y-Y+ZDXDZ*X0X Mt#X)X*Y  EV  "
			"FY NSGVGS Y-Y MQFVFQ   X 3W@S 6X 1W>W 9X   =\\KW   >SEW<PCS  6XFX  @V  +SCVAWES    %V      "
			"\"W6W :UGU        &XFX -V<TFU#V/U\"V<TFU<X ,|\"|\"|\"|\"|\"|\"w MX ,X -X -X -X -X -X -X -X ,X,X+X="
			"WJX'X)X-X)X-X)X-X)X-X)X LZIZ!XKW5X,X/X'X/X'X/X'X/X GX <s HW5X\"W7W LW7W LW7W LW7W LW7W LW7W"
			" LW7W MW ,W ,W ,W ,W )W ,W ,W ,W /W2W NW6W!W2W\"W2W\"W2W\"W2W\"W2W  &WIW@W NW6W MW6W MW6W MW6W"
			" FWHW BW3W IWHW     DW@VAW)W+W)WJT?UKW)WJT5W)W+W$W -WHX     M~X&WJT   ;eMQMe+jNQNj!m Bl W4"
			"W NW6W HXBX >WJX FW                 LX       <~X JSEX 6WCV?V@W7ZMW  *W #W   !V      !W -X6"
			"X =X 4Z  GX#~ /X NX5X @X >X4X /X     De J~X Je DX 8U@V:WDV>|\"X3X$X ,X-Y*X -X .X*X+X/X'X -X"
			" -XB[ FX -XA\\AX0X=XKX'X*Y,X .X*Y,X8Y +X AX <Y1Y KWHW ?WJV=VJW/YCY <X 7Z 4W &W (W       EW6"
			"W JX5X$X -X5X!X (W 0W5X MW6W MW ,W ,WIY <W ,W7W7W=W6W!X4X NX5X$X5X MW  CX EW ,W6W GXHX ?WK"
			"VDVKW!XNY :WIX =Y #X :V  MX     BUCVMVBT/W9Y IW5W#W<W FW3X!W *W6W EVIX ?X@W GW6W!W=Y=W3XDW"
			"DX!W=W JWCWCW\"X4X%X4W >W <W6W LX4X HY 7X ,X-X)ZEXEZ)X0X Lr\"X)X)Y  FV  GY NUHVHU Y/Y MSGVGS"
			"  !X 3XAS 6X 0W@W 8X   ;\\NW   =TEX@RDT  5XFY  BV  +TDV@WGT    %V      \"W6W :UGU        (YF"
			"X ,V=TEU#V0U!V=TEU<X ,|\"|\"|\"|\"|\"|\"w MX ,X -X -X -X -X -X -X -X ,X-Y+X=XKX'X*Y-X*Y-X*Y-X*Y-"
			"X*Y MZGZ\"XLW5Y,Y1Y'Y1Y'Y1Y'Y1Y GX <r GW4X$W6W MW6W MW6W MW6W MW6W MW6W MW6X NX -X -X -X -X"
			" *W ,W ,W ,W /W2W NW6W!X4X\"X4X\"X4X\"X4X\"X4X  &WIV@X NW6W MW6W MW6W MW6W FWIX BX5X IWIX     "
			"CWAVAW(W,W(WJR=SJW(WJR4W(W,W$W -XIX     M~X&WJS   :dLQLd+iMQNj!l @j NW4W NW6W HW@W >WJW DW"
			"                 MX       .VCV :SDW 6VBV?V@W6b  )W #W   !V      !V +X8X <X 5Z  FX#~ /X MW5"
			"X @X >X4X /X     Ad L~X Ld AX 8VAV:WDU=|\"X3X$Y -X-Y*X -X .Y+X+X/X'X -X -XA[ GX -XA\\AX0X<WK"
			"X'Y+X+X .Y+Y,X7X +X AX ;X1X JXJX ?XLW=WLX/XAY =X 7Y 3W %V (W       EW7X JX5W\"W ,W5X W (W 0"
			"W5X MW6W MW ,W ,WHY =W ,W7W7W=W6W W4W MX5W\"W5X MW  BX FW ,W6W FWHW >WKVDVKW\"XLX 9WJW =Z #X"
			" :V  MX     AUEVKVDU/X:Y IW5W#W<W EW4W!X *W6W EVJX >X@W GW6W!W=Y=W2WDWDW W=W JWCWCW\"X4W#W4"
			"W >W <W6W LW2W IY 6X ,Y/Y(ZFXFZ(X0X Kp!Y+X'Y  GV  HY NWIVIW Y1Y MUHVHU  \"X 2WAS 6X 0YDY 8X"
			"   :c   <TE[FUDS  3XFY  CV  *SDV@WGS    $V      \"W6W :UGU        )YFX ,V=TDU$V0V\"V=TDU=X +"
			"|\"|\"|\"|\"|\"|#x MY -X -X -X -X -X -X -X -X ,X-Y+X<WKX'Y+X,Y+X,Y+X,Y+X,Y+X MZEZ#YNW4X*X1X%X1X"
			"%X1X%X1X FX <p EW4X$W7X MW7X MW7X MW7X MW7X MW7X MW7Y MW ,W ,W ,W ,W *W ,W ,W ,W .W4W MW6W"
			" W4W W4W W4W W4W W4W  $WKV?W MW6W MW6W MW6W MW6W EWJW AX5W GWJW     BXBVBW'X.W'XJP;QJW'XJP"
			"4W'X.W#V ,XIW     L~X%WLT   :dLQLc*iMQMi k ?i NW4W NW6W IX@X ?XLX DW                 MY   "
			"    0VBV :SDW 7VAV?V@X6a  )W #W   !V      !V +X8X <X 6Z  EX#~ 0Y MW5X AY >X4X 0X     =d ~X"
			" d   LUAW<XEV>X2X#X3X#X -X.Y)X -X -X+X+X/X'X -X -X@[ HX -X@Z@X0X<XLX&X+X+X -X+X+X7Y ,X AX "
			";X1X JXJX ?XLV;VLX0YAY =X 8Z 3W %V (W       EW7X JX5W\"W ,W5X W (W 0W5X MW6W MW ,W ,WGY >W "
			",W7W7W=W6W W4W MX5W\"W5X MW  BX FW ,W7X FWHW >WLVBVLW#YKX :WJW =Y !W :V  MW     @VHXJWHV-W:"
			"Y IW5W#W<W EW4W!W )W6W EWKX ?X?X HW6W!X>Y>W1WDWDW W=W JWCWCW\"X4W#W4W >W <W6W MX2X KY 5X +Y"
			"1Y'[GXH\\(X0X Jn NX+X&Y  HV  IY NYJVJY Y3Y MWIVIW  #X 2WAS 6X 0[H[ 8X :V %`   :TEiET  2YGY "
			" DV  *TEV?WIT    $V      \"W6W :UGU        *YGY ,V<SCU%V0V\"V<SCU=X ,X2X$X2X$X2X$X2X$X2X$X2X"
			"$X8X LX -X -X -X -X -X -X -X -X ,X.Y*X<XLX&X+X+X+X+X+X+X+X+X+X NZCZ#`3X*X1X%X1X%X1X%X1X FX"
			" <m BW3W$W7X MW7X MW7X MW7X MW7X MW7X MW7Y MW ,W ,W ,W ,W *W ,W ,W ,W .W4W MW6W W4W W4W W4"
			"W W4W W4W 5Z IWLV>W MW7X MW7X MW7X MW7X EWJW AX5W GWJW     AXCVCW%X0W%X0W%X0W%X0W\"V +WJX  "
			"   ?X 2WLT   9bKQKb)gLQMh Mi =g MW4W MV6W IX@X ?XLX CW                 MX       0VBV :SDW "
			"7VAV?V@X5_  (W #W   !V      \"W +X8X <X 7Z  DX 5X 'X LX7X @X =X4X 0X     ;e   Le   JUAW<XFV"
			"=X1W#X3X#Y .X.Y)X -X -Y,X+X/X'X -X -X?[ IX -X@Z@X0X;XMX&Y-Y+X -Y-Y+X6X ,X AX ;X1X IXLX >XL"
			"V;VLX1Y?Y >X 9Z 2W %W )W       EW7X JX5W\"X -W5X X )W 0X7Y MW6W MW ,W ,WFY ?W ,W7W7W=W6W W4"
			"W MX5W\"W5X MW  AW FW ,W7X FXJX =WMVBVMW#YJY ;WKX >Y  W :V  MW     ?dId,W;Z IW5W#W=W DW4W!W"
			" )W6W DVKW >X>W HW6W W>Y>W1WDWDW W=W JWCWDX\"X4W#W4W >W ;V7W LX2X LY 4X *X1X%]JXJ]'X0X Hj L"
			"Y-Y%Y  IV  JY LYKVKY MY5Y MYJVJY  $X 2XBS 6X 2q 9X :V #\\   7TDgFT  /XFX  EV  )TFV>VJT    #"
			"V      \"W6W :UGU        +XFX *V=TCU%V1V!V=TCU=X ,X1W$X1W$X1W$X1W$X1W$X2X%X7X LY .X -X -X -"
			"X -X -X -X -X ,X.Y*X;XMX&Y-Y+Y-Y+Y-Y+Y-Y+Y-Y ZAZ$_3Y*X1X%X1X%X1X%X1X FX <i >W3W$W7X MW7X M"
			"W7X MW7X MW7X MW7X MW7Z NX -X -X -X -X +W ,W ,W ,W .W4W MW6W W4W W4W W4W W4W W4W 5Z IWMV=W"
			" MW7X MW7X MW7X MW7X EWKX AX5W GWKX     @XDVDX$X2X$X2X$X2X$X2X\"V +XKW     ?X 1WMT   7`JQKa"
			"'fLQLf Kg <f LW4W MW8W HW>W >WLW BX                 NY       1VBV :SDW 8V@V?V?W4]  &V $W  "
			"  V      \"V *Y:Y <X 8Z  DY 5X 'X KW7X @X =X5Y 1Y     8e  #e   GU@W>YGW>X0X$X4Y\"Y /X/Y(X -X"
			" ,Y-X+X/X'X -X -X>[ JX -X@Z@X0X;XMX%Y/Y*X ,Y/Y*X6Y -X AX ;Y3Y IXLX =WLV;VLW0X=Y ?X :Z 1W $"
			"V )W       EW8Y JY7X\"X -X7Y X )W 0X7Y MW6W MW ,W ,WEY @W ,W7W7W=W6W X6X MY7X\"X7Y MW  AW FW"
			" ,X8X EWJW <WMVBVMW#XHX :WLW >Y  NW :V  MW     >bGc,W;[ JW6X#W=W DX6X!W )W6W DVLX >W=X IW7"
			"X W>Y>W1XEWEX W=W IWDWDW!Y6X#X6X >W ;W8W MX0X MY 4X *Y3Y$^LXL^&X0X Ff IY/Y#Y  JV  KY JYLVL"
			"Y KY7Y KYKVKY  #X 2XBS 6X 3t ;X :V ![   8TCfFT  .XFX  FV  )UGV>WKT            MW7X :UGU   "
			"     ,XFX *V=TBU&V2W!V=TBU=X -X0X&X0X&X0X&X0X&X0X&X0W%X7X KY /X -X -X -X -X -X -X -X ,X/Y)"
			"X;XMX%Y/Y)Y/Y)Y/Y)Y/Y)Y/Y Z?Z$^4Y)Y3Y%Y3Y%Y3Y%Y3Y FX <X -W3W$W8Y MW8Y MW8Y MW8Y MW8Y MW8Y "
			"MW8[ NX -X -X -X -X +W ,W ,W ,W .X6X MW6W X6X X6X X6X X6X X6X 5Z I_=X MX8X MX8X MX8X MX8X "
			"DWLW @Y7X FWLW     >XEVFY\"X5Y\"X5Y\"X5Y\"X5Y!V *WLX     @X /WNT   7`JQJ_&eKQKe Je :d KW4W MW8"
			"W HW>X ?XNX AX                 Y       1VCV 9SDW 9V?V?V?X4\\  &W %W    V      \"V )X:X ;X 9Z"
			"  CX 4X (Y KW7X AX <Y6Y 1X     4e  )e   DVAX@ZHW=X0X$X4Y\"Y*P&X0Z(X -X ,Y-X+X/X'X -X -X=[ K"
			"X -X?X?X0X:XNX%Y/Y*X ,Y/Y*X5X .Y AX :X3X HXLX =XNW;WNX1Y=Y ?X ;Z 0W $V )W       EW8Y JY7W "
			"W ,W7Y NX *W /W8Z MW6W MW ,W ,WDY AW ,W7W7W=W6W NW6W LY7W W7Y MW  AW FW ,X9Y EWJW <WMVBVMW"
			"$XFX ;WMX ?Y  MW :V  MW     =`Ea+X<[ JW6W\"W>W BW6W W )W6W DWMX ?X=X IX8X W?[?W0WEWEW NW=W "
			"IWDWDW!Y6W!W6W =W ;W8W MX0X NY 3X )Y5Y\"z%X0X C` FY/Y\"X  JV  KX HYMVMY IX7X IYLVLY  \"X 1XCS"
			" 6X 4v <X :V  [   8TBbET  ,WEW  FV  (T$T            LX8X :UGU        ,WEW )V=m,V3W V=mCX -"
			"X0X&X0X&X0X&X0X&X0X&X0X&X7X KY*P&X -X -X -X -X -X -X -X ,X0Z)X:XNX%Y/Y)Y/Y)Y/Y)Y/Y)Y/Y!Z=Z"
			"%]3Y(X3X#X3X#X3X#X3X EX <X -W3W$W8Y MW8Y MW8Y MW8Y MW8Y MW8Y MW8[ MW ,X -X -X -X ,W ,W ,W "
			",W -W6W LW6W NW6W MW6W MW6W MW6W MW6W 4Z H^=W LX9Y MX9Y MX9Y MX9Y DWMX @Y7W EWMX     =Y8Y "
			"Y8Y Y8Y Y8Y Y8Y V *WLX     AX .WNT   6^IQI]$cKRJc Id 8c KW4W MX:X IX>X ?XNX AY            "
			"     Y4P       VBV 9SDW 9V?V?V?Y4Z  %W %W    V      #W )X:X ;X :Z  CY 4X (Y KX9Y AX ;X6X 1"
			"Y     1e  /e   @U@XB[JX<X/W$X4X Y,Q&X1Z'X -X +Y.X+X/X'X -X -X<[ LX -X?X?X0X:XNX$Y1Y)X +Y1Y"
			")X5Y /X @X :X4Y GXNX <XNV9VNX2Y;Y @X ;Y /W $W *W       EW9Z JZ9X X -X9Z NX *W /X9Z MW6W MW"
			" ,W ,WCY BW ,W7W7W=W6W NX8X LZ9X X9Z MW  AW FW +W9Y EXLX <WNV@VNW%YEX ;WNW ?Y  LW :V  MW  "
			"   <^C_)W=\\ JX7W\"W>W BX8X W )W6W CVNX >W;W IX8X X@[@X0XFWEW NW=W IWDWEX!Z8X!X8X =W :W:W LX"
			"0X Y 2X (Y7Y Nv#X0X ?X AY1Y V  IV  JV FYNVNY GV5V GYMVMY  !X 1XCS 6X 5x =X :V  MZ   8T?ZBT"
			"  *VDV  FV  'T&T            KX8X :UGU        ,VDV )V<m-V3V NV<mCX -X/W&X/W&X/W&X/W&X/W&X0X"
			"'X6X JY,Q&X -X -X -X -X -X -X -X ,X1Z(X:XNX$Y1Y'Y1Y'Y1Y'Y1Y'Y1Y!Z;Z%[3Y'X4Y#X4Y#X4Y#X4Y EX"
			" <X -W3W$W9Z MW9Z MW9Z MW9Z MW9Z MW9Z MW9] NX -X -X -X -X ,W ,W ,W ,W -X8X LW6W NX8X MX8X "
			"MX8X MX8X MX8X 4Z H]=X KW9Y LW9Y LW9Y LW9Y CWNW ?Z9X DWNW     ;Y;Z MY;Z MY;Z MY;Z MY;Z NV "
			"*XMW     AY -[   3ZHRH[\"aJRI` Fb 6a JW4W LW:W HX=W >WNX @Y                !Z6Q       VBV K"
			"P>SEW 9V>WAW>X3Z  &W %W    V      #V 'X<X :X ;Z  BY 4X )Y IW9X AY ;Y8Y 2Y     .d  1d   >U?"
			"ZH^MZ<X.X%X5Y NY.R&X2Z&X -X *Y/X+X/X'X -X -X;[ MX -X&X0X9a$Z3Y(X *Y3Y(X4X$P-Y @X :Y5Y GXNX"
			" <XNV9VNX2X9Y AX <Z /W #V *W       EX:Z JZ9X NX .X9Z MX +W .X;[ MW6W MW ,W ,WBY CW ,W7W7W="
			"W6W NX9Y LZ9X X9Z MW  AW FW +W:Z DWLW :^@^$XDY <WNW @Z  LW :V  MW     ;\\@['X>\\ JX8X\"W?W AX"
			"9Y X *W6W CVNX ?X;X JX9Y NW@[@W/XFWFX NW=W IXEWEX!Z8X!X8W ;W ;W;X MX.X\"Y 1X 'Y9Y Lt\"X0X ?X"
			" @Y3Y MT  HV  IT Dj ET3T EYNVNY   X 0XDS 6X 6ZM`LY >X :V  LY   7T)T  (UCU     ET(T        "
			"    JX9Y :UGU        ,UCU )V;m.V3V NV;mCY7P HX.X(X.X(X.X(X.X(X.X(X.X(X6X IY.R&X -X -X -X -"
			"X -X -X -X ,X2Z'X9a$Z3Y&Z3Y&Z3Y&Z3Y&Z3Y!Z9Z&Z3Y&Y5Y#Y5Y#Y5Y#Y5Y EX <X -W3W$X:Z MX:Z MX:Z M"
			"X:Z MX:Z MX:Z MX:^ NX -X -X -X -X -W ,W ,W ,W -X8X LW6W NX9Y MX9Y MX9Y MX9Y MX9Y 4Z H\\=Y K"
			"W:Z LW:Z LW:Z LW:Z CWNW ?Z9X DWNW     :[@[ K[@[ K[@[ K[@[ K[@[ MV )WNX     AX ,[   1WGRFW "
			"N_IRH^ Da 5_ IW4W LX<X HW<W >` >Y                !Y8S   MX   +VBV KQ?SFX 9V=VAV=Y6]  &V &W"
			"    NV BX   1X 1V 'Y>Y :X <Z  BY 3X GP3Z IX;Y AX :Y9Z 2X GX -X  7a  1a .X 6V@iNa;X.X%X6Z N"
			"Z1T&X4\\&X -X *Z0X+X/X'X -X -X:[ NX -X&X0X9a#Z5Z(X *Z5Z(X4Y%R/Y @X 9Y7Y EWNW :WNV9VNW2Y9Y A"
			"X =Z .W #V *W       EX;[ J[;X MY .X;[ MY2P JW .Y=\\ MW6W MW ,W ,WAY DW ,W7W7W=W6W MX:X K[;X"
			" MX;[ MW /P4X FX ,X<[ DXNX :^@^%XBX <` @Y  KW :V  MW     8V;W%X?^ KY9X!V@X @X:X NX *W6W C_"
			" >X:W JY;Z NXB]BX.XGWGX MW=W HXFWFX [:X NX:X ;W :W<W LX.X\"Y 1X &Y;Y Ip X0X ?X @Z5Z LR  GV "
			" HR Bh CR1R Cj   NX 0YES 6X 7ZJ\\IY ?X :V  KY   8U+U  'TBT     DU+T            IY;Z :UGU   "
			"     ,TBT (V;m.V4V MV;mCY8Q HX.X(X.X(X.X(X.X(X.X(X.X)X5X IZ1T&X -X -X -X -X -X -X -X ,X4\\'"
			"X9a#Z5Z%Z5Z%Z5Z%Z5Z%Z5Z\"Z7Z&Z5Z%Y7Y!Y7Y!Y7Y!Y7Y DX <X -W4X$X;[ MX;[ MX;[ MX;[ MX;[ MX;[ MX"
			";`3P=Y .Y2P LY2P LY2P LY2P LW ,W ,W ,W ,X:X KW6W MX:X KX:X KX:X KX:X KX:X 3Z GZ<X JX<[ LX<"
			"[ LX<[ LX<[ C` ?[;X C`     9_J_ I_J_ I_J_ I_J_ I_J_ LV )`     AX +Z    S <[GRFZ A_ 4^ HW4W"
			" KX>X HX<X ?` =Z                \"Y:T   MX   +VCV JSASFX :V<VAV<Y8_  'W 'W    NV BX   1X 2W"
			" &X>X 9X =Z 1P2Z 3X GQ5Z GX=Y @X 9Y:Y KP8Z GX -X  4^  1^ +X 5U?gM_9W,W%X7Z L[4U&X6]%X -X )"
			"[2X+X/X'X -X -X9[ X -X&X0X8`\"Z7Z'X )Z7Z'X3X%T2Y ?X 9Z9Z E` :_9_3Y7Y BX >Z -W #W +W       D"
			"X=\\ J\\=Y LY7P HY=\\ LY5R JW -Y?] MW6W MW ,W ,W@Y EW ,W7W7W=W6W MY<Y K\\=Y MY=\\ MW /R6W DW ,Y"
			"=[ CWNW 9^@^&X@X <^ @Y  JW :V  MW       HXA` LZ;X V@W ?Y<Y MX +W6W B^ ?X9W JZ<Z NXB]BX.YHW"
			"HY MW=W HYGWGY \\<Y NY<X :W :X>X LX.X#Y 0X %Y=Z Gl MX0X ?X ?Z7Z JP  FV  GP @f AP/P Ah   MX "
			"/YFSDP BX 8ZFVEY @X :V  JX   7V.U  %SAS     CU.U            HZ<Z :UGU        ,SAS (V:m/V5W"
			" MV:mBY;S HW,W(W,W(W,W(W,W(W,W(X.X)X5X H[4U&X -X -X -X -X -X -X -X ,X6]&X8`\"Z7Z#Z7Z#Z7Z#Z7"
			"Z#Z7Z\"Z5Z&[8Z$Z9Z!Z9Z!Z9Z!Z9Z DX <X -W4W\"X=\\ LX=\\ LX=\\ LX=\\ LX=\\ LX=\\ LX=b6R<Y7P GY5R KY5R"
			" KY5R KY5R LW ,W ,W ,W ,Y<Y KW6W MY<Y KY<Y KY<Y KY<Y KY<Y 3Z GY<Y JY=[ LY=[ LY=[ LY=[ B^ >"
			"\\=Y B^     7r Gr Gr Gr Gr KV (_     BX )Y    S 8RBSCR <] 2\\ GW4W KZBZ HX;W >_ <[          "
			"      $[=U   MX   ,VBV JUCSHY :V;WCW<[<b  (W 'W    NV BX   1X 2W &Y@Y 9X >Z 0R5Z 2X GT9[ G"
			"Y?Z AY 9[>[ KR;Z FX -X  1[  1[ (X 5V>dL^9X,X&X9[ J[7W&X9_$X -X (\\6Z+X/X'X -X -X8[!X -X&X0X"
			"8`![;[&X ([;[&X3Y&W7[ ?X 8Z;Z D` :^7^3X5Y CX ?Z ,W #W +W       DY?] J]?Y KZ:R GY?] LZ8T JW"
			" -ZA^ MW6W MW ,W ,W?Y FW ,W7W7W=W6W LY>Y J]?Y KY?] MW /T9X DX ,Y@] CWNW 9]>]'Y@Y =^ AY  IW"
			" :V  MW       HYCXNW L\\>Y VAX >Y>Y LY ,W6W B] >X9X K[>[ MXDVMVDX,YIWIY LW=W GYHWHY N]>Y LY"
			">Y :X :X@X LX,X%Y /X $ZAZ Ch KX0X ?X >[;[   ?V   6d   >f   LX /[HSFR BX 9Z3Y AX :V  IX   7"
			"V1V  #R@R     BU0U            G[>[ :UGU        ,R@R 'V(U)V6W LV(U<Z>U IX,X*X,X*X,X*X,X*X,X"
			"*X,X*W4X G[7W&X -X -X -X -X -X -X -X ,X9_%X8`![;[![;[![;[![;[![;[\"Z3Z(];[\"Z;Z NZ;Z NZ;Z NZ"
			";Z CX <X -WJP;X\"Y?] LY?] LY?] LY?] LY?] LY?] LY?XNZ9T<Z:R GZ8T KZ8T KZ8T KZ8T LW ,W ,W ,W "
			"+Y>Y JW6W LY>Y IY>Y IY>Y IY>Y IY>Y 2Z FY>Y HY@] KY@] KY@] KY@] B^ >]?Y A^     6o Do Do Do "
			"Do IV (_     CX (Y    S (S ,[ 0[ GW4W J\\H\\ GW:W >^ :\\                %[@W   MX   ,VBV JXFS"
			"IZ :V:WEW:\\@e  (V 'V    MV BX   1X 2V $ZDZ 8X ?Z /U;] 2X GV=\\ EZC[ @X 7[@[ JT?[ EX -X  /Y "
			" 1Y &X 5V=bK\\7X,X&X<^ I]=Z&X=b#X -X ']:\\+X/X'X -X -X7[\"X -X&X0X7_ \\?\\%X '\\?\\%X2X&Z<\\ >X 7["
			"?[ B^ 9^7^4Y5Y CX ?Y +W \"V +W       DZB_ J_CZ I[>T G[C_ K[=W JW ,\\GXNW MW6W MW ,W ,W>Y GW "
			",W7W7W=W6W KZBZ I_CZ J[C_ MW /W>Z DZ .ZB^ C` 8\\>\\&X>Y =\\ AY  HW :V  MW       GZFYNY N]AZ N"
			"WCX <ZBZ JZ:Q EW6W B] ?X7W K\\A^ NYFWMWFY,ZJWJY KW=X H[JWJ[ N_BZ JZBZ 8Y <ZDZ LX,X&Y .X #ZC"
			"Z >_ FX0X ?X =\\?\\   >V   5b   <d   KX .\\JSHT BX 8X2X @X :V  IX   5V4U   Q?Q     AV4V      "
			"      F\\A^ ;UGU        ,Q?Q 'V'U*V6W LV'U<[AW IX,X*X,X*X,X*X,X*X,X*X,X+X4X F]=Z&X -X -X -X"
			" -X -X -X -X ,X=b$X7_ \\?\\ N\\?\\ N\\?\\ N\\?\\ N\\?\\ X1X(`?\\ [?[ L[?[ L[?[ L[?[ BX <X -WJS@Z\"ZB_ "
			"LZB_ LZB_ LZB_ LZB_ LZB_ LZBYM\\>W;[>T F[=W J[=W J[=W J[=W LW ,W ,W ,W *ZBZ IW6W KZBZ GZBZ "
			"GZBZ GZBZ GZBZ 1Z F[BZ GZB^ KZB^ KZB^ KZB^ A\\ =_CZ ?\\     3l Al Al Al Al HV (^     BX (X  "
			"  NS (S ,Z .Y FW4W In GX:X ?^ 9_                (]FZ   MX   ,VBV J[ISL\\ :V9XGX9^Fi  )W )W "
			"   MV BX   1X 3W #[H[ Et Mx MZC_ 1X GZD^ C[G\\ @Y 7^F] IXF] DX -X  ,V  1V #X 4V<^IY5X*X'y G"
			"_D^&{!y NX &`B`+X/X'X -X -X6[#w LX&X0X7_ N^E^$X &^E^$X2Y'^C^ =X 7^E^ B^ 8]7]4Y3Y DX @~U&W "
			"\"W ,W       C\\HYNW JWNXG\\ H]EX F\\GXNW J]D[ JW +kMW MW6W MW ,W ,W=Y HW ,W7W7W=W6W K]H] IWNX"
			"G\\ I\\GXNW MW /[E\\ Be 9[GXNW B^ 7\\>\\'X<X =\\ AX  GW :V  MW       G\\IYM^$`F\\ MWEX ;]H] J]BV E"
			"W6W A\\ ?X7X L_GaKP#ZJYMYJZ*[LWL[ KW=Y H\\LWL\\ MWNXG] J]H\\ 7a C[H[ L~W'x MX 1iEi HX CX0X ?X "
			"<^E^   =V   4`   :b   JX -^MSLX Lz V0V ?X :V  HW   4V7V   MP>P     @W8W    3~W      :_GaKP"
			" @UGU        ,P>P 'V&U+V6V KV&U;]GZ JX*X,X*X,X*X,X*X,X*X,Y,Y,X4y7_D^&y Ny Ny Ny NX -X -X -"
			"X ,{\"X7_ N^E^ L^E^ L^E^ L^E^ L^E^ MV/V(dE^ N^E^ L^E^ L^E^ L^E^ BX <X -WJWF[ \\HYNW K\\HYNW K"
			"\\HYNW K\\HYNW K\\HYNW K\\HYNW K\\H[K^E[:]EX E]D[ I]D[ I]D[ I]D[ LW ,W ,W ,W )[F[ HW6W K]H] G]H"
			"] G]H] G]H] G]H] 1Z F]G] F[GXNW J[GXNW J[GXNW J[GXNW A\\ =WNXG\\ ?\\     1h =h =h =h =h FV ']"
			"     AV &W    T )T +X -X EW4W Hl FX9W ?^ 8~R                Jp   MX   ,VCV It 9V8XIX7sLZ  "
			"*W )W    MV BX   1X 3W #n Et Mx Mu 0X Gs Ao @X 5t In CX -X  )S  1S  X 4V9XFU1X*X'x Ex&z y "
			"NX %|*X/X'X -X -X5[$w LX&X0X6^ Mu#X %u#X1X'y =X 6u A^ 8]7]4X1X DX @~U&W \"W ,W       ClMW J"
			"WMk Fo EkMW Is JW *jMW MW6W MW ,W ,W<Y IW ,W7W7W=W6W Jp HWMk GkMW MW /q Ae 9kMW B^ 7\\=[(Y;"
			"X >\\ Av 6W :V  MW       FkL]$u LXGX 9p Hp EW6W A[ ?X6X LpN\\#hKh)s JW<] Lu LWNm Hp 6` Bl K~"
			"W'x MX 1iEi HX CX0X ?X ;u   <V   3^   8`   IX ,o Lz NT.T >X :V  HW   3X=X        )X<X    2"
			"~W      :pN\\ @UGU           V&U+V7i.V&U:o JX*X,X*X,X*X,X*X,X*X,X*X-X3y6x&y Ny Ny Ny NX -X "
			"-X -X ,z!X6^ Mu Ju Ju Ju Ju KT-T(} Lu Ju Ju Ju AX <X -WJk NlMW KlMW KlMW KlMW KlMW KlMW Kn"
			"Is9o Ds Hs Hs Hs LW ,W ,W ,W )p HW6W Jp Ep Ep Ep Ep   Ls EkMW JkMW JkMW JkMW A\\ =WMk >\\   "
			"  /c 8c 8c 8c 8c CV '\\     ?T %W    U *T *W ,V DW4W Gj EW8W >\\ 5~P                In   LX "
			"  -VBV Is 9V7g6qJZ  *V )V    LV BX   1X 3V !l Dt Mx Mt /X Gr ?m ?X 4r Hm BX -X  &P  1P  LX"
			" 3V 3X*X'w Cv%x My NX #x(X/X'X -X -X4[%w LX&X0X5] Ls\"X $s\"X1Y(w ;X 5s ?\\ 7\\5\\5Y1Y EX @~U&W"
			" !V ,W       BjLW JWMj Dn DjMW Hr JW )hLW MW6W MW ,W ,W;Y JW ,W7W7W=W6W In GWMj EjMW MW /p"
			" ?d 8iLW B^ 6Z<[)Y:Y >Z @v 6W :V  MW       EiK]$t JYLZ 7n Fo EW6W A[ ?X5W LWNfM\\\"gKg'q IW<"
			"] Ks KWMk Fn 5` Aj J~W'x MX 1iEi HX CX0X ?X :s   ;V   2\\   6^   HX +n Lz MR,R =X :V  HW   "
			"1ZEZ        %ZDZ    0~W      :WNfM\\ @UGU          !V%U,V6i/V%U9n JX*X,X*X,X*X,X*X,X*X,X*X-"
			"X3y5v%y Ny Ny Ny NX -X -X -X ,x NX5] Ls Hs Hs Hs Hs IR+R(WMs Js Hs Hs Hs @X <X -WJk MjLW J"
			"jLW JjLW JjLW JjLW JjLW JmHr8n Cr Gr Gr Gr LW ,W ,W ,W (n GW6W In Cn Cn Cn Cn   Ls CiLW Ii"
			"LW IiLW IiLW @Z <WMj <Z     +] 2] 2] 2] 2] @V &[     >R $V    NU *U *U *U DW4W Fh DW8X ?\\ "
			"4~                Hl   KX   -VBV Hp 8V5e4nGZ  +W +W    LV BX   1X 3V  j Ct Mx Mr -X Gq =j "
			">Y 3p Gl AX -X       2X 3W 5X(X(u ?s$v Ky NX \"v'X/X'X -X -X3[&w LX&X0X5] Kq!X #p X0X(v :X "
			"4p =\\ 7\\5\\6Y/Y FX @~U&W !V ,W       AhKW JWLh Bm ChLW Gq JW (eJW MW6W MW ,W ,W:Y KW ,W7W7W"
			"=W6W Hl FWLh ChLW MW /o >d 7gKW A\\ 5Z<Z(X8X >Z @v 6W :V  MW       DgI\\$s He 5l Dn EW6W @Y "
			">W4X MWMeM\\!eIe%o HW<] Jq JWLi Dk 2_ @h J~Y(x MX 1iEi HX CX0X ?X 9q   :V   1Z   4\\   GX *m"
			" Lz LP*P <X :V  HW   0m        \"l    .~W      :WMeM\\ @UGU          !V%U,V6i/V%U8l JX(X.X(X"
			".X(X.X(X.X(X.Y)X/X2y3s$y Ny Ny Ny NX -X -X -X ,v LX5] Kq Fq Fq Fq Fq GP)P'VKp Gp Ep Ep Ep "
			">X <X -WJj KhKW IhKW IhKW IhKW IhKW IhKW IjEq7m Bq Fq Fq Fq LW ,W ,W ,W &j EW6W Hl Al Al A"
			"l Al   Ls AgKW HgKW HgKW HgKW @Z <WLh ;Z               MV &[     =P \"U    V +V )S (S CW4W "
			"De DX8X ?\\ 2|                Fh   IX   -VBV Ek 6V4c1kEZ  +V +V    KV BW   0X 4W  Mf At Mx "
			"Mq ,X Go :h =X 0l Ej ?X -W       1X 2W 6X(X(s ;o\"s Hy NX  r%X/X'X -X -X2['w LX&X0X4\\ Im NX"
			" !m NX0Y(t 9X 2m ;Z 5[5[5X-X FX @~U&W !W -W       @fJW JWJe ?j AeJW En IW 'cIW MW6W MW ,W "
			",W9Y LW ,W7W7W=W6W Fh DWJe AeJW MW .m ;b 6eJW A\\ 5Z<Z)X6X >X ?v 6W :V  MW       CeG[$r Fc "
			"2h Am EW6W @Y ?X3W MWMdL\\ cGc#m GW;\\ Hm HWKg Ah /] ?f I~Y(x MX 1iEi HX CX0X ?X 7m   8V   0"
			"X   2Z   FX (j Kz   AX :V  HW   -g         Lh    ,~W      :WMdL\\ @UGU          \"V$U-V5i0V$"
			"U7i HX(X.X(X.X(X.X(X.X(X.X(X/X2y1o\"y Ny Ny Ny NX -X -X -X ,t JX4\\ Im Bm Bm Bm Bm  %VHm Dm "
			"Bm Bm Bm =X <X -WJh HfJW HfJW HfJW HfJW HfJW HfJW HhBn4j ?n Cn Cn Cn KW ,W ,W ,W %h DW6W F"
			"h =h =h =h =h   KVMi >eJW GeJW GeJW GeJW ?X ;WJe 9X               MW &Z       =U    W ,W *"
			"R &Q BW4W B` AW6W >[ /y                Dd   GX   -VCV Af 5V2a.gBZ  ,W -W    KV CX   0X 4V "
			" Kd @t Mx Km *X Ek 6d ;X .h Bh >X .X       1X 1W 7X(X(q 7j Np Ey NX  Mm\"X/X'X -X -X1[(w LX"
			"&X0X4\\ Gi LX  Ni LX/X$n 7X 0i 9Z 5[5[6Y-Y GX @~U&W  V -W       >cIW JWIb <g =bIW Ci FW %_G"
			"W MW6W MW ,W ,W8Y MW ,W7W7W=W6W Ef CWIb =bIW MW +h 8a 5cIW @Z 4Y:Y*Y5X ?X ?v 6W :V  MW    "
			"   AbDY$WMf Ca 0f >k EW6W @Y ?W2W MWK`I[ NaEa i EW;\\ Fi FWIc >e ,\\ =b G~Y(x MX 1iEi HX CX0"
			"X ?X 5i   6V   /V   0X   EX &f Iz   AX :V /P;W   *c         Gb    )~W      :WK`I[ @UGU    "
			"      #V#U.V4i1V#U6f FX(X.X(X.X(X.X(X.X(X.X(X/X2y/j Ny Ny Ny Ny NX -X -X -X ,p FX4\\ Gi >i "
			">i >i >i  $VEi @i >i >i >i ;X <X -WIf EcIW FcIW FcIW FcIW FcIW FcIW Fd>i0g ;i >i >i >i HW "
			",W ,W ,W #d BW6W Ef ;f ;f ;f ;f   JUJe ;cIW FcIW FcIW FcIW ?X ;WIb 7X               MW %Y "
			"      =T    X -X )P %P AW4W ?Z >W6X ?Z ,w                B`   EX   .VBV <] 1V0]*b?[  -W -W"
			"    KV CW   /X 4V  I` >t Mx Hg 'X Bf 2` :X +d =b ;X .W       0X 1X 9X&X)m 0d Kj ?y NX  Jg "
			"NX/X'X -X -X0[)w LX&X0X3[ Dc IX  Kf LX/Y!g 4X .e 7Z 5Z3Z7Y+Y HX @~U&W  V -W       =`GW JWG"
			"^ 7b 9^GW Ad CW \"YDW MW6W MW ,W ,W7Y NW ,W7W7W=W6W B` @WG^ 9^GW MW (c 2] 3_GW @Z 3X:X*Y4Y "
			"@X ?v 6W :V  MW       ?_AW$WKb @^ +` 9g CW6W ?W ?X2X NWJ^GY K]B^ Ke CW:[ Dd CWG_ 9` 'Y ;^ "
			"F~[)x MX 1iEi HX CX0X ?X 2c   3V   .T   .V   DX $b Gz   AX :V /R>X   &[         ?Z    %~W "
			"     :WJ^GY ?UGU          #V +V +V 1b EX&X0X&X0X&X0X&X0X&X0Y'X1X1y,d Ky Ny Ny Ny NX -X -X "
			"-X ,j @X3[ Dc 8c 8c 8c 8c  !VBc ;e :e :e :e 9X <X -WFa B`GW E`GW E`GW E`GW E`GW E`GW D`:d*"
			"b 7d 9d 9d 9d EW ,W ,W ,W !` @W6W B` 5` 5` 5` 5`   HVHa 7_GW D_GW D_GW D_GW ?X ;WG^ 5X    "
			"           MW         7S                   @r                >Y         BS .V,W#Z   ;V -V "
			"    7W     ;W  EX     ;\\   6] +Z   5\\ 5Z   <W         7X     %\\       <]    \"X         ([ "
			"  4c   E]   /[          (W  W .W       :Y #X 0Z 2X *\\   $W    &W         .Z =WDX 3XDW   I["
			"   0Y       8W   -W :V  MW       <Z ;WH[ 9Y &Z 1]  LW ?W   >WGXBU FX=X E` \"W >] @WDY 3Z   "
			"2X               C[           >T     :[       KV /TAY                          EWGXBU =UGU"
			"   BT       6V +V +V ,Y               ?\\                    +[ 0[ 0[ 0[ 0[   KT=[ 2[ 0[ 0["
			" 0[     7Z ;Y .Y .Y .Y .Y .Y -Y2\\\"Z /\\ 1\\ 1\\ 1\\         CZ   3Z /Z /Z /Z /Z   FVCZ 1Y .Y ."
			"Y .Y ,W :WDX 2W               LW         7R                                             #S"
			"       >W /W     8W     :V                      \"W         5X                  )X         "
			"    &Z                  CW  NV .W                   :W    %W           @W  :W             "
			" -X   -W :V  MW         LW        FW ?W   >W    NW   0W =W                                "
			"      3S       GV /XGZ                          DW  HUGU   AT                            %"
			"T                               'R                             JT                         "
			"      #T         (X :W  NX               LW                                               "
			"        7S       =V /V     7W     :V                      \"W         4X'Q                 "
			"&Y             %Z                  DW  NV .W                   :W    %W           @W  :W  "
			"            -W   ,W :V  MW         LW        FW ?W   >W    NW   0W =W                     "
			"                 3S       GV /j                          CW  HUGU   @T                    "
			"        %T                               'P                             HT                "
			"               \"Q         'W 9W  NW               KW                                      "
			"                 7S       =W 1W     7V     :W                      \"V         2X)R        "
			"         &X             #Z                  EW  NW /W                   :W    %W          "
			" @W  :W              -W   ,X ;V  NX         LW        FW ?W   >W    NW   0W =W            "
			"                          3S       GV /j                          CW  HUGU   @U           "
			"                 &U                                                             U         "
			"                      \"P         'W 9W  NW               KV                               "
			"                        6S       <V 1V     6V     :V                      !V         1Y-U "
			"                'X             \"Z                  FW  MV /W                   ;X    %W   "
			"        @W  :W              .X   +W ;V  NW         KW        FW ?W   >W    NW   0W =W     "
			"                                 3S       GV /h                          AW  HUGU   ?T    "
			"                        %T                                                             NT "
			"                                        )X 9W  X               KV                         "
			"                              6S       <W 3V     6V     9V                      \"V        "
			" /Z1X                 (X             !Z                  Ga (V 9a                   ;W    "
			"$W           @W  :W              .W   *W ;V  NW         KW        FW ?W   >W    NW   0W =W"
			"                                      3S       GV .f                          @W  HUGU   ?"
			"U                            &U                                                           "
			"  U                                         *W 8W  W               JV                     "
			"                                  6S       ;V 3V     6V     :W                      \"V    "
			"     .[5[                 *Y              Z                  Ha (W :a                   <X"
			"    $W           @W  :W              /X   *X <V  X         KW        FW ?W   >W    NW   0W"
			" =W                                      3S       GV +a                          >W  HUGU "
			"  >T                            %T                                                        "
			"     NT                                         +X 8W !X              (VIV                "
			"                                       6S       :V 5V     5U     9W                      \""
			"U         +\\;]                 )X              MZ                  Ia (W :a               "
			"    =Y    %W           ?W  :W              /W   )[ ?V #[         KW        FW ?W   >W    N"
			"W   0W =W                                      3S       GV 'Z                          ;W "
			" HUGU   >U                            &U                                                  "
			"           U                                         ,W 7W !W              'VIV           "
			"                                            6S       :V 6W     6V                         "
			"   4V         *_C`                 )Y              LZ                  Ja   :a            "
			"      (P7Y    $W           ?W  :W              0X   (b GV +b         JW        FW ?W   >W "
			"   NW   0W =W                                      3S       GV                            "
			"7W  HUGU   >U                            &U                                               "
			"              U                                         -X 7W \"X              'VJW        "
			"                                               6S       9V 7V     5U                      "
			"      3U         'x                 (Z              KZ                  Ka   :a           "
			"       (R:Z    $W           ?W  :W              0X   (b GV +b         JW        FW ?W   >W"
			"    NW   0W =W                                      3S       GV                           "
			" 7W     #U                            &U                                                  "
			"           U                                         -X 7W \"X              &UJW           "
			"                                            6S       9W 9W                                "
			"            Bu                 ([              IZ                  La   :a                "
			"  (T>[    $X           ?W  :W              1X   &a GV +a         IW        FW ?W   >W    N"
			"W   0W =W                                      3S       GV                            7W  "
			"   $V                            'V                                                       "
			"     !V                                         .X 6W #X              %VLW                "
			"                                       5S                                                 "
			"    2p                 -a                                                       8XE]    %Y"
			"           >W  :W              3Z   $_ GV +_         GW        FW ?W   >W    NW   0W =W   "
			"                                   3S       GV                            7W     /QGW     "
			"                       2QGW                                                            ,QG"
			"W                                         0Z 6W %Z              %a                        "
			"                               5S                                                     0l  "
			"               +a                                                       8p    +_          "
			" >W  :W              ;a   !] GV +]         EW        FW ?W   >W    NW   0W =W             "
			"                         3S       GV                            7W     /`                 "
			"           1`                                                            +`               "
			"                          7a 5W -a              #`                                        "
			"                                                                     >e                 '`"
			"                                                       7o    *^           =W  :W          "
			"    ;`    KY GV +Y         AW        FW ?W   >W    NW   0W =W                             "
			"         3S       GV                            7W     /`                            1`   "
			"                                                         +`                               "
			"          7` 4W -`              \"_                                                        "
			"                                                     8\\                 #_                "
			"                       \"}              3n    )^           =W  :W              ;`     9V   "
			"        BW        FW ?W   >W    NW   0W =W                                             'V "
			"                           7W     /_                            0_                        "
			"                                    *_                                         6` 4W -`   "
			"           !]                                                                             "
			"                                                  -]                                      "
			"  }              3l    ']           <W  :W              ;_     8V           BW        FW ?"
			"W   >W    NW   0W =W                                             'V                       "
			"     7W     /^                            /^                                              "
			"              )^                                         5_ 3W -_               N[        "
			"                                                                                          "
			"                             ,[                                        M}              2j "
			"   &\\           ;W  :W              ;^     7V           BW        FW ?W   >W    NW   0W =W"
			"                                                                          7W     -Y       "
			"                     *Y                                                            $Y     "
			"                                    2^ 2W -^               LX                             "
			"                                                                                          "
			"        *X                                        J}              /d    #Z           9W  :"
			"W              ;\\     5V           BW        FW ?W   >W    NW   0W =W                     "
			"                                                     7W                                   "
			"                                                                                          "
			"            /\\ 0W                 HT                                                      "
			"                                                                                          "
			"                        I}              *[     NW           6W  :W              ;Z     3V "
			"          BW        FW ?W   >W    NW   0W =W                                              "
			"                            7W                                                            "
			"                                                                             /Z .W        "
			"                                                                                          "
			"                                                                                       =} "
			"                                                                                          "
			"                                                                                          "
			"                                                                                          "
			"                                    D" };

		// Define a 40x38 'danger' color logo (used by cimg::dialog()).
		static const unsigned char logo40x38[4576] = {
			177,200,200,200,3,123,123,0,36,200,200,200,1,123,123,0,2,255,255,0,1,189,189,189,1,0,0,0,34,200,200,200,
			1,123,123,0,4,255,255,0,1,189,189,189,1,0,0,0,1,123,123,123,32,200,200,200,1,123,123,0,5,255,255,0,1,0,0,
			0,2,123,123,123,30,200,200,200,1,123,123,0,6,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,29,200,200,200,
			1,123,123,0,7,255,255,0,1,0,0,0,2,123,123,123,28,200,200,200,1,123,123,0,8,255,255,0,1,189,189,189,1,0,0,0,
			2,123,123,123,27,200,200,200,1,123,123,0,9,255,255,0,1,0,0,0,2,123,123,123,26,200,200,200,1,123,123,0,10,255,
			255,0,1,189,189,189,1,0,0,0,2,123,123,123,25,200,200,200,1,123,123,0,3,255,255,0,1,189,189,189,3,0,0,0,1,189,
			189,189,3,255,255,0,1,0,0,0,2,123,123,123,24,200,200,200,1,123,123,0,4,255,255,0,5,0,0,0,3,255,255,0,1,189,
			189,189,1,0,0,0,2,123,123,123,23,200,200,200,1,123,123,0,4,255,255,0,5,0,0,0,4,255,255,0,1,0,0,0,2,123,123,123,
			22,200,200,200,1,123,123,0,5,255,255,0,5,0,0,0,4,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,21,200,200,200,
			1,123,123,0,5,255,255,0,5,0,0,0,5,255,255,0,1,0,0,0,2,123,123,123,20,200,200,200,1,123,123,0,6,255,255,0,5,0,0,
			0,5,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,19,200,200,200,1,123,123,0,6,255,255,0,1,123,123,0,3,0,0,0,1,
			123,123,0,6,255,255,0,1,0,0,0,2,123,123,123,18,200,200,200,1,123,123,0,7,255,255,0,1,189,189,189,3,0,0,0,1,189,
			189,189,6,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,17,200,200,200,1,123,123,0,8,255,255,0,3,0,0,0,8,255,255,
			0,1,0,0,0,2,123,123,123,16,200,200,200,1,123,123,0,9,255,255,0,1,123,123,0,1,0,0,0,1,123,123,0,8,255,255,0,1,189,
			189,189,1,0,0,0,2,123,123,123,15,200,200,200,1,123,123,0,9,255,255,0,1,189,189,189,1,0,0,0,1,189,189,189,9,255,
			255,0,1,0,0,0,2,123,123,123,14,200,200,200,1,123,123,0,11,255,255,0,1,0,0,0,10,255,255,0,1,189,189,189,1,0,0,0,2,
			123,123,123,13,200,200,200,1,123,123,0,23,255,255,0,1,0,0,0,2,123,123,123,12,200,200,200,1,123,123,0,11,255,255,0,
			1,189,189,189,2,0,0,0,1,189,189,189,9,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,11,200,200,200,1,123,123,0,11,
			255,255,0,4,0,0,0,10,255,255,0,1,0,0,0,2,123,123,123,10,200,200,200,1,123,123,0,12,255,255,0,4,0,0,0,10,255,255,0,
			1,189,189,189,1,0,0,0,2,123,123,123,9,200,200,200,1,123,123,0,12,255,255,0,1,189,189,189,2,0,0,0,1,189,189,189,11,
			255,255,0,1,0,0,0,2,123,123,123,9,200,200,200,1,123,123,0,27,255,255,0,1,0,0,0,3,123,123,123,8,200,200,200,1,123,
			123,0,26,255,255,0,1,189,189,189,1,0,0,0,3,123,123,123,9,200,200,200,1,123,123,0,24,255,255,0,1,189,189,189,1,0,0,
			0,4,123,123,123,10,200,200,200,1,123,123,0,24,0,0,0,5,123,123,123,12,200,200,200,27,123,123,123,14,200,200,200,25,
			123,123,123,86,200,200,200,91,49,124,118,124,71,32,124,95,49,56,114,52,82,121,0 };

		//! Get/set default output stream for the \CImg library messages.
		/**
		\param file Desired output stream. Set to \c 0 to get the currently used output stream only.
		\return Currently used output stream.
		**/
		inline std::FILE* output(std::FILE *file) {
			cimg::mutex(1);
			static std::FILE *res = cimg::_stderr();
			if (file) res = file;
			cimg::mutex(1, 0);
			return res;
		}

		// Return number of available CPU cores.
		inline unsigned int nb_cpus() {
			unsigned int res = 1;
#if cimg_OS==2
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			res = (unsigned int)sysinfo.dwNumberOfProcessors;
#elif cimg_OS == 1
			res = (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
			return res ? res : 1U;
		}

		// Lock/unlock mutex for CImg multi-thread programming.
		inline int mutex(const unsigned int n, const int lock_mode) {
			switch (lock_mode) {
			case 0: cimg::Mutex_attr().unlock(n); return 0;
			case 1: cimg::Mutex_attr().lock(n); return 0;
			default: return cimg::Mutex_attr().trylock(n);
			}
		}

		//! Display a warning message on the default output stream.
		/**
		\param format C-string containing the format of the message, as with <tt>std::printf()</tt>.
		\note If configuration macro \c cimg_strict_warnings is set, this function throws a
		\c CImgWarningException instead.
		\warning As the first argument is a format string, it is highly recommended to write
		\code
		cimg::warn("%s",warning_message);
		\endcode
		instead of
		\code
		cimg::warn(warning_message);
		\endcode
		if \c warning_message can be arbitrary, to prevent nasty memory access.
		**/
		inline void warn(const char *const format, ...) {
			if (cimg::exception_mode() >= 1) {
				char *const message = new char[16384];
				std::va_list ap;
				va_start(ap, format);
				cimg_vsnprintf(message, 16384, format, ap);
				va_end(ap);
#ifdef cimg_strict_warnings
				throw CImgWarningException(message);
#else
				std::fprintf(cimg::output(), "\n%s[CImg] *** Warning ***%s%s\n", cimg::t_red, cimg::t_normal, message);
#endif
				delete[] message;
			}
		}

		// Execute an external system command.
		/**
		\param command C-string containing the command line to execute.
		\param module_name Module name.
		\return Status value of the executed command, whose meaning is OS-dependent.
		\note This function is similar to <tt>std::system()</tt>
		but it does not open an extra console windows
		on Windows-based systems.
		**/
		inline int system(const char *const command, const char *const module_name = 0) {
			cimg::unused(module_name);
#ifdef cimg_no_system_calls
			return -1;
#else
#if cimg_OS==1
			const unsigned int l = (unsigned int)std::strlen(command);
			if (l) {
				char *const ncommand = new char[l + 16];
				std::strncpy(ncommand, command, l);
				std::strcpy(ncommand + l, " 2> /dev/null"); // Make command silent.
				const int out_val = std::system(ncommand);
				delete[] ncommand;
				return out_val;
			}
			else return -1;
#elif cimg_OS==2
			PROCESS_INFORMATION pi;
			STARTUPINFO si;
			std::memset(&pi, 0, sizeof(PROCESS_INFORMATION));
			std::memset(&si, 0, sizeof(STARTUPINFO));
			GetStartupInfo(&si);
			si.cb = sizeof(si);
			si.wShowWindow = SW_HIDE;
			si.dwFlags |= SW_HIDE | STARTF_USESHOWWINDOW;
			const BOOL res = CreateProcess((LPCTSTR)module_name, (LPTSTR)command, 0, 0, FALSE, 0, 0, 0, &si, &pi);
			if (res) {
				WaitForSingleObject(pi.hProcess, INFINITE);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				return 0;
			}
			else return std::system(command);
#else
			return std::system(command);
#endif
#endif
		}

		//! Return a reference to a temporary variable of type T.
		template<typename T>
		inline T& temporary(const T&) {
			static T temp;
			return temp;
		}

		//! Exchange values of variables \c a and \c b.
		template<typename T>
		inline void swap(T& a, T& b) { T t = a; a = b; b = t; }

		//! Exchange values of variables (\c a1,\c a2) and (\c b1,\c b2).
		template<typename T1, typename T2>
		inline void swap(T1& a1, T1& b1, T2& a2, T2& b2) {
			cimg::swap(a1, b1); cimg::swap(a2, b2);
		}

		//! Exchange values of variables (\c a1,\c a2,\c a3) and (\c b1,\c b2,\c b3).
		template<typename T1, typename T2, typename T3>
		inline void swap(T1& a1, T1& b1, T2& a2, T2& b2, T3& a3, T3& b3) {
			cimg::swap(a1, b1, a2, b2); cimg::swap(a3, b3);
		}

		//! Exchange values of variables (\c a1,\c a2,...,\c a4) and (\c b1,\c b2,...,\c b4).
		template<typename T1, typename T2, typename T3, typename T4>
		inline void swap(T1& a1, T1& b1, T2& a2, T2& b2, T3& a3, T3& b3, T4& a4, T4& b4) {
			cimg::swap(a1, b1, a2, b2, a3, b3); cimg::swap(a4, b4);
		}

		//! Exchange values of variables (\c a1,\c a2,...,\c a5) and (\c b1,\c b2,...,\c b5).
		template<typename T1, typename T2, typename T3, typename T4, typename T5>
		inline void swap(T1& a1, T1& b1, T2& a2, T2& b2, T3& a3, T3& b3, T4& a4, T4& b4, T5& a5, T5& b5) {
			cimg::swap(a1, b1, a2, b2, a3, b3, a4, b4); cimg::swap(a5, b5);
		}

		//! Exchange values of variables (\c a1,\c a2,...,\c a6) and (\c b1,\c b2,...,\c b6).
		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		inline void swap(T1& a1, T1& b1, T2& a2, T2& b2, T3& a3, T3& b3, T4& a4, T4& b4, T5& a5, T5& b5, T6& a6, T6& b6) {
			cimg::swap(a1, b1, a2, b2, a3, b3, a4, b4, a5, b5); cimg::swap(a6, b6);
		}

		//! Exchange values of variables (\c a1,\c a2,...,\c a7) and (\c b1,\c b2,...,\c b7).
		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		inline void swap(T1& a1, T1& b1, T2& a2, T2& b2, T3& a3, T3& b3, T4& a4, T4& b4, T5& a5, T5& b5, T6& a6, T6& b6,
			T7& a7, T7& b7) {
			cimg::swap(a1, b1, a2, b2, a3, b3, a4, b4, a5, b5, a6, b6); cimg::swap(a7, b7);
		}

		//! Exchange values of variables (\c a1,\c a2,...,\c a8) and (\c b1,\c b2,...,\c b8).
		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		inline void swap(T1& a1, T1& b1, T2& a2, T2& b2, T3& a3, T3& b3, T4& a4, T4& b4, T5& a5, T5& b5, T6& a6, T6& b6,
			T7& a7, T7& b7, T8& a8, T8& b8) {
			cimg::swap(a1, b1, a2, b2, a3, b3, a4, b4, a5, b5, a6, b6, a7, b7); cimg::swap(a8, b8);
		}

		//! Return the endianness of the current architecture.
		/**
		\return \c false for <i>Little Endian</i> or \c true for <i>Big Endian</i>.
		**/
		inline bool endianness() {
			const int x = 1;
			return ((unsigned char*)&x)[0] ? false : true;
		}

		//! Reverse endianness of all elements in a memory buffer.
		/**
		\param[in,out] buffer Memory buffer whose endianness must be reversed.
		\param size Number of buffer elements to reverse.
		**/
		template<typename T>
		inline void invert_endianness(T* const buffer, const cimg_ulong size) {
			if (size) switch (sizeof(T)) {
			case 1: break;
			case 2: { for (unsigned short *ptr = (unsigned short*)buffer + size; ptr>(unsigned short*)buffer; ) {
				const unsigned short val = *(--ptr);
				*ptr = (unsigned short)((val >> 8) | ((val << 8)));
			}
			} break;
			case 4: { for (unsigned int *ptr = (unsigned int*)buffer + size; ptr>(unsigned int*)buffer; ) {
				const unsigned int val = *(--ptr);
				*ptr = (val >> 24) | ((val >> 8) & 0xff00) | ((val << 8) & 0xff0000) | (val << 24);
			}
			} break;
			default: { for (T* ptr = buffer + size; ptr>buffer; ) {
				unsigned char *pb = (unsigned char*)(--ptr), *pe = pb + sizeof(T);
				for (int i = 0; i<(int)sizeof(T) / 2; ++i) swap(*(pb++), *(--pe));
			}
			}
			}
		}

		//! Reverse endianness of a single variable.
		/**
		\param[in,out] a Variable to reverse.
		\return Reference to reversed variable.
		**/
		template<typename T>
		inline T& invert_endianness(T& a) {
			invert_endianness(&a, 1);
			return a;
		}

		// Conversion functions to get more precision when trying to store unsigned ints values as floats.
		inline unsigned int float2uint(const float f) {
			int tmp = 0;
			std::memcpy(&tmp, &f, sizeof(float));
			if (tmp >= 0) return (unsigned int)f;
			unsigned int u;
			// use memcpy instead of assignment to avoid undesired optimizations by C++-compiler.
			std::memcpy(&u, &f, sizeof(float));
			return ((u) << 1) >> 1; // set sign bit to 0.
		}

		inline float uint2float(const unsigned int u) {
			if (u<(1U << 19)) return (float)u;  // Consider safe storage of unsigned int as floats until 19bits (i.e 524287).
			float f;
			const unsigned int v = u | (1U << (8 * sizeof(unsigned int) - 1)); // set sign bit to 1.
																			   // use memcpy instead of simple assignment to avoid undesired optimizations by C++-compiler.
			std::memcpy(&f, &v, sizeof(float));
			return f;
		}

		//! Return the value of a system timer, with a millisecond precision.
		/**
		\note The timer does not necessarily starts from \c 0.
		**/
		inline cimg_ulong time() {
#if cimg_OS==1
			struct timeval st_time;
			gettimeofday(&st_time, 0);
			return (cimg_ulong)(st_time.tv_usec / 1000 + st_time.tv_sec * 1000);
#elif cimg_OS==2
			SYSTEMTIME st_time;
			GetLocalTime(&st_time);
			return (cimg_ulong)(st_time.wMilliseconds + 1000 * (st_time.wSecond + 60 * (st_time.wMinute + 60 * st_time.wHour)));
#else
			return 0;
#endif
		}

		// Implement a tic/toc mechanism to display elapsed time of algorithms.
		inline cimg_ulong tictoc(const bool is_tic);

		//! Start tic/toc timer for time measurement between code instructions.
		/**
		\return Current value of the timer (same value as time()).
		**/
		inline cimg_ulong tic() {
			return cimg::tictoc(true);
		}

		//! End tic/toc timer and displays elapsed time from last call to tic().
		/**
		\return Time elapsed (in ms) since last call to tic().
		**/
		inline cimg_ulong toc() {
			return cimg::tictoc(false);
		}

		//! Sleep for a given numbers of milliseconds.
		/**
		\param milliseconds Number of milliseconds to wait for.
		\note This function frees the CPU ressources during the sleeping time.
		It can be used to temporize your program properly, without wasting CPU time.
		**/
		inline void sleep(const unsigned int milliseconds) {
#if cimg_OS==1
			struct timespec tv;
			tv.tv_sec = milliseconds / 1000;
			tv.tv_nsec = (milliseconds % 1000) * 1000000;
			nanosleep(&tv, 0);
#elif cimg_OS==2
			Sleep(milliseconds);
#else
			cimg::unused(milliseconds);
#endif
		}

		inline unsigned int _wait(const unsigned int milliseconds, cimg_ulong& timer) {
			if (!timer) timer = cimg::time();
			const cimg_ulong current_time = cimg::time();
			if (current_time >= timer + milliseconds) { timer = current_time; return 0; }
			const unsigned int time_diff = (unsigned int)(timer + milliseconds - current_time);
			timer = current_time + time_diff;
			cimg::sleep(time_diff);
			return time_diff;
		}

		//! Wait for a given number of milliseconds since the last call to wait().
		/**
		\param milliseconds Number of milliseconds to wait for.
		\return Number of milliseconds elapsed since the last call to wait().
		\note Same as sleep() with a waiting time computed with regard to the last call
		of wait(). It may be used to temporize your program properly, without wasting CPU time.
		**/
		inline cimg_long wait(const unsigned int milliseconds) {
			cimg::mutex(3);
			static cimg_ulong timer = 0;
			if (!timer) timer = cimg::time();
			cimg::mutex(3, 0);
			return _wait(milliseconds, timer);
		}

		// Random number generators.
		// CImg may use its own Random Number Generator (RNG) if configuration macro 'cimg_use_rng' is set.
		// Use it for instance when you have to deal with concurrent threads trying to call std::srand()
		// at the same time!
#ifdef cimg_use_rng

#include <stdint.h>

		// Use a custom RNG.
		inline unsigned int _rand(const unsigned int seed = 0, const bool set_seed = false) {
			static cimg_ulong next = 0xB16B00B5;
			cimg::mutex(4);
			if (set_seed) next = (cimg_ulong)seed;
			else next = next * 1103515245 + 12345U;
			cimg::mutex(4, 0);
			return (unsigned int)(next & 0xFFFFFFU);
		}

		inline unsigned int srand() {
			unsigned int t = (unsigned int)cimg::time();
#if cimg_OS==1
			t += (unsigned int)getpid();
#elif cimg_OS==2
			t += (unsigned int)_getpid();
#endif
			return cimg::_rand(t, true);
		}

		inline unsigned int srand(const unsigned int seed) {
			return _rand(seed, true);
		}

		inline double rand(const double val_min, const double val_max) {
			const double val = cimg::_rand() / 16777215.;
			return val_min + (val_max - val_min)*val;
		}

#else

		// Use the system RNG.
		inline unsigned int srand() {
			const unsigned int t = (unsigned int)cimg::time();
#if cimg_OS==1 || defined(__BORLANDC__)
			std::srand(t + (unsigned int)getpid());
#elif cimg_OS==2
			std::srand(t + (unsigned int)_getpid());
#else
			std::srand(t);
#endif
			return t;
		}

		inline unsigned int srand(const unsigned int seed) {
			std::srand(seed);
			return seed;
		}

		//! Return a random variable uniformely distributed between [val_min,val_max].
		/**
		**/
		inline double rand(const double val_min, const double val_max) {
			const double val = (double)std::rand() / RAND_MAX;
			return val_min + (val_max - val_min)*val;
		}
#endif

		//! Return a random variable uniformely distributed between [0,val_max].
		/**
		**/
		inline double rand(const double val_max = 1) {
			return cimg::rand(0, val_max);
		}

		//! Return a random variable following a gaussian distribution and a standard deviation of 1.
		/**
		**/
		inline double grand() {
			double x1, w;
			do {
				const double x2 = cimg::rand(-1, 1);
				x1 = cimg::rand(-1, 1);
				w = x1*x1 + x2*x2;
			} while (w <= 0 || w >= 1.0);
			return x1*std::sqrt((-2 * std::log(w)) / w);
		}

		//! Return a random variable following a Poisson distribution of parameter z.
		/**
		**/
		inline unsigned int prand(const double z) {
			if (z <= 1.0e-10) return 0;
			if (z>100) return (unsigned int)((std::sqrt(z) * cimg::grand()) + z);
			unsigned int k = 0;
			const double y = std::exp(-z);
			for (double s = 1.0; s >= y; ++k) s *= cimg::rand();
			return k - 1;
		}

		//! Cut (i.e. clamp) value in specified interval.
		template<typename T, typename t>
		inline T cut(const T& val, const t& val_min, const t& val_max) {
			return val<val_min ? (T)val_min : val>val_max ? (T)val_max : val;
		}

		//! Bitwise-rotate value on the left.
		template<typename T>
		inline T rol(const T& a, const unsigned int n = 1) {
			return n ? (T)((a << n) | (a >> ((sizeof(T) << 3) - n))) : a;
		}

		inline float rol(const float a, const unsigned int n = 1) {
			return (float)rol((int)a, n);
		}

		inline double rol(const double a, const unsigned int n = 1) {
			return (double)rol((cimg_long)a, n);
		}

		inline double rol(const long double a, const unsigned int n = 1) {
			return (double)rol((cimg_long)a, n);
		}

#ifdef cimg_use_half
		inline half rol(const half a, const unsigned int n = 1) {
			return (half)rol((int)a, n);
		}
#endif

		//! Bitwise-rotate value on the right.
		template<typename T>
		inline T ror(const T& a, const unsigned int n = 1) {
			return n ? (T)((a >> n) | (a << ((sizeof(T) << 3) - n))) : a;
		}

		inline float ror(const float a, const unsigned int n = 1) {
			return (float)ror((int)a, n);
		}

		inline double ror(const double a, const unsigned int n = 1) {
			return (double)ror((cimg_long)a, n);
		}

		inline double ror(const long double a, const unsigned int n = 1) {
			return (double)ror((cimg_long)a, n);
		}

#ifdef cimg_use_half
		inline half ror(const half a, const unsigned int n = 1) {
			return (half)ror((int)a, n);
		}
#endif

		//! Return absolute value of a value.
		template<typename T>
		inline T abs(const T& a) {
			return a >= 0 ? a : -a;
		}
		inline bool abs(const bool a) {
			return a;
		}
		inline int abs(const unsigned char a) {
			return (int)a;
		}
		inline int abs(const unsigned short a) {
			return (int)a;
		}
		inline int abs(const unsigned int a) {
			return (int)a;
		}
		inline int abs(const int a) {
			return std::abs(a);
		}
		inline cimg_int64 abs(const cimg_uint64 a) {
			return (cimg_int64)a;
		}
		inline double abs(const double a) {
			return std::fabs(a);
		}
		inline float abs(const float a) {
			return (float)std::fabs((double)a);
		}

		//! Return square of a value.
		template<typename T>
		inline T sqr(const T& val) {
			return val*val;
		}

		//! Return <tt>1 + log_10(x)</tt> of a value \c x.
		inline int xln(const int x) {
			return x>0 ? (int)(1 + std::log10((double)x)) : 1;
		}

		//! Return the minimum between three values.
		template<typename t>
		inline t min(const t& a, const t& b, const t& c) {
			return std::min(std::min(a, b), c);
		}

		//! Return the minimum between four values.
		template<typename t>
		inline t min(const t& a, const t& b, const t& c, const t& d) {
			return std::min(std::min(a, b), std::min(c, d));
		}

		//! Return the maximum between three values.
		template<typename t>
		inline t max(const t& a, const t& b, const t& c) {
			return std::max(std::max(a, b), c);
		}

		//! Return the maximum between four values.
		template<typename t>
		inline t max(const t& a, const t& b, const t& c, const t& d) {
			return std::max(std::max(a, b), std::max(c, d));
		}

		//! Return the sign of a value.
		template<typename T>
		inline T sign(const T& x) {
			return (T)(x<0 ? -1 : x>0);
		}

		//! Return the nearest power of 2 higher than given value.
		template<typename T>
		inline cimg_ulong nearest_pow2(const T& x) {
			cimg_ulong i = 1;
			while (x>i) i <<= 1;
			return i;
		}

		//! Return the sinc of a given value.
		inline double sinc(const double x) {
			return x ? std::sin(x) / x : 1;
		}

		//! Return the modulo of a value.
		/**
		\param x Input value.
		\param m Modulo value.
		\note This modulo function accepts negative and floating-points modulo numbers, as well as variables of any type.
		**/
		template<typename T>
		inline T mod(const T& x, const T& m) {
			const double dx = (double)x, dm = (double)m;
			return (T)(dx - dm * std::floor(dx / dm));
		}
		inline int mod(const bool x, const bool m) {
			return m ? (x ? 1 : 0) : 0;
		}
		inline int mod(const unsigned char x, const unsigned char m) {
			return x%m;
		}
		inline int mod(const char x, const char m) {
#if defined(CHAR_MAX) && CHAR_MAX==255
			return x%m;
#else
			return x >= 0 ? x%m : (x%m ? m + x%m : 0);
#endif
		}
		inline int mod(const unsigned short x, const unsigned short m) {
			return x%m;
		}
		inline int mod(const short x, const short m) {
			return x >= 0 ? x%m : (x%m ? m + x%m : 0);
		}
		inline int mod(const unsigned int x, const unsigned int m) {
			return (int)(x%m);
		}
		inline int mod(const int x, const int m) {
			return x >= 0 ? x%m : (x%m ? m + x%m : 0);
		}
		inline cimg_int64 mod(const cimg_uint64 x, const cimg_uint64 m) {
			return x%m;
		}
		inline cimg_int64 mod(const cimg_int64 x, const cimg_int64 m) {
			return x >= 0 ? x%m : (x%m ? m + x%m : 0);
		}

		//! Return the min-mod of two values.
		/**
		\note <i>minmod(\p a,\p b)</i> is defined to be:
		- <i>minmod(\p a,\p b) = min(\p a,\p b)</i>, if \p a and \p b have the same sign.
		- <i>minmod(\p a,\p b) = 0</i>, if \p a and \p b have different signs.
		**/
		template<typename T>
		inline T minmod(const T& a, const T& b) {
			return a*b <= 0 ? 0 : (a>0 ? (a<b ? a : b) : (a<b ? b : a));
		}

		//! Return base-2 logarithm of a value.
		inline double log2(const double x) {
			const double base = std::log(2.0);
			return std::log(x) / base;
		}

		template<typename T>
		inline T round(const T& x) {
			return (T)std::floor((_cimg_Tfloat)x + 0.5f);
		}

		//! Return rounded value.
		/**
		\param x Value to be rounded.
		\param y Rounding precision.
		\param rounding_type Type of rounding operation (\c 0 = nearest, \c -1 = backward, \c 1 = forward).
		\return Rounded value, having the same type as input value \c x.
		**/
		template<typename T>
		inline T round(const T& x, const double y, const int rounding_type = 0) {
			if (y <= 0) return x;
			if (y == 1) switch (rounding_type) {
			case 0: return round(x);
			case 1: return (T)std::ceil((_cimg_Tfloat)x);
			default: return (T)std::floor((_cimg_Tfloat)x);
			}
			const double sx = (double)x / y, floor = std::floor(sx), delta = sx - floor;
			return (T)(y*(rounding_type<0 ? floor : rounding_type>0 ? std::ceil(sx) : delta<0.5 ? floor : std::ceil(sx)));
		}

		//! Return x^(1/3).
		template<typename T>
		inline double cbrt(const T& x) {
#if cimg_use_cpp11==1
			return std::cbrt(x);
#else
			return x >= 0 ? std::pow((double)x, 1.0 / 3) : -std::pow(-(double)x, 1.0 / 3);
#endif
		}

		// Code to compute fast median from 2,3,5,7,9,13,25 and 49 values.
		// (contribution by RawTherapee: http://rawtherapee.com/).
		template<typename T>
		inline T median(T val0, T val1) {
			return (val0 + val1) / 2;
		}

		template<typename T>
		inline T median(T val0, T val1, T val2) {
			return std::max(std::min(val0, val1), std::min(val2, std::max(val0, val1)));
		}

		template<typename T>
		inline T median(T val0, T val1, T val2, T val3, T val4) {
			T tmp = std::min(val0, val1);
			val1 = std::max(val0, val1); val0 = tmp; tmp = std::min(val3, val4); val4 = std::max(val3, val4);
			val3 = std::max(val0, tmp);  val1 = std::min(val1, val4); tmp = std::min(val1, val2); val2 = std::max(val1, val2);
			val1 = tmp; tmp = std::min(val2, val3);
			return std::max(val1, tmp);
		}

		template<typename T>
		inline T median(T val0, T val1, T val2, T val3, T val4, T val5, T val6) {
			T tmp = std::min(val0, val5);
			val5 = std::max(val0, val5); val0 = tmp; tmp = std::min(val0, val3); val3 = std::max(val0, val3); val0 = tmp;
			tmp = std::min(val1, val6); val6 = std::max(val1, val6); val1 = tmp; tmp = std::min(val2, val4);
			val4 = std::max(val2, val4); val2 = tmp; val1 = std::max(val0, val1); tmp = std::min(val3, val5);
			val5 = std::max(val3, val5); val3 = tmp; tmp = std::min(val2, val6); val6 = std::max(val2, val6);
			val3 = std::max(tmp, val3); val3 = std::min(val3, val6); tmp = std::min(val4, val5); val4 = std::max(val1, tmp);
			tmp = std::min(val1, tmp); val3 = std::max(tmp, val3);
			return std::min(val3, val4);
		}

		template<typename T>
		inline T median(T val0, T val1, T val2, T val3, T val4, T val5, T val6, T val7, T val8) {
			T tmp = std::min(val1, val2);
			val2 = std::max(val1, val2); val1 = tmp; tmp = std::min(val4, val5);
			val5 = std::max(val4, val5); val4 = tmp; tmp = std::min(val7, val8);
			val8 = std::max(val7, val8); val7 = tmp; tmp = std::min(val0, val1);
			val1 = std::max(val0, val1); val0 = tmp; tmp = std::min(val3, val4);
			val4 = std::max(val3, val4); val3 = tmp; tmp = std::min(val6, val7);
			val7 = std::max(val6, val7); val6 = tmp; tmp = std::min(val1, val2);
			val2 = std::max(val1, val2); val1 = tmp; tmp = std::min(val4, val5);
			val5 = std::max(val4, val5); val4 = tmp; tmp = std::min(val7, val8);
			val8 = std::max(val7, val8); val3 = std::max(val0, val3); val5 = std::min(val5, val8);
			val7 = std::max(val4, tmp); tmp = std::min(val4, tmp); val6 = std::max(val3, val6);
			val4 = std::max(val1, tmp); val2 = std::min(val2, val5); val4 = std::min(val4, val7);
			tmp = std::min(val4, val2); val2 = std::max(val4, val2); val4 = std::max(val6, tmp);
			return std::min(val4, val2);
		}

		template<typename T>
		inline T median(T val0, T val1, T val2, T val3, T val4, T val5, T val6, T val7, T val8, T val9, T val10, T val11,
			T val12) {
			T tmp = std::min(val1, val7);
			val7 = std::max(val1, val7); val1 = tmp; tmp = std::min(val9, val11); val11 = std::max(val9, val11); val9 = tmp;
			tmp = std::min(val3, val4);  val4 = std::max(val3, val4); val3 = tmp; tmp = std::min(val5, val8);
			val8 = std::max(val5, val8); val5 = tmp; tmp = std::min(val0, val12); val12 = std::max(val0, val12);
			val0 = tmp; tmp = std::min(val2, val6); val6 = std::max(val2, val6); val2 = tmp; tmp = std::min(val0, val1);
			val1 = std::max(val0, val1); val0 = tmp; tmp = std::min(val2, val3); val3 = std::max(val2, val3); val2 = tmp;
			tmp = std::min(val4, val6);  val6 = std::max(val4, val6); val4 = tmp; tmp = std::min(val8, val11);
			val11 = std::max(val8, val11); val8 = tmp; tmp = std::min(val7, val12); val12 = std::max(val7, val12); val7 = tmp;
			tmp = std::min(val5, val9); val9 = std::max(val5, val9); val5 = tmp; tmp = std::min(val0, val2);
			val2 = std::max(val0, val2); val0 = tmp; tmp = std::min(val3, val7); val7 = std::max(val3, val7); val3 = tmp;
			tmp = std::min(val10, val11); val11 = std::max(val10, val11); val10 = tmp; tmp = std::min(val1, val4);
			val4 = std::max(val1, val4); val1 = tmp; tmp = std::min(val6, val12); val12 = std::max(val6, val12); val6 = tmp;
			tmp = std::min(val7, val8); val8 = std::max(val7, val8); val7 = tmp; val11 = std::min(val11, val12);
			tmp = std::min(val4, val9); val9 = std::max(val4, val9); val4 = tmp; tmp = std::min(val6, val10);
			val10 = std::max(val6, val10); val6 = tmp; tmp = std::min(val3, val4); val4 = std::max(val3, val4); val3 = tmp;
			tmp = std::min(val5, val6); val6 = std::max(val5, val6); val5 = tmp; val8 = std::min(val8, val9);
			val10 = std::min(val10, val11); tmp = std::min(val1, val7); val7 = std::max(val1, val7); val1 = tmp;
			tmp = std::min(val2, val6); val6 = std::max(val2, val6); val2 = tmp; val3 = std::max(val1, val3);
			tmp = std::min(val4, val7); val7 = std::max(val4, val7); val4 = tmp; val8 = std::min(val8, val10);
			val5 = std::max(val0, val5); val5 = std::max(val2, val5); tmp = std::min(val6, val8); val8 = std::max(val6, val8);
			val5 = std::max(val3, val5); val7 = std::min(val7, val8); val6 = std::max(val4, tmp); tmp = std::min(val4, tmp);
			val5 = std::max(tmp, val5); val6 = std::min(val6, val7);
			return std::max(val5, val6);
		}

		template<typename T>
		inline T median(T val0, T val1, T val2, T val3, T val4,
			T val5, T val6, T val7, T val8, T val9,
			T val10, T val11, T val12, T val13, T val14,
			T val15, T val16, T val17, T val18, T val19,
			T val20, T val21, T val22, T val23, T val24) {
			T tmp = std::min(val0, val1);
			val1 = std::max(val0, val1); val0 = tmp; tmp = std::min(val3, val4); val4 = std::max(val3, val4);
			val3 = tmp; tmp = std::min(val2, val4); val4 = std::max(val2, val4); val2 = std::min(tmp, val3);
			val3 = std::max(tmp, val3); tmp = std::min(val6, val7); val7 = std::max(val6, val7); val6 = tmp;
			tmp = std::min(val5, val7); val7 = std::max(val5, val7); val5 = std::min(tmp, val6); val6 = std::max(tmp, val6);
			tmp = std::min(val9, val10); val10 = std::max(val9, val10); val9 = tmp; tmp = std::min(val8, val10);
			val10 = std::max(val8, val10); val8 = std::min(tmp, val9); val9 = std::max(tmp, val9);
			tmp = std::min(val12, val13); val13 = std::max(val12, val13); val12 = tmp; tmp = std::min(val11, val13);
			val13 = std::max(val11, val13); val11 = std::min(tmp, val12); val12 = std::max(tmp, val12);
			tmp = std::min(val15, val16); val16 = std::max(val15, val16); val15 = tmp; tmp = std::min(val14, val16);
			val16 = std::max(val14, val16); val14 = std::min(tmp, val15); val15 = std::max(tmp, val15);
			tmp = std::min(val18, val19); val19 = std::max(val18, val19); val18 = tmp; tmp = std::min(val17, val19);
			val19 = std::max(val17, val19); val17 = std::min(tmp, val18); val18 = std::max(tmp, val18);
			tmp = std::min(val21, val22); val22 = std::max(val21, val22); val21 = tmp; tmp = std::min(val20, val22);
			val22 = std::max(val20, val22); val20 = std::min(tmp, val21); val21 = std::max(tmp, val21);
			tmp = std::min(val23, val24); val24 = std::max(val23, val24); val23 = tmp; tmp = std::min(val2, val5);
			val5 = std::max(val2, val5); val2 = tmp; tmp = std::min(val3, val6); val6 = std::max(val3, val6); val3 = tmp;
			tmp = std::min(val0, val6); val6 = std::max(val0, val6); val0 = std::min(tmp, val3); val3 = std::max(tmp, val3);
			tmp = std::min(val4, val7); val7 = std::max(val4, val7); val4 = tmp; tmp = std::min(val1, val7);
			val7 = std::max(val1, val7); val1 = std::min(tmp, val4); val4 = std::max(tmp, val4); tmp = std::min(val11, val14);
			val14 = std::max(val11, val14); val11 = tmp; tmp = std::min(val8, val14); val14 = std::max(val8, val14);
			val8 = std::min(tmp, val11); val11 = std::max(tmp, val11); tmp = std::min(val12, val15);
			val15 = std::max(val12, val15); val12 = tmp; tmp = std::min(val9, val15); val15 = std::max(val9, val15);
			val9 = std::min(tmp, val12); val12 = std::max(tmp, val12); tmp = std::min(val13, val16);
			val16 = std::max(val13, val16); val13 = tmp; tmp = std::min(val10, val16); val16 = std::max(val10, val16);
			val10 = std::min(tmp, val13); val13 = std::max(tmp, val13); tmp = std::min(val20, val23);
			val23 = std::max(val20, val23); val20 = tmp; tmp = std::min(val17, val23); val23 = std::max(val17, val23);
			val17 = std::min(tmp, val20); val20 = std::max(tmp, val20); tmp = std::min(val21, val24);
			val24 = std::max(val21, val24); val21 = tmp; tmp = std::min(val18, val24); val24 = std::max(val18, val24);
			val18 = std::min(tmp, val21); val21 = std::max(tmp, val21); tmp = std::min(val19, val22);
			val22 = std::max(val19, val22); val19 = tmp; val17 = std::max(val8, val17); tmp = std::min(val9, val18);
			val18 = std::max(val9, val18); val9 = tmp; tmp = std::min(val0, val18); val18 = std::max(val0, val18);
			val9 = std::max(tmp, val9); tmp = std::min(val10, val19); val19 = std::max(val10, val19); val10 = tmp;
			tmp = std::min(val1, val19); val19 = std::max(val1, val19); val1 = std::min(tmp, val10);
			val10 = std::max(tmp, val10); tmp = std::min(val11, val20); val20 = std::max(val11, val20); val11 = tmp;
			tmp = std::min(val2, val20); val20 = std::max(val2, val20); val11 = std::max(tmp, val11);
			tmp = std::min(val12, val21); val21 = std::max(val12, val21); val12 = tmp; tmp = std::min(val3, val21);
			val21 = std::max(val3, val21); val3 = std::min(tmp, val12); val12 = std::max(tmp, val12);
			tmp = std::min(val13, val22); val22 = std::max(val13, val22); val4 = std::min(val4, val22);
			val13 = std::max(val4, tmp); tmp = std::min(val4, tmp); val4 = tmp; tmp = std::min(val14, val23);
			val23 = std::max(val14, val23); val14 = tmp; tmp = std::min(val5, val23); val23 = std::max(val5, val23);
			val5 = std::min(tmp, val14); val14 = std::max(tmp, val14); tmp = std::min(val15, val24);
			val24 = std::max(val15, val24); val15 = tmp; val6 = std::min(val6, val24); tmp = std::min(val6, val15);
			val15 = std::max(val6, val15); val6 = tmp; tmp = std::min(val7, val16); val7 = std::min(tmp, val19);
			tmp = std::min(val13, val21); val15 = std::min(val15, val23); tmp = std::min(val7, tmp);
			val7 = std::min(tmp, val15); val9 = std::max(val1, val9); val11 = std::max(val3, val11);
			val17 = std::max(val5, val17); val17 = std::max(val11, val17); val17 = std::max(val9, val17);
			tmp = std::min(val4, val10); val10 = std::max(val4, val10); val4 = tmp; tmp = std::min(val6, val12);
			val12 = std::max(val6, val12); val6 = tmp; tmp = std::min(val7, val14); val14 = std::max(val7, val14);
			val7 = tmp; tmp = std::min(val4, val6); val6 = std::max(val4, val6); val7 = std::max(tmp, val7);
			tmp = std::min(val12, val14); val14 = std::max(val12, val14); val12 = tmp; val10 = std::min(val10, val14);
			tmp = std::min(val6, val7); val7 = std::max(val6, val7); val6 = tmp; tmp = std::min(val10, val12);
			val12 = std::max(val10, val12); val10 = std::max(val6, tmp); tmp = std::min(val6, tmp);
			val17 = std::max(tmp, val17); tmp = std::min(val12, val17); val17 = std::max(val12, val17); val12 = tmp;
			val7 = std::min(val7, val17); tmp = std::min(val7, val10); val10 = std::max(val7, val10); val7 = tmp;
			tmp = std::min(val12, val18); val18 = std::max(val12, val18); val12 = std::max(val7, tmp);
			val10 = std::min(val10, val18); tmp = std::min(val12, val20); val20 = std::max(val12, val20); val12 = tmp;
			tmp = std::min(val10, val20);
			return std::max(tmp, val12);
		}

		template<typename T>
		inline T median(T val0, T val1, T val2, T val3, T val4, T val5, T val6,
			T val7, T val8, T val9, T val10, T val11, T val12, T val13,
			T val14, T val15, T val16, T val17, T val18, T val19, T val20,
			T val21, T val22, T val23, T val24, T val25, T val26, T val27,
			T val28, T val29, T val30, T val31, T val32, T val33, T val34,
			T val35, T val36, T val37, T val38, T val39, T val40, T val41,
			T val42, T val43, T val44, T val45, T val46, T val47, T val48) {
			T tmp = std::min(val0, val32);
			val32 = std::max(val0, val32); val0 = tmp; tmp = std::min(val1, val33); val33 = std::max(val1, val33); val1 = tmp;
			tmp = std::min(val2, val34); val34 = std::max(val2, val34); val2 = tmp; tmp = std::min(val3, val35);
			val35 = std::max(val3, val35); val3 = tmp; tmp = std::min(val4, val36); val36 = std::max(val4, val36); val4 = tmp;
			tmp = std::min(val5, val37); val37 = std::max(val5, val37); val5 = tmp; tmp = std::min(val6, val38);
			val38 = std::max(val6, val38); val6 = tmp; tmp = std::min(val7, val39); val39 = std::max(val7, val39); val7 = tmp;
			tmp = std::min(val8, val40); val40 = std::max(val8, val40); val8 = tmp; tmp = std::min(val9, val41);
			val41 = std::max(val9, val41); val9 = tmp; tmp = std::min(val10, val42); val42 = std::max(val10, val42);
			val10 = tmp; tmp = std::min(val11, val43); val43 = std::max(val11, val43); val11 = tmp;
			tmp = std::min(val12, val44); val44 = std::max(val12, val44); val12 = tmp; tmp = std::min(val13, val45);
			val45 = std::max(val13, val45); val13 = tmp; tmp = std::min(val14, val46); val46 = std::max(val14, val46);
			val14 = tmp; tmp = std::min(val15, val47); val47 = std::max(val15, val47); val15 = tmp;
			tmp = std::min(val16, val48); val48 = std::max(val16, val48); val16 = tmp; tmp = std::min(val0, val16);
			val16 = std::max(val0, val16); val0 = tmp; tmp = std::min(val1, val17); val17 = std::max(val1, val17);
			val1 = tmp; tmp = std::min(val2, val18); val18 = std::max(val2, val18); val2 = tmp; tmp = std::min(val3, val19);
			val19 = std::max(val3, val19); val3 = tmp; tmp = std::min(val4, val20); val20 = std::max(val4, val20); val4 = tmp;
			tmp = std::min(val5, val21); val21 = std::max(val5, val21); val5 = tmp; tmp = std::min(val6, val22);
			val22 = std::max(val6, val22); val6 = tmp; tmp = std::min(val7, val23); val23 = std::max(val7, val23); val7 = tmp;
			tmp = std::min(val8, val24); val24 = std::max(val8, val24); val8 = tmp; tmp = std::min(val9, val25);
			val25 = std::max(val9, val25); val9 = tmp; tmp = std::min(val10, val26); val26 = std::max(val10, val26);
			val10 = tmp; tmp = std::min(val11, val27); val27 = std::max(val11, val27); val11 = tmp;
			tmp = std::min(val12, val28); val28 = std::max(val12, val28); val12 = tmp; tmp = std::min(val13, val29);
			val29 = std::max(val13, val29); val13 = tmp; tmp = std::min(val14, val30); val30 = std::max(val14, val30);
			val14 = tmp; tmp = std::min(val15, val31); val31 = std::max(val15, val31); val15 = tmp;
			tmp = std::min(val32, val48); val48 = std::max(val32, val48); val32 = tmp; tmp = std::min(val16, val32);
			val32 = std::max(val16, val32); val16 = tmp; tmp = std::min(val17, val33); val33 = std::max(val17, val33);
			val17 = tmp; tmp = std::min(val18, val34); val34 = std::max(val18, val34); val18 = tmp;
			tmp = std::min(val19, val35); val35 = std::max(val19, val35); val19 = tmp; tmp = std::min(val20, val36);
			val36 = std::max(val20, val36); val20 = tmp; tmp = std::min(val21, val37); val37 = std::max(val21, val37);
			val21 = tmp; tmp = std::min(val22, val38); val38 = std::max(val22, val38); val22 = tmp;
			tmp = std::min(val23, val39); val39 = std::max(val23, val39); val23 = tmp; tmp = std::min(val24, val40);
			val40 = std::max(val24, val40); val24 = tmp; tmp = std::min(val25, val41); val41 = std::max(val25, val41);
			val25 = tmp; tmp = std::min(val26, val42); val42 = std::max(val26, val42); val26 = tmp;
			tmp = std::min(val27, val43); val43 = std::max(val27, val43); val27 = tmp; tmp = std::min(val28, val44);
			val44 = std::max(val28, val44); val28 = tmp; tmp = std::min(val29, val45); val45 = std::max(val29, val45);
			val29 = tmp; tmp = std::min(val30, val46); val46 = std::max(val30, val46); val30 = tmp;
			tmp = std::min(val31, val47); val47 = std::max(val31, val47); val31 = tmp; tmp = std::min(val0, val8);
			val8 = std::max(val0, val8); val0 = tmp; tmp = std::min(val1, val9); val9 = std::max(val1, val9); val1 = tmp;
			tmp = std::min(val2, val10); val10 = std::max(val2, val10); val2 = tmp; tmp = std::min(val3, val11);
			val11 = std::max(val3, val11); val3 = tmp; tmp = std::min(val4, val12); val12 = std::max(val4, val12); val4 = tmp;
			tmp = std::min(val5, val13); val13 = std::max(val5, val13); val5 = tmp; tmp = std::min(val6, val14);
			val14 = std::max(val6, val14); val6 = tmp; tmp = std::min(val7, val15); val15 = std::max(val7, val15); val7 = tmp;
			tmp = std::min(val16, val24); val24 = std::max(val16, val24); val16 = tmp; tmp = std::min(val17, val25);
			val25 = std::max(val17, val25); val17 = tmp; tmp = std::min(val18, val26); val26 = std::max(val18, val26);
			val18 = tmp; tmp = std::min(val19, val27); val27 = std::max(val19, val27); val19 = tmp;
			tmp = std::min(val20, val28); val28 = std::max(val20, val28); val20 = tmp; tmp = std::min(val21, val29);
			val29 = std::max(val21, val29); val21 = tmp; tmp = std::min(val22, val30); val30 = std::max(val22, val30);
			val22 = tmp; tmp = std::min(val23, val31); val31 = std::max(val23, val31); val23 = tmp;
			tmp = std::min(val32, val40); val40 = std::max(val32, val40); val32 = tmp; tmp = std::min(val33, val41);
			val41 = std::max(val33, val41); val33 = tmp; tmp = std::min(val34, val42); val42 = std::max(val34, val42);
			val34 = tmp; tmp = std::min(val35, val43); val43 = std::max(val35, val43); val35 = tmp;
			tmp = std::min(val36, val44); val44 = std::max(val36, val44); val36 = tmp; tmp = std::min(val37, val45);
			val45 = std::max(val37, val45); val37 = tmp; tmp = std::min(val38, val46); val46 = std::max(val38, val46);
			val38 = tmp; tmp = std::min(val39, val47); val47 = std::max(val39, val47); val39 = tmp;
			tmp = std::min(val8, val32); val32 = std::max(val8, val32); val8 = tmp; tmp = std::min(val9, val33);
			val33 = std::max(val9, val33); val9 = tmp; tmp = std::min(val10, val34); val34 = std::max(val10, val34);
			val10 = tmp; tmp = std::min(val11, val35); val35 = std::max(val11, val35); val11 = tmp;
			tmp = std::min(val12, val36); val36 = std::max(val12, val36); val12 = tmp; tmp = std::min(val13, val37);
			val37 = std::max(val13, val37); val13 = tmp; tmp = std::min(val14, val38); val38 = std::max(val14, val38);
			val14 = tmp; tmp = std::min(val15, val39); val39 = std::max(val15, val39); val15 = tmp;
			tmp = std::min(val24, val48); val48 = std::max(val24, val48); val24 = tmp; tmp = std::min(val8, val16);
			val16 = std::max(val8, val16); val8 = tmp; tmp = std::min(val9, val17); val17 = std::max(val9, val17);
			val9 = tmp; tmp = std::min(val10, val18); val18 = std::max(val10, val18); val10 = tmp;
			tmp = std::min(val11, val19); val19 = std::max(val11, val19); val11 = tmp; tmp = std::min(val12, val20);
			val20 = std::max(val12, val20); val12 = tmp; tmp = std::min(val13, val21); val21 = std::max(val13, val21);
			val13 = tmp; tmp = std::min(val14, val22); val22 = std::max(val14, val22); val14 = tmp;
			tmp = std::min(val15, val23); val23 = std::max(val15, val23); val15 = tmp; tmp = std::min(val24, val32);
			val32 = std::max(val24, val32); val24 = tmp; tmp = std::min(val25, val33); val33 = std::max(val25, val33);
			val25 = tmp; tmp = std::min(val26, val34); val34 = std::max(val26, val34); val26 = tmp;
			tmp = std::min(val27, val35); val35 = std::max(val27, val35); val27 = tmp; tmp = std::min(val28, val36);
			val36 = std::max(val28, val36); val28 = tmp; tmp = std::min(val29, val37); val37 = std::max(val29, val37);
			val29 = tmp; tmp = std::min(val30, val38); val38 = std::max(val30, val38); val30 = tmp;
			tmp = std::min(val31, val39); val39 = std::max(val31, val39); val31 = tmp; tmp = std::min(val40, val48);
			val48 = std::max(val40, val48); val40 = tmp; tmp = std::min(val0, val4); val4 = std::max(val0, val4);
			val0 = tmp; tmp = std::min(val1, val5); val5 = std::max(val1, val5); val1 = tmp; tmp = std::min(val2, val6);
			val6 = std::max(val2, val6); val2 = tmp; tmp = std::min(val3, val7); val7 = std::max(val3, val7); val3 = tmp;
			tmp = std::min(val8, val12); val12 = std::max(val8, val12); val8 = tmp; tmp = std::min(val9, val13);
			val13 = std::max(val9, val13); val9 = tmp; tmp = std::min(val10, val14); val14 = std::max(val10, val14);
			val10 = tmp; tmp = std::min(val11, val15); val15 = std::max(val11, val15); val11 = tmp;
			tmp = std::min(val16, val20); val20 = std::max(val16, val20); val16 = tmp; tmp = std::min(val17, val21);
			val21 = std::max(val17, val21); val17 = tmp; tmp = std::min(val18, val22); val22 = std::max(val18, val22);
			val18 = tmp; tmp = std::min(val19, val23); val23 = std::max(val19, val23); val19 = tmp;
			tmp = std::min(val24, val28); val28 = std::max(val24, val28); val24 = tmp; tmp = std::min(val25, val29);
			val29 = std::max(val25, val29); val25 = tmp; tmp = std::min(val26, val30); val30 = std::max(val26, val30);
			val26 = tmp; tmp = std::min(val27, val31); val31 = std::max(val27, val31); val27 = tmp;
			tmp = std::min(val32, val36); val36 = std::max(val32, val36); val32 = tmp; tmp = std::min(val33, val37);
			val37 = std::max(val33, val37); val33 = tmp; tmp = std::min(val34, val38); val38 = std::max(val34, val38);
			val34 = tmp; tmp = std::min(val35, val39); val39 = std::max(val35, val39); val35 = tmp;
			tmp = std::min(val40, val44); val44 = std::max(val40, val44); val40 = tmp; tmp = std::min(val41, val45);
			val45 = std::max(val41, val45); val41 = tmp; tmp = std::min(val42, val46); val46 = std::max(val42, val46);
			val42 = tmp; tmp = std::min(val43, val47); val47 = std::max(val43, val47); val43 = tmp;
			tmp = std::min(val4, val32); val32 = std::max(val4, val32); val4 = tmp; tmp = std::min(val5, val33);
			val33 = std::max(val5, val33); val5 = tmp; tmp = std::min(val6, val34); val34 = std::max(val6, val34);
			val6 = tmp; tmp = std::min(val7, val35); val35 = std::max(val7, val35); val7 = tmp;
			tmp = std::min(val12, val40); val40 = std::max(val12, val40); val12 = tmp; tmp = std::min(val13, val41);
			val41 = std::max(val13, val41); val13 = tmp; tmp = std::min(val14, val42); val42 = std::max(val14, val42);
			val14 = tmp; tmp = std::min(val15, val43); val43 = std::max(val15, val43); val15 = tmp;
			tmp = std::min(val20, val48); val48 = std::max(val20, val48); val20 = tmp; tmp = std::min(val4, val16);
			val16 = std::max(val4, val16); val4 = tmp; tmp = std::min(val5, val17); val17 = std::max(val5, val17);
			val5 = tmp; tmp = std::min(val6, val18); val18 = std::max(val6, val18); val6 = tmp;
			tmp = std::min(val7, val19); val19 = std::max(val7, val19); val7 = tmp; tmp = std::min(val12, val24);
			val24 = std::max(val12, val24); val12 = tmp; tmp = std::min(val13, val25); val25 = std::max(val13, val25);
			val13 = tmp; tmp = std::min(val14, val26); val26 = std::max(val14, val26); val14 = tmp;
			tmp = std::min(val15, val27); val27 = std::max(val15, val27); val15 = tmp; tmp = std::min(val20, val32);
			val32 = std::max(val20, val32); val20 = tmp; tmp = std::min(val21, val33); val33 = std::max(val21, val33);
			val21 = tmp; tmp = std::min(val22, val34); val34 = std::max(val22, val34); val22 = tmp;
			tmp = std::min(val23, val35); val35 = std::max(val23, val35); val23 = tmp; tmp = std::min(val28, val40);
			val40 = std::max(val28, val40); val28 = tmp; tmp = std::min(val29, val41); val41 = std::max(val29, val41);
			val29 = tmp; tmp = std::min(val30, val42); val42 = std::max(val30, val42); val30 = tmp;
			tmp = std::min(val31, val43); val43 = std::max(val31, val43); val31 = tmp; tmp = std::min(val36, val48);
			val48 = std::max(val36, val48); val36 = tmp; tmp = std::min(val4, val8); val8 = std::max(val4, val8);
			val4 = tmp; tmp = std::min(val5, val9); val9 = std::max(val5, val9); val5 = tmp; tmp = std::min(val6, val10);
			val10 = std::max(val6, val10); val6 = tmp; tmp = std::min(val7, val11); val11 = std::max(val7, val11); val7 = tmp;
			tmp = std::min(val12, val16); val16 = std::max(val12, val16); val12 = tmp; tmp = std::min(val13, val17);
			val17 = std::max(val13, val17); val13 = tmp; tmp = std::min(val14, val18); val18 = std::max(val14, val18);
			val14 = tmp; tmp = std::min(val15, val19); val19 = std::max(val15, val19); val15 = tmp;
			tmp = std::min(val20, val24); val24 = std::max(val20, val24); val20 = tmp; tmp = std::min(val21, val25);
			val25 = std::max(val21, val25); val21 = tmp; tmp = std::min(val22, val26); val26 = std::max(val22, val26);
			val22 = tmp; tmp = std::min(val23, val27); val27 = std::max(val23, val27); val23 = tmp;
			tmp = std::min(val28, val32); val32 = std::max(val28, val32); val28 = tmp; tmp = std::min(val29, val33);
			val33 = std::max(val29, val33); val29 = tmp; tmp = std::min(val30, val34); val34 = std::max(val30, val34);
			val30 = tmp; tmp = std::min(val31, val35); val35 = std::max(val31, val35); val31 = tmp;
			tmp = std::min(val36, val40); val40 = std::max(val36, val40); val36 = tmp; tmp = std::min(val37, val41);
			val41 = std::max(val37, val41); val37 = tmp; tmp = std::min(val38, val42); val42 = std::max(val38, val42);
			val38 = tmp; tmp = std::min(val39, val43); val43 = std::max(val39, val43); val39 = tmp;
			tmp = std::min(val44, val48); val48 = std::max(val44, val48); val44 = tmp; tmp = std::min(val0, val2);
			val2 = std::max(val0, val2); val0 = tmp; tmp = std::min(val1, val3); val3 = std::max(val1, val3); val1 = tmp;
			tmp = std::min(val4, val6); val6 = std::max(val4, val6); val4 = tmp; tmp = std::min(val5, val7);
			val7 = std::max(val5, val7); val5 = tmp; tmp = std::min(val8, val10); val10 = std::max(val8, val10); val8 = tmp;
			tmp = std::min(val9, val11); val11 = std::max(val9, val11); val9 = tmp; tmp = std::min(val12, val14);
			val14 = std::max(val12, val14); val12 = tmp; tmp = std::min(val13, val15); val15 = std::max(val13, val15);
			val13 = tmp; tmp = std::min(val16, val18); val18 = std::max(val16, val18); val16 = tmp;
			tmp = std::min(val17, val19); val19 = std::max(val17, val19); val17 = tmp; tmp = std::min(val20, val22);
			val22 = std::max(val20, val22); val20 = tmp; tmp = std::min(val21, val23); val23 = std::max(val21, val23);
			val21 = tmp; tmp = std::min(val24, val26); val26 = std::max(val24, val26); val24 = tmp;
			tmp = std::min(val25, val27); val27 = std::max(val25, val27); val25 = tmp; tmp = std::min(val28, val30);
			val30 = std::max(val28, val30); val28 = tmp; tmp = std::min(val29, val31); val31 = std::max(val29, val31);
			val29 = tmp; tmp = std::min(val32, val34); val34 = std::max(val32, val34); val32 = tmp;
			tmp = std::min(val33, val35); val35 = std::max(val33, val35); val33 = tmp; tmp = std::min(val36, val38);
			val38 = std::max(val36, val38); val36 = tmp; tmp = std::min(val37, val39); val39 = std::max(val37, val39);
			val37 = tmp; tmp = std::min(val40, val42); val42 = std::max(val40, val42); val40 = tmp;
			tmp = std::min(val41, val43); val43 = std::max(val41, val43); val41 = tmp; tmp = std::min(val44, val46);
			val46 = std::max(val44, val46); val44 = tmp; tmp = std::min(val45, val47); val47 = std::max(val45, val47);
			val45 = tmp; tmp = std::min(val2, val32); val32 = std::max(val2, val32); val2 = tmp; tmp = std::min(val3, val33);
			val33 = std::max(val3, val33); val3 = tmp; tmp = std::min(val6, val36); val36 = std::max(val6, val36); val6 = tmp;
			tmp = std::min(val7, val37); val37 = std::max(val7, val37); val7 = tmp; tmp = std::min(val10, val40);
			val40 = std::max(val10, val40); val10 = tmp; tmp = std::min(val11, val41); val41 = std::max(val11, val41);
			val11 = tmp; tmp = std::min(val14, val44); val44 = std::max(val14, val44); val14 = tmp;
			tmp = std::min(val15, val45); val45 = std::max(val15, val45); val15 = tmp; tmp = std::min(val18, val48);
			val48 = std::max(val18, val48); val18 = tmp; tmp = std::min(val2, val16); val16 = std::max(val2, val16);
			val2 = tmp; tmp = std::min(val3, val17); val17 = std::max(val3, val17); val3 = tmp;
			tmp = std::min(val6, val20); val20 = std::max(val6, val20); val6 = tmp; tmp = std::min(val7, val21);
			val21 = std::max(val7, val21); val7 = tmp; tmp = std::min(val10, val24); val24 = std::max(val10, val24);
			val10 = tmp; tmp = std::min(val11, val25); val25 = std::max(val11, val25); val11 = tmp;
			tmp = std::min(val14, val28); val28 = std::max(val14, val28); val14 = tmp; tmp = std::min(val15, val29);
			val29 = std::max(val15, val29); val15 = tmp; tmp = std::min(val18, val32); val32 = std::max(val18, val32);
			val18 = tmp; tmp = std::min(val19, val33); val33 = std::max(val19, val33); val19 = tmp;
			tmp = std::min(val22, val36); val36 = std::max(val22, val36); val22 = tmp; tmp = std::min(val23, val37);
			val37 = std::max(val23, val37); val23 = tmp; tmp = std::min(val26, val40); val40 = std::max(val26, val40);
			val26 = tmp; tmp = std::min(val27, val41); val41 = std::max(val27, val41); val27 = tmp;
			tmp = std::min(val30, val44); val44 = std::max(val30, val44); val30 = tmp; tmp = std::min(val31, val45);
			val45 = std::max(val31, val45); val31 = tmp; tmp = std::min(val34, val48); val48 = std::max(val34, val48);
			val34 = tmp; tmp = std::min(val2, val8); val8 = std::max(val2, val8); val2 = tmp; tmp = std::min(val3, val9);
			val9 = std::max(val3, val9); val3 = tmp; tmp = std::min(val6, val12); val12 = std::max(val6, val12); val6 = tmp;
			tmp = std::min(val7, val13); val13 = std::max(val7, val13); val7 = tmp; tmp = std::min(val10, val16);
			val16 = std::max(val10, val16); val10 = tmp; tmp = std::min(val11, val17); val17 = std::max(val11, val17);
			val11 = tmp; tmp = std::min(val14, val20); val20 = std::max(val14, val20); val14 = tmp;
			tmp = std::min(val15, val21); val21 = std::max(val15, val21); val15 = tmp; tmp = std::min(val18, val24);
			val24 = std::max(val18, val24); val18 = tmp; tmp = std::min(val19, val25); val25 = std::max(val19, val25);
			val19 = tmp; tmp = std::min(val22, val28); val28 = std::max(val22, val28); val22 = tmp;
			tmp = std::min(val23, val29); val29 = std::max(val23, val29); val23 = tmp; tmp = std::min(val26, val32);
			val32 = std::max(val26, val32); val26 = tmp; tmp = std::min(val27, val33); val33 = std::max(val27, val33);
			val27 = tmp; tmp = std::min(val30, val36); val36 = std::max(val30, val36); val30 = tmp;
			tmp = std::min(val31, val37); val37 = std::max(val31, val37); val31 = tmp; tmp = std::min(val34, val40);
			val40 = std::max(val34, val40); val34 = tmp; tmp = std::min(val35, val41); val41 = std::max(val35, val41);
			val35 = tmp; tmp = std::min(val38, val44); val44 = std::max(val38, val44); val38 = tmp;
			tmp = std::min(val39, val45); val45 = std::max(val39, val45); val39 = tmp; tmp = std::min(val42, val48);
			val48 = std::max(val42, val48); val42 = tmp; tmp = std::min(val2, val4); val4 = std::max(val2, val4);
			val2 = tmp; tmp = std::min(val3, val5); val5 = std::max(val3, val5); val3 = tmp; tmp = std::min(val6, val8);
			val8 = std::max(val6, val8); val6 = tmp; tmp = std::min(val7, val9); val9 = std::max(val7, val9); val7 = tmp;
			tmp = std::min(val10, val12); val12 = std::max(val10, val12); val10 = tmp; tmp = std::min(val11, val13);
			val13 = std::max(val11, val13); val11 = tmp; tmp = std::min(val14, val16); val16 = std::max(val14, val16);
			val14 = tmp; tmp = std::min(val15, val17); val17 = std::max(val15, val17); val15 = tmp;
			tmp = std::min(val18, val20); val20 = std::max(val18, val20); val18 = tmp; tmp = std::min(val19, val21);
			val21 = std::max(val19, val21); val19 = tmp; tmp = std::min(val22, val24); val24 = std::max(val22, val24);
			val22 = tmp; tmp = std::min(val23, val25); val25 = std::max(val23, val25); val23 = tmp;
			tmp = std::min(val26, val28); val28 = std::max(val26, val28); val26 = tmp; tmp = std::min(val27, val29);
			val29 = std::max(val27, val29); val27 = tmp; tmp = std::min(val30, val32); val32 = std::max(val30, val32);
			val30 = tmp; tmp = std::min(val31, val33); val33 = std::max(val31, val33); val31 = tmp;
			tmp = std::min(val34, val36); val36 = std::max(val34, val36); val34 = tmp; tmp = std::min(val35, val37);
			val37 = std::max(val35, val37); val35 = tmp; tmp = std::min(val38, val40); val40 = std::max(val38, val40);
			val38 = tmp; tmp = std::min(val39, val41); val41 = std::max(val39, val41); val39 = tmp;
			tmp = std::min(val42, val44); val44 = std::max(val42, val44); val42 = tmp; tmp = std::min(val43, val45);
			val45 = std::max(val43, val45); val43 = tmp; tmp = std::min(val46, val48); val48 = std::max(val46, val48);
			val46 = tmp; val1 = std::max(val0, val1); val3 = std::max(val2, val3); val5 = std::max(val4, val5);
			val7 = std::max(val6, val7); val9 = std::max(val8, val9); val11 = std::max(val10, val11);
			val13 = std::max(val12, val13); val15 = std::max(val14, val15); val17 = std::max(val16, val17);
			val19 = std::max(val18, val19); val21 = std::max(val20, val21); val23 = std::max(val22, val23);
			val24 = std::min(val24, val25); val26 = std::min(val26, val27); val28 = std::min(val28, val29);
			val30 = std::min(val30, val31); val32 = std::min(val32, val33); val34 = std::min(val34, val35);
			val36 = std::min(val36, val37); val38 = std::min(val38, val39); val40 = std::min(val40, val41);
			val42 = std::min(val42, val43); val44 = std::min(val44, val45); val46 = std::min(val46, val47);
			val32 = std::max(val1, val32); val34 = std::max(val3, val34); val36 = std::max(val5, val36);
			val38 = std::max(val7, val38); val9 = std::min(val9, val40); val11 = std::min(val11, val42);
			val13 = std::min(val13, val44); val15 = std::min(val15, val46); val17 = std::min(val17, val48);
			val24 = std::max(val9, val24); val26 = std::max(val11, val26); val28 = std::max(val13, val28);
			val30 = std::max(val15, val30); val17 = std::min(val17, val32); val19 = std::min(val19, val34);
			val21 = std::min(val21, val36); val23 = std::min(val23, val38); val24 = std::max(val17, val24);
			val26 = std::max(val19, val26); val21 = std::min(val21, val28); val23 = std::min(val23, val30);
			val24 = std::max(val21, val24); val23 = std::min(val23, val26);
			return std::max(val23, val24);
		}

		//! Return sqrt(x^2 + y^2).
		template<typename T>
		inline T hypot(const T x, const T y) {
			return std::sqrt(x*x + y*y);
		}

		template<typename T>
		inline T hypot(const T x, const T y, const T z) {
			return std::sqrt(x*x + y*y + z*z);
		}

		template<typename T>
		inline T _hypot(const T x, const T y) { // Slower but more precise version
			T nx = cimg::abs(x), ny = cimg::abs(y), t;
			if (nx<ny) { t = nx; nx = ny; }
			else t = ny;
			if (nx>0) { t /= nx; return nx*std::sqrt(1 + t*t); }
			return 0;
		}

		//! Return the factorial of n
		inline double factorial(const int n) {
			if (n<0) return cimg::type<double>::nan();
			if (n<2) return 1;
			double res = 2;
			for (int i = 3; i <= n; ++i) res *= i;
			return res;
		}

		//! Return the number of permutations of k objects in a set of n objects.
		inline double permutations(const int k, const int n, const bool with_order) {
			if (n<0 || k<0) return cimg::type<double>::nan();
			if (k>n) return 0;
			double res = 1;
			for (int i = n; i >= n - k + 1; --i) res *= i;
			return with_order ? res : res / cimg::factorial(k);
		}

		inline double _fibonacci(int exp) {
			double
				base = (1 + std::sqrt(5.0)) / 2,
				result = 1 / std::sqrt(5.0);
			while (exp) {
				if (exp & 1) result *= base;
				exp >>= 1;
				base *= base;
			}
			return result;
		}

		//! Calculate fibonacci number.
		// (Precise up to n = 78, less precise for n>78).
		inline double fibonacci(const int n) {
			if (n<0) return cimg::type<double>::nan();
			if (n<3) return 1;
			if (n<11) {
				cimg_uint64 fn1 = 1, fn2 = 1, fn = 0;
				for (int i = 3; i <= n; ++i) { fn = fn1 + fn2; fn2 = fn1; fn1 = fn; }
				return (double)fn;
			}
			if (n<75) // precise up to n = 74, faster than the integer calculation above for n>10
				return (double)((cimg_uint64)(_fibonacci(n) + 0.5));

			if (n<94) { // precise up to n = 78, less precise for n>78 up to n = 93, overflows for n>93
				cimg_uint64
					fn1 = (cimg_uint64)1304969544928657U,
					fn2 = (cimg_uint64)806515533049393U,
					fn = 0;
				for (int i = 75; i <= n; ++i) { fn = fn1 + fn2; fn2 = fn1; fn1 = fn; }
				return (double)fn;
			}
			return _fibonacci(n); // Not precise, but better than the wrong overflowing calculation
		}

		//! Convert ascii character to lower case.
		inline char lowercase(const char x) {
			return (char)((x<'A' || x>'Z') ? x : x - 'A' + 'a');
		}
		inline double lowercase(const double x) {
			return (double)((x<'A' || x>'Z') ? x : x - 'A' + 'a');
		}

		//! Convert C-string to lower case.
		inline void lowercase(char *const str) {
			if (str) for (char *ptr = str; *ptr; ++ptr) *ptr = lowercase(*ptr);
		}

		//! Convert ascii character to upper case.
		inline char uppercase(const char x) {
			return (char)((x<'a' || x>'z') ? x : x - 'a' + 'A');
		}

		inline double uppercase(const double x) {
			return (double)((x<'a' || x>'z') ? x : x - 'a' + 'A');
		}

		//! Convert C-string to upper case.
		inline void uppercase(char *const str) {
			if (str) for (char *ptr = str; *ptr; ++ptr) *ptr = uppercase(*ptr);
		}

		//! Read value in a C-string.
		/**
		\param str C-string containing the float value to read.
		\return Read value.
		\note Same as <tt>std::atof()</tt> extended to manage the retrieval of fractions from C-strings,
		as in <em>"1/2"</em>.
		**/
		inline double atof(const char *const str) {
			double x = 0, y = 1;
			return str && cimg_sscanf(str, "%lf/%lf", &x, &y)>0 ? x / y : 0;
		}

		//! Compare the first \p l characters of two C-strings, ignoring the case.
		/**
		\param str1 C-string.
		\param str2 C-string.
		\param l Number of characters to compare.
		\return \c 0 if the two strings are equal, something else otherwise.
		\note This function has to be defined since it is not provided by all C++-compilers (not ANSI).
		**/
		inline int strncasecmp(const char *const str1, const char *const str2, const int l) {
			if (!l) return 0;
			if (!str1) return str2 ? -1 : 0;
			const char *nstr1 = str1, *nstr2 = str2;
			int k, diff = 0; for (k = 0; k<l && !(diff = lowercase(*nstr1) - lowercase(*nstr2)); ++k) { ++nstr1; ++nstr2; }
			return k != l ? diff : 0;
		}

		//! Compare two C-strings, ignoring the case.
		/**
		\param str1 C-string.
		\param str2 C-string.
		\return \c 0 if the two strings are equal, something else otherwise.
		\note This function has to be defined since it is not provided by all C++-compilers (not ANSI).
		**/
		inline int strcasecmp(const char *const str1, const char *const str2) {
			if (!str1) return str2 ? -1 : 0;
			const int
				l1 = (int)std::strlen(str1),
				l2 = (int)std::strlen(str2);
			return cimg::strncasecmp(str1, str2, 1 + (l1<l2 ? l1 : l2));
		}

		//! Ellipsize a string.
		/**
		\param str C-string.
		\param l Max number of characters.
		\param is_ending Tell if the dots are placed at the end or at the center of the ellipsized string.
		**/
		inline char *strellipsize(char *const str, const unsigned int l = 64,
			const bool is_ending = true) {
			if (!str) return str;
			const unsigned int nl = l<5 ? 5 : l, ls = (unsigned int)std::strlen(str);
			if (ls <= nl) return str;
			if (is_ending) std::strcpy(str + nl - 5, "(...)");
			else {
				const unsigned int ll = (nl - 5) / 2 + 1 - (nl % 2), lr = nl - ll - 5;
				std::strcpy(str + ll, "(...)");
				std::memmove(str + ll + 5, str + ls - lr, lr);
			}
			str[nl] = 0;
			return str;
		}

		//! Ellipsize a string.
		/**
		\param str C-string.
		\param res output C-string.
		\param l Max number of characters.
		\param is_ending Tell if the dots are placed at the end or at the center of the ellipsized string.
		**/
		inline char *strellipsize(const char *const str, char *const res, const unsigned int l = 64,
			const bool is_ending = true) {
			const unsigned int nl = l<5 ? 5 : l, ls = (unsigned int)std::strlen(str);
			if (ls <= nl) { std::strcpy(res, str); return res; }
			if (is_ending) {
				std::strncpy(res, str, nl - 5);
				std::strcpy(res + nl - 5, "(...)");
			}
			else {
				const unsigned int ll = (nl - 5) / 2 + 1 - (nl % 2), lr = nl - ll - 5;
				std::strncpy(res, str, ll);
				std::strcpy(res + ll, "(...)");
				std::strncpy(res + ll + 5, str + ls - lr, lr);
			}
			res[nl] = 0;
			return res;
		}

		//! Remove delimiters on the start and/or end of a C-string.
		/**
		\param[in,out] str C-string to work with (modified at output).
		\param delimiter Delimiter character code to remove.
		\param is_symmetric Tells if the removal is done only if delimiters are symmetric
		(both at the beginning and the end of \c s).
		\param is_iterative Tells if the removal is done if several iterations are possible.
		\return \c true if delimiters have been removed, \c false otherwise.
		**/
		inline bool strpare(char *const str, const char delimiter,
			const bool is_symmetric, const bool is_iterative) {
			if (!str) return false;
			const int l = (int)std::strlen(str);
			int p, q;
			if (is_symmetric) for (p = 0, q = l - 1; p<q && str[p] == delimiter && str[q] == delimiter; ) {
				--q; ++p; if (!is_iterative) break;
			}
			else {
				for (p = 0; p<l && str[p] == delimiter; ) { ++p; if (!is_iterative) break; }
				for (q = l - 1; q>p && str[q] == delimiter; ) { --q; if (!is_iterative) break; }
			}
			const int n = q - p + 1;
			if (n != l) { std::memmove(str, str + p, (unsigned int)n); str[n] = 0; return true; }
			return false;
		}

		//! Remove white spaces on the start and/or end of a C-string.
		inline bool strpare(char *const str, const bool is_symmetric, const bool is_iterative) {
			if (!str) return false;
			const int l = (int)std::strlen(str);
			int p, q;
			if (is_symmetric) for (p = 0, q = l - 1; p<q && (signed char)str[p] <= ' ' && (signed char)str[q] <= ' '; ) {
				--q; ++p; if (!is_iterative) break;
			}
			else {
				for (p = 0; p<l && (signed char)str[p] <= ' '; ) { ++p; if (!is_iterative) break; }
				for (q = l - 1; q>p && (signed char)str[q] <= ' '; ) { --q; if (!is_iterative) break; }
			}
			const int n = q - p + 1;
			if (n != l) { std::memmove(str, str + p, (unsigned int)n); str[n] = 0; return true; }
			return false;
		}

		//! Replace reserved characters (for Windows filename) by another character.
		/**
		\param[in,out] str C-string to work with (modified at output).
		\param[in] c Replacement character.
		**/
		inline void strwindows_reserved(char *const str, const char c = '_') {
			for (char *s = str; *s; ++s) {
				const char i = *s;
				if (i == '<' || i == '>' || i == ':' || i == '\"' || i == '/' || i == '\\' || i == '|' || i == '?' || i == '*') *s = c;
			}
		}

		//! Replace escape sequences in C-strings by their binary ascii values.
		/**
		\param[in,out] str C-string to work with (modified at output).
		**/
		inline void strunescape(char *const str) {
#define cimg_strunescape(ci,co) case ci : *nd = co; ++ns; break;
			unsigned int val = 0;
			for (char *ns = str, *nd = str; *ns || (bool)(*nd = 0); ++nd) if (*ns == '\\') switch (*(++ns)) {
				cimg_strunescape('a', '\a');
				cimg_strunescape('b', '\b');
				cimg_strunescape('e', 0x1B);
				cimg_strunescape('f', '\f');
				cimg_strunescape('n', '\n');
				cimg_strunescape('r', '\r');
				cimg_strunescape('t', '\t');
				cimg_strunescape('v', '\v');
				cimg_strunescape('\\', '\\');
				cimg_strunescape('\'', '\'');
				cimg_strunescape('\"', '\"');
				cimg_strunescape('\?', '\?');
			case 0: *nd = 0; break;
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
				cimg_sscanf(ns, "%o", &val); while (*ns >= '0' && *ns <= '7') ++ns;
				*nd = (char)val; break;
			case 'x':
				cimg_sscanf(++ns, "%x", &val);
				while ((*ns >= '0' && *ns <= '9') || (*ns >= 'a' && *ns <= 'f') || (*ns >= 'A' && *ns <= 'F')) ++ns;
				*nd = (char)val; break;
			default: *nd = *(ns++);
			}
			else *nd = *(ns++);
		}

		// Return a temporary string describing the size of a memory buffer.
		inline const char *strbuffersize(const cimg_ulong size);

		// Return string that identifies the running OS.
		inline const char *stros() {
#if defined(linux) || defined(__linux) || defined(__linux__)
			static const char *const str = "Linux";
#elif defined(sun) || defined(__sun)
			static const char *const str = "Sun OS";
#elif defined(BSD) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined (__DragonFly__)
			static const char *const str = "BSD";
#elif defined(sgi) || defined(__sgi)
			static const char *const str = "Irix";
#elif defined(__MACOSX__) || defined(__APPLE__)
			static const char *const str = "Mac OS";
#elif defined(unix) || defined(__unix) || defined(__unix__)
			static const char *const str = "Generic Unix";
#elif defined(_MSC_VER) || defined(WIN32)  || defined(_WIN32) || defined(__WIN32__) || \
  defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
			static const char *const str = "Windows";
#else
			const char
				*const _str1 = std::getenv("OSTYPE"),
				*const _str2 = _str1 ? _str1 : std::getenv("OS"),
				*const str = _str2 ? _str2 : "Unknown OS";
#endif
			return str;
		}

		//! Return the basename of a filename.
		inline const char* basename(const char *const s, const char separator = cimg_file_separator) {
			const char *p = 0, *np = s;
			while (np >= s && (p = np)) np = std::strchr(np, separator) + 1;
			return p;
		}

		// Return a random filename.
		inline const char* filenamerand() {
			cimg::mutex(6);
			static char randomid[9];
			cimg::srand();
			for (unsigned int k = 0; k<8; ++k) {
				const int v = (int)cimg::rand(65535) % 3;
				randomid[k] = (char)(v == 0 ? ('0' + ((int)cimg::rand(65535) % 10)) :
					(v == 1 ? ('a' + ((int)cimg::rand(65535) % 26)) :
					('A' + ((int)cimg::rand(65535) % 26))));
			}
			cimg::mutex(6, 0);
			return randomid;
		}

		// Convert filename as a Windows-style filename (short path name).
		inline void winformat_string(char *const str) {
			if (str && *str) {
#if cimg_OS==2
				char *const nstr = new char[MAX_PATH];
				if (GetShortPathNameA(str, nstr, MAX_PATH)) std::strcpy(str, nstr);
				delete[] nstr;
#endif
			}
		}

		// Open a file (with wide character support on Windows).
		inline std::FILE *win_fopen(const char *const path, const char *const mode);

		//! Open a file.
		/**
		\param path Path of the filename to open.
		\param mode C-string describing the opening mode.
		\return Opened file.
		\note Same as <tt>std::fopen()</tt> but throw a \c CImgIOException when
		the specified file cannot be opened, instead of returning \c 0.
		**/
		inline std::FILE *fopen(const char *const path, const char *const mode) {
			if (!path)
				throw CImgArgumentException("cimg::fopen(): Specified file path is (null).");
			if (!mode)
				throw CImgArgumentException("cimg::fopen(): File '%s', specified mode is (null).",
					path);
			std::FILE *res = 0;
			if (*path == '-' && (!path[1] || path[1] == '.')) {
				res = (*mode == 'r') ? cimg::_stdin() : cimg::_stdout();
#if cimg_OS==2
				if (*mode && mode[1] == 'b') { // Force stdin/stdout to be in binary mode.
#ifdef __BORLANDC__
					if (setmode(_fileno(res), 0x8000) == -1) res = 0;
#else
					if (_setmode(_fileno(res), 0x8000) == -1) res = 0;
#endif
				}
#endif
			}
			else res = std_fopen(path, mode);
			if (!res) throw CImgIOException("cimg::fopen(): Failed to open file '%s' with mode '%s'.",
				path, mode);
			return res;
		}

		//! Close a file.
		/**
		\param file File to close.
		\return \c 0 if file has been closed properly, something else otherwise.
		\note Same as <tt>std::fclose()</tt> but display a warning message if
		the file has not been closed properly.
		**/
		inline int fclose(std::FILE *file) {
			if (!file) { warn("cimg::fclose(): Specified file is (null)."); return 0; }
			if (file == cimg::_stdin(false) || file == cimg::_stdout(false)) return 0;
			const int errn = std::fclose(file);
			if (errn != 0) warn("cimg::fclose(): Error code %d returned during file closing.",
				errn);
			return errn;
		}

		//! Version of 'fseek()' that supports >=64bits offsets everywhere (for Windows).
		inline int fseek(FILE *stream, cimg_long offset, int origin) {
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
			return _fseeki64(stream, (__int64)offset, origin);
#else
			return std::fseek(stream, offset, origin);
#endif
		}

		//! Version of 'ftell()' that supports >=64bits offsets everywhere (for Windows).
		inline cimg_long ftell(FILE *stream) {
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
			return (cimg_long)_ftelli64(stream);
#else
			return (cimg_long)std::ftell(stream);
#endif
		}

		//! Check if a path is a directory.
		/**
		\param path Specified path to test.
		**/
		inline bool is_directory(const char *const path) {
			if (!path || !*path) return false;
#if cimg_OS==1
			struct stat st_buf;
			return (!stat(path, &st_buf) && S_ISDIR(st_buf.st_mode));
#elif cimg_OS==2
			const unsigned int res = (unsigned int)GetFileAttributesA(path);
			return res == INVALID_FILE_ATTRIBUTES ? false : (res & 16);
#else
			return false;
#endif
		}

		//! Check if a path is a file.
		/**
		\param path Specified path to test.
		**/
		inline bool is_file(const char *const path) {
			if (!path || !*path) return false;
			std::FILE *const file = std_fopen(path, "rb");
			if (!file) return false;
			std::fclose(file);
			return !is_directory(path);
		}

		//! Get last write time of a given file or directory.
		/**
		\param path Specified path to get attributes from.
		\param attr Type of requested time attribute.
		Can be { 0=year | 1=month | 2=day | 3=day of week | 4=hour | 5=minute | 6=second }
		\return -1 if requested attribute could not be read.
		**/
		inline int fdate(const char *const path, const unsigned int attr) {
			int res = -1;
			if (!path || !*path || attr>6) return -1;
			cimg::mutex(6);
#if cimg_OS==2
			HANDLE file = CreateFileA(path, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (file != INVALID_HANDLE_VALUE) {
				FILETIME _ft;
				SYSTEMTIME ft;
				if (GetFileTime(file, 0, 0, &_ft) && FileTimeToSystemTime(&_ft, &ft))
					res = (int)(attr == 0 ? ft.wYear : attr == 1 ? ft.wMonth : attr == 2 ? ft.wDay : attr == 3 ? ft.wDayOfWeek :
						attr == 4 ? ft.wHour : attr == 5 ? ft.wMinute : ft.wSecond);
				CloseHandle(file);
			}
#elif cimg_OS==1
			struct stat st_buf;
			if (!stat(path, &st_buf)) {
				const time_t _ft = st_buf.st_mtime;
				const struct tm& ft = *std::localtime(&_ft);
				res = (int)(attr == 0 ? ft.tm_year + 1900 : attr == 1 ? ft.tm_mon + 1 : attr == 2 ? ft.tm_mday : attr == 3 ? ft.tm_wday :
					attr == 4 ? ft.tm_hour : attr == 5 ? ft.tm_min : ft.tm_sec);
			}
#endif
			cimg::mutex(6, 0);
			return res;
		}

		//! Get current local time.
		/**
		\param attr Type of requested time attribute.
		Can be { 0=year | 1=month | 2=day | 3=day of week | 4=hour | 5=minute | 6=second }
		**/
		inline int date(const unsigned int attr) {
			int res;
			cimg::mutex(6);
#if cimg_OS==2
			SYSTEMTIME st;
			GetLocalTime(&st);
			res = (int)(attr == 0 ? st.wYear : attr == 1 ? st.wMonth : attr == 2 ? st.wDay : attr == 3 ? st.wDayOfWeek :
				attr == 4 ? st.wHour : attr == 5 ? st.wMinute : st.wSecond);
#else
			time_t _st;
			std::time(&_st);
			struct tm *st = std::localtime(&_st);
			res = (int)(attr == 0 ? st->tm_year + 1900 : attr == 1 ? st->tm_mon + 1 : attr == 2 ? st->tm_mday : attr == 3 ? st->tm_wday :
				attr == 4 ? st->tm_hour : attr == 5 ? st->tm_min : st->tm_sec);
#endif
			cimg::mutex(6, 0);
			return res;
		}

		// Get/set path to store temporary files.
		inline const char* temporary_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the <i>Program Files/</i> directory (Windows only).
#if cimg_OS==2
		inline const char* programfiles_path(const char *const user_path = 0, const bool reinit_path = false);
#endif

		// Get/set path to the ImageMagick's \c convert binary.
		inline const char* imagemagick_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the GraphicsMagick's \c gm binary.
		inline const char* graphicsmagick_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the XMedcon's \c medcon binary.
		inline const char* medcon_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the FFMPEG's \c ffmpeg binary.
		inline const char *ffmpeg_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the \c gzip binary.
		inline const char *gzip_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the \c gunzip binary.
		inline const char *gunzip_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the \c dcraw binary.
		inline const char *dcraw_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the \c wget binary.
		inline const char *wget_path(const char *const user_path = 0, const bool reinit_path = false);

		// Get/set path to the \c curl binary.
		inline const char *curl_path(const char *const user_path = 0, const bool reinit_path = false);

		//! Split filename into two C-strings \c body and \c extension.
		/**
		filename and body must not overlap!
		**/
		inline const char *split_filename(const char *const filename, char *const body = 0) {
			if (!filename) { if (body) *body = 0; return 0; }
			const char *p = 0; for (const char *np = filename; np >= filename && (p = np); np = std::strchr(np, '.') + 1) {}
			if (p == filename) {
				if (body) std::strcpy(body, filename);
				return filename + std::strlen(filename);
			}
			const unsigned int l = (unsigned int)(p - filename - 1);
			if (body) { if (l) std::memcpy(body, filename, l); body[l] = 0; }
			return p;
		}

		//! Generate a numbered version of a filename.
		inline char* number_filename(const char *const filename, const int number,
			const unsigned int digits, char *const str) {
			if (!filename) { if (str) *str = 0; return 0; }
			char *const format = new char[1024], *const body = new char[1024];
			const char *const ext = cimg::split_filename(filename, body);
			if (*ext) cimg_snprintf(format, 1024, "%%s_%%.%ud.%%s", digits);
			else cimg_snprintf(format, 1024, "%%s_%%.%ud", digits);
			cimg_sprintf(str, format, body, number, ext);
			delete[] format; delete[] body;
			return str;
		}

		//! Read data from file.
		/**
		\param[out] ptr Pointer to memory buffer that will contain the binary data read from file.
		\param nmemb Number of elements to read.
		\param stream File to read data from.
		\return Number of read elements.
		\note Same as <tt>std::fread()</tt> but may display warning message if all elements could not be read.
		**/
		template<typename T>
		inline size_t fread(T *const ptr, const size_t nmemb, std::FILE *stream) {
			if (!ptr || !stream)
				throw CImgArgumentException("cimg::fread(): Invalid reading request of %u %s%s from file %p to buffer %p.",
					nmemb, cimg::type<T>::string(), nmemb>1 ? "s" : "", stream, ptr);
			if (!nmemb) return 0;
			const size_t wlimitT = 63 * 1024 * 1024, wlimit = wlimitT / sizeof(T);
			size_t to_read = nmemb, al_read = 0, l_to_read = 0, l_al_read = 0;
			do {
				l_to_read = (to_read * sizeof(T))<wlimitT ? to_read : wlimit;
				l_al_read = std::fread((void*)(ptr + al_read), sizeof(T), l_to_read, stream);
				al_read += l_al_read;
				to_read -= l_al_read;
			} while (l_to_read == l_al_read && to_read>0);
			if (to_read>0)
				warn("cimg::fread(): Only %lu/%lu elements could be read from file.",
				(unsigned long)al_read, (unsigned long)nmemb);
			return al_read;
		}

		//! Write data to file.
		/**
		\param ptr Pointer to memory buffer containing the binary data to write on file.
		\param nmemb Number of elements to write.
		\param[out] stream File to write data on.
		\return Number of written elements.
		\note Similar to <tt>std::fwrite</tt> but may display warning messages if all elements could not be written.
		**/
		template<typename T>
		inline size_t fwrite(const T *ptr, const size_t nmemb, std::FILE *stream) {
			if (!ptr || !stream)
				throw CImgArgumentException("cimg::fwrite(): Invalid writing request of %u %s%s from buffer %p to file %p.",
					nmemb, cimg::type<T>::string(), nmemb>1 ? "s" : "", ptr, stream);
			if (!nmemb) return 0;
			const size_t wlimitT = 63 * 1024 * 1024, wlimit = wlimitT / sizeof(T);
			size_t to_write = nmemb, al_write = 0, l_to_write = 0, l_al_write = 0;
			do {
				l_to_write = (to_write * sizeof(T))<wlimitT ? to_write : wlimit;
				l_al_write = std::fwrite((void*)(ptr + al_write), sizeof(T), l_to_write, stream);
				al_write += l_al_write;
				to_write -= l_al_write;
			} while (l_to_write == l_al_write && to_write>0);
			if (to_write>0)
				warn("cimg::fwrite(): Only %lu/%lu elements could be written in file.",
				(unsigned long)al_write, (unsigned long)nmemb);
			return al_write;
		}

		//! Create an empty file.
		/**
		\param file Input file (can be \c 0 if \c filename is set).
		\param filename Filename, as a C-string (can be \c 0 if \c file is set).
		**/
		inline void fempty(std::FILE *const file, const char *const filename) {
			if (!file && !filename)
				throw CImgArgumentException("cimg::fempty(): Specified filename is (null).");
			std::FILE *const nfile = file ? file : cimg::fopen(filename, "wb");
			if (!file) cimg::fclose(nfile);
		}

		// Try to guess format from an image file.
		inline const char *ftype(std::FILE *const file, const char *const filename);

		// Load file from network as a local temporary file.
		inline char *load_network(const char *const url, char *const filename_local,
			const unsigned int timeout = 0, const bool try_fallback = false,
			const char *const referer = 0);

		//! Return options specified on the command line.
		inline const char* option(const char *const name, const int argc, const char *const *const argv,
			const char *const defaut, const char *const usage, const bool reset_static) {
			static bool first = true, visu = false;
			if (reset_static) { first = true; return 0; }
			const char *res = 0;
			if (first) {
				first = false;
				visu = cimg::option("-h", argc, argv, (char*)0, (char*)0, false) != 0;
				visu |= cimg::option("-help", argc, argv, (char*)0, (char*)0, false) != 0;
				visu |= cimg::option("--help", argc, argv, (char*)0, (char*)0, false) != 0;
			}
			if (!name && visu) {
				if (usage) {
					std::fprintf(cimg::output(), "\n %s%s%s", cimg::t_red, cimg::basename(argv[0]), cimg::t_normal);
					std::fprintf(cimg::output(), ": %s", usage);
					std::fprintf(cimg::output(), " (%s, %s)\n\n", cimg_date, cimg_time);
				}
				if (defaut) std::fprintf(cimg::output(), "%s\n", defaut);
			}
			if (name) {
				if (argc>0) {
					int k = 0;
					while (k<argc && std::strcmp(argv[k], name)) ++k;
					res = (k++ == argc ? defaut : (k == argc ? argv[--k] : argv[k]));
				}
				else res = defaut;
				if (visu && usage) std::fprintf(cimg::output(), "    %s%-16s%s %-24s %s%s%s\n",
					cimg::t_bold, name, cimg::t_normal, res ? res : "0",
					cimg::t_green, usage, cimg::t_normal);
			}
			return res;
		}

		inline const char* option(const char *const name, const int argc, const char *const *const argv,
			const char *const defaut, const char *const usage = 0) {
			return option(name, argc, argv, defaut, usage, false);
		}

		inline bool option(const char *const name, const int argc, const char *const *const argv,
			const bool defaut, const char *const usage = 0) {
			const char *const s = cimg::option(name, argc, argv, (char*)0);
			const bool res = s ? (cimg::strcasecmp(s, "false") && cimg::strcasecmp(s, "off") && cimg::strcasecmp(s, "0")) : defaut;
			cimg::option(name, 0, 0, res ? "true" : "false", usage);
			return res;
		}

		inline int option(const char *const name, const int argc, const char *const *const argv,
			const int defaut, const char *const usage = 0) {
			const char *const s = cimg::option(name, argc, argv, (char*)0);
			const int res = s ? std::atoi(s) : defaut;
			char *const tmp = new char[256];
			cimg_snprintf(tmp, 256, "%d", res);
			cimg::option(name, 0, 0, tmp, usage);
			delete[] tmp;
			return res;
		}

		inline char option(const char *const name, const int argc, const char *const *const argv,
			const char defaut, const char *const usage = 0) {
			const char *const s = cimg::option(name, argc, argv, (char*)0);
			const char res = s ? *s : defaut;
			char tmp[8];
			*tmp = res; tmp[1] = 0;
			cimg::option(name, 0, 0, tmp, usage);
			return res;
		}

		inline float option(const char *const name, const int argc, const char *const *const argv,
			const float defaut, const char *const usage = 0) {
			const char *const s = cimg::option(name, argc, argv, (char*)0);
			const float res = s ? (float)cimg::atof(s) : defaut;
			char *const tmp = new char[256];
			cimg_snprintf(tmp, 256, "%g", res);
			cimg::option(name, 0, 0, tmp, usage);
			delete[] tmp;
			return res;
		}

		inline double option(const char *const name, const int argc, const char *const *const argv,
			const double defaut, const char *const usage = 0) {
			const char *const s = cimg::option(name, argc, argv, (char*)0);
			const double res = s ? cimg::atof(s) : defaut;
			char *const tmp = new char[256];
			cimg_snprintf(tmp, 256, "%g", res);
			cimg::option(name, 0, 0, tmp, usage);
			delete[] tmp;
			return res;
		}

		//! Print information about \CImg environement variables.
		/**
		\note Output is done on the default output stream.
		**/
		inline void info() {
			std::fprintf(cimg::output(), "\n %s%sCImg Library %u.%u.%u%s, compiled %s ( %s ) with the following flags:\n\n",
				cimg::t_red, cimg::t_bold, cimg_version / 100, (cimg_version / 10) % 10, cimg_version % 10,
				cimg::t_normal, cimg_date, cimg_time);

			std::fprintf(cimg::output(), "  > Operating System:       %s%-13s%s %s('cimg_OS'=%d)%s\n",
				cimg::t_bold,
				cimg_OS == 1 ? "Unix" : (cimg_OS == 2 ? "Windows" : "Unknow"),
				cimg::t_normal, cimg::t_green,
				cimg_OS,
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > CPU endianness:         %s%s Endian%s\n",
				cimg::t_bold,
				cimg::endianness() ? "Big" : "Little",
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Verbosity mode:         %s%-13s%s %s('cimg_verbosity'=%d)%s\n",
				cimg::t_bold,
				cimg_verbosity == 0 ? "Quiet" :
				cimg_verbosity == 1 ? "Console" :
				cimg_verbosity == 2 ? "Dialog" :
				cimg_verbosity == 3 ? "Console+Warnings" : "Dialog+Warnings",
				cimg::t_normal, cimg::t_green,
				cimg_verbosity,
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Stricts warnings:       %s%-13s%s %s('cimg_strict_warnings' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_strict_warnings
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Support for C++11:      %s%-13s%s %s('cimg_use_cpp11'=%d)%s\n",
				cimg::t_bold,
				cimg_use_cpp11 ? "Yes" : "No",
				cimg::t_normal, cimg::t_green,
				(int)cimg_use_cpp11,
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Using VT100 messages:   %s%-13s%s %s('cimg_use_vt100' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_vt100
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Display type:           %s%-13s%s %s('cimg_display'=%d)%s\n",
				cimg::t_bold,
				cimg_display == 0 ? "No display" : cimg_display == 1 ? "X11" : cimg_display == 2 ? "Windows GDI" : "Unknown",
				cimg::t_normal, cimg::t_green,
				(int)cimg_display,
				cimg::t_normal);

#if cimg_display==1
			std::fprintf(cimg::output(), "  > Using XShm for X11:     %s%-13s%s %s('cimg_use_xshm' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_xshm
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Using XRand for X11:    %s%-13s%s %s('cimg_use_xrandr' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_xrandr
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);
#endif
			std::fprintf(cimg::output(), "  > Using OpenMP:           %s%-13s%s %s('cimg_use_openmp' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_openmp
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);
			std::fprintf(cimg::output(), "  > Using PNG library:      %s%-13s%s %s('cimg_use_png' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_png
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);
			std::fprintf(cimg::output(), "  > Using JPEG library:     %s%-13s%s %s('cimg_use_jpeg' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_jpeg
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Using TIFF library:     %s%-13s%s %s('cimg_use_tiff' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_tiff
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Using Magick++ library: %s%-13s%s %s('cimg_use_magick' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_magick
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Using FFTW3 library:    %s%-13s%s %s('cimg_use_fftw3' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_fftw3
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);

			std::fprintf(cimg::output(), "  > Using LAPACK library:   %s%-13s%s %s('cimg_use_lapack' %s)%s\n",
				cimg::t_bold,
#ifdef cimg_use_lapack
				"Yes", cimg::t_normal, cimg::t_green, "defined",
#else
				"No", cimg::t_normal, cimg::t_green, "undefined",
#endif
				cimg::t_normal);

			char *const tmp = new char[1024];
			cimg_snprintf(tmp, 1024, "\"%.1020s\"", cimg::imagemagick_path());
			std::fprintf(cimg::output(), "  > Path of ImageMagick:    %s%-13s%s\n",
				cimg::t_bold,
				tmp,
				cimg::t_normal);

			cimg_snprintf(tmp, 1024, "\"%.1020s\"", cimg::graphicsmagick_path());
			std::fprintf(cimg::output(), "  > Path of GraphicsMagick: %s%-13s%s\n",
				cimg::t_bold,
				tmp,
				cimg::t_normal);

			cimg_snprintf(tmp, 1024, "\"%.1020s\"", cimg::medcon_path());
			std::fprintf(cimg::output(), "  > Path of 'medcon':       %s%-13s%s\n",
				cimg::t_bold,
				tmp,
				cimg::t_normal);

			cimg_snprintf(tmp, 1024, "\"%.1020s\"", cimg::temporary_path());
			std::fprintf(cimg::output(), "  > Temporary path:         %s%-13s%s\n",
				cimg::t_bold,
				tmp,
				cimg::t_normal);

			std::fprintf(cimg::output(), "\n");
			delete[] tmp;
		}

		// Declare LAPACK function signatures if LAPACK support is enabled.
#ifdef cimg_use_lapack
		template<typename T>
		inline void getrf(int &N, T *lapA, int *IPIV, int &INFO) {
			dgetrf_(&N, &N, lapA, &N, IPIV, &INFO);
		}

		inline void getrf(int &N, float *lapA, int *IPIV, int &INFO) {
			sgetrf_(&N, &N, lapA, &N, IPIV, &INFO);
		}

		template<typename T>
		inline void getri(int &N, T *lapA, int *IPIV, T* WORK, int &LWORK, int &INFO) {
			dgetri_(&N, lapA, &N, IPIV, WORK, &LWORK, &INFO);
		}

		inline void getri(int &N, float *lapA, int *IPIV, float* WORK, int &LWORK, int &INFO) {
			sgetri_(&N, lapA, &N, IPIV, WORK, &LWORK, &INFO);
		}

		template<typename T>
		inline void gesvd(char &JOB, int &M, int &N, T *lapA, int &MN,
			T *lapS, T *lapU, T *lapV, T *WORK, int &LWORK, int &INFO) {
			dgesvd_(&JOB, &JOB, &M, &N, lapA, &MN, lapS, lapU, &M, lapV, &N, WORK, &LWORK, &INFO);
		}

		inline void gesvd(char &JOB, int &M, int &N, float *lapA, int &MN,
			float *lapS, float *lapU, float *lapV, float *WORK, int &LWORK, int &INFO) {
			sgesvd_(&JOB, &JOB, &M, &N, lapA, &MN, lapS, lapU, &M, lapV, &N, WORK, &LWORK, &INFO);
		}

		template<typename T>
		inline void getrs(char &TRANS, int &N, T *lapA, int *IPIV, T *lapB, int &INFO) {
			int one = 1;
			dgetrs_(&TRANS, &N, &one, lapA, &N, IPIV, lapB, &N, &INFO);
		}

		inline void getrs(char &TRANS, int &N, float *lapA, int *IPIV, float *lapB, int &INFO) {
			int one = 1;
			sgetrs_(&TRANS, &N, &one, lapA, &N, IPIV, lapB, &N, &INFO);
		}

		template<typename T>
		inline void syev(char &JOB, char &UPLO, int &N, T *lapA, T *lapW, T *WORK, int &LWORK, int &INFO) {
			dsyev_(&JOB, &UPLO, &N, lapA, &N, lapW, WORK, &LWORK, &INFO);
		}

		inline void syev(char &JOB, char &UPLO, int &N, float *lapA, float *lapW, float *WORK, int &LWORK, int &INFO) {
			ssyev_(&JOB, &UPLO, &N, lapA, &N, lapW, WORK, &LWORK, &INFO);
		}

		template<typename T>
		inline void sgels(char & TRANS, int &M, int &N, int &NRHS, T* lapA, int &LDA,
			T* lapB, int &LDB, T* WORK, int &LWORK, int &INFO) {
			dgels_(&TRANS, &M, &N, &NRHS, lapA, &LDA, lapB, &LDB, WORK, &LWORK, &INFO);
		}

		inline void sgels(char & TRANS, int &M, int &N, int &NRHS, float* lapA, int &LDA,
			float* lapB, int &LDB, float* WORK, int &LWORK, int &INFO) {
			sgels_(&TRANS, &M, &N, &NRHS, lapA, &LDA, lapB, &LDB, WORK, &LWORK, &INFO);
		}

#endif

		// End of the 'cimg' namespace
	}

	/*------------------------------------------------
	#
	#
	#   Definition of mathematical operators and
	#   external functions.
	#
	#
	-------------------------------------------------*/

#define _cimg_create_ext_operators(typ) \
  template<typename T> \
  inline CImg<typename cimg::superset<T,typ>::type> operator+(const typ val, const CImg<T>& img) { \
    return img + val; \
  } \
  template<typename T> \
  inline CImg<typename cimg::superset<T,typ>::type> operator-(const typ val, const CImg<T>& img) { \
    typedef typename cimg::superset<T,typ>::type Tt; \
    return CImg<Tt>(img._width,img._height,img._depth,img._spectrum,val)-=img; \
  } \
  template<typename T> \
  inline CImg<typename cimg::superset<T,typ>::type> operator*(const typ val, const CImg<T>& img) { \
    return img*val; \
  } \
  template<typename T> \
  inline CImg<typename cimg::superset<T,typ>::type> operator/(const typ val, const CImg<T>& img) { \
    return val*img.get_invert(); \
  } \
  template<typename T> \
  inline CImg<typename cimg::superset<T,typ>::type> operator&(const typ val, const CImg<T>& img) { \
    return img & val; \
  } \
  template<typename T> \
  inline CImg<typename cimg::superset<T,typ>::type> operator|(const typ val, const CImg<T>& img) { \
    return img | val; \
  } \
  template<typename T> \
  inline CImg<typename cimg::superset<T,typ>::type> operator^(const typ val, const CImg<T>& img) { \
    return img ^ val; \
  } \
  template<typename T> \
  inline bool operator==(const typ val, const CImg<T>& img) {   \
    return img == val; \
  } \
  template<typename T> \
  inline bool operator!=(const typ val, const CImg<T>& img) { \
    return img != val; \
  }

	_cimg_create_ext_operators(bool)
		_cimg_create_ext_operators(unsigned char)
		_cimg_create_ext_operators(char)
		_cimg_create_ext_operators(signed char)
		_cimg_create_ext_operators(unsigned short)
		_cimg_create_ext_operators(short)
		_cimg_create_ext_operators(unsigned int)
		_cimg_create_ext_operators(int)
		_cimg_create_ext_operators(cimg_uint64)
		_cimg_create_ext_operators(cimg_int64)
		_cimg_create_ext_operators(float)
		_cimg_create_ext_operators(double)
		_cimg_create_ext_operators(long double)

		template<typename T>
	inline CImg<_cimg_Tfloat> operator+(const char *const expression, const CImg<T>& img) {
		return img + expression;
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> operator-(const char *const expression, const CImg<T>& img) {
		return CImg<_cimg_Tfloat>(img, false).fill(expression, true) -= img;
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> operator*(const char *const expression, const CImg<T>& img) {
		return img*expression;
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> operator/(const char *const expression, const CImg<T>& img) {
		return expression*img.get_invert();
	}

	template<typename T>
	inline CImg<T> operator&(const char *const expression, const CImg<T>& img) {
		return img & expression;
	}

	template<typename T>
	inline CImg<T> operator|(const char *const expression, const CImg<T>& img) {
		return img | expression;
	}

	template<typename T>
	inline CImg<T> operator^(const char *const expression, const CImg<T>& img) {
		return img ^ expression;
	}

	template<typename T>
	inline bool operator==(const char *const expression, const CImg<T>& img) {
		return img == expression;
	}

	template<typename T>
	inline bool operator!=(const char *const expression, const CImg<T>& img) {
		return img != expression;
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> sqr(const CImg<T>& instance) {
		return instance.get_sqr();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> sqrt(const CImg<T>& instance) {
		return instance.get_sqrt();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> exp(const CImg<T>& instance) {
		return instance.get_exp();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> log(const CImg<T>& instance) {
		return instance.get_log();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> log2(const CImg<T>& instance) {
		return instance.get_log2();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> log10(const CImg<T>& instance) {
		return instance.get_log10();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> abs(const CImg<T>& instance) {
		return instance.get_abs();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> sign(const CImg<T>& instance) {
		return instance.get_sign();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> cos(const CImg<T>& instance) {
		return instance.get_cos();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> sin(const CImg<T>& instance) {
		return instance.get_sin();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> sinc(const CImg<T>& instance) {
		return instance.get_sinc();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> tan(const CImg<T>& instance) {
		return instance.get_tan();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> acos(const CImg<T>& instance) {
		return instance.get_acos();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> asin(const CImg<T>& instance) {
		return instance.get_asin();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> atan(const CImg<T>& instance) {
		return instance.get_atan();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> cosh(const CImg<T>& instance) {
		return instance.get_cosh();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> sinh(const CImg<T>& instance) {
		return instance.get_sinh();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> tanh(const CImg<T>& instance) {
		return instance.get_tanh();
	}

	template<typename T>
	inline CImg<T> transpose(const CImg<T>& instance) {
		return instance.get_transpose();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> invert(const CImg<T>& instance) {
		return instance.get_invert();
	}

	template<typename T>
	inline CImg<_cimg_Tfloat> pseudoinvert(const CImg<T>& instance) {
		return instance.get_pseudoinvert();
	}

}

#endif // !_CIMG_MAIN