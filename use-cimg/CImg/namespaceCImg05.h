#ifndef _CIMG_NAMESPACE
#define _CIMG_NAMESPACE


#include "classCImgList04.h"

namespace cimg_library_suffixed {
/*
#---------------------------------------------
#
# Completion of previously declared functions
#
#----------------------------------------------
*/

namespace cimg {

	// Functions to return standard streams 'stdin', 'stdout' and 'stderr'.
	// (throw a CImgIOException when macro 'cimg_use_r' is defined).
	inline FILE* _stdin(const bool throw_exception) {
#ifndef cimg_use_r
		cimg::unused(throw_exception);
		return stdin;
#else
		if (throw_exception) {
			cimg::exception_mode(0);
			throw CImgIOException("cimg::stdin(): Reference to 'stdin' stream not allowed in R mode "
				"('cimg_use_r' is defined).");
		}
		return 0;
#endif
	}

	inline FILE* _stdout(const bool throw_exception) {
#ifndef cimg_use_r
		cimg::unused(throw_exception);
		return stdout;
#else
		if (throw_exception) {
			cimg::exception_mode(0);
			throw CImgIOException("cimg::stdout(): Reference to 'stdout' stream not allowed in R mode "
				"('cimg_use_r' is defined).");
		}
		return 0;
#endif
	}

	inline FILE* _stderr(const bool throw_exception) {
#ifndef cimg_use_r
		cimg::unused(throw_exception);
		return stderr;
#else
		if (throw_exception) {
			cimg::exception_mode(0);
			throw CImgIOException("cimg::stderr(): Reference to 'stderr' stream not allowed in R mode "
				"('cimg_use_r' is defined).");
		}
		return 0;
#endif
	}

	// Open a file (with wide character support on Windows).
	inline std::FILE *win_fopen(const char *const path, const char *const mode) {
#if cimg_OS==2
		// Convert 'path' to a wide-character string.
		int err = MultiByteToWideChar(CP_UTF8, 0, path, -1, 0, 0);
		if (!err) return std_fopen(path, mode);
		CImg<wchar_t> wpath(err);
		err = MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, err);
		if (!err) return std_fopen(path, mode);

		// Convert 'mode' to a wide-character string.
		err = MultiByteToWideChar(CP_UTF8, 0, mode, -1, 0, 0);
		if (!err) return std_fopen(path, mode);
		CImg<wchar_t> wmode(err);
		err = MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, err);
		if (!err) return std_fopen(path, mode);
		return _wfopen(wpath, wmode);
#else
		return std_fopen(path, mode);
#endif
	}

	//! Get/set path to store temporary files.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path where temporary files can be saved.
	**/
	inline const char* temporary_path(const char *const user_path, const bool reinit_path) {
#define _cimg_test_temporary_path(p)                                    \
    if (!path_found) {                                                  \
      cimg_snprintf(s_path,s_path.width(),"%s",p);                      \
      cimg_snprintf(tmp,tmp._width,"%s%c%s",s_path.data(),cimg_file_separator,filename_tmp._data); \
      if ((file=std_fopen(tmp,"wb"))!=0) { cimg::fclose(file); std::remove(tmp); path_found = true; } \
    }
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			CImg<char> tmp(1024), filename_tmp(256);
			std::FILE *file = 0;
			cimg_snprintf(filename_tmp, filename_tmp._width, "%s.tmp", cimg::filenamerand());
			char *tmpPath = std::getenv("TMP");
			if (!tmpPath) { tmpPath = std::getenv("TEMP"); winformat_string(tmpPath); }
			if (tmpPath) _cimg_test_temporary_path(tmpPath);
#if cimg_OS==2
			_cimg_test_temporary_path("C:\\WINNT\\Temp");
			_cimg_test_temporary_path("C:\\WINDOWS\\Temp");
			_cimg_test_temporary_path("C:\\Temp");
			_cimg_test_temporary_path("C:");
			_cimg_test_temporary_path("D:\\WINNT\\Temp");
			_cimg_test_temporary_path("D:\\WINDOWS\\Temp");
			_cimg_test_temporary_path("D:\\Temp");
			_cimg_test_temporary_path("D:");
#else
			_cimg_test_temporary_path("/tmp");
			_cimg_test_temporary_path("/var/tmp");
#endif
			if (!path_found) {
				*s_path = 0;
				std::strncpy(tmp, filename_tmp, tmp._width - 1);
				if ((file = std_fopen(tmp, "wb")) != 0) { cimg::fclose(file); std::remove(tmp); path_found = true; }
			}
			if (!path_found) {
				cimg::mutex(7, 0);
				throw CImgIOException("cimg::temporary_path(): Failed to locate path for writing temporary files.\n");
			}
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the <i>Program Files/</i> directory (Windows only).
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the program files.
	**/
#if cimg_OS==2
	inline const char* programfiles_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(MAX_PATH);
			*s_path = 0;
			// Note: in the following line, 0x26 = CSIDL_PROGRAM_FILES (not defined on every compiler).
#if !defined(__INTEL_COMPILER)
			if (!SHGetSpecialFolderPathA(0, s_path, 0x0026, false)) {
				const char *const pfPath = std::getenv("PROGRAMFILES");
				if (pfPath) std::strncpy(s_path, pfPath, MAX_PATH - 1);
				else std::strcpy(s_path, "C:\\PROGRA~1");
			}
#else
			std::strcpy(s_path, "C:\\PROGRA~1");
#endif
		}
		cimg::mutex(7, 0);
		return s_path;
	}
