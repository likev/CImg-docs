
# Files IO in CImg.

The CImg Library can NATIVELY handle the following file formats :

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
