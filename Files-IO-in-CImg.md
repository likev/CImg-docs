
# Files IO in CImg.

### The CImg Library can NATIVELY handle the following file formats :

- RAW : consists in a very simple header (in ascii), then the image data.
- ASC (Ascii)
- HDR (Analyze 7.5)
- INR (Inrimage)
- PPM/PGM (Portable Pixmap)
- BMP (uncompressed)
- PAN (Pandore-5)
- DLM (Matlab ASCII)

**If ImageMagick is installed（or just place ImageMagick's tool 'convert' to the same folder of your c++ file）**, 
The CImg Library can load/save image in formats: JPG, GIF, PNG, TIF,... 

### Source Code Analysis

save("demo.png")-->save_png("demo.png")-->_save_png(0, "demo.png")-->
```c++
const CImg<T>& _save_png(std::FILE *const file, const char *const filename,
	const unsigned int bytes_per_pixel = 0) const 
{
	//....
	#ifndef cimg_use_png
	cimg::unused(bytes_per_pixel);
	if (!file) return save_other(filename);
	#else
	//....
}

const CImg<T>& save_other(const char *const filename, const unsigned int quality = 100) const 
{
	//...
	
	//try use libMagick++
	try { save_magick(filename); }
	catch (CImgException&) {
		//try uses convert, an external executable binary provided by ImageMagick
		try { save_imagemagick_external(filename, quality); }
		catch (CImgException&) {
			//try uses gm, an external executable binary provided by GraphicsMagick
			try { save_graphicsmagick_external(filename, quality); }
			catch (CImgException&) {
				is_saved = false;
			}
		}
	}
	
	//...
}

const CImg<T>& save_imagemagick_external(const char *const filename, 
				const unsigned int quality = 100) const 
{
	//...
#ifdef cimg_use_png
#define _cimg_sie_ext1 "png"
#define _cimg_sie_ext2 "png"
#else
#define _cimg_sie_ext1 "pgm"
#define _cimg_sie_ext2 "ppm"
#endif
	CImg<charT> command(1024), filename_tmp(256);
	std::FILE *file;
	do {
		cimg_snprintf(filename_tmp, filename_tmp._width, "%s%c%s.%s", cimg::temporary_path(),
			cimg_file_separator, cimg::filenamerand(), 
			_spectrum == 1 ? _cimg_sie_ext1 : _cimg_sie_ext2);
		if ((file = std_fopen(filename_tmp, "rb")) != 0) cimg::fclose(file);
	} while (file);
#ifdef cimg_use_png
	save_png(filename_tmp);
#else
	save_pnm(filename_tmp);
#endif
	cimg_snprintf(command, command._width, "%s -quality %u \"%s\" \"%s\"",
		cimg::imagemagick_path(), quality,
		CImg<charT>::string(filename_tmp)._system_strescape().data(),
		CImg<charT>::string(filename)._system_strescape().data());
	cimg::system(command);
	
	//...
}
```