#endif

	//! Get/set path to the ImageMagick's \c convert binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c convert binary.
	**/
	inline const char* imagemagick_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			const char *const pf_path = programfiles_path();
			for (int l = 0; l<2 && !path_found; ++l) {
				const char *const s_exe = l ? "convert" : "magick";
				cimg_snprintf(s_path, s_path._width, ".\\%s.exe", s_exe);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				for (int k = 32; k >= 10 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "%s\\IMAGEM~1.%.2d-\\%s.exe", pf_path, k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 9; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "%s\\IMAGEM~1.%d-Q\\%s.exe", pf_path, k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "%s\\IMAGEM~1.%d\\%s.exe", pf_path, k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 10 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "%s\\IMAGEM~1.%.2d-\\VISUA~1\\BIN\\%s.exe", pf_path, k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 9; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "%s\\IMAGEM~1.%d-Q\\VISUA~1\\BIN\\%s.exe", pf_path, k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "%s\\IMAGEM~1.%d\\VISUA~1\\BIN\\%s.exe", pf_path, k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 10 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "C:\\IMAGEM~1.%.2d-\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 9; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "C:\\IMAGEM~1.%d-Q\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "C:\\IMAGEM~1.%d\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 10 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "C:\\IMAGEM~1.%.2d-\\VISUA~1\\BIN\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 9; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "C:\\IMAGEM~1.%d-Q\\VISUA~1\\BIN\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "C:\\IMAGEM~1.%d\\VISUA~1\\BIN\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 10 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "D:\\IMAGEM~1.%.2d-\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 9; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "D:\\IMAGEM~1.%d-Q\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "D:\\IMAGEM~1.%d\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 10 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "D:\\IMAGEM~1.%.2d-\\VISUA~1\\BIN\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 9; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "D:\\IMAGEM~1.%d-Q\\VISUA~1\\BIN\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				for (int k = 32; k >= 0 && !path_found; --k) {
					cimg_snprintf(s_path, s_path._width, "D:\\IMAGEM~1.%d\\VISUA~1\\BIN\\%s.exe", k, s_exe);
					if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
				}
				if (!path_found) cimg_snprintf(s_path, s_path._width, "%s.exe", s_exe);
			}
#else
			std::strcpy(s_path, "./magick");
			if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			if (!path_found) {
				std::strcpy(s_path, "./convert");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "convert");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the GraphicsMagick's \c gm binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c gm binary.
	**/
	inline const char* graphicsmagick_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			const char *const pf_path = programfiles_path();
			if (!path_found) {
				std::strcpy(s_path, ".\\gm.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 10 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "%s\\GRAPHI~1.%.2d-\\gm.exe", pf_path, k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 9; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "%s\\GRAPHI~1.%d-Q\\gm.exe", pf_path, k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "%s\\GRAPHI~1.%d\\gm.exe", pf_path, k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 10 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "%s\\GRAPHI~1.%.2d-\\VISUA~1\\BIN\\gm.exe", pf_path, k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 9; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "%s\\GRAPHI~1.%d-Q\\VISUA~1\\BIN\\gm.exe", pf_path, k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "%s\\GRAPHI~1.%d\\VISUA~1\\BIN\\gm.exe", pf_path, k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 10 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "C:\\GRAPHI~1.%.2d-\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 9; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "C:\\GRAPHI~1.%d-Q\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "C:\\GRAPHI~1.%d\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 10 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "C:\\GRAPHI~1.%.2d-\\VISUA~1\\BIN\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 9; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "C:\\GRAPHI~1.%d-Q\\VISUA~1\\BIN\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "C:\\GRAPHI~1.%d\\VISUA~1\\BIN\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 10 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "D:\\GRAPHI~1.%.2d-\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 9; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "D:\\GRAPHI~1.%d-Q\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "D:\\GRAPHI~1.%d\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 10 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "D:\\GRAPHI~1.%.2d-\\VISUA~1\\BIN\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 9; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "D:\\GRAPHI~1.%d-Q\\VISUA~1\\BIN\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			for (int k = 32; k >= 0 && !path_found; --k) {
				cimg_snprintf(s_path, s_path._width, "D:\\GRAPHI~1.%d\\VISUA~1\\BIN\\gm.exe", k);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "gm.exe");
#else
			if (!path_found) {
				std::strcpy(s_path, "./gm");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "gm");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the XMedcon's \c medcon binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c medcon binary.
	**/
	inline const char* medcon_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			const char *const pf_path = programfiles_path();
			if (!path_found) {
				std::strcpy(s_path, ".\\medcon.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) {
				cimg_snprintf(s_path, s_path._width, "%s\\XMedCon\\bin\\medcon.bat", pf_path);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) {
				cimg_snprintf(s_path, s_path._width, "%s\\XMedCon\\bin\\medcon.exe", pf_path);
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) {
				std::strcpy(s_path, "C:\\XMedCon\\bin\\medcon.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "medcon.exe");
#else
			if (!path_found) {
				std::strcpy(s_path, "./medcon");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "medcon");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the FFMPEG's \c ffmpeg binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c ffmpeg binary.
	**/
	inline const char *ffmpeg_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			if (!path_found) {
				std::strcpy(s_path, ".\\ffmpeg.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "ffmpeg.exe");
#else
			if (!path_found) {
				std::strcpy(s_path, "./ffmpeg");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "ffmpeg");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the \c gzip binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c gzip binary.
	**/
	inline const char *gzip_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			if (!path_found) {
				std::strcpy(s_path, ".\\gzip.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "gzip.exe");
#else
			if (!path_found) {
				std::strcpy(s_path, "./gzip");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "gzip");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the \c gunzip binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c gunzip binary.
	**/
	inline const char *gunzip_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			if (!path_found) {
				std::strcpy(s_path, ".\\gunzip.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "gunzip.exe");
#else
			if (!path_found) {
				std::strcpy(s_path, "./gunzip");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "gunzip");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the \c dcraw binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c dcraw binary.
	**/
	inline const char *dcraw_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			if (!path_found) {
				std::strcpy(s_path, ".\\dcraw.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "dcraw.exe");
#else
			if (!path_found) {
				std::strcpy(s_path, "./dcraw");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "dcraw");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the \c wget binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c wget binary.
	**/
	inline const char *wget_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			if (!path_found) {
				std::strcpy(s_path, ".\\wget.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "wget.exe");
#else
			if (!path_found) {
				std::strcpy(s_path, "./wget");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "wget");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	//! Get/set path to the \c curl binary.
	/**
	\param user_path Specified path, or \c 0 to get the path currently used.
	\param reinit_path Force path to be recalculated (may take some time).
	\return Path containing the \c curl binary.
	**/
	inline const char *curl_path(const char *const user_path, const bool reinit_path) {
		static CImg<char> s_path;
		cimg::mutex(7);
		if (reinit_path) s_path.assign();
		if (user_path) {
			if (!s_path) s_path.assign(1024);
			std::strncpy(s_path, user_path, 1023);
		}
		else if (!s_path) {
			s_path.assign(1024);
			bool path_found = false;
			std::FILE *file = 0;
#if cimg_OS==2
			if (!path_found) {
				std::strcpy(s_path, ".\\curl.exe");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "curl.exe");
#else
			if (!path_found) {
				std::strcpy(s_path, "./curl");
				if ((file = std_fopen(s_path, "r")) != 0) { cimg::fclose(file); path_found = true; }
			}
			if (!path_found) std::strcpy(s_path, "curl");
#endif
			winformat_string(s_path);
		}
		cimg::mutex(7, 0);
		return s_path;
	}

	// [internal] Sorting function, used by cimg::files().
	inline int _sort_files(const void* a, const void* b) {
		const CImg<char> &sa = *(CImg<char>*)a, &sb = *(CImg<char>*)b;
		return std::strcmp(sa._data, sb._data);
	}

	//! Return list of files/directories in specified directory.
	/**
	\param path Path to the directory. Set to 0 for current directory.
	\param is_pattern Tell if specified path has a matching pattern in it.
	\param mode Output type, can be primary { 0=files only | 1=folders only | 2=files + folders }.
	\param include_path Tell if \c path must be included in resulting filenames.
	\return A list of filenames.
	**/
	inline CImgList<char> files(const char *const path, const bool is_pattern = false,
		const unsigned int mode = 2, const bool include_path = false) {
		if (!path || !*path) return files("*", true, mode, include_path);
		CImgList<char> res;

		// If path is a valid folder name, ignore argument 'is_pattern'.
		const bool _is_pattern = is_pattern && !cimg::is_directory(path);
		bool is_root = false, is_current = false;
		cimg::unused(is_root, is_current);

		// Clean format of input path.
		CImg<char> pattern, _path = CImg<char>::string(path);
#if cimg_OS==2
		for (char *ps = _path; *ps; ++ps) if (*ps == '\\') *ps = '/';
#endif
		char *pd = _path;
		for (char *ps = pd; *ps; ++ps) { if (*ps != '/' || *ps != *(ps + 1)) *(pd++) = *ps; }
		*pd = 0;
		unsigned int lp = (unsigned int)std::strlen(_path);
		if (!_is_pattern && lp && _path[lp - 1] == '/') {
			_path[lp - 1] = 0; --lp;
#if cimg_OS!=2
			is_root = !*_path;
#endif
		}

		// Separate folder path and matching pattern.
		if (_is_pattern) {
			const unsigned int bpos = (unsigned int)(cimg::basename(_path, '/') - _path.data());
			CImg<char>::string(_path).move_to(pattern);
			if (bpos) {
				_path[bpos - 1] = 0; // End 'path' at last slash.
#if cimg_OS!=2
				is_root = !*_path;
#endif
			}
			else { // No path to folder specified, assuming current folder.
				is_current = true; *_path = 0;
			}
			lp = (unsigned int)std::strlen(_path);
		}

		// Windows version.
#if cimg_OS==2
		if (!_is_pattern) {
			pattern.assign(lp + 3);
			std::memcpy(pattern, _path, lp);
			pattern[lp] = '/'; pattern[lp + 1] = '*'; pattern[lp + 2] = 0;
		}
		WIN32_FIND_DATAA file_data;
		const HANDLE dir = FindFirstFileA(pattern.data(), &file_data);
		if (dir == INVALID_HANDLE_VALUE) return CImgList<char>::const_empty();
		do {
			const char *const filename = file_data.cFileName;
			if (*filename != '.' || (filename[1] && (filename[1] != '.' || filename[2]))) {
				const unsigned int lf = (unsigned int)std::strlen(filename);
				const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
				if ((!mode && !is_directory) || (mode == 1 && is_directory) || mode >= 2) {
					if (include_path) {
						CImg<char> full_filename((lp ? lp + 1 : 0) + lf + 1);
						if (lp) { std::memcpy(full_filename, _path, lp); full_filename[lp] = '/'; }
						std::memcpy(full_filename._data + (lp ? lp + 1 : 0), filename, lf + 1);
						full_filename.move_to(res);
					}
					else CImg<char>(filename, lf + 1).move_to(res);
				}
			}
		} while (FindNextFileA(dir, &file_data));
		FindClose(dir);

		// Unix version (posix).
#elif cimg_OS == 1
		DIR *const dir = opendir(is_root ? "/" : is_current ? "." : _path.data());
		if (!dir) return CImgList<char>::const_empty();
		struct dirent *ent;
		while ((ent = readdir(dir)) != 0) {
			const char *const filename = ent->d_name;
			if (*filename != '.' || (filename[1] && (filename[1] != '.' || filename[2]))) {
				const unsigned int lf = (unsigned int)std::strlen(filename);
				CImg<char> full_filename(lp + lf + 2);

				if (!is_current) {
					full_filename.assign(lp + lf + 2);
					if (lp) std::memcpy(full_filename, _path, lp);
					full_filename[lp] = '/';
					std::memcpy(full_filename._data + lp + 1, filename, lf + 1);
				}
				else full_filename.assign(filename, lf + 1);

				struct stat st;
				if (stat(full_filename, &st) == -1) continue;
				const bool is_directory = (st.st_mode & S_IFDIR) != 0;
				if ((!mode && !is_directory) || (mode == 1 && is_directory) || mode == 2) {
					if (include_path) {
						if (!_is_pattern || (_is_pattern && !fnmatch(pattern, full_filename, 0)))
							full_filename.move_to(res);
					}
					else {
						if (!_is_pattern || (_is_pattern && !fnmatch(pattern, full_filename, 0)))
							CImg<char>(filename, lf + 1).move_to(res);
					}
				}
			}
		}
		closedir(dir);
#endif

		// Sort resulting list by lexicographic order.
		if (res._width >= 2) std::qsort(res._data, res._width, sizeof(CImg<char>), _sort_files);

		return res;
	}

	//! Try to guess format from an image file.
	/**
	\param file Input file (can be \c 0 if \c filename is set).
	\param filename Filename, as a C-string (can be \c 0 if \c file is set).
	\return C-string containing the guessed file format, or \c 0 if nothing has been guessed.
	**/
	inline const char *ftype(std::FILE *const file, const char *const filename) {
		if (!file && !filename)
			throw CImgArgumentException("cimg::ftype(): Specified filename is (null).");
		static const char
			*const _pnm = "pnm",
			*const _pfm = "pfm",
			*const _bmp = "bmp",
			*const _gif = "gif",
			*const _jpg = "jpg",
			*const _off = "off",
			*const _pan = "pan",
			*const _png = "png",
			*const _tif = "tif",
			*const _inr = "inr",
			*const _dcm = "dcm";
		const char *f_type = 0;
		CImg<char> header;
		const unsigned int omode = cimg::exception_mode();
		cimg::exception_mode(0);
		try {
			header._load_raw(file, filename, 512, 1, 1, 1, false, false, 0);
			const unsigned char *const uheader = (unsigned char*)header._data;
			if (!std::strncmp(header, "OFF\n", 4)) f_type = _off; // OFF.
			else if (!std::strncmp(header, "#INRIMAGE", 9)) f_type = _inr; // INRIMAGE.
			else if (!std::strncmp(header, "PANDORE", 7)) f_type = _pan; // PANDORE.
			else if (!std::strncmp(header.data() + 128, "DICM", 4)) f_type = _dcm; // DICOM.
			else if (uheader[0] == 0xFF && uheader[1] == 0xD8 && uheader[2] == 0xFF) f_type = _jpg;  // JPEG.
			else if (header[0] == 'B' && header[1] == 'M') f_type = _bmp;  // BMP.
			else if (header[0] == 'G' && header[1] == 'I' && header[2] == 'F' && header[3] == '8' && header[5] == 'a' && // GIF.
				(header[4] == '7' || header[4] == '9')) f_type = _gif;
			else if (uheader[0] == 0x89 && uheader[1] == 0x50 && uheader[2] == 0x4E && uheader[3] == 0x47 &&  // PNG.
				uheader[4] == 0x0D && uheader[5] == 0x0A && uheader[6] == 0x1A && uheader[7] == 0x0A) f_type = _png;
			else if ((uheader[0] == 0x49 && uheader[1] == 0x49) || (uheader[0] == 0x4D && uheader[1] == 0x4D)) f_type = _tif; // TIFF.
			else { // PNM or PFM.
				CImgList<char> _header = header.get_split(CImg<char>::vector('\n'), 0, false);
				cimglist_for(_header, l) {
					if (_header(l, 0) == '#') continue;
					if (_header[l]._height == 2 && _header(l, 0) == 'P') {
						const char c = _header(l, 1);
						if (c == 'f' || c == 'F') { f_type = _pfm; break; }
						if (c >= '1' && c <= '9') { f_type = _pnm; break; }
					}
					f_type = 0; break;
				}
			}
		}
		catch (CImgIOException&) {}
		cimg::exception_mode(omode);
		return f_type;
	}

	//! Load file from network as a local temporary file.
	/**
	\param url URL of the filename, as a C-string.
	\param[out] filename_local C-string containing the path to a local copy of \c filename.
	\param timeout Maximum time (in seconds) authorized for downloading the file from the URL.
	\param try_fallback When using libcurl, tells using system calls as fallbacks in case of libcurl failure.
	\param referer Referer used, as a C-string.
	\return Value of \c filename_local.
	\note Use the \c libcurl library, or the external binaries \c wget or \c curl to perform the download.
	**/
	inline char *load_network(const char *const url, char *const filename_local,
		const unsigned int timeout, const bool try_fallback,
		const char *const referer) {
		if (!url)
			throw CImgArgumentException("cimg::load_network(): Specified URL is (null).");
		if (!filename_local)
			throw CImgArgumentException("cimg::load_network(): Specified destination string is (null).");

		const char *const __ext = cimg::split_filename(url), *const _ext = (*__ext && __ext>url) ? __ext - 1 : __ext;
		CImg<char> ext = CImg<char>::string(_ext);
		std::FILE *file = 0;
		*filename_local = 0;
		if (ext._width>16 || !cimg::strncasecmp(ext, "cgi", 3)) *ext = 0;
		else cimg::strwindows_reserved(ext);
		do {
			cimg_snprintf(filename_local, 256, "%s%c%s%s",
				cimg::temporary_path(), cimg_file_separator, cimg::filenamerand(), ext._data);
			if ((file = std_fopen(filename_local, "rb")) != 0) cimg::fclose(file);
		} while (file);

#ifdef cimg_use_curl
		const unsigned int omode = cimg::exception_mode();
		cimg::exception_mode(0);
		try {
			CURL *curl = 0;
			CURLcode res;
			curl = curl_easy_init();
			if (curl) {
				file = cimg::fopen(filename_local, "wb");
				curl_easy_setopt(curl, CURLOPT_URL, url);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 0);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
				if (timeout) curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)timeout);
				if (std::strchr(url, '?')) curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
				if (referer) curl_easy_setopt(curl, CURLOPT_REFERER, referer);
				res = curl_easy_perform(curl);
				curl_easy_cleanup(curl);
				cimg::fseek(file, 0, SEEK_END); // Check if file size is 0.
				const cimg_ulong siz = cimg::ftell(file);
				cimg::fclose(file);
				if (siz>0 && res == CURLE_OK) {
					cimg::exception_mode(omode);
					return filename_local;
				}
				else std::remove(filename_local);
			}
		}
		catch (...) {}
		cimg::exception_mode(omode);
		if (!try_fallback) throw CImgIOException("cimg::load_network(): Failed to load file '%s' with libcurl.", url);
#endif

		CImg<char> command((unsigned int)std::strlen(url) + 64);
		cimg::unused(try_fallback);

		// Try with 'curl' first.
		if (timeout) {
			if (referer)
				cimg_snprintf(command, command._width, "%s -e %s -m %u -f --silent --compressed -o \"%s\" \"%s\"",
					cimg::curl_path(), referer, timeout, filename_local, url);
			else
				cimg_snprintf(command, command._width, "%s -m %u -f --silent --compressed -o \"%s\" \"%s\"",
					cimg::curl_path(), timeout, filename_local, url);
		}
		else {
			if (referer)
				cimg_snprintf(command, command._width, "%s -e %s -f --silent --compressed -o \"%s\" \"%s\"",
					cimg::curl_path(), referer, filename_local, url);
			else
				cimg_snprintf(command, command._width, "%s -f --silent --compressed -o \"%s\" \"%s\"",
					cimg::curl_path(), filename_local, url);
		}
		cimg::system(command);

		if (!(file = std_fopen(filename_local, "rb"))) {

			// Try with 'wget' otherwise.
			if (timeout) {
				if (referer)
					cimg_snprintf(command, command._width, "%s --referer=%s -T %u -q -r -l 0 --no-cache -O \"%s\" \"%s\"",
						cimg::wget_path(), referer, timeout, filename_local, url);
				else
					cimg_snprintf(command, command._width, "%s -T %u -q -r -l 0 --no-cache -O \"%s\" \"%s\"",
						cimg::wget_path(), timeout, filename_local, url);
			}
			else {
				if (referer)
					cimg_snprintf(command, command._width, "%s --referer=%s -q -r -l 0 --no-cache -O \"%s\" \"%s\"",
						cimg::wget_path(), referer, filename_local, url);
				else
					cimg_snprintf(command, command._width, "%s -q -r -l 0 --no-cache -O \"%s\" \"%s\"",
						cimg::wget_path(), filename_local, url);
			}
			cimg::system(command);

			if (!(file = std_fopen(filename_local, "rb")))
				throw CImgIOException("cimg::load_network(): Failed to load file '%s' with external commands "
					"'wget' or 'curl'.", url);
			cimg::fclose(file);

			// Try gunzip it.
			cimg_snprintf(command, command._width, "%s.gz", filename_local);
			std::rename(filename_local, command);
			cimg_snprintf(command, command._width, "%s --quiet \"%s.gz\"",
				gunzip_path(), filename_local);
			cimg::system(command);
			file = std_fopen(filename_local, "rb");
			if (!file) {
				cimg_snprintf(command, command._width, "%s.gz", filename_local);
				std::rename(command, filename_local);
				file = std_fopen(filename_local, "rb");
			}
		}
		cimg::fseek(file, 0, SEEK_END); // Check if file size is 0.
		if (std::ftell(file) <= 0)
			throw CImgIOException("cimg::load_network(): Failed to load URL '%s' with external commands "
				"'wget' or 'curl'.", url);
		cimg::fclose(file);
		return filename_local;
	}

	// Implement a tic/toc mechanism to display elapsed time of algorithms.
	inline cimg_ulong tictoc(const bool is_tic) {
		cimg::mutex(2);
		static CImg<cimg_ulong> times(64);
		static unsigned int pos = 0;
		const cimg_ulong t1 = cimg::time();
		if (is_tic) { // Tic.
			times[pos++] = t1;
			if (pos >= times._width)
				throw CImgArgumentException("cimg::tic(): Too much calls to 'cimg::tic()' without calls to 'cimg::toc()'.");
			cimg::mutex(2, 0);
			return t1;
		}
		// Toc.
		if (!pos)
			throw CImgArgumentException("cimg::toc(): No previous call to 'cimg::tic()' has been made.");
		const cimg_ulong
			t0 = times[--pos],
			dt = t1 >= t0 ? (t1 - t0) : cimg::type<cimg_ulong>::max();
		const unsigned int
			edays = (unsigned int)(dt / 86400000.0),
			ehours = (unsigned int)((dt - edays*86400000.0) / 3600000.0),
			emin = (unsigned int)((dt - edays*86400000.0 - ehours*3600000.0) / 60000.0),
			esec = (unsigned int)((dt - edays*86400000.0 - ehours*3600000.0 - emin*60000.0) / 1000.0),
			ems = (unsigned int)(dt - edays*86400000.0 - ehours*3600000.0 - emin*60000.0 - esec*1000.0);
		if (!edays && !ehours && !emin && !esec)
			std::fprintf(cimg::output(), "%s[CImg]%*sElapsed time: %u ms%s\n",
				cimg::t_red, 1 + 2 * pos, "", ems, cimg::t_normal);
		else {
			if (!edays && !ehours && !emin)
				std::fprintf(cimg::output(), "%s[CImg]%*sElapsed time: %u sec %u ms%s\n",
					cimg::t_red, 1 + 2 * pos, "", esec, ems, cimg::t_normal);
			else {
				if (!edays && !ehours)
					std::fprintf(cimg::output(), "%s[CImg]%*sElapsed time: %u min %u sec %u ms%s\n",
						cimg::t_red, 1 + 2 * pos, "", emin, esec, ems, cimg::t_normal);
				else {
					if (!edays)
						std::fprintf(cimg::output(), "%s[CImg]%*sElapsed time: %u hours %u min %u sec %u ms%s\n",
							cimg::t_red, 1 + 2 * pos, "", ehours, emin, esec, ems, cimg::t_normal);
					else {
						std::fprintf(cimg::output(), "%s[CImg]%*sElapsed time: %u days %u hours %u min %u sec %u ms%s\n",
							cimg::t_red, 1 + 2 * pos, "", edays, ehours, emin, esec, ems, cimg::t_normal);
					}
				}
			}
		}
		cimg::mutex(2, 0);
		return dt;
	}

	// Return a temporary string describing the size of a memory buffer.
	inline const char *strbuffersize(const cimg_ulong size) {
		static CImg<char> res(256);
		cimg::mutex(5);
		if (size<1024LU) cimg_snprintf(res, res._width, "%lu byte%s", (unsigned long)size, size>1 ? "s" : "");
		else if (size<1024 * 1024LU) { const float nsize = size / 1024.0f; cimg_snprintf(res, res._width, "%.1f Kio", nsize); }
		else if (size<1024 * 1024 * 1024LU) {
			const float nsize = size / (1024 * 1024.0f); cimg_snprintf(res, res._width, "%.1f Mio", nsize);
		}
		else { const float nsize = size / (1024 * 1024 * 1024.0f); cimg_snprintf(res, res._width, "%.1f Gio", nsize); }
		cimg::mutex(5, 0);
		return res;
	}

	//! Display a simple dialog box, and wait for the user's response.
	/**
	\param title Title of the dialog window.
	\param msg Main message displayed inside the dialog window.
	\param button1_label Label of the 1st button.
	\param button2_label Label of the 2nd button (\c 0 to hide button).
	\param button3_label Label of the 3rd button (\c 0 to hide button).
	\param button4_label Label of the 4th button (\c 0 to hide button).
	\param button5_label Label of the 5th button (\c 0 to hide button).
	\param button6_label Label of the 6th button (\c 0 to hide button).
	\param logo Image logo displayed at the left of the main message.
	\param is_centered Tells if the dialog window must be centered on the screen.
	\return Indice of clicked button (from \c 0 to \c 5), or \c -1 if the dialog window has been closed by the user.
	\note
	- Up to 6 buttons can be defined in the dialog window.
	- The function returns when a user clicked one of the button or closed the dialog window.
	- If a button text is set to 0, the corresponding button (and the followings) will not appear in the dialog box.
	At least one button must be specified.
	**/
	template<typename t>
	inline int dialog(const char *const title, const char *const msg,
		const char *const button1_label, const char *const button2_label,
		const char *const button3_label, const char *const button4_label,
		const char *const button5_label, const char *const button6_label,
		const CImg<t>& logo, const bool is_centered = false) {
#if cimg_display==0
		cimg::unused(title, msg, button1_label, button2_label, button3_label, button4_label, button5_label, button6_label,
			logo._data, is_centered);
		throw CImgIOException("cimg::dialog(): No display available.");
#else
		static const unsigned char
			black[] = { 0,0,0 }, white[] = { 255,255,255 }, gray[] = { 200,200,200 }, gray2[] = { 150,150,150 };

		// Create buttons and canvas graphics
		CImgList<unsigned char> buttons, cbuttons, sbuttons;
		if (button1_label) {
			CImg<unsigned char>().draw_text(0, 0, button1_label, black, gray, 1, 13).move_to(buttons);
			if (button2_label) {
				CImg<unsigned char>().draw_text(0, 0, button2_label, black, gray, 1, 13).move_to(buttons);
				if (button3_label) {
					CImg<unsigned char>().draw_text(0, 0, button3_label, black, gray, 1, 13).move_to(buttons);
					if (button4_label) {
						CImg<unsigned char>().draw_text(0, 0, button4_label, black, gray, 1, 13).move_to(buttons);
						if (button5_label) {
							CImg<unsigned char>().draw_text(0, 0, button5_label, black, gray, 1, 13).move_to(buttons);
							if (button6_label) {
								CImg<unsigned char>().draw_text(0, 0, button6_label, black, gray, 1, 13).move_to(buttons);
							}
						}
					}
				}
			}
		}
		if (!buttons._width)
			throw CImgArgumentException("cimg::dialog(): No buttons have been defined.");
		cimglist_for(buttons, l) buttons[l].resize(-100, -100, 1, 3);

		unsigned int bw = 0, bh = 0;
		cimglist_for(buttons, l) { bw = std::max(bw, buttons[l]._width); bh = std::max(bh, buttons[l]._height); }
		bw += 8; bh += 8;
		if (bw<64) bw = 64;
		if (bw>128) bw = 128;
		if (bh<24) bh = 24;
		if (bh>48) bh = 48;

		CImg<unsigned char> button(bw, bh, 1, 3);
		button.draw_rectangle(0, 0, bw - 1, bh - 1, gray);
		button.draw_line(0, 0, bw - 1, 0, white).draw_line(0, bh - 1, 0, 0, white);
		button.draw_line(bw - 1, 0, bw - 1, bh - 1, black).draw_line(bw - 1, bh - 1, 0, bh - 1, black);
		button.draw_line(1, bh - 2, bw - 2, bh - 2, gray2).draw_line(bw - 2, bh - 2, bw - 2, 1, gray2);
		CImg<unsigned char> sbutton(bw, bh, 1, 3);
		sbutton.draw_rectangle(0, 0, bw - 1, bh - 1, gray);
		sbutton.draw_line(0, 0, bw - 1, 0, black).draw_line(bw - 1, 0, bw - 1, bh - 1, black);
		sbutton.draw_line(bw - 1, bh - 1, 0, bh - 1, black).draw_line(0, bh - 1, 0, 0, black);
		sbutton.draw_line(1, 1, bw - 2, 1, white).draw_line(1, bh - 2, 1, 1, white);
		sbutton.draw_line(bw - 2, 1, bw - 2, bh - 2, black).draw_line(bw - 2, bh - 2, 1, bh - 2, black);
		sbutton.draw_line(2, bh - 3, bw - 3, bh - 3, gray2).draw_line(bw - 3, bh - 3, bw - 3, 2, gray2);
		sbutton.draw_line(4, 4, bw - 5, 4, black, 1, 0xAAAAAAAA, true).draw_line(bw - 5, 4, bw - 5, bh - 5, black, 1, 0xAAAAAAAA, false);
		sbutton.draw_line(bw - 5, bh - 5, 4, bh - 5, black, 1, 0xAAAAAAAA, false).draw_line(4, bh - 5, 4, 4, black, 1, 0xAAAAAAAA, false);
		CImg<unsigned char> cbutton(bw, bh, 1, 3);
		cbutton.draw_rectangle(0, 0, bw - 1, bh - 1, black).draw_rectangle(1, 1, bw - 2, bh - 2, gray2).
			draw_rectangle(2, 2, bw - 3, bh - 3, gray);
		cbutton.draw_line(4, 4, bw - 5, 4, black, 1, 0xAAAAAAAA, true).draw_line(bw - 5, 4, bw - 5, bh - 5, black, 1, 0xAAAAAAAA, false);
		cbutton.draw_line(bw - 5, bh - 5, 4, bh - 5, black, 1, 0xAAAAAAAA, false).draw_line(4, bh - 5, 4, 4, black, 1, 0xAAAAAAAA, false);

		cimglist_for(buttons, ll) {
			CImg<unsigned char>(cbutton).
				draw_image(1 + (bw - buttons[ll].width()) / 2, 1 + (bh - buttons[ll].height()) / 2, buttons[ll]).
				move_to(cbuttons);
			CImg<unsigned char>(sbutton).
				draw_image((bw - buttons[ll].width()) / 2, (bh - buttons[ll].height()) / 2, buttons[ll]).
				move_to(sbuttons);
			CImg<unsigned char>(button).
				draw_image((bw - buttons[ll].width()) / 2, (bh - buttons[ll].height()) / 2, buttons[ll]).
				move_to(buttons[ll]);
		}

		CImg<unsigned char> canvas;
		if (msg)
			((CImg<unsigned char>().draw_text(0, 0, "%s", gray, 0, 1, 13, msg) *= -1) += 200).resize(-100, -100, 1, 3).move_to(canvas);

		const unsigned int
			bwall = (buttons._width - 1)*(12 + bw) + bw,
			w = cimg::max(196U, 36 + logo._width + canvas._width, 24 + bwall),
			h = cimg::max(96U, 36 + canvas._height + bh, 36 + logo._height + bh),
			lx = 12 + (canvas._data ? 0 : ((w - 24 - logo._width) / 2)),
			ly = (h - 12 - bh - logo._height) / 2,
			tx = lx + logo._width + 12,
			ty = (h - 12 - bh - canvas._height) / 2,
			bx = (w - bwall) / 2,
			by = h - 12 - bh;

		if (canvas._data)
			canvas = CImg<unsigned char>(w, h, 1, 3).
			draw_rectangle(0, 0, w - 1, h - 1, gray).
			draw_line(0, 0, w - 1, 0, white).draw_line(0, h - 1, 0, 0, white).
			draw_line(w - 1, 0, w - 1, h - 1, black).draw_line(w - 1, h - 1, 0, h - 1, black).
			draw_image(tx, ty, canvas);
		else
			canvas = CImg<unsigned char>(w, h, 1, 3).
			draw_rectangle(0, 0, w - 1, h - 1, gray).
			draw_line(0, 0, w - 1, 0, white).draw_line(0, h - 1, 0, 0, white).
			draw_line(w - 1, 0, w - 1, h - 1, black).draw_line(w - 1, h - 1, 0, h - 1, black);
		if (logo._data) canvas.draw_image(lx, ly, logo);

		unsigned int xbuttons[6] = { 0 };
		cimglist_for(buttons, lll) { xbuttons[lll] = bx + (bw + 12)*lll; canvas.draw_image(xbuttons[lll], by, buttons[lll]); }

		// Open window and enter events loop
		CImgDisplay disp(canvas, title ? title : " ", 0, false, is_centered ? true : false);
		if (is_centered) disp.move((CImgDisplay::screen_width() - disp.width()) / 2,
			(CImgDisplay::screen_height() - disp.height()) / 2);
		bool stop_flag = false, refresh = false;
		int oselected = -1, oclicked = -1, selected = -1, clicked = -1;
		while (!disp.is_closed() && !stop_flag) {
			if (refresh) {
				if (clicked >= 0)
					CImg<unsigned char>(canvas).draw_image(xbuttons[clicked], by, cbuttons[clicked]).display(disp);
				else {
					if (selected >= 0)
						CImg<unsigned char>(canvas).draw_image(xbuttons[selected], by, sbuttons[selected]).display(disp);
					else canvas.display(disp);
				}
				refresh = false;
			}
			disp.wait(15);
			if (disp.is_resized()) disp.resize(disp, false);

			if (disp.button() & 1) {
				oclicked = clicked;
				clicked = -1;
				cimglist_for(buttons, l)
					if (disp.mouse_y() >= (int)by && disp.mouse_y()<(int)(by + bh) &&
						disp.mouse_x() >= (int)xbuttons[l] && disp.mouse_x()<(int)(xbuttons[l] + bw)) {
						clicked = selected = l;
						refresh = true;
					}
				if (clicked != oclicked) refresh = true;
			}
			else if (clicked >= 0) stop_flag = true;

			if (disp.key()) {
				oselected = selected;
				switch (disp.key()) {
				case cimg::keyESC: selected = -1; stop_flag = true; break;
				case cimg::keyENTER: if (selected<0) selected = 0; stop_flag = true; break;
				case cimg::keyTAB:
				case cimg::keyARROWRIGHT:
				case cimg::keyARROWDOWN: selected = (selected + 1) % buttons.width(); break;
				case cimg::keyARROWLEFT:
				case cimg::keyARROWUP: selected = (selected + buttons.width() - 1) % buttons.width(); break;
				}
				disp.set_key();
				if (selected != oselected) refresh = true;
			}
		}
		if (!disp) selected = -1;
		return selected;
#endif
	}

	//! Display a simple dialog box, and wait for the user's response \specialization.
	inline int dialog(const char *const title, const char *const msg,
		const char *const button1_label, const char *const button2_label, const char *const button3_label,
		const char *const button4_label, const char *const button5_label, const char *const button6_label,
		const bool is_centered) {
		return dialog(title, msg, button1_label, button2_label, button3_label, button4_label, button5_label, button6_label,
			CImg<unsigned char>::_logo40x38(), is_centered);
	}

	//! Evaluate math expression.
	/**
	\param expression C-string describing the formula to evaluate.
	\param x Value of the pre-defined variable \c x.
	\param y Value of the pre-defined variable \c y.
	\param z Value of the pre-defined variable \c z.
	\param c Value of the pre-defined variable \c c.
	\return Result of the formula evaluation.
	\note Set \c expression to \c 0 to keep evaluating the last specified \c expression.
	\par Example
	\code
	const double
	res1 = cimg::eval("cos(x)^2 + sin(y)^2",2,2),  // will return '1'.
	res2 = cimg::eval(0,1,1);                    // will return '1' too.
	\endcode
	**/
	inline double eval(const char *const expression, const double x, const double y, const double z, const double c) {
		static const CImg<float> empty;
		return empty.eval(expression, x, y, z, c);
	}

	template<typename t>
	inline CImg<typename cimg::superset<double, t>::type> eval(const char *const expression, const CImg<t>& xyzc) {
		static const CImg<float> empty;
		return empty.eval(expression, xyzc);
	}

	// End of cimg:: namespace
}

// End of cimg_library:: namespace
}

//! Short alias name.
namespace cil = cimg_library_suffixed;

#ifdef _cimg_redefine_False
#define False 0
#endif
#ifdef _cimg_redefine_True
#define True 1
#endif
#ifdef _cimg_redefine_min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifdef _cimg_redefine_max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#ifdef _cimg_redefine_PI
#define PI 3.141592653589793238462643383
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // !_CIMG_NAMESPACE
