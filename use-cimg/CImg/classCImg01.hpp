#ifndef _CLASS_CIMG_HPP_01
#define _CLASS_CIMG_HPP_01


#include "classCImg03.h"

namespace cimg_library_suffixed {  
		
		template<typename T>  
		CImg<typename CImg<T>::intT> CImg<T>::_select(CImgDisplay &disp, const char *const title,
			const unsigned int feature_type, unsigned int *const XYZ,
			const int origX, const int origY, const int origZ,
			const bool exit_on_anykey,
			const bool reset_view3d,
			const bool force_display_z_coord) const {
		if (is_empty()) return CImg<intT>(1, feature_type == 0 ? 3 : 6, 1, 1, -1);
		if (!disp) {
			disp.assign(cimg_fitscreen(_width, _height, _depth), title ? title : 0, 1);
			if (!title) disp.set_title("CImg<%s> (%ux%ux%ux%u)", pixel_type(), _width, _height, _depth, _spectrum);
		}
		else if (title) disp.set_title("%s", title);

		CImg<T> thumb;
		if (width()>disp.screen_width() || height()>disp.screen_height())
			get_resize(cimg_fitscreen(width(), height(), 1), 1, -100).move_to(thumb);

		const unsigned int old_normalization = disp.normalization();
		bool old_is_resized = disp.is_resized();
		disp._normalization = 0;
		disp.show().set_key(0).set_wheel().show_mouse();

		static const unsigned char foreground_color[] = { 255,255,255 }, background_color[] = { 0,0,0 };

		int area = 0, starting_area = 0, clicked_area = 0, phase = 0,
			X0 = (int)((XYZ ? XYZ[0] : (_width - 1) / 2) % _width),
			Y0 = (int)((XYZ ? XYZ[1] : (_height - 1) / 2) % _height),
			Z0 = (int)((XYZ ? XYZ[2] : (_depth - 1) / 2) % _depth),
			X1 = -1, Y1 = -1, Z1 = -1,
			X3d = -1, Y3d = -1,
			oX3d = X3d, oY3d = -1,
			omx = -1, omy = -1;
		float X = -1, Y = -1, Z = -1;
		unsigned int old_button = 0, key = 0;

		bool shape_selected = false, text_down = false, visible_cursor = true;
		static CImg<floatT> pose3d;
		static bool is_view3d = false, is_axes = true;
		if (reset_view3d) { pose3d.assign(); is_view3d = false; }
		CImg<floatT> points3d, opacities3d, sel_opacities3d;
		CImgList<uintT> primitives3d, sel_primitives3d;
		CImgList<ucharT> colors3d, sel_colors3d;
		CImg<ucharT> visu, visu0, view3d;
		CImg<charT> text(1024); *text = 0;

		while (!key && !disp.is_closed() && !shape_selected) {

			// Handle mouse motion and selection
			int
				mx = disp.mouse_x(),
				my = disp.mouse_y();

			const float
				mX = mx<0 ? -1.0f : (float)mx*(width() + (depth()>1 ? depth() : 0)) / disp.width(),
				mY = my<0 ? -1.0f : (float)my*(height() + (depth()>1 ? depth() : 0)) / disp.height();

			area = 0;
			if (mX >= 0 && mY >= 0 && mX<width() && mY<height()) { area = 1; X = mX; Y = mY; Z = (float)(phase ? Z1 : Z0); }
			if (mX >= 0 && mX<width() && mY >= height()) { area = 2; X = mX; Z = mY - _height; Y = (float)(phase ? Y1 : Y0); }
			if (mY >= 0 && mX >= width() && mY<height()) { area = 3; Y = mY; Z = mX - _width; X = (float)(phase ? X1 : X0); }
			if (mX >= width() && mY >= height()) area = 4;
			if (disp.button()) { if (!clicked_area) clicked_area = area; }
			else clicked_area = 0;

			CImg<charT> filename(32);

			switch (key = disp.key()) {
#if cimg_OS!=2
			case cimg::keyCTRLRIGHT:
#endif
			case 0: case cimg::keyCTRLLEFT: key = 0; break;
			case cimg::keyPAGEUP:
				if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) { disp.set_wheel(1); key = 0; } break;
			case cimg::keyPAGEDOWN:
				if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) { disp.set_wheel(-1); key = 0; } break;
			case cimg::keyA: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				is_axes = !is_axes; disp.set_key(key, false); key = 0; visu0.assign();
			} break;
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
				disp.set_fullscreen(false).resize(cimg_fitscreen(_width, _height, _depth), false)._is_resized = true;
				disp.set_key(key, false); key = 0; visu0.assign();
			} break;
			case cimg::keyF: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				disp.resize(disp.screen_width(), disp.screen_height(), false).toggle_fullscreen()._is_resized = true;
				disp.set_key(key, false); key = 0; visu0.assign();
			} break;
			case cimg::keyV: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				is_view3d = !is_view3d; disp.set_key(key, false); key = 0; visu0.assign();
			} break;
			case cimg::keyS: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
				static unsigned int snap_number = 0;
				std::FILE *file;
				do {
					cimg_snprintf(filename, filename._width, cimg_appname "_%.4u.bmp", snap_number++);
					if ((file = std_fopen(filename, "r")) != 0) cimg::fclose(file);
				} while (file);
				if (visu0) {
					(+visu0).draw_text(0, 0, " Saving snapshot... ", foreground_color, background_color, 0.7f, 13).display(disp);
					visu0.save(filename);
					(+visu0).draw_text(0, 0, " Snapshot '%s' saved. ", foreground_color, background_color, 0.7f, 13, filename._data).
						display(disp);
				}
				disp.set_key(key, false); key = 0;
			} break;
			case cimg::keyO: if (disp.is_keyCTRLLEFT() || disp.is_keyCTRLRIGHT()) {
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
				(+visu0).draw_text(0, 0, " Saving instance... ", foreground_color, background_color, 0.7f, 13).display(disp);
				save(filename);
				(+visu0).draw_text(0, 0, " Instance '%s' saved. ", foreground_color, background_color, 0.7f, 13, filename._data).
					display(disp);
				disp.set_key(key, false); key = 0;
			} break;
			}

			switch (area) {

			case 0: // When mouse is out of image range.
				mx = my = -1; X = Y = Z = -1;
				break;

			case 1: case 2: case 3: // When mouse is over the XY,XZ or YZ projections.
				if (disp.button() & 1 && phase<2 && clicked_area == area) { // When selection has been started (1st step).
					if (_depth>1 && (X1 != (int)X || Y1 != (int)Y || Z1 != (int)Z)) visu0.assign();
					X1 = (int)X; Y1 = (int)Y; Z1 = (int)Z;
				}
				if (!(disp.button() & 1) && phase >= 2 && clicked_area != area) { // When selection is at 2nd step (for volumes).
					switch (starting_area) {
					case 1: if (Z1 != (int)Z) visu0.assign(); Z1 = (int)Z; break;
					case 2: if (Y1 != (int)Y) visu0.assign(); Y1 = (int)Y; break;
					case 3: if (X1 != (int)X) visu0.assign(); X1 = (int)X; break;
					}
				}
				if (disp.button() & 2 && clicked_area == area) { // When moving through the image/volume.
					if (phase) {
						if (_depth>1 && (X1 != (int)X || Y1 != (int)Y || Z1 != (int)Z)) visu0.assign();
						X1 = (int)X; Y1 = (int)Y; Z1 = (int)Z;
					}
					else {
						if (_depth>1 && (X0 != (int)X || Y0 != (int)Y || Z0 != (int)Z)) visu0.assign();
						X0 = (int)X; Y0 = (int)Y; Z0 = (int)Z;
					}
				}
				if (disp.button() & 4) {
					X = (float)X0; Y = (float)Y0; Z = (float)Z0; phase = area = clicked_area = starting_area = 0;
					visu0.assign();
				}
				if (disp.wheel()) { // When moving through the slices of the volume (with mouse wheel).
					if (_depth>1 && !disp.is_keyCTRLLEFT() && !disp.is_keyCTRLRIGHT() &&
						!disp.is_keySHIFTLEFT() && !disp.is_keySHIFTRIGHT() &&
						!disp.is_keyALT() && !disp.is_keyALTGR()) {
						switch (area) {
						case 1:
							if (phase) Z = (float)(Z1 += disp.wheel()); else Z = (float)(Z0 += disp.wheel());
							visu0.assign(); break;
						case 2:
							if (phase) Y = (float)(Y1 += disp.wheel()); else Y = (float)(Y0 += disp.wheel());
							visu0.assign(); break;
						case 3:
							if (phase) X = (float)(X1 += disp.wheel()); else X = (float)(X0 += disp.wheel());
							visu0.assign(); break;
						}
						disp.set_wheel();
					}
					else key = ~0U;
				}
				if ((disp.button() & 1) != old_button) { // When left button has just been pressed or released.
					switch (phase) {
					case 0:
						if (area == clicked_area) {
							X0 = X1 = (int)X; Y0 = Y1 = (int)Y; Z0 = Z1 = (int)Z; starting_area = area; ++phase;
						} break;
					case 1:
						if (area == starting_area) {
							X1 = (int)X; Y1 = (int)Y; Z1 = (int)Z; ++phase;
						}
						else if (!(disp.button() & 1)) { X = (float)X0; Y = (float)Y0; Z = (float)Z0; phase = 0; visu0.assign(); }
						break;
					case 2: ++phase; break;
					}
					old_button = disp.button() & 1;
				}
				break;

			case 4: // When mouse is over the 3d view.
				if (is_view3d && points3d) {
					X3d = mx - width()*disp.width() / (width() + (depth()>1 ? depth() : 0));
					Y3d = my - height()*disp.height() / (height() + (depth()>1 ? depth() : 0));
					if (oX3d<0) { oX3d = X3d; oY3d = Y3d; }
					// Left + right buttons: reset.
					if ((disp.button() & 3) == 3) { pose3d.assign(); view3d.assign(); oX3d = oY3d = X3d = Y3d = -1; }
					else if (disp.button() & 1 && pose3d && (oX3d != X3d || oY3d != Y3d)) { // Left button: rotate.
						const float
							R = 0.45f*std::min(view3d._width, view3d._height),
							R2 = R*R,
							u0 = (float)(oX3d - view3d.width() / 2),
							v0 = (float)(oY3d - view3d.height() / 2),
							u1 = (float)(X3d - view3d.width() / 2),
							v1 = (float)(Y3d - view3d.height() / 2),
							n0 = cimg::hypot(u0, v0),
							n1 = cimg::hypot(u1, v1),
							nu0 = n0>R ? (u0*R / n0) : u0,
							nv0 = n0>R ? (v0*R / n0) : v0,
							nw0 = (float)std::sqrt(std::max(0.0f, R2 - nu0*nu0 - nv0*nv0)),
							nu1 = n1>R ? (u1*R / n1) : u1,
							nv1 = n1>R ? (v1*R / n1) : v1,
							nw1 = (float)std::sqrt(std::max(0.0f, R2 - nu1*nu1 - nv1*nv1)),
							u = nv0*nw1 - nw0*nv1,
							v = nw0*nu1 - nu0*nw1,
							w = nv0*nu1 - nu0*nv1,
							n = cimg::hypot(u, v, w),
							alpha = (float)std::asin(n / R2) * 180 / cimg::PI;
						pose3d.draw_image(CImg<floatT>::rotation_matrix(u, v, w, -alpha)*pose3d.get_crop(0, 0, 2, 2));
						view3d.assign();
					}
					else if (disp.button() & 2 && pose3d && oY3d != Y3d) {  // Right button: zoom.
						pose3d(3, 2) -= (oY3d - Y3d)*1.5f; view3d.assign();
					}
					if (disp.wheel()) { // Wheel: zoom
						pose3d(3, 2) -= disp.wheel() * 15; view3d.assign(); disp.set_wheel();
					}
					if (disp.button() & 4 && pose3d && (oX3d != X3d || oY3d != Y3d)) { // Middle button: shift.
						pose3d(3, 0) -= oX3d - X3d; pose3d(3, 1) -= oY3d - Y3d; view3d.assign();
					}
					oX3d = X3d; oY3d = Y3d;
				}
				mx = my = -1; X = Y = Z = -1;
				break;
			}

			if (phase) {
				if (!feature_type) shape_selected = phase ? true : false;
				else {
					if (_depth>1) shape_selected = (phase == 3) ? true : false;
					else shape_selected = (phase == 2) ? true : false;
				}
			}

			if (X0<0) X0 = 0;
			if (X0 >= width()) X0 = width() - 1;
			if (Y0<0) Y0 = 0;
			if (Y0 >= height()) Y0 = height() - 1;
			if (Z0<0) Z0 = 0;
			if (Z0 >= depth()) Z0 = depth() - 1;
			if (X1<1) X1 = 0;
			if (X1 >= width()) X1 = width() - 1;
			if (Y1<0) Y1 = 0;
			if (Y1 >= height()) Y1 = height() - 1;
			if (Z1<0) Z1 = 0;
			if (Z1 >= depth()) Z1 = depth() - 1;

			// Draw visualization image on the display
			if (mx != omx || my != omy || !visu0 || (_depth>1 && !view3d)) {

				if (!visu0) { // Create image of projected planes.
					if (thumb) thumb.__get_select(disp, old_normalization, phase ? X1 : X0, phase ? Y1 : Y0, phase ? Z1 : Z0).move_to(visu0);
					else __get_select(disp, old_normalization, phase ? X1 : X0, phase ? Y1 : Y0, phase ? Z1 : Z0).move_to(visu0);
					visu0.resize(disp);
					view3d.assign();
					points3d.assign();
				}

				if (is_view3d && _depth>1 && !view3d) { // Create 3d view for volumetric images.
					const unsigned int
						_x3d = (unsigned int)cimg::round((float)_width*visu0._width / (_width + _depth), 1, 1),
						_y3d = (unsigned int)cimg::round((float)_height*visu0._height / (_height + _depth), 1, 1),
						x3d = _x3d >= visu0._width ? visu0._width - 1 : _x3d,
						y3d = _y3d >= visu0._height ? visu0._height - 1 : _y3d;
					CImg<ucharT>(1, 2, 1, 1, 64, 128).resize(visu0._width - x3d, visu0._height - y3d, 1, visu0._spectrum, 3).
						move_to(view3d);
					if (!points3d) {
						get_projections3d(primitives3d, colors3d, phase ? X1 : X0, phase ? Y1 : Y0, phase ? Z1 : Z0, true).move_to(points3d);
						points3d.append(CImg<floatT>(8, 3, 1, 1,
							0, _width - 1, _width - 1, 0, 0, _width - 1, _width - 1, 0,
							0, 0, _height - 1, _height - 1, 0, 0, _height - 1, _height - 1,
							0, 0, 0, 0, _depth - 1, _depth - 1, _depth - 1, _depth - 1), 'x');
						CImg<uintT>::vector(12, 13).move_to(primitives3d); CImg<uintT>::vector(13, 14).move_to(primitives3d);
						CImg<uintT>::vector(14, 15).move_to(primitives3d); CImg<uintT>::vector(15, 12).move_to(primitives3d);
						CImg<uintT>::vector(16, 17).move_to(primitives3d); CImg<uintT>::vector(17, 18).move_to(primitives3d);
						CImg<uintT>::vector(18, 19).move_to(primitives3d); CImg<uintT>::vector(19, 16).move_to(primitives3d);
						CImg<uintT>::vector(12, 16).move_to(primitives3d); CImg<uintT>::vector(13, 17).move_to(primitives3d);
						CImg<uintT>::vector(14, 18).move_to(primitives3d); CImg<uintT>::vector(15, 19).move_to(primitives3d);
						colors3d.insert(12, CImg<ucharT>::vector(255, 255, 255));
						opacities3d.assign(primitives3d.width(), 1, 1, 1, 0.5f);
						if (!phase) {
							opacities3d[0] = opacities3d[1] = opacities3d[2] = 0.8f;
							sel_primitives3d.assign();
							sel_colors3d.assign();
							sel_opacities3d.assign();
						}
						else {
							if (feature_type == 2) {
								points3d.append(CImg<floatT>(8, 3, 1, 1,
									X0, X1, X1, X0, X0, X1, X1, X0,
									Y0, Y0, Y1, Y1, Y0, Y0, Y1, Y1,
									Z0, Z0, Z0, Z0, Z1, Z1, Z1, Z1), 'x');
								sel_primitives3d.assign();
								CImg<uintT>::vector(20, 21).move_to(sel_primitives3d);
								CImg<uintT>::vector(21, 22).move_to(sel_primitives3d);
								CImg<uintT>::vector(22, 23).move_to(sel_primitives3d);
								CImg<uintT>::vector(23, 20).move_to(sel_primitives3d);
								CImg<uintT>::vector(24, 25).move_to(sel_primitives3d);
								CImg<uintT>::vector(25, 26).move_to(sel_primitives3d);
								CImg<uintT>::vector(26, 27).move_to(sel_primitives3d);
								CImg<uintT>::vector(27, 24).move_to(sel_primitives3d);
								CImg<uintT>::vector(20, 24).move_to(sel_primitives3d);
								CImg<uintT>::vector(21, 25).move_to(sel_primitives3d);
								CImg<uintT>::vector(22, 26).move_to(sel_primitives3d);
								CImg<uintT>::vector(23, 27).move_to(sel_primitives3d);
							}
							else {
								points3d.append(CImg<floatT>(2, 3, 1, 1,
									X0, X1,
									Y0, Y1,
									Z0, Z1), 'x');
								sel_primitives3d.assign(CImg<uintT>::vector(20, 21));
							}
							sel_colors3d.assign(sel_primitives3d._width, CImg<ucharT>::vector(255, 255, 255));
							sel_opacities3d.assign(sel_primitives3d._width, 1, 1, 1, 0.8f);
						}
						points3d.shift_object3d(-0.5f*(_width - 1), -0.5f*(_height - 1), -0.5f*(_depth - 1)).resize_object3d();
						points3d *= 0.75f*std::min(view3d._width, view3d._height);
					}

					if (!pose3d) CImg<floatT>(4, 3, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0).move_to(pose3d);
					CImg<floatT> zbuffer3d(view3d._width, view3d._height, 1, 1, 0);
					const CImg<floatT> rotated_points3d = pose3d.get_crop(0, 0, 2, 2)*points3d;
					if (sel_primitives3d)
						view3d.draw_object3d(pose3d(3, 0) + 0.5f*view3d._width,
							pose3d(3, 1) + 0.5f*view3d._height,
							pose3d(3, 2),
							rotated_points3d, sel_primitives3d, sel_colors3d, sel_opacities3d,
							2, true, 500, 0, 0, 0, 0, 0, zbuffer3d);
					view3d.draw_object3d(pose3d(3, 0) + 0.5f*view3d._width,
						pose3d(3, 1) + 0.5f*view3d._height,
						pose3d(3, 2),
						rotated_points3d, primitives3d, colors3d, opacities3d,
						2, true, 500, 0, 0, 0, 0, 0, zbuffer3d);
					visu0.draw_image(x3d, y3d, view3d);
				}
				visu = visu0;

				if (X<0 || Y<0 || Z<0) { if (!visible_cursor) { disp.show_mouse(); visible_cursor = true; } }
				else {
					if (is_axes) { if (visible_cursor) { disp.hide_mouse(); visible_cursor = false; } }
					else { if (!visible_cursor) { disp.show_mouse(); visible_cursor = true; } }
					const int d = (depth()>1) ? depth() : 0;
					int
						_vX = (int)X, _vY = (int)Y, _vZ = (int)Z,
						w = disp.width(), W = width() + d,
						h = disp.height(), H = height() + d,
						_xp = (int)(_vX*(float)w / W), xp = _xp + ((int)(_xp*(float)W / w) != _vX),
						_yp = (int)(_vY*(float)h / H), yp = _yp + ((int)(_yp*(float)H / h) != _vY),
						_xn = (int)((_vX + 1.0f)*w / W - 1), xn = _xn + ((int)((_xn + 1.0f)*W / w) != _vX + 1),
						_yn = (int)((_vY + 1.0f)*h / H - 1), yn = _yn + ((int)((_yn + 1.0f)*H / h) != _vY + 1),
						_zxp = (int)((_vZ + width())*(float)w / W), zxp = _zxp + ((int)(_zxp*(float)W / w) != _vZ + width()),
						_zyp = (int)((_vZ + height())*(float)h / H), zyp = _zyp + ((int)(_zyp*(float)H / h) != _vZ + height()),
						_zxn = (int)((_vZ + width() + 1.0f)*w / W - 1),
						zxn = _zxn + ((int)((_zxn + 1.0f)*W / w) != _vZ + width() + 1),
						_zyn = (int)((_vZ + height() + 1.0f)*h / H - 1),
						zyn = _zyn + ((int)((_zyn + 1.0f)*H / h) != _vZ + height() + 1),
						_xM = (int)(width()*(float)w / W - 1), xM = _xM + ((int)((_xM + 1.0f)*W / w) != width()),
						_yM = (int)(height()*(float)h / H - 1), yM = _yM + ((int)((_yM + 1.0f)*H / h) != height()),
						xc = (xp + xn) / 2,
						yc = (yp + yn) / 2,
						zxc = (zxp + zxn) / 2,
						zyc = (zyp + zyn) / 2,
						xf = (int)(X*w / W),
						yf = (int)(Y*h / H),
						zxf = (int)((Z + width())*w / W),
						zyf = (int)((Z + height())*h / H);

					if (is_axes) { // Draw axes.
						visu.draw_line(0, yf, visu.width() - 1, yf, foreground_color, 0.7f, 0xFF00FF00).
							draw_line(0, yf, visu.width() - 1, yf, background_color, 0.7f, 0x00FF00FF).
							draw_line(xf, 0, xf, visu.height() - 1, foreground_color, 0.7f, 0xFF00FF00).
							draw_line(xf, 0, xf, visu.height() - 1, background_color, 0.7f, 0x00FF00FF);
						if (_depth>1)
							visu.draw_line(zxf, 0, zxf, yM, foreground_color, 0.7f, 0xFF00FF00).
							draw_line(zxf, 0, zxf, yM, background_color, 0.7f, 0x00FF00FF).
							draw_line(0, zyf, xM, zyf, foreground_color, 0.7f, 0xFF00FF00).
							draw_line(0, zyf, xM, zyf, background_color, 0.7f, 0x00FF00FF);
					}

					// Draw box cursor.
					if (xn - xp >= 4 && yn - yp >= 4) visu.draw_rectangle(xp, yp, xn, yn, foreground_color, 0.2f).
						draw_rectangle(xp, yp, xn, yn, foreground_color, 1, 0xAAAAAAAA).
						draw_rectangle(xp, yp, xn, yn, background_color, 1, 0x55555555);
					if (_depth>1) {
						if (yn - yp >= 4 && zxn - zxp >= 4) visu.draw_rectangle(zxp, yp, zxn, yn, background_color, 0.2f).
							draw_rectangle(zxp, yp, zxn, yn, foreground_color, 1, 0xAAAAAAAA).
							draw_rectangle(zxp, yp, zxn, yn, background_color, 1, 0x55555555);
						if (xn - xp >= 4 && zyn - zyp >= 4) visu.draw_rectangle(xp, zyp, xn, zyn, background_color, 0.2f).
							draw_rectangle(xp, zyp, xn, zyn, foreground_color, 1, 0xAAAAAAAA).
							draw_rectangle(xp, zyp, xn, zyn, background_color, 1, 0x55555555);
					}

					// Draw selection.
					if (phase) {
						const int
							_xp0 = (int)(X0*(float)w / W), xp0 = _xp0 + ((int)(_xp0*(float)W / w) != X0),
							_yp0 = (int)(Y0*(float)h / H), yp0 = _yp0 + ((int)(_yp0*(float)H / h) != Y0),
							_xn0 = (int)((X0 + 1.0f)*w / W - 1), xn0 = _xn0 + ((int)((_xn0 + 1.0f)*W / w) != X0 + 1),
							_yn0 = (int)((Y0 + 1.0f)*h / H - 1), yn0 = _yn0 + ((int)((_yn0 + 1.0f)*H / h) != Y0 + 1),
							_zxp0 = (int)((Z0 + width())*(float)w / W), zxp0 = _zxp0 + ((int)(_zxp0*(float)W / w) != Z0 + width()),
							_zyp0 = (int)((Z0 + height())*(float)h / H), zyp0 = _zyp0 + ((int)(_zyp0*(float)H / h) != Z0 + height()),
							_zxn0 = (int)((Z0 + width() + 1.0f)*w / W - 1),
							zxn0 = _zxn0 + ((int)((_zxn0 + 1.0f)*W / w) != Z0 + width() + 1),
							_zyn0 = (int)((Z0 + height() + 1.0f)*h / H - 1),
							zyn0 = _zyn0 + ((int)((_zyn0 + 1.0f)*H / h) != Z0 + height() + 1),
							xc0 = (xp0 + xn0) / 2,
							yc0 = (yp0 + yn0) / 2,
							zxc0 = (zxp0 + zxn0) / 2,
							zyc0 = (zyp0 + zyn0) / 2;

						switch (feature_type) {
						case 1: {
							visu.draw_arrow(xc0, yc0, xc, yc, background_color, 0.9f, 30, 5, 0x55555555).
								draw_arrow(xc0, yc0, xc, yc, foreground_color, 0.9f, 30, 5, 0xAAAAAAAA);
							if (d) {
								visu.draw_arrow(zxc0, yc0, zxc, yc, background_color, 0.9f, 30, 5, 0x55555555).
									draw_arrow(zxc0, yc0, zxc, yc, foreground_color, 0.9f, 30, 5, 0xAAAAAAAA).
									draw_arrow(xc0, zyc0, xc, zyc, background_color, 0.9f, 30, 5, 0x55555555).
									draw_arrow(xc0, zyc0, xc, zyc, foreground_color, 0.9f, 30, 5, 0xAAAAAAAA);
							}
						} break;
						case 2: {
							visu.draw_rectangle(X0<X1 ? xp0 : xp, Y0<Y1 ? yp0 : yp, X0<X1 ? xn : xn0, Y0<Y1 ? yn : yn0, background_color, 0.2f).
								draw_rectangle(X0<X1 ? xp0 : xp, Y0<Y1 ? yp0 : yp, X0<X1 ? xn : xn0, Y0<Y1 ? yn : yn0, background_color, 0.9f, 0x55555555).
								draw_rectangle(X0<X1 ? xp0 : xp, Y0<Y1 ? yp0 : yp, X0<X1 ? xn : xn0, Y0<Y1 ? yn : yn0, foreground_color, 0.9f, 0xAAAAAAAA).
								draw_arrow(xc0, yc0, xc, yc, background_color, 0.5f, 30, 5, 0x55555555).
								draw_arrow(xc0, yc0, xc, yc, foreground_color, 0.5f, 30, 5, 0xAAAAAAAA);
							if (d) {
								visu.draw_rectangle(Z0<Z1 ? zxp0 : zxp, Y0<Y1 ? yp0 : yp, Z0<Z1 ? zxn : zxn0, Y0<Y1 ? yn : yn0, background_color, 0.2f).
									draw_rectangle(Z0<Z1 ? zxp0 : zxp, Y0<Y1 ? yp0 : yp, Z0<Z1 ? zxn : zxn0, Y0<Y1 ? yn : yn0,
										background_color, 0.9f, 0x55555555).
									draw_rectangle(Z0<Z1 ? zxp0 : zxp, Y0<Y1 ? yp0 : yp, Z0<Z1 ? zxn : zxn0, Y0<Y1 ? yn : yn0,
										foreground_color, 0.9f, 0xAAAAAAAA).
									draw_arrow(zxc0, yc0, zxc, yc, background_color, 0.5f, 30, 5, 0x55555555).
									draw_arrow(zxc0, yc0, zxc, yc, foreground_color, 0.5f, 30, 5, 0xAAAAAAAA).
									draw_rectangle(X0<X1 ? xp0 : xp, Z0<Z1 ? zyp0 : zyp, X0<X1 ? xn : xn0, Z0<Z1 ? zyn : zyn0,
										background_color, 0.2f).
									draw_rectangle(X0<X1 ? xp0 : xp, Z0<Z1 ? zyp0 : zyp, X0<X1 ? xn : xn0, Z0<Z1 ? zyn : zyn0,
										background_color, 0.9f, 0x55555555).
									draw_rectangle(X0<X1 ? xp0 : xp, Z0<Z1 ? zyp0 : zyp, X0<X1 ? xn : xn0, Z0<Z1 ? zyn : zyn0,
										foreground_color, 0.9f, 0xAAAAAAAA).
									draw_arrow(xp0, zyp0, xn, zyn, background_color, 0.5f, 30, 5, 0x55555555).
									draw_arrow(xp0, zyp0, xn, zyn, foreground_color, 0.5f, 30, 5, 0xAAAAAAAA);
							}
						} break;
						case 3: {
							visu.draw_ellipse(xc0, yc0,
								(float)cimg::abs(xc - xc0),
								(float)cimg::abs(yc - yc0), 0, background_color, 0.2f).
								draw_ellipse(xc0, yc0,
								(float)cimg::abs(xc - xc0),
									(float)cimg::abs(yc - yc0), 0, foreground_color, 0.9f, ~0U).
								draw_point(xc0, yc0, foreground_color, 0.9f);
							if (d) {
								visu.draw_ellipse(zxc0, yc0, (float)cimg::abs(zxc - zxc0), (float)cimg::abs(yc - yc0), 0,
									background_color, 0.2f).
									draw_ellipse(zxc0, yc0, (float)cimg::abs(zxc - zxc0), (float)cimg::abs(yc - yc0), 0,
										foreground_color, 0.9f, ~0U).
									draw_point(zxc0, yc0, foreground_color, 0.9f).
									draw_ellipse(xc0, zyc0, (float)cimg::abs(xc - xc0), (float)cimg::abs(zyc - zyc0), 0,
										background_color, 0.2f).
									draw_ellipse(xc0, zyc0, (float)cimg::abs(xc - xc0), (float)cimg::abs(zyc - zyc0), 0,
										foreground_color, 0.9f, ~0U).
									draw_point(xc0, zyc0, foreground_color, 0.9f);
							}
						} break;
						}
					}

					// Draw text info.
					if (my >= 0 && my<13) text_down = true; else if (my >= visu.height() - 13) text_down = false;
					if (!feature_type || !phase) {
						if (X >= 0 && Y >= 0 && Z >= 0 && X<width() && Y<height() && Z<depth()) {
							if (_depth>1 || force_display_z_coord)
								cimg_snprintf(text, text._width, " Point (%d,%d,%d) = [ ", origX + (int)X, origY + (int)Y, origZ + (int)Z);
							else cimg_snprintf(text, text._width, " Point (%d,%d) = [ ", origX + (int)X, origY + (int)Y);
							CImg<T> values = get_vector_at(X, Y, Z);
							const bool is_large_spectrum = values._height>16;
							if (is_large_spectrum)
								values.draw_image(0, 8, values.get_rows(values._height - 8, values._height - 1)).resize(1, 16, 1, 1, 0);
							char *ctext = text._data + std::strlen(text), *const ltext = text._data + 512;
							for (unsigned int c = 0; c<values._height && ctext<ltext; ++c) {
								cimg_snprintf(ctext, 24, cimg::type<T>::format_s(),
									cimg::type<T>::format(values[c]));
								ctext += std::strlen(ctext);
								if (c == 7 && is_large_spectrum) {
									cimg_snprintf(ctext, 24, " (...)");
									ctext += std::strlen(ctext);
								}
								*(ctext++) = ' '; *ctext = 0;
							}
							std::strcpy(text._data + std::strlen(text), "] ");
						}
					}
					else switch (feature_type) {
					case 1: {
						const double dX = (double)(X0 - X1), dY = (double)(Y0 - Y1), dZ = (double)(Z0 - Z1),
							length = cimg::hypot(dX, dY, dZ);
						if (_depth>1 || force_display_z_coord)
							cimg_snprintf(text, text._width, " Vect (%d,%d,%d)-(%d,%d,%d), Length = %g ",
								origX + X0, origY + Y0, origZ + Z0, origX + X1, origY + Y1, origZ + Z1, length);
						else cimg_snprintf(text, text._width, " Vect (%d,%d)-(%d,%d), Length = %g ",
							origX + X0, origY + Y0, origX + X1, origY + Y1, length);
					} break;
					case 2: {
						const double dX = (double)(X0 - X1), dY = (double)(Y0 - Y1), dZ = (double)(Z0 - Z1),
							length = cimg::hypot(dX, dY, dZ);
						if (_depth>1 || force_display_z_coord)
							cimg_snprintf(text, text._width, " Box (%d,%d,%d)-(%d,%d,%d), Size = (%d,%d,%d), Length = %g ",
								origX + (X0<X1 ? X0 : X1), origY + (Y0<Y1 ? Y0 : Y1), origZ + (Z0<Z1 ? Z0 : Z1),
								origX + (X0<X1 ? X1 : X0), origY + (Y0<Y1 ? Y1 : Y0), origZ + (Z0<Z1 ? Z1 : Z0),
								1 + cimg::abs(X0 - X1), 1 + cimg::abs(Y0 - Y1), 1 + cimg::abs(Z0 - Z1), length);
						else cimg_snprintf(text, text._width, " Box (%d,%d)-(%d,%d), Size = (%d,%d), Length = %g ",
							origX + (X0<X1 ? X0 : X1), origY + (Y0<Y1 ? Y0 : Y1),
							origX + (X0<X1 ? X1 : X0), origY + (Y0<Y1 ? Y1 : Y0),
							1 + cimg::abs(X0 - X1), 1 + cimg::abs(Y0 - Y1), length);
					} break;
					default:
						if (_depth>1 || force_display_z_coord)
							cimg_snprintf(text, text._width, " Ellipse (%d,%d,%d)-(%d,%d,%d), Radii = (%d,%d,%d) ",
								origX + X0, origY + Y0, origZ + Z0, origX + X1, origY + Y1, origZ + Z1,
								1 + cimg::abs(X0 - X1), 1 + cimg::abs(Y0 - Y1), 1 + cimg::abs(Z0 - Z1));
						else cimg_snprintf(text, text._width, " Ellipse (%d,%d)-(%d,%d), Radii = (%d,%d) ",
							origX + X0, origY + Y0, origX + X1, origY + Y1,
							1 + cimg::abs(X0 - X1), 1 + cimg::abs(Y0 - Y1));
					}
					if (phase || (mx >= 0 && my >= 0))
						visu.draw_text(0, text_down ? visu.height() - 13 : 0, text, foreground_color, background_color, 0.7f, 13);
				}

				disp.display(visu).wait();
			}
			else if (!shape_selected) disp.wait();
			if (disp.is_resized()) { disp.resize(false)._is_resized = false; old_is_resized = true; visu0.assign(); }
			omx = mx; omy = my;
			if (!exit_on_anykey && key && key != cimg::keyESC &&
				(key != cimg::keyW || (!disp.is_keyCTRLLEFT() && !disp.is_keyCTRLRIGHT()))) {
				key = 0;
			}
		}

		// Return result.
		CImg<intT> res(1, feature_type == 0 ? 3 : 6, 1, 1, -1);
		if (XYZ) { XYZ[0] = (unsigned int)X0; XYZ[1] = (unsigned int)Y0; XYZ[2] = (unsigned int)Z0; }
		if (shape_selected) {
			if (feature_type == 2) {
				if (X0>X1) cimg::swap(X0, X1);
				if (Y0>Y1) cimg::swap(Y0, Y1);
				if (Z0>Z1) cimg::swap(Z0, Z1);
			}
			if (X1<0 || Y1<0 || Z1<0) X0 = Y0 = Z0 = X1 = Y1 = Z1 = -1;
			switch (feature_type) {
			case 1: case 2: res[0] = X0; res[1] = Y0; res[2] = Z0; res[3] = X1; res[4] = Y1; res[5] = Z1; break;
			case 3:
				res[3] = cimg::abs(X1 - X0); res[4] = cimg::abs(Y1 - Y0); res[5] = cimg::abs(Z1 - Z0); // keep no break here!
			default: res[0] = X0; res[1] = Y0; res[2] = Z0;
			}
		}
		if (!exit_on_anykey || !(disp.button() & 4)) disp.set_button();
		if (!visible_cursor) disp.show_mouse();
		disp._normalization = old_normalization;
		disp._is_resized = old_is_resized;
		if (key != ~0U) disp.set_key(key);
		return res;
	}

		
		template<typename T>  
		template<typename tz, typename tp, typename tf, typename tc, typename to>
	CImg<T>& CImg<T>::_draw_object3d(void *const pboard, CImg<tz>& zbuffer,
		const float X, const float Y, const float Z,
		const CImg<tp>& vertices,
		const CImgList<tf>& primitives,
		const CImgList<tc>& colors,
		const to& opacities,
		const unsigned int render_type,
		const bool is_double_sided, const float focale,
		const float lightx, const float lighty, const float lightz,
		const float specular_lightness, const float specular_shininess,
		const float sprite_scale)
	{
		typedef typename cimg::superset2<tp, tz, float>::type tpfloat;
		typedef typename to::value_type _to;
		if (is_empty() || !vertices || !primitives) return *this;
		CImg<char> error_message(1024);
		if (!vertices.is_object3d(primitives, colors, opacities, false, error_message))
			throw CImgArgumentException(_cimg_instance
				"draw_object3d(): Invalid specified 3d object (%u,%u) (%s).",
				cimg_instance, vertices._width, primitives._width, error_message.data());
#ifndef cimg_use_board
		if (pboard) return *this;
#endif
		if (render_type == 5) cimg::mutex(10);  // Static variable used in this case, breaks thread-safety.

		const float
			nspec = 1 - (specular_lightness<0.0f ? 0.0f : (specular_lightness>1.0f ? 1.0f : specular_lightness)),
			nspec2 = 1 + (specular_shininess<0.0f ? 0.0f : specular_shininess),
			nsl1 = (nspec2 - 1) / cimg::sqr(nspec - 1),
			nsl2 = 1 - 2 * nsl1*nspec,
			nsl3 = nspec2 - nsl1 - nsl2;

		// Create light texture for phong-like rendering.
		CImg<floatT> light_texture;
		if (render_type == 5) {
			if (colors._width>primitives._width) {
				static CImg<floatT> default_light_texture;
				static const tc *lptr = 0;
				static tc ref_values[64] = { 0 };
				const CImg<tc>& img = colors.back();
				bool is_same_texture = (lptr == img._data);
				if (is_same_texture)
					for (unsigned int r = 0, j = 0; j<8; ++j)
						for (unsigned int i = 0; i<8; ++i)
							if (ref_values[r++] != img(i*img._width / 9, j*img._height / 9, 0, (i + j) % img._spectrum)) {
								is_same_texture = false; break;
							}
				if (!is_same_texture || default_light_texture._spectrum<_spectrum) {
					(default_light_texture.assign(img, false) /= 255).resize(-100, -100, 1, _spectrum);
					lptr = colors.back().data();
					for (unsigned int r = 0, j = 0; j<8; ++j)
						for (unsigned int i = 0; i<8; ++i)
							ref_values[r++] = img(i*img._width / 9, j*img._height / 9, 0, (i + j) % img._spectrum);
				}
				light_texture.assign(default_light_texture, true);
			}
			else {
				static CImg<floatT> default_light_texture;
				static float olightx = 0, olighty = 0, olightz = 0, ospecular_shininess = 0;
				if (!default_light_texture ||
					lightx != olightx || lighty != olighty || lightz != olightz ||
					specular_shininess != ospecular_shininess || default_light_texture._spectrum<_spectrum) {
					default_light_texture.assign(512, 512);
					const float
						dlx = lightx - X,
						dly = lighty - Y,
						dlz = lightz - Z,
						nl = cimg::hypot(dlx, dly, dlz),
						nlx = (default_light_texture._width - 1) / 2 * (1 + dlx / nl),
						nly = (default_light_texture._height - 1) / 2 * (1 + dly / nl),
						white[] = { 1 };
					default_light_texture.draw_gaussian(nlx, nly, default_light_texture._width / 3.0f, white);
					cimg_forXY(default_light_texture, x, y) {
						const float factor = default_light_texture(x, y);
						if (factor>nspec) default_light_texture(x, y) = std::min(2.0f, nsl1*factor*factor + nsl2*factor + nsl3);
					}
					default_light_texture.resize(-100, -100, 1, _spectrum);
					olightx = lightx; olighty = lighty; olightz = lightz; ospecular_shininess = specular_shininess;
				}
				light_texture.assign(default_light_texture, true);
			}
		}

		// Compute 3d to 2d projection.
		CImg<tpfloat> projections(vertices._width, 2);
		tpfloat parallzmin = cimg::type<tpfloat>::max();
		const float absfocale = focale ? cimg::abs(focale) : 0;
		if (absfocale) {
			cimg_pragma_openmp(parallel for cimg_openmp_if(projections.size()>4096))
				cimg_forX(projections, l) { // Perspective projection
				const tpfloat
					x = (tpfloat)vertices(l, 0),
					y = (tpfloat)vertices(l, 1),
					z = (tpfloat)vertices(l, 2);
				const tpfloat projectedz = z + Z + absfocale;
				projections(l, 1) = Y + absfocale*y / projectedz;
				projections(l, 0) = X + absfocale*x / projectedz;
			}
		}
		else {
			cimg_pragma_openmp(parallel for cimg_openmp_if(projections.size()>4096))
				cimg_forX(projections, l) { // Parallel projection
				const tpfloat
					x = (tpfloat)vertices(l, 0),
					y = (tpfloat)vertices(l, 1),
					z = (tpfloat)vertices(l, 2);
				if (z<parallzmin) parallzmin = z;
				projections(l, 1) = Y + y;
				projections(l, 0) = X + x;
			}
		}

		const float _focale = absfocale ? absfocale : (1e5f - parallzmin);
		float zmax = 0;
		if (zbuffer) zmax = vertices.get_shared_row(2).max();

		// Compute visible primitives.
		CImg<uintT> visibles(primitives._width, 1, 1, 1, ~0U);
		CImg<tpfloat> zrange(primitives._width);
		const tpfloat zmin = absfocale ? (tpfloat)(1.5f - absfocale) : cimg::type<tpfloat>::min();
		bool is_forward = zbuffer ? true : false;

		cimg_pragma_openmp(parallel for cimg_openmp_if(primitives.size()>4096))
			cimglist_for(primitives, l) {
			const CImg<tf>& primitive = primitives[l];
			switch (primitive.size()) {
			case 1: { // Point
				CImg<_to> _opacity;
				__draw_object3d(opacities, l, _opacity);
				if (l <= colors.width() && (colors[l].size() != _spectrum || _opacity)) is_forward = false;
				const unsigned int i0 = (unsigned int)primitive(0);
				const tpfloat z0 = Z + vertices(i0, 2);
				if (z0>zmin) {
					visibles(l) = (unsigned int)l;
					zrange(l) = z0;
				}
			} break;
			case 5: { // Sphere
				const unsigned int
					i0 = (unsigned int)primitive(0),
					i1 = (unsigned int)primitive(1);
				const tpfloat
					Xc = 0.5f*((float)vertices(i0, 0) + (float)vertices(i1, 0)),
					Yc = 0.5f*((float)vertices(i0, 1) + (float)vertices(i1, 1)),
					Zc = 0.5f*((float)vertices(i0, 2) + (float)vertices(i1, 2)),
					_zc = Z + Zc,
					zc = _zc + _focale,
					xc = X + Xc*(absfocale ? absfocale / zc : 1),
					yc = Y + Yc*(absfocale ? absfocale / zc : 1),
					radius = 0.5f*cimg::hypot(vertices(i1, 0) - vertices(i0, 0),
						vertices(i1, 1) - vertices(i0, 1),
						vertices(i1, 2) - vertices(i0, 2))*(absfocale ? absfocale / zc : 1),
					xm = xc - radius,
					ym = yc - radius,
					xM = xc + radius,
					yM = yc + radius;
				if (xM >= 0 && xm<_width && yM >= 0 && ym<_height && _zc>zmin) {
					visibles(l) = (unsigned int)l;
					zrange(l) = _zc;
				}
				is_forward = false;
			} break;
			case 2: // Segment
			case 6: {
				const unsigned int
					i0 = (unsigned int)primitive(0),
					i1 = (unsigned int)primitive(1);
				const tpfloat
					x0 = projections(i0, 0), y0 = projections(i0, 1), z0 = Z + vertices(i0, 2),
					x1 = projections(i1, 0), y1 = projections(i1, 1), z1 = Z + vertices(i1, 2);
				tpfloat xm, xM, ym, yM;
				if (x0<x1) { xm = x0; xM = x1; }
				else { xm = x1; xM = x0; }
				if (y0<y1) { ym = y0; yM = y1; }
				else { ym = y1; yM = y0; }
				if (xM >= 0 && xm<_width && yM >= 0 && ym<_height && z0>zmin && z1>zmin) {
					visibles(l) = (unsigned int)l;
					zrange(l) = (z0 + z1) / 2;
				}
			} break;
			case 3:  // Triangle
			case 9: {
				const unsigned int
					i0 = (unsigned int)primitive(0),
					i1 = (unsigned int)primitive(1),
					i2 = (unsigned int)primitive(2);
				const tpfloat
					x0 = projections(i0, 0), y0 = projections(i0, 1), z0 = Z + vertices(i0, 2),
					x1 = projections(i1, 0), y1 = projections(i1, 1), z1 = Z + vertices(i1, 2),
					x2 = projections(i2, 0), y2 = projections(i2, 1), z2 = Z + vertices(i2, 2);
				tpfloat xm, xM, ym, yM;
				if (x0<x1) { xm = x0; xM = x1; }
				else { xm = x1; xM = x0; }
				if (x2<xm) xm = x2;
				if (x2>xM) xM = x2;
				if (y0<y1) { ym = y0; yM = y1; }
				else { ym = y1; yM = y0; }
				if (y2<ym) ym = y2;
				if (y2>yM) yM = y2;
				if (xM >= 0 && xm<_width && yM >= 0 && ym<_height && z0>zmin && z1>zmin && z2>zmin) {
					const tpfloat d = (x1 - x0)*(y2 - y0) - (x2 - x0)*(y1 - y0);
					if (is_double_sided || d<0) {
						visibles(l) = (unsigned int)l;
						zrange(l) = (z0 + z1 + z2) / 3;
					}
				}
			} break;
			case 4: // Rectangle
			case 12: {
				const unsigned int
					i0 = (unsigned int)primitive(0),
					i1 = (unsigned int)primitive(1),
					i2 = (unsigned int)primitive(2),
					i3 = (unsigned int)primitive(3);
				const tpfloat
					x0 = projections(i0, 0), y0 = projections(i0, 1), z0 = Z + vertices(i0, 2),
					x1 = projections(i1, 0), y1 = projections(i1, 1), z1 = Z + vertices(i1, 2),
					x2 = projections(i2, 0), y2 = projections(i2, 1), z2 = Z + vertices(i2, 2),
					x3 = projections(i3, 0), y3 = projections(i3, 1), z3 = Z + vertices(i3, 2);
				tpfloat xm, xM, ym, yM;
				if (x0<x1) { xm = x0; xM = x1; }
				else { xm = x1; xM = x0; }
				if (x2<xm) xm = x2;
				if (x2>xM) xM = x2;
				if (x3<xm) xm = x3;
				if (x3>xM) xM = x3;
				if (y0<y1) { ym = y0; yM = y1; }
				else { ym = y1; yM = y0; }
				if (y2<ym) ym = y2;
				if (y2>yM) yM = y2;
				if (y3<ym) ym = y3;
				if (y3>yM) yM = y3;
				if (xM >= 0 && xm<_width && yM >= 0 && ym<_height && z0>zmin && z1>zmin && z2>zmin && z3>zmin) {
					const float d = (x1 - x0)*(y2 - y0) - (x2 - x0)*(y1 - y0);
					if (is_double_sided || d<0) {
						visibles(l) = (unsigned int)l;
						zrange(l) = (z0 + z1 + z2 + z3) / 4;
					}
				}
			} break;
			default:
				if (render_type == 5) cimg::mutex(10, 0);
				throw CImgArgumentException(_cimg_instance
					"draw_object3d(): Invalid primitive[%u] with size %u "
					"(should have size 1,2,3,4,5,6,9 or 12).",
					cimg_instance,
					l, primitive.size());
			}
		}

		// Force transparent primitives to be drawn last when zbuffer is activated
		// (and if object contains no spheres or sprites).
		if (is_forward)
			cimglist_for(primitives, l)
			if (___draw_object3d(opacities, l) != 1) zrange(l) = 2 * zmax - zrange(l);

		// Sort only visibles primitives.
		unsigned int *p_visibles = visibles._data;
		tpfloat *p_zrange = zrange._data;
		const tpfloat *ptrz = p_zrange;
		cimg_for(visibles, ptr, unsigned int) {
			if (*ptr != ~0U) { *(p_visibles++) = *ptr; *(p_zrange++) = *ptrz; }
			++ptrz;
		}
		const unsigned int nb_visibles = (unsigned int)(p_zrange - zrange._data);
		if (!nb_visibles) {
			if (render_type == 5) cimg::mutex(10, 0);
			return *this;
		}
		CImg<uintT> permutations;
		CImg<tpfloat>(zrange._data, nb_visibles, 1, 1, 1, true).sort(permutations, is_forward);

		// Compute light properties
		CImg<floatT> lightprops;
		switch (render_type) {
		case 3: { // Flat Shading
			lightprops.assign(nb_visibles);
			cimg_pragma_openmp(parallel for cimg_openmp_if(nb_visibles>4096))
				cimg_forX(lightprops, l) {
				const CImg<tf>& primitive = primitives(visibles(permutations(l)));
				const unsigned int psize = (unsigned int)primitive.size();
				if (psize == 3 || psize == 4 || psize == 9 || psize == 12) {
					const unsigned int
						i0 = (unsigned int)primitive(0),
						i1 = (unsigned int)primitive(1),
						i2 = (unsigned int)primitive(2);
					const tpfloat
						x0 = (tpfloat)vertices(i0, 0), y0 = (tpfloat)vertices(i0, 1), z0 = (tpfloat)vertices(i0, 2),
						x1 = (tpfloat)vertices(i1, 0), y1 = (tpfloat)vertices(i1, 1), z1 = (tpfloat)vertices(i1, 2),
						x2 = (tpfloat)vertices(i2, 0), y2 = (tpfloat)vertices(i2, 1), z2 = (tpfloat)vertices(i2, 2),
						dx1 = x1 - x0, dy1 = y1 - y0, dz1 = z1 - z0,
						dx2 = x2 - x0, dy2 = y2 - y0, dz2 = z2 - z0,
						nx = dy1*dz2 - dz1*dy2,
						ny = dz1*dx2 - dx1*dz2,
						nz = dx1*dy2 - dy1*dx2,
						norm = 1e-5f + cimg::hypot(nx, ny, nz),
						lx = X + (x0 + x1 + x2) / 3 - lightx,
						ly = Y + (y0 + y1 + y2) / 3 - lighty,
						lz = Z + (z0 + z1 + z2) / 3 - lightz,
						nl = 1e-5f + cimg::hypot(lx, ly, lz),
						factor = std::max(cimg::abs(-lx*nx - ly*ny - lz*nz) / (norm*nl), (tpfloat)0);
					lightprops[l] = factor <= nspec ? factor : (nsl1*factor*factor + nsl2*factor + nsl3);
				}
				else lightprops[l] = 1;
			}
		} break;

		case 4: // Gouraud Shading
		case 5: { // Phong-Shading
			CImg<tpfloat> vertices_normals(vertices._width, 6, 1, 1, 0);
			cimg_pragma_openmp(parallel for cimg_openmp_if(nb_visibles>4096))
				for (unsigned int l = 0; l<nb_visibles; ++l) {
					const CImg<tf>& primitive = primitives[visibles(l)];
					const unsigned int psize = (unsigned int)primitive.size();
					const bool
						triangle_flag = (psize == 3) || (psize == 9),
						rectangle_flag = (psize == 4) || (psize == 12);
					if (triangle_flag || rectangle_flag) {
						const unsigned int
							i0 = (unsigned int)primitive(0),
							i1 = (unsigned int)primitive(1),
							i2 = (unsigned int)primitive(2),
							i3 = rectangle_flag ? (unsigned int)primitive(3) : 0;
						const tpfloat
							x0 = (tpfloat)vertices(i0, 0), y0 = (tpfloat)vertices(i0, 1), z0 = (tpfloat)vertices(i0, 2),
							x1 = (tpfloat)vertices(i1, 0), y1 = (tpfloat)vertices(i1, 1), z1 = (tpfloat)vertices(i1, 2),
							x2 = (tpfloat)vertices(i2, 0), y2 = (tpfloat)vertices(i2, 1), z2 = (tpfloat)vertices(i2, 2),
							dx1 = x1 - x0, dy1 = y1 - y0, dz1 = z1 - z0,
							dx2 = x2 - x0, dy2 = y2 - y0, dz2 = z2 - z0,
							nnx = dy1*dz2 - dz1*dy2,
							nny = dz1*dx2 - dx1*dz2,
							nnz = dx1*dy2 - dy1*dx2,
							norm = 1e-5f + cimg::hypot(nnx, nny, nnz),
							nx = nnx / norm,
							ny = nny / norm,
							nz = nnz / norm;
						unsigned int ix = 0, iy = 1, iz = 2;
						if (is_double_sided && nz>0) { ix = 3; iy = 4; iz = 5; }
						vertices_normals(i0, ix) += nx; vertices_normals(i0, iy) += ny; vertices_normals(i0, iz) += nz;
						vertices_normals(i1, ix) += nx; vertices_normals(i1, iy) += ny; vertices_normals(i1, iz) += nz;
						vertices_normals(i2, ix) += nx; vertices_normals(i2, iy) += ny; vertices_normals(i2, iz) += nz;
						if (rectangle_flag) {
							vertices_normals(i3, ix) += nx; vertices_normals(i3, iy) += ny; vertices_normals(i3, iz) += nz;
						}
					}
				}

			if (is_double_sided) cimg_forX(vertices_normals, p) {
				const float
					nx0 = vertices_normals(p, 0), ny0 = vertices_normals(p, 1), nz0 = vertices_normals(p, 2),
					nx1 = vertices_normals(p, 3), ny1 = vertices_normals(p, 4), nz1 = vertices_normals(p, 5),
					n0 = nx0*nx0 + ny0*ny0 + nz0*nz0, n1 = nx1*nx1 + ny1*ny1 + nz1*nz1;
				if (n1>n0) {
					vertices_normals(p, 0) = -nx1;
					vertices_normals(p, 1) = -ny1;
					vertices_normals(p, 2) = -nz1;
				}
			}

			if (render_type == 4) {
				lightprops.assign(vertices._width);
				cimg_pragma_openmp(parallel for cimg_openmp_if(nb_visibles>4096))
					cimg_forX(lightprops, l) {
					const tpfloat
						nx = vertices_normals(l, 0),
						ny = vertices_normals(l, 1),
						nz = vertices_normals(l, 2),
						norm = 1e-5f + cimg::hypot(nx, ny, nz),
						lx = X + vertices(l, 0) - lightx,
						ly = Y + vertices(l, 1) - lighty,
						lz = Z + vertices(l, 2) - lightz,
						nl = 1e-5f + cimg::hypot(lx, ly, lz),
						factor = std::max((-lx*nx - ly*ny - lz*nz) / (norm*nl), (tpfloat)0);
					lightprops[l] = factor <= nspec ? factor : (nsl1*factor*factor + nsl2*factor + nsl3);
				}
			}
			else {
				const unsigned int
					lw2 = light_texture._width / 2 - 1,
					lh2 = light_texture._height / 2 - 1;
				lightprops.assign(vertices._width, 2);
				cimg_pragma_openmp(parallel for cimg_openmp_if(nb_visibles>4096))
					cimg_forX(lightprops, l) {
					const tpfloat
						nx = vertices_normals(l, 0),
						ny = vertices_normals(l, 1),
						nz = vertices_normals(l, 2),
						norm = 1e-5f + cimg::hypot(nx, ny, nz),
						nnx = nx / norm,
						nny = ny / norm;
					lightprops(l, 0) = lw2*(1 + nnx);
					lightprops(l, 1) = lh2*(1 + nny);
				}
			}
		} break;
		}

		// Draw visible primitives
		const CImg<tc> default_color(1, _spectrum, 1, 1, (tc)200);
		CImg<_to> _opacity;

		for (unsigned int l = 0; l<nb_visibles; ++l) {
			const unsigned int n_primitive = visibles(permutations(l));
			const CImg<tf>& primitive = primitives[n_primitive];
			const CImg<tc>
				&__color = n_primitive<colors._width ? colors[n_primitive] : CImg<tc>(),
				_color = (__color && __color.size() != _spectrum && __color._spectrum<_spectrum) ?
				__color.get_resize(-100, -100, -100, _spectrum, 0) : CImg<tc>(),
				&color = _color ? _color : (__color ? __color : default_color);
			const tc *const pcolor = color._data;
			const float opacity = __draw_object3d(opacities, n_primitive, _opacity);

#ifdef cimg_use_board
			LibBoard::Board &board = *(LibBoard::Board*)pboard;
#endif

			switch (primitive.size()) {
			case 1: { // Colored point or sprite
				const unsigned int n0 = (unsigned int)primitive[0];
				const int x0 = (int)projections(n0, 0), y0 = (int)projections(n0, 1);

				if (_opacity.is_empty()) { // Scalar opacity.

					if (color.size() == _spectrum) { // Colored point.
						draw_point(x0, y0, pcolor, opacity);
#ifdef cimg_use_board
						if (pboard) {
							board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
							board.drawDot((float)x0, height() - (float)y0);
						}
#endif
					}
					else { // Sprite.
						const tpfloat z = Z + vertices(n0, 2);
						const float factor = focale<0 ? 1 : sprite_scale*(absfocale ? absfocale / (z + absfocale) : 1);
						const unsigned int
							_sw = (unsigned int)(color._width*factor),
							_sh = (unsigned int)(color._height*factor),
							sw = _sw ? _sw : 1, sh = _sh ? _sh : 1;
						const int nx0 = x0 - (int)sw / 2, ny0 = y0 - (int)sh / 2;
						if (sw <= 3 * _width / 2 && sh <= 3 * _height / 2 &&
							(nx0 + (int)sw / 2 >= 0 || nx0 - (int)sw / 2<width() || ny0 + (int)sh / 2 >= 0 || ny0 - (int)sh / 2<height())) {
							const CImg<tc>
								_sprite = (sw != color._width || sh != color._height) ?
								color.get_resize(sw, sh, 1, -100, render_type <= 3 ? 1 : 3) : CImg<tc>(),
								&sprite = _sprite ? _sprite : color;
							draw_image(nx0, ny0, sprite, opacity);
#ifdef cimg_use_board
							if (pboard) {
								board.setPenColorRGBi(128, 128, 128);
								board.setFillColor(LibBoard::Color::Null);
								board.drawRectangle((float)nx0, height() - (float)ny0, sw, sh);
							}
#endif
						}
					}
				}
				else { // Opacity mask.
					const tpfloat z = Z + vertices(n0, 2);
					const float factor = focale<0 ? 1 : sprite_scale*(absfocale ? absfocale / (z + absfocale) : 1);
					const unsigned int
						_sw = (unsigned int)(std::max(color._width, _opacity._width)*factor),
						_sh = (unsigned int)(std::max(color._height, _opacity._height)*factor),
						sw = _sw ? _sw : 1, sh = _sh ? _sh : 1;
					const int nx0 = x0 - (int)sw / 2, ny0 = y0 - (int)sh / 2;
					if (sw <= 3 * _width / 2 && sh <= 3 * _height / 2 &&
						(nx0 + (int)sw / 2 >= 0 || nx0 - (int)sw / 2<width() || ny0 + (int)sh / 2 >= 0 || ny0 - (int)sh / 2<height())) {
						const CImg<tc>
							_sprite = (sw != color._width || sh != color._height) ?
							color.get_resize(sw, sh, 1, -100, render_type <= 3 ? 1 : 3) : CImg<tc>(),
							&sprite = _sprite ? _sprite : color;
						const CImg<_to>
							_nopacity = (sw != _opacity._width || sh != _opacity._height) ?
							_opacity.get_resize(sw, sh, 1, -100, render_type <= 3 ? 1 : 3) : CImg<_to>(),
							&nopacity = _nopacity ? _nopacity : _opacity;
						draw_image(nx0, ny0, sprite, nopacity);
#ifdef cimg_use_board
						if (pboard) {
							board.setPenColorRGBi(128, 128, 128);
							board.setFillColor(LibBoard::Color::Null);
							board.drawRectangle((float)nx0, height() - (float)ny0, sw, sh);
						}
#endif
					}
				}
			} break;
			case 2: { // Colored line
				const unsigned int
					n0 = (unsigned int)primitive[0],
					n1 = (unsigned int)primitive[1];
				const int
					x0 = (int)projections(n0, 0), y0 = (int)projections(n0, 1),
					x1 = (int)projections(n1, 0), y1 = (int)projections(n1, 1);
				const float
					z0 = vertices(n0, 2) + Z + _focale,
					z1 = vertices(n1, 2) + Z + _focale;
				if (render_type) {
					if (zbuffer) draw_line(zbuffer, x0, y0, z0, x1, y1, z1, pcolor, opacity);
					else draw_line(x0, y0, x1, y1, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.drawLine((float)x0, height() - (float)y0, x1, height() - (float)y1);
					}
#endif
				}
				else {
					draw_point(x0, y0, pcolor, opacity).draw_point(x1, y1, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.drawDot((float)x0, height() - (float)y0);
						board.drawDot((float)x1, height() - (float)y1);
					}
#endif
				}
			} break;
			case 5: { // Colored sphere
				const unsigned int
					n0 = (unsigned int)primitive[0],
					n1 = (unsigned int)primitive[1],
					is_wireframe = (unsigned int)primitive[2];
				const float
					Xc = 0.5f*((float)vertices(n0, 0) + (float)vertices(n1, 0)),
					Yc = 0.5f*((float)vertices(n0, 1) + (float)vertices(n1, 1)),
					Zc = 0.5f*((float)vertices(n0, 2) + (float)vertices(n1, 2)),
					zc = Z + Zc + _focale,
					xc = X + Xc*(absfocale ? absfocale / zc : 1),
					yc = Y + Yc*(absfocale ? absfocale / zc : 1),
					radius = 0.5f*cimg::hypot(vertices(n1, 0) - vertices(n0, 0),
						vertices(n1, 1) - vertices(n0, 1),
						vertices(n1, 2) - vertices(n0, 2))*(absfocale ? absfocale / zc : 1);
				switch (render_type) {
				case 0:
					draw_point((int)xc, (int)yc, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.drawDot(xc, height() - yc);
					}
#endif
					break;
				case 1:
					draw_circle((int)xc, (int)yc, (int)radius, pcolor, opacity, ~0U);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.setFillColor(LibBoard::Color::Null);
						board.drawCircle(xc, height() - yc, radius);
					}
#endif
					break;
				default:
					if (is_wireframe) draw_circle((int)xc, (int)yc, (int)radius, pcolor, opacity, ~0U);
					else draw_circle((int)xc, (int)yc, (int)radius, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						if (!is_wireframe) board.fillCircle(xc, height() - yc, radius);
						else {
							board.setFillColor(LibBoard::Color::Null);
							board.drawCircle(xc, height() - yc, radius);
						}
					}
#endif
					break;
				}
			} break;
			case 6: { // Textured line
				if (!__color) {
					if (render_type == 5) cimg::mutex(10, 0);
					throw CImgArgumentException(_cimg_instance
						"draw_object3d(): Undefined texture for line primitive [%u].",
						cimg_instance, n_primitive);
				}
				const unsigned int
					n0 = (unsigned int)primitive[0],
					n1 = (unsigned int)primitive[1];
				const int
					tx0 = (int)primitive[2], ty0 = (int)primitive[3],
					tx1 = (int)primitive[4], ty1 = (int)primitive[5],
					x0 = (int)projections(n0, 0), y0 = (int)projections(n0, 1),
					x1 = (int)projections(n1, 0), y1 = (int)projections(n1, 1);
				const float
					z0 = vertices(n0, 2) + Z + _focale,
					z1 = vertices(n1, 2) + Z + _focale;
				if (render_type) {
					if (zbuffer) draw_line(zbuffer, x0, y0, z0, x1, y1, z1, color, tx0, ty0, tx1, ty1, opacity);
					else draw_line(x0, y0, x1, y1, color, tx0, ty0, tx1, ty1, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.drawLine((float)x0, height() - (float)y0, (float)x1, height() - (float)y1);
					}
#endif
				}
				else {
					draw_point(x0, y0, color.get_vector_at(tx0 <= 0 ? 0 : tx0 >= color.width() ? color.width() - 1 : tx0,
						ty0 <= 0 ? 0 : ty0 >= color.height() ? color.height() - 1 : ty0)._data, opacity).
						draw_point(x1, y1, color.get_vector_at(tx1 <= 0 ? 0 : tx1 >= color.width() ? color.width() - 1 : tx1,
							ty1 <= 0 ? 0 : ty1 >= color.height() ? color.height() - 1 : ty1)._data, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.drawDot((float)x0, height() - (float)y0);
						board.drawDot((float)x1, height() - (float)y1);
					}
#endif
				}
			} break;
			case 3: { // Colored triangle
				const unsigned int
					n0 = (unsigned int)primitive[0],
					n1 = (unsigned int)primitive[1],
					n2 = (unsigned int)primitive[2];
				const int
					x0 = (int)projections(n0, 0), y0 = (int)projections(n0, 1),
					x1 = (int)projections(n1, 0), y1 = (int)projections(n1, 1),
					x2 = (int)projections(n2, 0), y2 = (int)projections(n2, 1);
				const float
					z0 = vertices(n0, 2) + Z + _focale,
					z1 = vertices(n1, 2) + Z + _focale,
					z2 = vertices(n2, 2) + Z + _focale;
				switch (render_type) {
				case 0:
					draw_point(x0, y0, pcolor, opacity).draw_point(x1, y1, pcolor, opacity).draw_point(x2, y2, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.drawDot((float)x0, height() - (float)y0);
						board.drawDot((float)x1, height() - (float)y1);
						board.drawDot((float)x2, height() - (float)y2);
					}
#endif
					break;
				case 1:
					if (zbuffer)
						draw_line(zbuffer, x0, y0, z0, x1, y1, z1, pcolor, opacity).draw_line(zbuffer, x0, y0, z0, x2, y2, z2, pcolor, opacity).
						draw_line(zbuffer, x1, y1, z1, x2, y2, z2, pcolor, opacity);
					else
						draw_line(x0, y0, x1, y1, pcolor, opacity).draw_line(x0, y0, x2, y2, pcolor, opacity).
						draw_line(x1, y1, x2, y2, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.drawLine((float)x0, height() - (float)y0, (float)x1, height() - (float)y1);
						board.drawLine((float)x0, height() - (float)y0, (float)x2, height() - (float)y2);
						board.drawLine((float)x1, height() - (float)y1, (float)x2, height() - (float)y2);
					}
#endif
					break;
				case 2:
					if (zbuffer) draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, pcolor, opacity);
					else draw_triangle(x0, y0, x1, y1, x2, y2, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x1, height() - (float)y1,
							(float)x2, height() - (float)y2);
					}
#endif
					break;
				case 3:
					if (zbuffer) draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, pcolor, opacity, lightprops(l));
					else _draw_triangle(x0, y0, x1, y1, x2, y2, pcolor, opacity, lightprops(l));
#ifdef cimg_use_board
					if (pboard) {
						const float lp = std::min(lightprops(l), 1);
						board.setPenColorRGBi((unsigned char)(color[0] * lp),
							(unsigned char)(color[1] * lp),
							(unsigned char)(color[2] * lp),
							(unsigned char)(opacity * 255));
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x1, height() - (float)y1,
							(float)x2, height() - (float)y2);
					}
#endif
					break;
				case 4:
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, pcolor,
							lightprops(n0), lightprops(n1), lightprops(n2), opacity);
					else draw_triangle(x0, y0, x1, y1, x2, y2, pcolor, lightprops(n0), lightprops(n1), lightprops(n2), opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi((unsigned char)(color[0]),
							(unsigned char)(color[1]),
							(unsigned char)(color[2]),
							(unsigned char)(opacity * 255));
						board.fillGouraudTriangle((float)x0, height() - (float)y0, lightprops(n0),
							(float)x1, height() - (float)y1, lightprops(n1),
							(float)x2, height() - (float)y2, lightprops(n2));
					}
#endif
					break;
				case 5: {
					const unsigned int
						lx0 = (unsigned int)lightprops(n0, 0), ly0 = (unsigned int)lightprops(n0, 1),
						lx1 = (unsigned int)lightprops(n1, 0), ly1 = (unsigned int)lightprops(n1, 1),
						lx2 = (unsigned int)lightprops(n2, 0), ly2 = (unsigned int)lightprops(n2, 1);
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, pcolor, light_texture, lx0, ly0, lx1, ly1, lx2, ly2, opacity);
					else draw_triangle(x0, y0, x1, y1, x2, y2, pcolor, light_texture, lx0, ly0, lx1, ly1, lx2, ly2, opacity);
#ifdef cimg_use_board
					if (pboard) {
						const float
							l0 = light_texture((int)(light_texture.width() / 2 * (1 + lightprops(n0, 0))),
							(int)(light_texture.height() / 2 * (1 + lightprops(n0, 1)))),
							l1 = light_texture((int)(light_texture.width() / 2 * (1 + lightprops(n1, 0))),
							(int)(light_texture.height() / 2 * (1 + lightprops(n1, 1)))),
							l2 = light_texture((int)(light_texture.width() / 2 * (1 + lightprops(n2, 0))),
							(int)(light_texture.height() / 2 * (1 + lightprops(n2, 1))));
						board.setPenColorRGBi((unsigned char)(color[0]),
							(unsigned char)(color[1]),
							(unsigned char)(color[2]),
							(unsigned char)(opacity * 255));
						board.fillGouraudTriangle((float)x0, height() - (float)y0, l0,
							(float)x1, height() - (float)y1, l1,
							(float)x2, height() - (float)y2, l2);
					}
#endif
				} break;
				}
			} break;
			case 4: { // Colored rectangle
				const unsigned int
					n0 = (unsigned int)primitive[0],
					n1 = (unsigned int)primitive[1],
					n2 = (unsigned int)primitive[2],
					n3 = (unsigned int)primitive[3];
				const int
					x0 = (int)projections(n0, 0), y0 = (int)projections(n0, 1),
					x1 = (int)projections(n1, 0), y1 = (int)projections(n1, 1),
					x2 = (int)projections(n2, 0), y2 = (int)projections(n2, 1),
					x3 = (int)projections(n3, 0), y3 = (int)projections(n3, 1);
				const float
					z0 = vertices(n0, 2) + Z + _focale,
					z1 = vertices(n1, 2) + Z + _focale,
					z2 = vertices(n2, 2) + Z + _focale,
					z3 = vertices(n3, 2) + Z + _focale;

				switch (render_type) {
				case 0:
					draw_point(x0, y0, pcolor, opacity).draw_point(x1, y1, pcolor, opacity).
						draw_point(x2, y2, pcolor, opacity).draw_point(x3, y3, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.drawDot((float)x0, height() - (float)y0);
						board.drawDot((float)x1, height() - (float)y1);
						board.drawDot((float)x2, height() - (float)y2);
						board.drawDot((float)x3, height() - (float)y3);
					}
#endif
					break;
				case 1:
					if (zbuffer)
						draw_line(zbuffer, x0, y0, z0, x1, y1, z1, pcolor, opacity).draw_line(zbuffer, x1, y1, z1, x2, y2, z2, pcolor, opacity).
						draw_line(zbuffer, x2, y2, z2, x3, y3, z3, pcolor, opacity).draw_line(zbuffer, x3, y3, z3, x0, y0, z0, pcolor, opacity);
					else
						draw_line(x0, y0, x1, y1, pcolor, opacity).draw_line(x1, y1, x2, y2, pcolor, opacity).
						draw_line(x2, y2, x3, y3, pcolor, opacity).draw_line(x3, y3, x0, y0, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.drawLine((float)x0, height() - (float)y0, (float)x1, height() - (float)y1);
						board.drawLine((float)x1, height() - (float)y1, (float)x2, height() - (float)y2);
						board.drawLine((float)x2, height() - (float)y2, (float)x3, height() - (float)y3);
						board.drawLine((float)x3, height() - (float)y3, (float)x0, height() - (float)y0);
					}
#endif
					break;
				case 2:
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, pcolor, opacity).
						draw_triangle(zbuffer, x0, y0, z0, x2, y2, z2, x3, y3, z3, pcolor, opacity);
					else
						draw_triangle(x0, y0, x1, y1, x2, y2, pcolor, opacity).draw_triangle(x0, y0, x2, y2, x3, y3, pcolor, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(color[0], color[1], color[2], (unsigned char)(opacity * 255));
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x1, height() - (float)y1,
							(float)x2, height() - (float)y2);
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x2, height() - (float)y2,
							(float)x3, height() - (float)y3);
					}
#endif
					break;
				case 3:
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, pcolor, opacity, lightprops(l)).
						draw_triangle(zbuffer, x0, y0, z0, x2, y2, z2, x3, y3, z3, pcolor, opacity, lightprops(l));
					else
						_draw_triangle(x0, y0, x1, y1, x2, y2, pcolor, opacity, lightprops(l)).
						_draw_triangle(x0, y0, x2, y2, x3, y3, pcolor, opacity, lightprops(l));
#ifdef cimg_use_board
					if (pboard) {
						const float lp = std::min(lightprops(l), 1);
						board.setPenColorRGBi((unsigned char)(color[0] * lp),
							(unsigned char)(color[1] * lp),
							(unsigned char)(color[2] * lp), (unsigned char)(opacity * 255));
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x1, height() - (float)y1,
							(float)x2, height() - (float)y2);
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x2, height() - (float)y2,
							(float)x3, height() - (float)y3);
					}
#endif
					break;
				case 4: {
					const float
						lightprop0 = lightprops(n0), lightprop1 = lightprops(n1),
						lightprop2 = lightprops(n2), lightprop3 = lightprops(n3);
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, pcolor, lightprop0, lightprop1, lightprop2, opacity).
						draw_triangle(zbuffer, x0, y0, z0, x2, y2, z2, x3, y3, z3, pcolor, lightprop0, lightprop2, lightprop3, opacity);
					else
						draw_triangle(x0, y0, x1, y1, x2, y2, pcolor, lightprop0, lightprop1, lightprop2, opacity).
						draw_triangle(x0, y0, x2, y2, x3, y3, pcolor, lightprop0, lightprop2, lightprop3, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi((unsigned char)(color[0]),
							(unsigned char)(color[1]),
							(unsigned char)(color[2]),
							(unsigned char)(opacity * 255));
						board.fillGouraudTriangle((float)x0, height() - (float)y0, lightprop0,
							(float)x1, height() - (float)y1, lightprop1,
							(float)x2, height() - (float)y2, lightprop2);
						board.fillGouraudTriangle((float)x0, height() - (float)y0, lightprop0,
							(float)x2, height() - (float)y2, lightprop2,
							(float)x3, height() - (float)y3, lightprop3);
					}
#endif
				} break;
				case 5: {
					const unsigned int
						lx0 = (unsigned int)lightprops(n0, 0), ly0 = (unsigned int)lightprops(n0, 1),
						lx1 = (unsigned int)lightprops(n1, 0), ly1 = (unsigned int)lightprops(n1, 1),
						lx2 = (unsigned int)lightprops(n2, 0), ly2 = (unsigned int)lightprops(n2, 1),
						lx3 = (unsigned int)lightprops(n3, 0), ly3 = (unsigned int)lightprops(n3, 1);
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, pcolor, light_texture, lx0, ly0, lx1, ly1, lx2, ly2, opacity).
						draw_triangle(zbuffer, x0, y0, z0, x2, y2, z2, x3, y3, z3, pcolor, light_texture, lx0, ly0, lx2, ly2, lx3, ly3, opacity);
					else
						draw_triangle(x0, y0, x1, y1, x2, y2, pcolor, light_texture, lx0, ly0, lx1, ly1, lx2, ly2, opacity).
						draw_triangle(x0, y0, x2, y2, x3, y3, pcolor, light_texture, lx0, ly0, lx2, ly2, lx3, ly3, opacity);
#ifdef cimg_use_board
					if (pboard) {
						const float
							l0 = light_texture((int)(light_texture.width() / 2 * (1 + lx0)), (int)(light_texture.height() / 2 * (1 + ly0))),
							l1 = light_texture((int)(light_texture.width() / 2 * (1 + lx1)), (int)(light_texture.height() / 2 * (1 + ly1))),
							l2 = light_texture((int)(light_texture.width() / 2 * (1 + lx2)), (int)(light_texture.height() / 2 * (1 + ly2))),
							l3 = light_texture((int)(light_texture.width() / 2 * (1 + lx3)), (int)(light_texture.height() / 2 * (1 + ly3)));
						board.setPenColorRGBi((unsigned char)(color[0]),
							(unsigned char)(color[1]),
							(unsigned char)(color[2]),
							(unsigned char)(opacity * 255));
						board.fillGouraudTriangle((float)x0, height() - (float)y0, l0,
							(float)x1, height() - (float)y1, l1,
							(float)x2, height() - (float)y2, l2);
						board.fillGouraudTriangle((float)x0, height() - (float)y0, l0,
							(float)x2, height() - (float)y2, l2,
							(float)x3, height() - (float)y3, l3);
					}
#endif
				} break;
				}
			} break;
			case 9: { // Textured triangle
				if (!__color) {
					if (render_type == 5) cimg::mutex(10, 0);
					throw CImgArgumentException(_cimg_instance
						"draw_object3d(): Undefined texture for triangle primitive [%u].",
						cimg_instance, n_primitive);
				}
				const unsigned int
					n0 = (unsigned int)primitive[0],
					n1 = (unsigned int)primitive[1],
					n2 = (unsigned int)primitive[2];
				const int
					tx0 = (int)primitive[3], ty0 = (int)primitive[4],
					tx1 = (int)primitive[5], ty1 = (int)primitive[6],
					tx2 = (int)primitive[7], ty2 = (int)primitive[8],
					x0 = (int)projections(n0, 0), y0 = (int)projections(n0, 1),
					x1 = (int)projections(n1, 0), y1 = (int)projections(n1, 1),
					x2 = (int)projections(n2, 0), y2 = (int)projections(n2, 1);
				const float
					z0 = vertices(n0, 2) + Z + _focale,
					z1 = vertices(n1, 2) + Z + _focale,
					z2 = vertices(n2, 2) + Z + _focale;
				switch (render_type) {
				case 0:
					draw_point(x0, y0, color.get_vector_at(tx0 <= 0 ? 0 : tx0 >= color.width() ? color.width() - 1 : tx0,
						ty0 <= 0 ? 0 : ty0 >= color.height() ? color.height() - 1 : ty0)._data, opacity).
						draw_point(x1, y1, color.get_vector_at(tx1 <= 0 ? 0 : tx1 >= color.width() ? color.width() - 1 : tx1,
							ty1 <= 0 ? 0 : ty1 >= color.height() ? color.height() - 1 : ty1)._data, opacity).
						draw_point(x2, y2, color.get_vector_at(tx2 <= 0 ? 0 : tx2 >= color.width() ? color.width() - 1 : tx2,
							ty2 <= 0 ? 0 : ty2 >= color.height() ? color.height() - 1 : ty2)._data, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.drawDot((float)x0, height() - (float)y0);
						board.drawDot((float)x1, height() - (float)y1);
						board.drawDot((float)x2, height() - (float)y2);
					}
#endif
					break;
				case 1:
					if (zbuffer)
						draw_line(zbuffer, x0, y0, z0, x1, y1, z1, color, tx0, ty0, tx1, ty1, opacity).
						draw_line(zbuffer, x0, y0, z0, x2, y2, z2, color, tx0, ty0, tx2, ty2, opacity).
						draw_line(zbuffer, x1, y1, z1, x2, y2, z2, color, tx1, ty1, tx2, ty2, opacity);
					else
						draw_line(x0, y0, z0, x1, y1, z1, color, tx0, ty0, tx1, ty1, opacity).
						draw_line(x0, y0, z0, x2, y2, z2, color, tx0, ty0, tx2, ty2, opacity).
						draw_line(x1, y1, z1, x2, y2, z2, color, tx1, ty1, tx2, ty2, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.drawLine((float)x0, height() - (float)y0, (float)x1, height() - (float)y1);
						board.drawLine((float)x0, height() - (float)y0, (float)x2, height() - (float)y2);
						board.drawLine((float)x1, height() - (float)y1, (float)x2, height() - (float)y2);
					}
#endif
					break;
				case 2:
					if (zbuffer) draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, opacity);
					else draw_triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x1, height() - (float)y1,
							(float)x2, height() - (float)y2);
					}
#endif
					break;
				case 3:
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, opacity, lightprops(l));
					else draw_triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, opacity, lightprops(l));
#ifdef cimg_use_board
					if (pboard) {
						const float lp = std::min(lightprops(l), 1);
						board.setPenColorRGBi((unsigned char)(128 * lp),
							(unsigned char)(128 * lp),
							(unsigned char)(128 * lp),
							(unsigned char)(opacity * 255));
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x1, height() - (float)y1,
							(float)x2, height() - (float)y2);
					}
#endif
					break;
				case 4:
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2,
							lightprops(n0), lightprops(n1), lightprops(n2), opacity);
					else
						draw_triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2,
							lightprops(n0), lightprops(n1), lightprops(n2), opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.fillGouraudTriangle((float)x0, height() - (float)y0, lightprops(n0),
							(float)x1, height() - (float)y1, lightprops(n1),
							(float)x2, height() - (float)y2, lightprops(n2));
					}
#endif
					break;
				case 5:
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, light_texture,
						(unsigned int)lightprops(n0, 0), (unsigned int)lightprops(n0, 1),
							(unsigned int)lightprops(n1, 0), (unsigned int)lightprops(n1, 1),
							(unsigned int)lightprops(n2, 0), (unsigned int)lightprops(n2, 1),
							opacity);
					else
						draw_triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, light_texture,
						(unsigned int)lightprops(n0, 0), (unsigned int)lightprops(n0, 1),
							(unsigned int)lightprops(n1, 0), (unsigned int)lightprops(n1, 1),
							(unsigned int)lightprops(n2, 0), (unsigned int)lightprops(n2, 1),
							opacity);
#ifdef cimg_use_board
					if (pboard) {
						const float
							l0 = light_texture((int)(light_texture.width() / 2 * (1 + lightprops(n0, 0))),
							(int)(light_texture.height() / 2 * (1 + lightprops(n0, 1)))),
							l1 = light_texture((int)(light_texture.width() / 2 * (1 + lightprops(n1, 0))),
							(int)(light_texture.height() / 2 * (1 + lightprops(n1, 1)))),
							l2 = light_texture((int)(light_texture.width() / 2 * (1 + lightprops(n2, 0))),
							(int)(light_texture.height() / 2 * (1 + lightprops(n2, 1))));
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.fillGouraudTriangle((float)x0, height() - (float)y0, l0,
							(float)x1, height() - (float)y1, l1,
							(float)x2, height() - (float)y2, l2);
					}
#endif
					break;
				}
			} break;
			case 12: { // Textured quadrangle
				if (!__color) {
					if (render_type == 5) cimg::mutex(10, 0);
					throw CImgArgumentException(_cimg_instance
						"draw_object3d(): Undefined texture for quadrangle primitive [%u].",
						cimg_instance, n_primitive);
				}
				const unsigned int
					n0 = (unsigned int)primitive[0],
					n1 = (unsigned int)primitive[1],
					n2 = (unsigned int)primitive[2],
					n3 = (unsigned int)primitive[3];
				const int
					tx0 = (int)primitive[4], ty0 = (int)primitive[5],
					tx1 = (int)primitive[6], ty1 = (int)primitive[7],
					tx2 = (int)primitive[8], ty2 = (int)primitive[9],
					tx3 = (int)primitive[10], ty3 = (int)primitive[11],
					x0 = (int)projections(n0, 0), y0 = (int)projections(n0, 1),
					x1 = (int)projections(n1, 0), y1 = (int)projections(n1, 1),
					x2 = (int)projections(n2, 0), y2 = (int)projections(n2, 1),
					x3 = (int)projections(n3, 0), y3 = (int)projections(n3, 1);
				const float
					z0 = vertices(n0, 2) + Z + _focale,
					z1 = vertices(n1, 2) + Z + _focale,
					z2 = vertices(n2, 2) + Z + _focale,
					z3 = vertices(n3, 2) + Z + _focale;

				switch (render_type) {
				case 0:
					draw_point(x0, y0, color.get_vector_at(tx0 <= 0 ? 0 : tx0 >= color.width() ? color.width() - 1 : tx0,
						ty0 <= 0 ? 0 : ty0 >= color.height() ? color.height() - 1 : ty0)._data, opacity).
						draw_point(x1, y1, color.get_vector_at(tx1 <= 0 ? 0 : tx1 >= color.width() ? color.width() - 1 : tx1,
							ty1 <= 0 ? 0 : ty1 >= color.height() ? color.height() - 1 : ty1)._data, opacity).
						draw_point(x2, y2, color.get_vector_at(tx2 <= 0 ? 0 : tx2 >= color.width() ? color.width() - 1 : tx2,
							ty2 <= 0 ? 0 : ty2 >= color.height() ? color.height() - 1 : ty2)._data, opacity).
						draw_point(x3, y3, color.get_vector_at(tx3 <= 0 ? 0 : tx3 >= color.width() ? color.width() - 1 : tx3,
							ty3 <= 0 ? 0 : ty3 >= color.height() ? color.height() - 1 : ty3)._data, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.drawDot((float)x0, height() - (float)y0);
						board.drawDot((float)x1, height() - (float)y1);
						board.drawDot((float)x2, height() - (float)y2);
						board.drawDot((float)x3, height() - (float)y3);
					}
#endif
					break;
				case 1:
					if (zbuffer)
						draw_line(zbuffer, x0, y0, z0, x1, y1, z1, color, tx0, ty0, tx1, ty1, opacity).
						draw_line(zbuffer, x1, y1, z1, x2, y2, z2, color, tx1, ty1, tx2, ty2, opacity).
						draw_line(zbuffer, x2, y2, z2, x3, y3, z3, color, tx2, ty2, tx3, ty3, opacity).
						draw_line(zbuffer, x3, y3, z3, x0, y0, z0, color, tx3, ty3, tx0, ty0, opacity);
					else
						draw_line(x0, y0, z0, x1, y1, z1, color, tx0, ty0, tx1, ty1, opacity).
						draw_line(x1, y1, z1, x2, y2, z2, color, tx1, ty1, tx2, ty2, opacity).
						draw_line(x2, y2, z2, x3, y3, z3, color, tx2, ty2, tx3, ty3, opacity).
						draw_line(x3, y3, z3, x0, y0, z0, color, tx3, ty3, tx0, ty0, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.drawLine((float)x0, height() - (float)y0, (float)x1, height() - (float)y1);
						board.drawLine((float)x1, height() - (float)y1, (float)x2, height() - (float)y2);
						board.drawLine((float)x2, height() - (float)y2, (float)x3, height() - (float)y3);
						board.drawLine((float)x3, height() - (float)y3, (float)x0, height() - (float)y0);
					}
#endif
					break;
				case 2:
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, opacity).
						draw_triangle(zbuffer, x0, y0, z0, x2, y2, z2, x3, y3, z3, color, tx0, ty0, tx2, ty2, tx3, ty3, opacity);
					else
						draw_triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, opacity).
						draw_triangle(x0, y0, z0, x2, y2, z2, x3, y3, z3, color, tx0, ty0, tx2, ty2, tx3, ty3, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x1, height() - (float)y1,
							(float)x2, height() - (float)y2);
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x2, height() - (float)y2,
							(float)x3, height() - (float)y3);
					}
#endif
					break;
				case 3:
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, opacity, lightprops(l)).
						draw_triangle(zbuffer, x0, y0, z0, x2, y2, z2, x3, y3, z3, color, tx0, ty0, tx2, ty2, tx3, ty3, opacity, lightprops(l));
					else
						draw_triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2, opacity, lightprops(l)).
						draw_triangle(x0, y0, z0, x2, y2, z2, x3, y3, z3, color, tx0, ty0, tx2, ty2, tx3, ty3, opacity, lightprops(l));
#ifdef cimg_use_board
					if (pboard) {
						const float lp = std::min(lightprops(l), 1);
						board.setPenColorRGBi((unsigned char)(128 * lp),
							(unsigned char)(128 * lp),
							(unsigned char)(128 * lp),
							(unsigned char)(opacity * 255));
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x1, height() - (float)y1,
							(float)x2, height() - (float)y2);
						board.fillTriangle((float)x0, height() - (float)y0,
							(float)x2, height() - (float)y2,
							(float)x3, height() - (float)y3);
					}
#endif
					break;
				case 4: {
					const float
						lightprop0 = lightprops(n0), lightprop1 = lightprops(n1),
						lightprop2 = lightprops(n2), lightprop3 = lightprops(n3);
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2,
							lightprop0, lightprop1, lightprop2, opacity).
						draw_triangle(zbuffer, x0, y0, z0, x2, y2, z2, x3, y3, z3, color, tx0, ty0, tx2, ty2, tx3, ty3,
							lightprop0, lightprop2, lightprop3, opacity);
					else
						draw_triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2,
							lightprop0, lightprop1, lightprop2, opacity).
						draw_triangle(x0, y0, z0, x2, y2, z2, x3, y3, z3, color, tx0, ty0, tx2, ty2, tx3, ty3,
							lightprop0, lightprop2, lightprop3, opacity);
#ifdef cimg_use_board
					if (pboard) {
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.fillGouraudTriangle((float)x0, height() - (float)y0, lightprop0,
							(float)x1, height() - (float)y1, lightprop1,
							(float)x2, height() - (float)y2, lightprop2);
						board.fillGouraudTriangle((float)x0, height() - (float)y0, lightprop0,
							(float)x2, height() - (float)y2, lightprop2,
							(float)x3, height() - (float)y3, lightprop3);
					}
#endif
				} break;
				case 5: {
					const unsigned int
						lx0 = (unsigned int)lightprops(n0, 0), ly0 = (unsigned int)lightprops(n0, 1),
						lx1 = (unsigned int)lightprops(n1, 0), ly1 = (unsigned int)lightprops(n1, 1),
						lx2 = (unsigned int)lightprops(n2, 0), ly2 = (unsigned int)lightprops(n2, 1),
						lx3 = (unsigned int)lightprops(n3, 0), ly3 = (unsigned int)lightprops(n3, 1);
					if (zbuffer)
						draw_triangle(zbuffer, x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2,
							light_texture, lx0, ly0, lx1, ly1, lx2, ly2, opacity).
						draw_triangle(zbuffer, x0, y0, z0, x2, y2, z2, x3, y3, z3, color, tx0, ty0, tx2, ty2, tx3, ty3,
							light_texture, lx0, ly0, lx2, ly2, lx3, ly3, opacity);
					else
						draw_triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color, tx0, ty0, tx1, ty1, tx2, ty2,
							light_texture, lx0, ly0, lx1, ly1, lx2, ly2, opacity).
						draw_triangle(x0, y0, z0, x2, y2, z2, x3, y3, z3, color, tx0, ty0, tx2, ty2, tx3, ty3,
							light_texture, lx0, ly0, lx2, ly2, lx3, ly3, opacity);
#ifdef cimg_use_board
					if (pboard) {
						const float
							l0 = light_texture((int)(light_texture.width() / 2 * (1 + lx0)), (int)(light_texture.height() / 2 * (1 + ly0))),
							l1 = light_texture((int)(light_texture.width() / 2 * (1 + lx1)), (int)(light_texture.height() / 2 * (1 + ly1))),
							l2 = light_texture((int)(light_texture.width() / 2 * (1 + lx2)), (int)(light_texture.height() / 2 * (1 + ly2))),
							l3 = light_texture((int)(light_texture.width() / 2 * (1 + lx3)), (int)(light_texture.height() / 2 * (1 + ly3)));
						board.setPenColorRGBi(128, 128, 128, (unsigned char)(opacity * 255));
						board.fillGouraudTriangle((float)x0, height() - (float)y0, l0,
							(float)x1, height() - (float)y1, l1,
							(float)x2, height() - (float)y2, l2);
						board.fillGouraudTriangle((float)x0, height() - (float)y0, l0,
							(float)x2, height() - (float)y2, l2,
							(float)x3, height() - (float)y3, l3);
					}
#endif
				} break;
				}
			} break;
			}
		}

		if (render_type == 5) cimg::mutex(10, 0);
		return *this;
	}



		template<typename T> template<typename t1, typename t2>
	CImg<typename CImg<T>::intT> CImg<T>::_patchmatch(const CImg<T>& patch_image,
		const unsigned int patch_width,
		const unsigned int patch_height,
		const unsigned int patch_depth,
		const unsigned int nb_iterations,
		const unsigned int nb_randoms,
		const CImg<t1> &guide,
		const bool is_matching_score,
		CImg<t2> &matching_score) const
	{
		if (is_empty()) return CImg<intT>::const_empty();
		if (patch_image._spectrum != _spectrum)
			throw CImgArgumentException(_cimg_instance
				"patchmatch(): Instance image and specified patch image (%u,%u,%u,%u,%p) "
				"have different spectrums.",
				cimg_instance,
				patch_image._width, patch_image._height, patch_image._depth, patch_image._spectrum,
				patch_image._data);
		if (patch_width>_width || patch_height>_height || patch_depth>_depth)
			throw CImgArgumentException(_cimg_instance
				"patchmatch(): Specified patch size %ux%ux%u is bigger than the dimensions "
				"of the instance image.",
				cimg_instance, patch_width, patch_height, patch_depth);
		if (patch_width>patch_image._width || patch_height>patch_image._height || patch_depth>patch_image._depth)
			throw CImgArgumentException(_cimg_instance
				"patchmatch(): Specified patch size %ux%ux%u is bigger than the dimensions "
				"of the patch image image (%u,%u,%u,%u,%p).",
				cimg_instance, patch_width, patch_height, patch_depth,
				patch_image._width, patch_image._height, patch_image._depth, patch_image._spectrum,
				patch_image._data);
		const unsigned int
			_constraint = patch_image._depth>1 ? 3 : 2,
			constraint = guide._spectrum>_constraint ? _constraint : 0;

		if (guide &&
			(guide._width != _width || guide._height != _height || guide._depth != _depth || guide._spectrum<_constraint))
			throw CImgArgumentException(_cimg_instance
				"patchmatch(): Specified guide (%u,%u,%u,%u,%p) has invalid dimensions "
				"considering instance and patch image image (%u,%u,%u,%u,%p).",
				cimg_instance,
				guide._width, guide._height, guide._depth, guide._spectrum, guide._data,
				patch_image._width, patch_image._height, patch_image._depth, patch_image._spectrum,
				patch_image._data);

		CImg<intT> map(_width, _height, _depth, patch_image._depth>1 ? 3 : 2);
		CImg<floatT> score(_width, _height, _depth);
		const int
			psizew = (int)patch_width, psizew1 = psizew / 2, psizew2 = psizew - psizew1 - 1,
			psizeh = (int)patch_height, psizeh1 = psizeh / 2, psizeh2 = psizeh - psizeh1 - 1,
			psized = (int)patch_depth, psized1 = psized / 2, psized2 = psized - psized1 - 1;

		if (_depth>1 || patch_image._depth>1) { // 3d version.

												// Initialize correspondence map.
			if (guide) cimg_forXYZ(*this, x, y, z) { // User-defined initialization.
				const int
					cx1 = x <= psizew1 ? x : (x<width() - psizew2 ? psizew1 : psizew + x - width()), cx2 = psizew - cx1 - 1,
					cy1 = y <= psizeh1 ? y : (y<height() - psizeh2 ? psizeh1 : psizeh + y - height()), cy2 = psizeh - cy1 - 1,
					cz1 = z <= psized1 ? z : (z<depth() - psized2 ? psized1 : psized + z - depth()), cz2 = psized - cz1 - 1,
					u = std::min(std::max((int)guide(x, y, z, 0), cx1), patch_image.width() - 1 - cx2),
					v = std::min(std::max((int)guide(x, y, z, 1), cy1), patch_image.height() - 1 - cy2),
					w = std::min(std::max((int)guide(x, y, z, 2), cz1), patch_image.depth() - 1 - cz2);
				map(x, y, z, 0) = u;
				map(x, y, z, 1) = v;
				map(x, y, z, 2) = w;
				score(x, y, z) = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
					x - cx1, y - cy1, z - cz1,
					u - cx1, v - cy1, w - cz1, cimg::type<float>::inf());
			}
			else cimg_forXYZ(*this, x, y, z) { // Random initialization.
				const int
					cx1 = x <= psizew1 ? x : (x<width() - psizew2 ? psizew1 : psizew + x - width()), cx2 = psizew - cx1 - 1,
					cy1 = y <= psizeh1 ? y : (y<height() - psizeh2 ? psizeh1 : psizeh + y - height()), cy2 = psizeh - cy1 - 1,
					cz1 = z <= psized1 ? z : (z<depth() - psized2 ? psized1 : psized + z - depth()), cz2 = psized - cz1 - 1,
					u = (int)cimg::round(cimg::rand(cx1, patch_image.width() - 1 - cx2)),
					v = (int)cimg::round(cimg::rand(cy1, patch_image.height() - 1 - cy2)),
					w = (int)cimg::round(cimg::rand(cz1, patch_image.depth() - 1 - cz2));
				map(x, y, z, 0) = u;
				map(x, y, z, 1) = v;
				map(x, y, z, 2) = w;
				score(x, y, z) = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
					x - cx1, y - cy1, z - cz1,
					u - cx1, v - cy1, w - cz1, cimg::type<float>::inf());
			}

			// Start iteration loop.
			for (unsigned int iter = 0; iter<nb_iterations; ++iter) {
				cimg_abort_test();
				const bool is_even = !(iter % 2);

				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(_width>64 && iter<nb_iterations - 2))
					cimg_forXYZ(*this, X, Y, Z) {
					const int
						x = is_even ? X : width() - 1 - X,
						y = is_even ? Y : height() - 1 - Y,
						z = is_even ? Z : depth() - 1 - Z;
					if (score(x, y, z) <= 1e-5 || (constraint && guide(x, y, z, constraint) != 0)) continue;
					const int
						cx1 = x <= psizew1 ? x : (x<width() - psizew2 ? psizew1 : psizew + x - width()), cx2 = psizew - cx1 - 1,
						cy1 = y <= psizeh1 ? y : (y<height() - psizeh2 ? psizeh1 : psizeh + y - height()), cy2 = psizeh - cy1 - 1,
						cz1 = z <= psized1 ? z : (z<depth() - psized2 ? psized1 : psized + z - depth()), cz2 = psized - cz1 - 1,
						xp = x - cx1,
						yp = y - cy1,
						zp = z - cz1;

					// Propagation.
					if (is_even) {
						if (x>0) { // Compare with left neighbor.
							const int u = map(x - 1, y, z, 0), v = map(x - 1, y, z, 1), w = map(x - 1, y, z, 2);
							if (u >= cx1 - 1 && u<patch_image.width() - 1 - cx2 &&
								v >= cy1 && v<patch_image.height() - cy2 &&
								w >= cz1 && w<patch_image.depth() - cz2) {
								const float
									current_score = score(x, y, z),
									D = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
										xp, yp, zp, u + 1 - cx1, v - cy1, w - cz1, current_score);
								if (D<current_score) { score(x, y, z) = D; map(x, y, z, 0) = u + 1; map(x, y, z, 1) = v; map(x, y, z, 2) = w; }
							}
						}
						if (y>0) { // Compare with up neighbor.
							const int u = map(x, y - 1, z, 0), v = map(x, y - 1, z, 1), w = map(x, y - 1, z, 2);
							if (u >= cx1 && u<patch_image.width() - cx2 &&
								v >= cy1 - 1 && v<patch_image.height() - 1 - cy2 &&
								w >= cz1 && w<patch_image.depth() - cx2) {
								const float
									current_score = score(x, y, z),
									D = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
										xp, yp, zp, u - cx1, v + 1 - cy1, w - cz1, current_score);
								if (D<current_score) { score(x, y, z) = D; map(x, y, z, 0) = u; map(x, y, z, 1) = v + 1; map(x, y, z, 2) = w; }
							}
						}
						if (z>0) { // Compare with backward neighbor.
							const int u = map(x, y, z - 1, 0), v = map(x, y, z - 1, 1), w = map(x, y, z - 1, 2);
							if (u >= cx1 && u<patch_image.width() - cx2 &&
								v >= cy1 && v<patch_image.height() - cy2 &&
								w >= cz1 - 1 && w<patch_image.depth() - 1 - cz2) {
								const float
									current_score = score(x, y, z),
									D = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
										xp, yp, zp, u - cx1, v - cy1, w + 1 - cz1, current_score);
								if (D<current_score) { score(x, y, z) = D; map(x, y, z, 0) = u; map(x, y, z, 1) = v; map(x, y, z, 2) = w + 1; }
							}
						}
					}
					else {
						if (x<width() - 1) { // Compare with right neighbor.
							const int u = map(x + 1, y, z, 0), v = map(x + 1, y, z, 1), w = map(x + 1, y, z, 2);
							if (u >= cx1 + 1 && u<patch_image.width() + 1 - cx2 &&
								v >= cy1 && v<patch_image.height() - cy2 &&
								w >= cz1 && w<patch_image.depth() - cz2) {
								const float
									current_score = score(x, y, z),
									D = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
										xp, yp, zp, u - 1 - cx1, v - cy1, w - cz1, current_score);
								if (D<current_score) { score(x, y, z) = D; map(x, y, z, 0) = u - 1; map(x, y, z, 1) = v; map(x, y, z, 2) = w; }
							}
						}
						if (y<height() - 1) { // Compare with bottom neighbor.
							const int u = map(x, y + 1, z, 0), v = map(x, y + 1, z, 1), w = map(x, y + 1, z, 2);
							if (u >= cx1 && u<patch_image.width() - cx2 &&
								v >= cy1 + 1 && v<patch_image.height() + 1 - cy2 &&
								w >= cz1 && w<patch_image.depth() - cz2) {
								const float
									current_score = score(x, y, z),
									D = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
										xp, yp, zp, u - cx1, v - 1 - cy1, w - cz1, current_score);
								if (D<current_score) { score(x, y, z) = D; map(x, y, z, 0) = u; map(x, y, z, 1) = v - 1; map(x, y, z, 2) = w; }
							}
						}
						if (z<depth() - 1) { // Compare with forward neighbor.
							const int u = map(x, y, z + 1, 0), v = map(x, y, z + 1, 1), w = map(x, y, z + 1, 2);
							if (u >= cx1 && u<patch_image.width() - cx2 &&
								v >= cy1 && v<patch_image.height() - cy2 &&
								w >= cz1 + 1 && w<patch_image.depth() + 1 - cz2) {
								const float
									current_score = score(x, y, z),
									D = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
										xp, yp, zp, u - cx1, v - cy1, w - 1 - cz1, current_score);
								if (D<current_score) { score(x, y, z) = D; map(x, y, z, 0) = u; map(x, y, z, 1) = v; map(x, y, z, 2) = w - 1; }
							}
						}
					}

					// Randomization.
					const int u = map(x, y, z, 0), v = map(x, y, z, 1), w = map(x, y, z, 2);
					float dw = (float)patch_image.width(), dh = (float)patch_image.height(), dd = (float)patch_image.depth();
					for (unsigned int i = 0; i<nb_randoms; ++i) {
						const int
							ui = (int)cimg::round(cimg::rand(std::max((float)cx1, u - dw),
								std::min(patch_image.width() - 1.0f - cx2, u + dw))),
							vi = (int)cimg::round(cimg::rand(std::max((float)cy1, v - dh),
								std::min(patch_image.height() - 1.0f - cy2, v + dh))),
							wi = (int)cimg::round(cimg::rand(std::max((float)cz1, w - dd),
								std::min(patch_image.depth() - 1.0f - cz2, w + dd)));
						if (ui != u || vi != v || wi != w) {
							const float
								current_score = score(x, y, z),
								D = _patchmatch(*this, patch_image, patch_width, patch_height, patch_depth,
									xp, yp, zp, ui - cx1, vi - cy1, wi - cz1, current_score);
							if (D<current_score) { score(x, y, z) = D; map(x, y, z, 0) = ui; map(x, y, z, 1) = vi; map(x, y, z, 2) = wi; }
							dw = std::max(5.0f, dw*0.5f); dh = std::max(5.0f, dh*0.5f); dd = std::max(5.0f, dd*0.5f);
						}
					}
				}
			}

		}
		else { // 2d version.

			   // Initialize correspondence map.
			if (guide) cimg_forXY(*this, x, y) { // Random initialization.
				const int
					cx1 = x <= psizew1 ? x : (x<width() - psizew2 ? psizew1 : psizew + x - width()), cx2 = psizew - cx1 - 1,
					cy1 = y <= psizeh1 ? y : (y<height() - psizeh2 ? psizeh1 : psizeh + y - height()), cy2 = psizeh - cy1 - 1,
					u = std::min(std::max((int)guide(x, y, 0), cx1), patch_image.width() - 1 - cx2),
					v = std::min(std::max((int)guide(x, y, 1), cy1), patch_image.height() - 1 - cy2);
				map(x, y, 0) = u;
				map(x, y, 1) = v;
				score(x, y) = _patchmatch(*this, patch_image, patch_width, patch_height,
					x - cx1, y - cy1, u - cx1, v - cy1, cimg::type<float>::inf());
			}
			else cimg_forXY(*this, x, y) { // Random initialization.
				const int
					cx1 = x <= psizew1 ? x : (x<width() - psizew2 ? psizew1 : psizew + x - width()), cx2 = psizew - cx1 - 1,
					cy1 = y <= psizeh1 ? y : (y<height() - psizeh2 ? psizeh1 : psizeh + y - height()), cy2 = psizeh - cy1 - 1,
					u = (int)cimg::round(cimg::rand(cx1, patch_image.width() - 1 - cx2)),
					v = (int)cimg::round(cimg::rand(cy1, patch_image.height() - 1 - cy2));
				map(x, y, 0) = u;
				map(x, y, 1) = v;
				score(x, y) = _patchmatch(*this, patch_image, patch_width, patch_height,
					x - cx1, y - cy1, u - cx1, v - cy1, cimg::type<float>::inf());
			}

			// Start iteration loop.
			for (unsigned int iter = 0; iter<nb_iterations; ++iter) {
				const bool is_even = !(iter % 2);

				cimg_pragma_openmp(parallel for cimg_openmp_if(_width>64 && iter<nb_iterations - 2))
					cimg_forXY(*this, X, Y) {
					const int
						x = is_even ? X : width() - 1 - X,
						y = is_even ? Y : height() - 1 - Y;
					if (score(x, y) <= 1e-5 || (constraint && guide(x, y, constraint) != 0)) continue;
					const int
						cx1 = x <= psizew1 ? x : (x<width() - psizew2 ? psizew1 : psizew + x - width()), cx2 = psizew - cx1 - 1,
						cy1 = y <= psizeh1 ? y : (y<height() - psizeh2 ? psizeh1 : psizeh + y - height()), cy2 = psizeh - cy1 - 1,
						xp = x - cx1,
						yp = y - cy1;

					// Propagation.
					if (is_even) {
						if (x>0) { // Compare with left neighbor.
							const int u = map(x - 1, y, 0), v = map(x - 1, y, 1);
							if (u >= cx1 - 1 && u<patch_image.width() - 1 - cx2 &&
								v >= cy1 && v<patch_image.height() - cy2) {
								const float
									current_score = score(x, y),
									D = _patchmatch(*this, patch_image, patch_width, patch_height,
										xp, yp, u + 1 - cx1, v - cy1, current_score);
								if (D<current_score) { score(x, y) = D; map(x, y, 0) = u + 1; map(x, y, 1) = v; }
							}
						}
						if (y>0) { // Compare with up neighbor.
							const int u = map(x, y - 1, 0), v = map(x, y - 1, 1);
							if (u >= cx1 && u<patch_image.width() - cx2 &&
								v >= cy1 - 1 && v<patch_image.height() - 1 - cy2) {
								const float
									current_score = score(x, y),
									D = _patchmatch(*this, patch_image, patch_width, patch_height,
										xp, yp, u - cx1, v + 1 - cy1, current_score);
								if (D<current_score) { score(x, y) = D; map(x, y, 0) = u; map(x, y, 1) = v + 1; }
							}
						}
					}
					else {
						if (x<width() - 1) { // Compare with right neighbor.
							const int u = map(x + 1, y, 0), v = map(x + 1, y, 1);
							if (u >= cx1 + 1 && u<patch_image.width() + 1 - cx2 &&
								v >= cy1 && v<patch_image.height() - cy2) {
								const float
									current_score = score(x, y),
									D = _patchmatch(*this, patch_image, patch_width, patch_height,
										xp, yp, u - 1 - cx1, v - cy1, current_score);
								if (D<current_score) { score(x, y) = D; map(x, y, 0) = u - 1; map(x, y, 1) = v; }
							}
						}
						if (y<height() - 1) { // Compare with bottom neighbor.
							const int u = map(x, y + 1, 0), v = map(x, y + 1, 1);
							if (u >= cx1 && u<patch_image.width() - cx2 &&
								v >= cy1 + 1 && v<patch_image.height() + 1 - cy2) {
								const float
									current_score = score(x, y),
									D = _patchmatch(*this, patch_image, patch_width, patch_height,
										xp, yp, u - cx1, v - 1 - cy1, current_score);
								if (D<current_score) { score(x, y) = D; map(x, y, 0) = u; map(x, y, 1) = v - 1; }
							}
						}
					}

					// Randomization.
					const int u = map(x, y, 0), v = map(x, y, 1);
					float dw = (float)patch_image.width(), dh = (float)patch_image.height();
					for (unsigned int i = 0; i<nb_randoms; ++i) {
						const int
							ui = (int)cimg::round(cimg::rand(std::max((float)cx1, u - dw),
								std::min(patch_image.width() - 1.0f - cx2, u + dw))),
							vi = (int)cimg::round(cimg::rand(std::max((float)cy1, v - dh),
								std::min(patch_image.height() - 1.0f - cy2, v + dh)));
						if (ui != u || vi != v) {
							const float
								current_score = score(x, y),
								D = _patchmatch(*this, patch_image, patch_width, patch_height,
									xp, yp, ui - cx1, vi - cy1, current_score);
							if (D<current_score) { score(x, y) = D; map(x, y, 0) = ui; map(x, y, 1) = vi; }
							dw = std::max(5.0f, dw*0.5f); dh = std::max(5.0f, dh*0.5f);
						}
					}
				}
			}
		}
		if (is_matching_score) score.move_to(matching_score);
		return map;
	}

		// Compilation procedure.
		template<typename T>
	unsigned int CImg<T>::_cimg_math_parser::compile(char *ss, char *se, const unsigned int depth, unsigned int *const p_ref) {
		if (depth>256) {
			cimg::strellipsize(expr, 64);
			throw CImgArgumentException("[" cimg_appname "_math_parser] "
				"CImg<%s>::%s: Call stack overflow (infinite recursion?), "
				"in expression '%s%s%s'.",
				pixel_type(), _cimg_mp_calling_function,
				(ss - 4)>expr._data ? "..." : "",
				(ss - 4)>expr._data ? ss - 4 : expr._data,
				se<&expr.back() ? "..." : "");
		}
		char c1, c2, c3, c4;

		// Simplify expression when possible.
		do {
			c2 = 0;
			if (ss<se) {
				while (*ss && ((signed char)*ss <= ' ' || *ss == ';')) ++ss;
				while (se>ss && ((signed char)(c1 = *(se - 1)) <= ' ' || c1 == ';')) --se;
			}
			while (*ss == '(' && *(se - 1) == ')' && std::strchr(ss, ')') == se - 1) {
				++ss; --se; c2 = 1;
			}
		} while (c2 && ss<se);

		if (se <= ss || !*ss) {
			cimg::strellipsize(expr, 64);
			throw CImgArgumentException("[" cimg_appname "_math_parser] "
				"CImg<%s>::%s: %s%s Missing %s, in expression '%s%s%s'.",
				pixel_type(), _cimg_mp_calling_function, s_op, *s_op ? ":" : "",
				*s_op == 'F' ? "argument" : "item",
				(ss_op - 4)>expr._data ? "..." : "",
				(ss_op - 4)>expr._data ? ss_op - 4 : expr._data,
				ss_op + std::strlen(ss_op)<&expr.back() ? "..." : "");
		}

		const char *const previous_s_op = s_op, *const previous_ss_op = ss_op;
		const unsigned int depth1 = depth + 1;
		unsigned int pos, p1, p2, p3, arg1, arg2, arg3, arg4, arg5, arg6;
		char
			*const se1 = se - 1, *const se2 = se - 2, *const se3 = se - 3,
			*const ss1 = ss + 1, *const ss2 = ss + 2, *const ss3 = ss + 3, *const ss4 = ss + 4,
			*const ss5 = ss + 5, *const ss6 = ss + 6, *const ss7 = ss + 7, *const ss8 = ss + 8,
			*s, *ps, *ns, *s0, *s1, *s2, *s3, sep = 0, end = 0;
		double val, val1, val2;
		mp_func op;

		// 'p_ref' is a 'unsigned int[7]' used to return a reference to an image or vector value
		// linked to the returned memory slot (reference that cannot be determined at compile time).
		// p_ref[0] can be { 0 = scalar (unlinked) | 1 = vector value | 2 = image value (offset) |
		//                   3 = image value (coordinates) | 4 = image value as a vector (offsets) |
		//                   5 = image value as a vector (coordinates) }.
		// Depending on p_ref[0], the remaining p_ref[k] have the following meaning:
		// When p_ref[0]==0, p_ref is actually unlinked.
		// When p_ref[0]==1, p_ref = [ 1, vector_ind, offset ].
		// When p_ref[0]==2, p_ref = [ 2, image_ind (or ~0U), is_relative, offset ].
		// When p_ref[0]==3, p_ref = [ 3, image_ind (or ~0U), is_relative, x, y, z, c ].
		// When p_ref[0]==4, p_ref = [ 4, image_ind (or ~0U), is_relative, offset ].
		// When p_ref[0]==5, p_ref = [ 5, image_ind (or ~0U), is_relative, x, y, z ].
		if (p_ref) { *p_ref = 0; p_ref[1] = p_ref[2] = p_ref[3] = p_ref[4] = p_ref[5] = p_ref[6] = ~0U; }

		const char saved_char = *se; *se = 0;
		const unsigned int clevel = level[ss - expr._data], clevel1 = clevel + 1;
		bool is_sth, is_relative;
		CImg<uintT> ref;
		CImgList<ulongT> _opcode;
		CImg<charT> variable_name;

		// Look for a single value or a pre-defined variable.
		int nb = cimg_sscanf(ss, "%lf%c%c", &val, &(sep = 0), &(end = 0));

#if cimg_OS==2
		// Check for +/-NaN and +/-inf as Microsoft's sscanf() version is not able
		// to read those particular values.
		if (!nb && (*ss == '+' || *ss == '-' || *ss == 'i' || *ss == 'I' || *ss == 'n' || *ss == 'N')) {
			is_sth = true;
			s = ss;
			if (*s == '+') ++s; else if (*s == '-') { ++s; is_sth = false; }
			if (!cimg::strcasecmp(s, "inf")) { val = cimg::type<double>::inf(); nb = 1; }
			else if (!cimg::strcasecmp(s, "nan")) { val = cimg::type<double>::nan(); nb = 1; }
			if (nb == 1 && !is_sth) val = -val;
		}
#endif
		if (nb == 1) _cimg_mp_constant(val);
		if (nb == 2 && sep == '%') _cimg_mp_constant(val / 100);

		if (ss1 == se) switch (*ss) { // One-char reserved variable
		case 'c': _cimg_mp_return(reserved_label['c'] != ~0U ? reserved_label['c'] : _cimg_mp_slot_c);
		case 'd': _cimg_mp_return(reserved_label['d'] != ~0U ? reserved_label['d'] : 20);
		case 'e': _cimg_mp_return(reserved_label['e'] != ~0U ? reserved_label['e'] : 27);
		case 'h': _cimg_mp_return(reserved_label['h'] != ~0U ? reserved_label['h'] : 19);
		case 'l': _cimg_mp_return(reserved_label['l'] != ~0U ? reserved_label['l'] : 26);
		case 'r': _cimg_mp_return(reserved_label['r'] != ~0U ? reserved_label['r'] : 22);
		case 's': _cimg_mp_return(reserved_label['s'] != ~0U ? reserved_label['s'] : 21);
		case 't': _cimg_mp_return(reserved_label['t'] != ~0U ? reserved_label['t'] : 17);
		case 'w': _cimg_mp_return(reserved_label['w'] != ~0U ? reserved_label['w'] : 18);
		case 'x': _cimg_mp_return(reserved_label['x'] != ~0U ? reserved_label['x'] : _cimg_mp_slot_x);
		case 'y': _cimg_mp_return(reserved_label['y'] != ~0U ? reserved_label['y'] : _cimg_mp_slot_y);
		case 'z': _cimg_mp_return(reserved_label['z'] != ~0U ? reserved_label['z'] : _cimg_mp_slot_z);
		case 'u':
			if (reserved_label['u'] != ~0U) _cimg_mp_return(reserved_label['u']);
			_cimg_mp_scalar2(mp_u, 0, 1);
		case 'g':
			if (reserved_label['g'] != ~0U) _cimg_mp_return(reserved_label['g']);
			_cimg_mp_scalar0(mp_g);
		case 'i':
			if (reserved_label['i'] != ~0U) _cimg_mp_return(reserved_label['i']);
			_cimg_mp_scalar0(mp_i);
		case 'I':
			_cimg_mp_op("Variable 'I'");
			if (reserved_label['I'] != ~0U) _cimg_mp_return(reserved_label['I']);
			_cimg_mp_check_vector0(imgin._spectrum);
			need_input_copy = true;
			pos = vector(imgin._spectrum);
			CImg<ulongT>::vector((ulongT)mp_Joff, pos, 0, 0, imgin._spectrum).move_to(code);
			_cimg_mp_return(pos);
		case 'R':
			if (reserved_label['R'] != ~0U) _cimg_mp_return(reserved_label['R']);
			need_input_copy = true;
			_cimg_mp_scalar6(mp_ixyzc, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, 0, 0, 0);
		case 'G':
			if (reserved_label['G'] != ~0U) _cimg_mp_return(reserved_label['G']);
			need_input_copy = true;
			_cimg_mp_scalar6(mp_ixyzc, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, 1, 0, 0);
		case 'B':
			if (reserved_label['B'] != ~0U) _cimg_mp_return(reserved_label['B']);
			need_input_copy = true;
			_cimg_mp_scalar6(mp_ixyzc, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, 2, 0, 0);
		case 'A':
			if (reserved_label['A'] != ~0U) _cimg_mp_return(reserved_label['A']);
			need_input_copy = true;
			_cimg_mp_scalar6(mp_ixyzc, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, 3, 0, 0);
		}
		else if (ss2 == se) { // Two-chars reserved variable
			arg1 = arg2 = ~0U;
			if (*ss == 'w' && *ss1 == 'h') // wh
				_cimg_mp_return(reserved_label[0] != ~0U ? reserved_label[0] : 23);
			if (*ss == 'p' && *ss1 == 'i') // pi
				_cimg_mp_return(reserved_label[3] != ~0U ? reserved_label[3] : 28);
			if (*ss == 'i') {
				if (*ss1 >= '0' && *ss1 <= '9') { // i0...i9
					pos = 19 + *ss1 - '0';
					if (reserved_label[pos] != ~0U) _cimg_mp_return(reserved_label[pos]);
					need_input_copy = true;
					_cimg_mp_scalar6(mp_ixyzc, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, pos - 19, 0, 0);
				}
				switch (*ss1) {
				case 'm': arg1 = 4; arg2 = 0; break; // im
				case 'M': arg1 = 5; arg2 = 1; break; // iM
				case 'a': arg1 = 6; arg2 = 2; break; // ia
				case 'v': arg1 = 7; arg2 = 3; break; // iv
				case 's': arg1 = 8; arg2 = 12; break; // is
				case 'p': arg1 = 9; arg2 = 13; break; // ip
				case 'c': // ic
					if (reserved_label[10] != ~0U) _cimg_mp_return(reserved_label[10]);
					if (mem_img_median == ~0U) mem_img_median = imgin ? constant(imgin.median()) : 0;
					_cimg_mp_return(mem_img_median);
					break;
				}
			}
			else if (*ss1 == 'm') switch (*ss) {
			case 'x': arg1 = 11; arg2 = 4; break; // xm
			case 'y': arg1 = 12; arg2 = 5; break; // ym
			case 'z': arg1 = 13; arg2 = 6; break; // zm
			case 'c': arg1 = 14; arg2 = 7; break; // cm
			}
			else if (*ss1 == 'M') switch (*ss) {
			case 'x': arg1 = 15; arg2 = 8; break; // xM
			case 'y': arg1 = 16; arg2 = 9; break; // yM
			case 'z': arg1 = 17; arg2 = 10; break; // zM
			case 'c': arg1 = 18; arg2 = 11; break; // cM
			}
			if (arg1 != ~0U) {
				if (reserved_label[arg1] != ~0U) _cimg_mp_return(reserved_label[arg1]);
				if (!img_stats) {
					img_stats.assign(1, 14, 1, 1, 0).fill(imgin.get_stats(), false);
					mem_img_stats.assign(1, 14, 1, 1, ~0U);
				}
				if (mem_img_stats[arg2] == ~0U) mem_img_stats[arg2] = constant(img_stats[arg2]);
				_cimg_mp_return(mem_img_stats[arg2]);
			}
		}
		else if (ss3 == se) { // Three-chars reserved variable
			if (*ss == 'w' && *ss1 == 'h' && *ss2 == 'd') // whd
				_cimg_mp_return(reserved_label[1] != ~0U ? reserved_label[1] : 24);
		}
		else if (ss4 == se) { // Four-chars reserved variable
			if (*ss == 'w' && *ss1 == 'h' && *ss2 == 'd' && *ss3 == 's') // whds
				_cimg_mp_return(reserved_label[2] != ~0U ? reserved_label[2] : 25);
		}

		pos = ~0U;
		is_sth = false;
		for (s0 = ss, s = ss1; s<se1; ++s)
			if (*s == ';' && level[s - expr._data] == clevel) { // Separator ';'
				arg1 = code_end._width;
				arg2 = compile(s0, s++, depth, 0);
				if (code_end._width == arg1) pos = arg2; // makes 'end()' return void
				is_sth = true;
				while (*s && ((signed char)*s <= ' ' || *s == ';')) ++s;
				s0 = s;
			}
		if (is_sth) {
			arg1 = code_end._width;
			arg2 = compile(s0, se, depth, p_ref);
			if (code_end._width == arg1) pos = arg2; // makes 'end()' return void
			_cimg_mp_return(pos);
		}

		// Declare / assign variable, vector value or image value.
		for (s = ss1, ps = ss, ns = ss2; s<se1; ++s, ++ps, ++ns)
			if (*s == '=' && *ns != '=' && *ps != '=' && *ps != '>' && *ps != '<' && *ps != '!' &&
				*ps != '+' && *ps != '-' && *ps != '*' && *ps != '/' && *ps != '%' &&
				*ps != '>' && *ps != '<' && *ps != '&' && *ps != '|' && *ps != '^' &&
				level[s - expr._data] == clevel) {
				variable_name.assign(ss, (unsigned int)(s + 1 - ss)).back() = 0;
				cimg::strpare(variable_name, false, true);
				const unsigned int l_variable_name = (unsigned int)std::strlen(variable_name);
				char *const ve1 = ss + l_variable_name - 1;
				_cimg_mp_op("Operator '='");

				// Assign image value (direct).
				if (l_variable_name>2 && (*ss == 'i' || *ss == 'j' || *ss == 'I' || *ss == 'J') && (*ss1 == '(' || *ss1 == '[') &&
					(reserved_label[*ss] == ~0U || *ss1 == '(' || !_cimg_mp_is_vector(reserved_label[*ss]))) {
					is_relative = *ss == 'j' || *ss == 'J';

					if (*ss1 == '[' && *ve1 == ']') { // i/j/I/J[_#ind,offset] = value
						is_parallelizable = false;
						if (*ss2 == '#') { // Index specified
							s0 = ss3; while (s0<ve1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
							p1 = compile(ss3, s0++, depth1, 0);
							_cimg_mp_check_list(true);
						}
						else { p1 = ~0U; s0 = ss2; }
						arg1 = compile(s0, ve1, depth1, 0); // Offset
						_cimg_mp_check_type(arg1, 0, 1, 0);
						arg2 = compile(s + 1, se, depth1, 0); // Value to assign
						if (_cimg_mp_is_vector(arg2)) {
							p2 = ~0U; // 'p2' must be the dimension of the vector-valued operand if any
							if (p1 == ~0U) p2 = imgin._spectrum;
							else if (_cimg_mp_is_constant(p1)) {
								p3 = (unsigned int)cimg::mod((int)mem[p1], listin.width());
								p2 = listin[p3]._spectrum;
							}
							_cimg_mp_check_vector0(p2);
						}
						else p2 = 0;
						_cimg_mp_check_type(arg2, 2, *ss >= 'i' ? 1 : 3, p2);

						if (p_ref) {
							*p_ref = _cimg_mp_is_vector(arg2) ? 4 : 2;
							p_ref[1] = p1;
							p_ref[2] = (unsigned int)is_relative;
							p_ref[3] = arg1;
							if (_cimg_mp_is_vector(arg2))
								set_variable_vector(arg2); // Prevent from being used in further optimization
							else if (_cimg_mp_is_comp(arg2)) memtype[arg2] = -2;
							if (p1 != ~0U && _cimg_mp_is_comp(p1)) memtype[p1] = -2;
							if (_cimg_mp_is_comp(arg1)) memtype[arg1] = -2;
						}


						if (p1 != ~0U) {
							if (!listout) _cimg_mp_return(arg2);
							if (*ss >= 'i')
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_joff : mp_list_set_ioff),
									arg2, p1, arg1).move_to(code);
							else if (_cimg_mp_is_scalar(arg2))
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Joff_s : mp_list_set_Ioff_s),
									arg2, p1, arg1).move_to(code);
							else
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Joff_v : mp_list_set_Ioff_v),
									arg2, p1, arg1, _cimg_mp_vector_size(arg2)).move_to(code);
						}
						else {
							if (!imgout) _cimg_mp_return(arg2);
							if (*ss >= 'i')
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_joff : mp_set_ioff),
									arg2, arg1).move_to(code);
							else if (_cimg_mp_is_scalar(arg2))
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Joff_s : mp_set_Ioff_s),
									arg2, arg1).move_to(code);
							else
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Joff_v : mp_set_Ioff_v),
									arg2, arg1, _cimg_mp_vector_size(arg2)).move_to(code);
						}
						_cimg_mp_return(arg2);
					}

					if (*ss1 == '(' && *ve1 == ')') { // i/j/I/J(_#ind,_x,_y,_z,_c) = value
						is_parallelizable = false;
						if (*ss2 == '#') { // Index specified
							s0 = ss3; while (s0<ve1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
							p1 = compile(ss3, s0++, depth1, 0);
							_cimg_mp_check_list(true);
						}
						else { p1 = ~0U; s0 = ss2; }
						arg1 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_x;
						arg2 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_y;
						arg3 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_z;
						arg4 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_c;
						arg5 = compile(s + 1, se, depth1, 0); // Value to assign
						if (s0<ve1) { // X or [ X,_Y,_Z,_C ]
							s1 = s0; while (s1<ve1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
							arg1 = compile(s0, s1, depth1, 0);
							if (_cimg_mp_is_vector(arg1)) { // Coordinates specified as a vector
								p2 = _cimg_mp_vector_size(arg1); // Vector size
								++arg1;
								if (p2>1) {
									arg2 = arg1 + 1;
									if (p2>2) {
										arg3 = arg2 + 1;
										if (p2>3) arg4 = arg3 + 1;
									}
								}
							}
							else if (s1<ve1) { // Y
								s2 = ++s1; while (s2<ve1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
								arg2 = compile(s1, s2, depth1, 0);
								if (s2<ve1) { // Z
									s3 = ++s2; while (s3<ve1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
									arg3 = compile(s2, s3, depth1, 0);
									if (s3<ve1) arg4 = compile(++s3, ve1, depth1, 0); // C
								}
							}
						}

						if (_cimg_mp_is_vector(arg5)) {
							p2 = ~0U; // 'p2' must be the dimension of the vector-valued operand if any
							if (p1 == ~0U) p2 = imgin._spectrum;
							else if (_cimg_mp_is_constant(p1)) {
								p3 = (unsigned int)cimg::mod((int)mem[p1], listin.width());
								p2 = listin[p3]._spectrum;
							}
							_cimg_mp_check_vector0(p2);
						}
						else p2 = 0;
						_cimg_mp_check_type(arg5, 2, *ss >= 'i' ? 1 : 3, p2);

						if (p_ref) {
							*p_ref = _cimg_mp_is_vector(arg5) ? 5 : 3;
							p_ref[1] = p1;
							p_ref[2] = (unsigned int)is_relative;
							p_ref[3] = arg1;
							p_ref[4] = arg2;
							p_ref[5] = arg3;
							p_ref[6] = arg4;
							if (_cimg_mp_is_vector(arg5))
								set_variable_vector(arg5); // Prevent from being used in further optimization
							else if (_cimg_mp_is_comp(arg5)) memtype[arg5] = -2;
							if (p1 != ~0U && _cimg_mp_is_comp(p1)) memtype[p1] = -2;
							if (_cimg_mp_is_comp(arg1)) memtype[arg1] = -2;
							if (_cimg_mp_is_comp(arg2)) memtype[arg2] = -2;
							if (_cimg_mp_is_comp(arg3)) memtype[arg3] = -2;
							if (_cimg_mp_is_comp(arg4)) memtype[arg4] = -2;
						}
						if (p1 != ~0U) {
							if (!listout) _cimg_mp_return(arg5);
							if (*ss >= 'i')
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_jxyzc : mp_list_set_ixyzc),
									arg5, p1, arg1, arg2, arg3, arg4).move_to(code);
							else if (_cimg_mp_is_scalar(arg5))
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Jxyz_s : mp_list_set_Ixyz_s),
									arg5, p1, arg1, arg2, arg3).move_to(code);
							else
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Jxyz_v : mp_list_set_Ixyz_v),
									arg5, p1, arg1, arg2, arg3, _cimg_mp_vector_size(arg5)).move_to(code);
						}
						else {
							if (!imgout) _cimg_mp_return(arg5);
							if (*ss >= 'i')
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_jxyzc : mp_set_ixyzc),
									arg5, arg1, arg2, arg3, arg4).move_to(code);
							else if (_cimg_mp_is_scalar(arg5))
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Jxyz_s : mp_set_Ixyz_s),
									arg5, arg1, arg2, arg3).move_to(code);
							else
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Jxyz_v : mp_set_Ixyz_v),
									arg5, arg1, arg2, arg3, _cimg_mp_vector_size(arg5)).move_to(code);
						}
						_cimg_mp_return(arg5);
					}
				}

				// Assign vector value (direct).
				if (l_variable_name>3 && *ve1 == ']' && *ss != '[') {
					s0 = ve1; while (s0>ss && (*s0 != '[' || level[s0 - expr._data] != clevel)) --s0;
					is_sth = true; // is_valid_variable_name?
					if (*ss >= '0' && *ss <= '9') is_sth = false;
					else for (ns = ss; ns<s0; ++ns)
						if (!is_varchar(*ns)) { is_sth = false; break; }
					if (is_sth && s0>ss) {
						variable_name[s0 - ss] = 0; // Remove brackets in variable name
						arg1 = ~0U; // Vector slot
						arg2 = compile(++s0, ve1, depth1, 0); // Index
						arg3 = compile(s + 1, se, depth1, 0); // Value to assign
						_cimg_mp_check_type(arg3, 2, 1, 0);

						if (variable_name[1]) { // Multi-char variable
							cimglist_for(variable_def, i) if (!std::strcmp(variable_name, variable_def[i])) {
								arg1 = variable_pos[i]; break;
							}
						}
						else arg1 = reserved_label[*variable_name]; // Single-char variable
						if (arg1 == ~0U) compile(ss, s0 - 1, depth1, 0); // Variable does not exist -> error
						else { // Variable already exists
							if (_cimg_mp_is_scalar(arg1)) compile(ss, s, depth1, 0); // Variable is not a vector -> error
							if (_cimg_mp_is_constant(arg2)) { // Constant index -> return corresponding variable slot directly
								nb = (int)mem[arg2];
								if (nb >= 0 && nb<(int)_cimg_mp_vector_size(arg1)) {
									arg1 += nb + 1;
									CImg<ulongT>::vector((ulongT)mp_copy, arg1, arg3).move_to(code);
									_cimg_mp_return(arg1);
								}
								compile(ss, s, depth1, 0); // Out-of-bounds reference -> error
							}

							// Case of non-constant index -> return assigned value + linked reference
							if (p_ref) {
								*p_ref = 1;
								p_ref[1] = arg1;
								p_ref[2] = arg2;
								if (_cimg_mp_is_comp(arg3)) memtype[arg3] = -2; // Prevent from being used in further optimization
								if (_cimg_mp_is_comp(arg2)) memtype[arg2] = -2;
							}
							CImg<ulongT>::vector((ulongT)mp_vector_set_off, arg3, arg1, (ulongT)_cimg_mp_vector_size(arg1),
								arg2, arg3).
								move_to(code);
							_cimg_mp_return(arg3);
						}
					}
				}

				// Assign user-defined macro.
				if (l_variable_name>2 && *ve1 == ')' && *ss != '(') {
					s0 = ve1; while (s0>ss && *s0 != '(') --s0;
					is_sth = std::strncmp(variable_name, "debug(", 6) &&
						std::strncmp(variable_name, "print(", 6); // is_valid_function_name?
					if (*ss >= '0' && *ss <= '9') is_sth = false;
					else for (ns = ss; ns<s0; ++ns)
						if (!is_varchar(*ns)) { is_sth = false; break; }

					if (is_sth && s0>ss) { // Looks like a valid function declaration
						s0 = variable_name._data + (s0 - ss);
						*s0 = 0;
						s1 = variable_name._data + l_variable_name - 1; // Pointer to closing parenthesis
						CImg<charT>(variable_name._data, (unsigned int)(s0 - variable_name._data + 1)).move_to(macro_def, 0);
						++s; while (*s && (signed char)*s <= ' ') ++s;
						CImg<charT>(s, (unsigned int)(se - s + 1)).move_to(macro_body, 0);

						p1 = 1; // Indice of current parsed argument
						for (s = s0 + 1; s <= s1; ++p1, s = ns + 1) { // Parse function arguments
							if (p1>24) {
								*se = saved_char;
								cimg::strellipsize(variable_name, 64);
								s0 = ss - 4>expr._data ? ss - 4 : expr._data;
								cimg::strellipsize(s0, 64);
								throw CImgArgumentException("[" cimg_appname "_math_parser] "
									"CImg<%s>::%s: %s: Too much specified arguments (>24) in macro "
									"definition '%s()', in expression '%s%s%s'.",
									pixel_type(), _cimg_mp_calling_function, s_op,
									variable_name._data,
									s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
							}
							while (*s && (signed char)*s <= ' ') ++s;
							if (*s == ')' && p1 == 1) break; // Function has no arguments

							s2 = s; // Start of the argument name
							is_sth = true; // is_valid_argument_name?
							if (*s >= '0' && *s <= '9') is_sth = false;
							else for (ns = s; ns<s1 && *ns != ',' && (signed char)*ns>' '; ++ns)
								if (!is_varchar(*ns)) { is_sth = false; break; }
							s3 = ns; // End of the argument name
							while (*ns && (signed char)*ns <= ' ') ++ns;
							if (!is_sth || s2 == s3 || (*ns != ',' && ns != s1)) {
								*se = saved_char;
								cimg::strellipsize(variable_name, 64);
								s0 = ss - 4>expr._data ? ss - 4 : expr._data;
								cimg::strellipsize(s0, 64);
								throw CImgArgumentException("[" cimg_appname "_math_parser] "
									"CImg<%s>::%s: %s: %s name specified for argument %u when defining "
									"macro '%s()', in expression '%s%s%s'.",
									pixel_type(), _cimg_mp_calling_function, s_op,
									is_sth ? "Empty" : "Invalid", p1,
									variable_name._data,
									s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
							}
							if (ns == s1 || *ns == ',') { // New argument found
								*s3 = 0;
								p2 = (unsigned int)(s3 - s2); // Argument length
								for (ps = std::strstr(macro_body[0], s2); ps; ps = std::strstr(ps, s2)) { // Replace by arg number
									if (!((ps>macro_body[0]._data && is_varchar(*(ps - 1))) ||
										(ps + p2<macro_body[0].end() && is_varchar(*(ps + p2))))) {
										if (ps>macro_body[0]._data && *(ps - 1) == '#') { // Remove pre-number sign
											*(ps - 1) = (char)p1;
											if (ps + p2<macro_body[0].end() && *(ps + p2) == '#') { // Has pre & post number signs
												std::memmove(ps, ps + p2 + 1, macro_body[0].end() - ps - p2 - 1);
												macro_body[0]._width -= p2 + 1;
											}
											else { // Has pre number sign only
												std::memmove(ps, ps + p2, macro_body[0].end() - ps - p2);
												macro_body[0]._width -= p2;
											}
										}
										else if (ps + p2<macro_body[0].end() && *(ps + p2) == '#') { // Remove post-number sign
											*(ps++) = (char)p1;
											std::memmove(ps, ps + p2, macro_body[0].end() - ps - p2);
											macro_body[0]._width -= p2;
										}
										else { // Not near a number sign
											if (p2<3) {
												ps -= (ulongT)macro_body[0]._data;
												macro_body[0].resize(macro_body[0]._width - p2 + 3, 1, 1, 1, 0);
												ps += (ulongT)macro_body[0]._data;
											}
											else macro_body[0]._width -= p2 - 3;
											std::memmove(ps + 3, ps + p2, macro_body[0].end() - ps - 3);
											*(ps++) = '(';
											*(ps++) = (char)p1;
											*(ps++) = ')';
										}
									}
									else ++ps;
								}
							}
						}

						// Store number of arguments.
						macro_def[0].resize(macro_def[0]._width + 1, 1, 1, 1, 0).back() = (char)(p1 - 1);

						// Detect parts of function body inside a string.
						is_inside_string(macro_body[0]).move_to(macro_body_is_string, 0);
						_cimg_mp_return_nan();
					}
				}

				// Check if the variable name could be valid. If not, this is probably an lvalue assignment.
				is_sth = true; // is_valid_variable_name?
				const bool is_const = l_variable_name>6 && !std::strncmp(variable_name, "const ", 6);

				s0 = variable_name._data;
				if (is_const) {
					s0 += 6; while ((signed char)*s0 <= ' ') ++s0;
					variable_name.resize(variable_name.end() - s0, 1, 1, 1, 0, 0, 1);
				}

				if (*variable_name >= '0' && *variable_name <= '9') is_sth = false;
				else for (ns = variable_name._data; *ns; ++ns)
					if (!is_varchar(*ns)) { is_sth = false; break; }

				// Assign variable (direct).
				if (is_sth) {
					arg3 = variable_name[1] ? ~0U : *variable_name; // One-char variable
					if (variable_name[1] && !variable_name[2]) { // Two-chars variable
						c1 = variable_name[0];
						c2 = variable_name[1];
						if (c1 == 'w' && c2 == 'h') arg3 = 0; // wh
						else if (c1 == 'p' && c2 == 'i') arg3 = 3; // pi
						else if (c1 == 'i') {
							if (c2 >= '0' && c2 <= '9') arg3 = 19 + c2 - '0'; // i0...i9
							else if (c2 == 'm') arg3 = 4; // im
							else if (c2 == 'M') arg3 = 5; // iM
							else if (c2 == 'a') arg3 = 6; // ia
							else if (c2 == 'v') arg3 = 7; // iv
							else if (c2 == 's') arg3 = 8; // is
							else if (c2 == 'p') arg3 = 9; // ip
							else if (c2 == 'c') arg3 = 10; // ic
						}
						else if (c2 == 'm') {
							if (c1 == 'x') arg3 = 11; // xm
							else if (c1 == 'y') arg3 = 12; // ym
							else if (c1 == 'z') arg3 = 13; // zm
							else if (c1 == 'c') arg3 = 14; // cm
						}
						else if (c2 == 'M') {
							if (c1 == 'x') arg3 = 15; // xM
							else if (c1 == 'y') arg3 = 16; // yM
							else if (c1 == 'z') arg3 = 17; // zM
							else if (c1 == 'c') arg3 = 18; // cM
						}
					}
					else if (variable_name[1] && variable_name[2] && !variable_name[3]) { // Three-chars variable
						c1 = variable_name[0];
						c2 = variable_name[1];
						c3 = variable_name[2];
						if (c1 == 'w' && c2 == 'h' && c3 == 'd') arg3 = 1; // whd
					}
					else if (variable_name[1] && variable_name[2] && variable_name[3] &&
						!variable_name[4]) { // Four-chars variable
						c1 = variable_name[0];
						c2 = variable_name[1];
						c3 = variable_name[2];
						c4 = variable_name[3];
						if (c1 == 'w' && c2 == 'h' && c3 == 'd' && c4 == 's') arg3 = 2; // whds
					}
					else if (!std::strcmp(variable_name, "interpolation")) arg3 = 29; // interpolation
					else if (!std::strcmp(variable_name, "boundary")) arg3 = 30; // boundary

					arg1 = ~0U;
					arg2 = compile(s + 1, se, depth1, 0);
					if (is_const) _cimg_mp_check_constant(arg2, 2, 0);

					if (arg3 != ~0U) // One-char variable, or variable in reserved_labels
						arg1 = reserved_label[arg3];
					else // Multi-char variable name : check for existing variable with same name
						cimglist_for(variable_def, i)
						if (!std::strcmp(variable_name, variable_def[i])) { arg1 = variable_pos[i]; break; }

					if (arg1 == ~0U) { // Create new variable
						if (_cimg_mp_is_vector(arg2)) { // Vector variable
							arg1 = is_comp_vector(arg2) ? arg2 : vector_copy(arg2);
							set_variable_vector(arg1);
						}
						else { // Scalar variable
							if (is_const) arg1 = arg2;
							else {
								arg1 = _cimg_mp_is_comp(arg2) ? arg2 : scalar1(mp_copy, arg2);
								memtype[arg1] = -1;
							}
						}

						if (arg3 != ~0U) reserved_label[arg3] = arg1;
						else {
							if (variable_def._width >= variable_pos._width) variable_pos.resize(-200, 1, 1, 1, 0);
							variable_pos[variable_def._width] = arg1;
							variable_name.move_to(variable_def);
						}

					}
					else { // Variable already exists -> assign a new value
						if (is_const || _cimg_mp_is_constant(arg1)) {
							*se = saved_char;
							cimg::strellipsize(variable_name, 64);
							s0 = ss - 4>expr._data ? ss - 4 : expr._data;
							cimg::strellipsize(s0, 64);
							throw CImgArgumentException("[" cimg_appname "_math_parser] "
								"CImg<%s>::%s: %s: Invalid assignment of %sconst variable '%s'%s, "
								"in expression '%s%s%s'.",
								pixel_type(), _cimg_mp_calling_function, s_op,
								_cimg_mp_is_constant(arg1) ? "already-defined " : "non-",
								variable_name._data,
								!_cimg_mp_is_constant(arg1) && is_const ? " as a new const variable" : "",
								s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
						}
						_cimg_mp_check_type(arg2, 2, _cimg_mp_is_vector(arg1) ? 3 : 1, _cimg_mp_vector_size(arg1));
						if (_cimg_mp_is_vector(arg1)) { // Vector
							if (_cimg_mp_is_vector(arg2)) // From vector
								CImg<ulongT>::vector((ulongT)mp_vector_copy, arg1, arg2, (ulongT)_cimg_mp_vector_size(arg1)).
								move_to(code);
							else // From scalar
								CImg<ulongT>::vector((ulongT)mp_vector_init, arg1, 1, (ulongT)_cimg_mp_vector_size(arg1), arg2).
								move_to(code);
						}
						else // Scalar
							CImg<ulongT>::vector((ulongT)mp_copy, arg1, arg2).move_to(code);
					}
					_cimg_mp_return(arg1);
				}

				// Assign lvalue (variable name was not valid for a direct assignment).
				arg1 = ~0U;
				is_sth = (bool)std::strchr(variable_name, '?'); // Contains_ternary_operator?
				if (is_sth) break; // Do nothing and make ternary operator prioritary over assignment

				if (l_variable_name>2 && (std::strchr(variable_name, '(') || std::strchr(variable_name, '['))) {
					ref.assign(7);
					arg1 = compile(ss, s, depth1, ref); // Lvalue slot
					arg2 = compile(s + 1, se, depth1, 0); // Value to assign

					if (*ref == 1) { // Vector value (scalar): V[k] = scalar
						_cimg_mp_check_type(arg2, 2, 1, 0);
						arg3 = ref[1]; // Vector slot
						arg4 = ref[2]; // Index
						if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
						CImg<ulongT>::vector((ulongT)mp_vector_set_off, arg2, arg3, (ulongT)_cimg_mp_vector_size(arg3), arg4, arg2).
							move_to(code);
						_cimg_mp_return(arg2);
					}

					if (*ref == 2) { // Image value (scalar): i/j[_#ind,off] = scalar
						is_parallelizable = false;
						_cimg_mp_check_type(arg2, 2, 1, 0);
						p1 = ref[1]; // Index
						is_relative = (bool)ref[2];
						arg3 = ref[3]; // Offset
						if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
						if (p1 != ~0U) {
							if (!listout) _cimg_mp_return(arg2);
							CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_joff : mp_list_set_ioff),
								arg2, p1, arg3).move_to(code);
						}
						else {
							if (!imgout) _cimg_mp_return(arg2);
							CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_joff : mp_set_ioff),
								arg2, arg3).move_to(code);
						}
						_cimg_mp_return(arg2);
					}

					if (*ref == 3) { // Image value (scalar): i/j(_#ind,_x,_y,_z,_c) = scalar
						is_parallelizable = false;
						_cimg_mp_check_type(arg2, 2, 1, 0);
						p1 = ref[1]; // Index
						is_relative = (bool)ref[2];
						arg3 = ref[3]; // X
						arg4 = ref[4]; // Y
						arg5 = ref[5]; // Z
						arg6 = ref[6]; // C
						if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
						if (p1 != ~0U) {
							if (!listout) _cimg_mp_return(arg2);
							CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_jxyzc : mp_list_set_ixyzc),
								arg2, p1, arg3, arg4, arg5, arg6).move_to(code);
						}
						else {
							if (!imgout) _cimg_mp_return(arg2);
							CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_jxyzc : mp_set_ixyzc),
								arg2, arg3, arg4, arg5, arg6).move_to(code);
						}
						_cimg_mp_return(arg2);
					}

					if (*ref == 4) { // Image value (vector): I/J[_#ind,off] = value
						is_parallelizable = false;
						_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
						p1 = ref[1]; // Index
						is_relative = (bool)ref[2];
						arg3 = ref[3]; // Offset
						if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
						if (p1 != ~0U) {
							if (!listout) _cimg_mp_return(arg2);
							if (_cimg_mp_is_scalar(arg2))
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Joff_s : mp_list_set_Ioff_s),
									arg2, p1, arg3).move_to(code);
							else
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Joff_v : mp_list_set_Ioff_v),
									arg2, p1, arg3, _cimg_mp_vector_size(arg2)).move_to(code);
						}
						else {
							if (!imgout) _cimg_mp_return(arg2);
							if (_cimg_mp_is_scalar(arg2))
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Joff_s : mp_set_Ioff_s),
									arg2, arg3).move_to(code);
							else
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Joff_v : mp_set_Ioff_v),
									arg2, arg3, _cimg_mp_vector_size(arg2)).move_to(code);
						}
						_cimg_mp_return(arg2);
					}

					if (*ref == 5) { // Image value (vector): I/J(_#ind,_x,_y,_z,_c) = value
						is_parallelizable = false;
						_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
						p1 = ref[1]; // Index
						is_relative = (bool)ref[2];
						arg3 = ref[3]; // X
						arg4 = ref[4]; // Y
						arg5 = ref[5]; // Z
						if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
						if (p1 != ~0U) {
							if (!listout) _cimg_mp_return(arg2);
							if (_cimg_mp_is_scalar(arg2))
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Jxyz_s : mp_list_set_Ixyz_s),
									arg2, p1, arg3, arg4, arg5).move_to(code);
							else
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Jxyz_v : mp_list_set_Ixyz_v),
									arg2, p1, arg3, arg4, arg5, _cimg_mp_vector_size(arg2)).move_to(code);
						}
						else {
							if (!imgout) _cimg_mp_return(arg2);
							if (_cimg_mp_is_scalar(arg2))
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Jxyz_s : mp_set_Ixyz_s),
									arg2, arg3, arg4, arg5).move_to(code);
							else
								CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Jxyz_v : mp_set_Ixyz_v),
									arg2, arg3, arg4, arg5, _cimg_mp_vector_size(arg2)).move_to(code);
						}
						_cimg_mp_return(arg2);
					}

					if (_cimg_mp_is_vector(arg1)) { // Vector variable: V = value
						_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
						if (_cimg_mp_is_vector(arg2)) // From vector
							CImg<ulongT>::vector((ulongT)mp_vector_copy, arg1, arg2, (ulongT)_cimg_mp_vector_size(arg1)).
							move_to(code);
						else // From scalar
							CImg<ulongT>::vector((ulongT)mp_vector_init, arg1, 1, (ulongT)_cimg_mp_vector_size(arg1), arg2).
							move_to(code);
						_cimg_mp_return(arg1);
					}

					if (_cimg_mp_is_variable(arg1)) { // Scalar variable: s = scalar
						_cimg_mp_check_type(arg2, 2, 1, 0);
						CImg<ulongT>::vector((ulongT)mp_copy, arg1, arg2).move_to(code);
						_cimg_mp_return(arg1);
					}
				}

				// No assignment expressions match -> error
				*se = saved_char;
				cimg::strellipsize(variable_name, 64);
				s0 = ss - 4>expr._data ? ss - 4 : expr._data;
				cimg::strellipsize(s0, 64);
				throw CImgArgumentException("[" cimg_appname "_math_parser] "
					"CImg<%s>::%s: %s: Invalid %slvalue '%s', "
					"in expression '%s%s%s'.",
					pixel_type(), _cimg_mp_calling_function, s_op,
					arg1 != ~0U && _cimg_mp_is_constant(arg1) ? "const " : "",
					variable_name._data,
					s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
			}

		// Apply unary/binary/ternary operators. The operator precedences should be the same as in C++.
		for (s = se2, ps = se3, ns = ps - 1; s>ss1; --s, --ps, --ns) // Here, ns = ps - 1
			if (*s == '=' && (*ps == '*' || *ps == '/' || *ps == '^') && *ns == *ps &&
				level[s - expr._data] == clevel) { // Self-operators for complex numbers only (**=,//=,^^=)
				_cimg_mp_op(*ps == '*' ? "Operator '**='" : *ps == '/' ? "Operator '//='" : "Operator '^^='");

				ref.assign(7);
				arg1 = compile(ss, ns, depth1, ref); // Vector slot
				arg2 = compile(s + 1, se, depth1, 0); // Right operand
				_cimg_mp_check_type(arg1, 1, 2, 2);
				_cimg_mp_check_type(arg2, 2, 3, 2);
				if (_cimg_mp_is_vector(arg2)) { // Complex **= complex
					if (*ps == '*')
						CImg<ulongT>::vector((ulongT)mp_complex_mul, arg1, arg1, arg2).move_to(code);
					else if (*ps == '/')
						CImg<ulongT>::vector((ulongT)mp_complex_div_vv, arg1, arg1, arg2).move_to(code);
					else
						CImg<ulongT>::vector((ulongT)mp_complex_pow_vv, arg1, arg1, arg2).move_to(code);
				}
				else { // Complex **= scalar
					if (*ps == '*') {
						if (arg2 == 1) _cimg_mp_return(arg1);
						self_vector_s(arg1, mp_self_mul, arg2);
					}
					else if (*ps == '/') {
						if (arg2 == 1) _cimg_mp_return(arg1);
						self_vector_s(arg1, mp_self_div, arg2);
					}
					else {
						if (arg2 == 1) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)mp_complex_pow_vs, arg1, arg1, arg2).move_to(code);
					}
				}

				// Write computed value back in image if necessary.
				if (*ref == 4) { // Image value (vector): I/J[_#ind,off] **= value
					is_parallelizable = false;
					p1 = ref[1]; // Index
					is_relative = (bool)ref[2];
					arg3 = ref[3]; // Offset
					if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
					if (p1 != ~0U) {
						if (!listout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Joff_v : mp_list_set_Ioff_v),
							arg1, p1, arg3, _cimg_mp_vector_size(arg1)).move_to(code);
					}
					else {
						if (!imgout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Joff_v : mp_set_Ioff_v),
							arg1, arg3, _cimg_mp_vector_size(arg1)).move_to(code);
					}

				}
				else if (*ref == 5) { // Image value (vector): I/J(_#ind,_x,_y,_z,_c) **= value
					is_parallelizable = false;
					p1 = ref[1]; // Index
					is_relative = (bool)ref[2];
					arg3 = ref[3]; // X
					arg4 = ref[4]; // Y
					arg5 = ref[5]; // Z
					if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
					if (p1 != ~0U) {
						if (!listout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Jxyz_v : mp_list_set_Ixyz_v),
							arg1, p1, arg3, arg4, arg5, _cimg_mp_vector_size(arg1)).move_to(code);
					}
					else {
						if (!imgout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Jxyz_v : mp_set_Ixyz_v),
							arg1, arg3, arg4, arg5, _cimg_mp_vector_size(arg1)).move_to(code);
					}
				}

				_cimg_mp_return(arg1);
			}

		for (s = se2, ps = se3, ns = ps - 1; s>ss1; --s, --ps, --ns) // Here, ns = ps - 1
			if (*s == '=' && (*ps == '+' || *ps == '-' || *ps == '*' || *ps == '/' || *ps == '%' ||
				*ps == '&' || *ps == '^' || *ps == '|' ||
				(*ps == '>' && *ns == '>') || (*ps == '<' && *ns == '<')) &&
				level[s - expr._data] == clevel) { // Self-operators (+=,-=,*=,/=,%=,>>=,<<=,&=,^=,|=)
				switch (*ps) {
				case '+': op = mp_self_add; _cimg_mp_op("Operator '+='"); break;
				case '-': op = mp_self_sub; _cimg_mp_op("Operator '-='"); break;
				case '*': op = mp_self_mul; _cimg_mp_op("Operator '*='"); break;
				case '/': op = mp_self_div; _cimg_mp_op("Operator '/='"); break;
				case '%': op = mp_self_modulo; _cimg_mp_op("Operator '%='"); break;
				case '<': op = mp_self_bitwise_left_shift; _cimg_mp_op("Operator '<<='"); break;
				case '>': op = mp_self_bitwise_right_shift; _cimg_mp_op("Operator '>>='"); break;
				case '&': op = mp_self_bitwise_and; _cimg_mp_op("Operator '&='"); break;
				case '|': op = mp_self_bitwise_or; _cimg_mp_op("Operator '|='"); break;
				default: op = mp_self_pow; _cimg_mp_op("Operator '^='"); break;
				}
				s1 = *ps == '>' || *ps == '<' ? ns : ps;

				ref.assign(7);
				arg1 = compile(ss, s1, depth1, ref); // Variable slot
				arg2 = compile(s + 1, se, depth1, 0); // Value to apply

													  // Check for particular case to be simplified.
				if ((op == mp_self_add || op == mp_self_sub) && !arg2) _cimg_mp_return(arg1);
				if ((op == mp_self_mul || op == mp_self_div) && arg2 == 1) _cimg_mp_return(arg1);

				// Apply operator on a copy to prevent modifying a constant or a variable.
				if (*ref && (_cimg_mp_is_constant(arg1) || _cimg_mp_is_vector(arg1) || _cimg_mp_is_variable(arg1))) {
					if (_cimg_mp_is_vector(arg1)) arg1 = vector_copy(arg1);
					else arg1 = scalar1(mp_copy, arg1);
				}

				if (*ref == 1) { // Vector value (scalar): V[k] += scalar
					_cimg_mp_check_type(arg2, 2, 1, 0);
					arg3 = ref[1]; // Vector slot
					arg4 = ref[2]; // Index
					if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
					CImg<ulongT>::vector((ulongT)op, arg1, arg2).move_to(code);
					CImg<ulongT>::vector((ulongT)mp_vector_set_off, arg1, arg3, (ulongT)_cimg_mp_vector_size(arg3), arg4, arg1).
						move_to(code);
					_cimg_mp_return(arg1);
				}

				if (*ref == 2) { // Image value (scalar): i/j[_#ind,off] += scalar
					is_parallelizable = false;
					_cimg_mp_check_type(arg2, 2, 1, 0);
					p1 = ref[1]; // Index
					is_relative = (bool)ref[2];
					arg3 = ref[3]; // Offset
					if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
					CImg<ulongT>::vector((ulongT)op, arg1, arg2).move_to(code);
					if (p1 != ~0U) {
						if (!listout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_joff : mp_list_set_ioff),
							arg1, p1, arg3).move_to(code);
					}
					else {
						if (!imgout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_joff : mp_set_ioff),
							arg1, arg3).move_to(code);
					}
					_cimg_mp_return(arg1);
				}

				if (*ref == 3) { // Image value (scalar): i/j(_#ind,_x,_y,_z,_c) += scalar
					is_parallelizable = false;
					_cimg_mp_check_type(arg2, 2, 1, 0);
					p1 = ref[1]; // Index
					is_relative = (bool)ref[2];
					arg3 = ref[3]; // X
					arg4 = ref[4]; // Y
					arg5 = ref[5]; // Z
					arg6 = ref[6]; // C
					if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
					CImg<ulongT>::vector((ulongT)op, arg1, arg2).move_to(code);
					if (p1 != ~0U) {
						if (!listout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_jxyzc : mp_list_set_ixyzc),
							arg1, p1, arg3, arg4, arg5, arg6).move_to(code);
					}
					else {
						if (!imgout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_jxyzc : mp_set_ixyzc),
							arg1, arg3, arg4, arg5, arg6).move_to(code);
					}
					_cimg_mp_return(arg1);
				}

				if (*ref == 4) { // Image value (vector): I/J[_#ind,off] += value
					is_parallelizable = false;
					_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
					p1 = ref[1]; // Index
					is_relative = (bool)ref[2];
					arg3 = ref[3]; // Offset
					if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
					if (_cimg_mp_is_scalar(arg2)) self_vector_s(arg1, op, arg2); else self_vector_v(arg1, op, arg2);
					if (p1 != ~0U) {
						if (!listout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Joff_v : mp_list_set_Ioff_v),
							arg1, p1, arg3, _cimg_mp_vector_size(arg1)).move_to(code);
					}
					else {
						if (!imgout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Joff_v : mp_set_Ioff_v),
							arg1, arg3, _cimg_mp_vector_size(arg1)).move_to(code);
					}
					_cimg_mp_return(arg1);
				}

				if (*ref == 5) { // Image value (vector): I/J(_#ind,_x,_y,_z,_c) += value
					is_parallelizable = false;
					_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
					p1 = ref[1]; // Index
					is_relative = (bool)ref[2];
					arg3 = ref[3]; // X
					arg4 = ref[4]; // Y
					arg5 = ref[5]; // Z
					if (p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
					if (_cimg_mp_is_scalar(arg2)) self_vector_s(arg1, op, arg2); else self_vector_v(arg1, op, arg2);
					if (p1 != ~0U) {
						if (!listout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Jxyz_v : mp_list_set_Ixyz_v),
							arg1, p1, arg3, arg4, arg5, _cimg_mp_vector_size(arg1)).move_to(code);
					}
					else {
						if (!imgout) _cimg_mp_return(arg1);
						CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Jxyz_v : mp_set_Ixyz_v),
							arg1, arg3, arg4, arg5, _cimg_mp_vector_size(arg1)).move_to(code);
					}
					_cimg_mp_return(arg1);
				}

				if (_cimg_mp_is_vector(arg1)) { // Vector variable: V += value
					_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
					if (_cimg_mp_is_vector(arg2)) self_vector_v(arg1, op, arg2); // Vector += vector
					else self_vector_s(arg1, op, arg2); // Vector += scalar
					_cimg_mp_return(arg1);
				}

				if (_cimg_mp_is_variable(arg1)) { // Scalar variable: s += scalar
					_cimg_mp_check_type(arg2, 2, 1, 0);
					CImg<ulongT>::vector((ulongT)op, arg1, arg2).move_to(code);
					_cimg_mp_return(arg1);
				}

				variable_name.assign(ss, (unsigned int)(s - ss)).back() = 0;
				cimg::strpare(variable_name, false, true);
				*se = saved_char;
				s0 = ss - 4>expr._data ? ss - 4 : expr._data;
				cimg::strellipsize(s0, 64);
				throw CImgArgumentException("[" cimg_appname "_math_parser] "
					"CImg<%s>::%s: %s: Invalid %slvalue '%s', "
					"in expression '%s%s%s'.",
					pixel_type(), _cimg_mp_calling_function, s_op,
					_cimg_mp_is_constant(arg1) ? "const " : "",
					variable_name._data,
					s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
			}

		for (s = ss1; s<se1; ++s)
			if (*s == '?' && level[s - expr._data] == clevel) { // Ternary operator 'cond?expr1:expr2'
				_cimg_mp_op("Operator '?:'");
				s1 = s + 1; while (s1<se1 && (*s1 != ':' || level[s1 - expr._data] != clevel)) ++s1;
				arg1 = compile(ss, s, depth1, 0);
				_cimg_mp_check_type(arg1, 1, 1, 0);
				if (_cimg_mp_is_constant(arg1)) {
					if ((bool)mem[arg1]) return compile(s + 1, *s1 != ':' ? se : s1, depth1, 0);
					else return *s1 != ':' ? 0 : compile(++s1, se, depth1, 0);
				}
				p2 = code._width;
				arg2 = compile(s + 1, *s1 != ':' ? se : s1, depth1, 0);
				p3 = code._width;
				arg3 = *s1 == ':' ? compile(++s1, se, depth1, 0) : _cimg_mp_is_vector(arg2) ? vector(_cimg_mp_vector_size(arg2), 0) : 0;
				_cimg_mp_check_type(arg3, 3, _cimg_mp_is_vector(arg2) ? 2 : 1, _cimg_mp_vector_size(arg2));
				arg4 = _cimg_mp_vector_size(arg2);
				if (arg4) pos = vector(arg4); else pos = scalar();
				CImg<ulongT>::vector((ulongT)mp_if, pos, arg1, arg2, arg3,
					p3 - p2, code._width - p3, arg4).move_to(code, p2);
				_cimg_mp_return(pos);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '|' && *ns == '|' && level[s - expr._data] == clevel) { // Logical or ('||')
				_cimg_mp_op("Operator '||'");
				arg1 = compile(ss, s, depth1, 0);
				_cimg_mp_check_type(arg1, 1, 1, 0);
				if (arg1>0 && arg1 <= 16) _cimg_mp_return(1);
				p2 = code._width;
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 1, 0);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
					_cimg_mp_constant(mem[arg1] || mem[arg2]);
				if (!arg1) _cimg_mp_return(arg2);
				pos = scalar();
				CImg<ulongT>::vector((ulongT)mp_logical_or, pos, arg1, arg2, code._width - p2).
					move_to(code, p2);
				_cimg_mp_return(pos);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '&' && *ns == '&' && level[s - expr._data] == clevel) { // Logical and ('&&')
				_cimg_mp_op("Operator '&&'");
				arg1 = compile(ss, s, depth1, 0);
				_cimg_mp_check_type(arg1, 1, 1, 0);
				if (!arg1) _cimg_mp_return(0);
				p2 = code._width;
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 1, 0);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
					_cimg_mp_constant(mem[arg1] && mem[arg2]);
				if (arg1>0 && arg1 <= 16) _cimg_mp_return(arg2);
				pos = scalar();
				CImg<ulongT>::vector((ulongT)mp_logical_and, pos, arg1, arg2, code._width - p2).
					move_to(code, p2);
				_cimg_mp_return(pos);
			}

		for (s = se2; s>ss; --s)
			if (*s == '|' && level[s - expr._data] == clevel) { // Bitwise or ('|')
				_cimg_mp_op("Operator '|'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 1, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_bitwise_or, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) {
					if (!arg2) _cimg_mp_return(arg1);
					_cimg_mp_vector2_vs(mp_bitwise_or, arg1, arg2);
				}
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) {
					if (!arg1) _cimg_mp_return(arg2);
					_cimg_mp_vector2_sv(mp_bitwise_or, arg1, arg2);
				}
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
					_cimg_mp_constant((longT)mem[arg1] | (longT)mem[arg2]);
				if (!arg2) _cimg_mp_return(arg1);
				if (!arg1) _cimg_mp_return(arg2);
				_cimg_mp_scalar2(mp_bitwise_or, arg1, arg2);
			}

		for (s = se2; s>ss; --s)
			if (*s == '&' && level[s - expr._data] == clevel) { // Bitwise and ('&')
				_cimg_mp_op("Operator '&'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 1, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_bitwise_and, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_bitwise_and, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_bitwise_and, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
					_cimg_mp_constant((longT)mem[arg1] & (longT)mem[arg2]);
				if (!arg1 || !arg2) _cimg_mp_return(0);
				_cimg_mp_scalar2(mp_bitwise_and, arg1, arg2);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '!' && *ns == '=' && level[s - expr._data] == clevel) { // Not equal to ('!=')
				_cimg_mp_op("Operator '!='");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				if (arg1 == arg2) _cimg_mp_return(0);
				p1 = _cimg_mp_vector_size(arg1);
				p2 = _cimg_mp_vector_size(arg2);
				if (p1 || p2) {
					if (p1 && p2 && p1 != p2) _cimg_mp_return(1);
					_cimg_mp_scalar6(mp_vector_neq, arg1, p1, arg2, p2, 11, 1);
				}
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] != mem[arg2]);
				_cimg_mp_scalar2(mp_neq, arg1, arg2);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '=' && *ns == '=' && level[s - expr._data] == clevel) { // Equal to ('==')
				_cimg_mp_op("Operator '=='");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				if (arg1 == arg2) _cimg_mp_return(1);
				p1 = _cimg_mp_vector_size(arg1);
				p2 = _cimg_mp_vector_size(arg2);
				if (p1 || p2) {
					if (p1 && p2 && p1 != p2) _cimg_mp_return(0);
					_cimg_mp_scalar6(mp_vector_eq, arg1, p1, arg2, p2, 11, 1);
				}
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] == mem[arg2]);
				_cimg_mp_scalar2(mp_eq, arg1, arg2);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '<' && *ns == '=' && level[s - expr._data] == clevel) { // Less or equal than ('<=')
				_cimg_mp_op("Operator '<='");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_lte, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_lte, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_lte, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] <= mem[arg2]);
				if (arg1 == arg2) _cimg_mp_return(1);
				_cimg_mp_scalar2(mp_lte, arg1, arg2);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '>' && *ns == '=' && level[s - expr._data] == clevel) { // Greater or equal than ('>=')
				_cimg_mp_op("Operator '>='");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_gte, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_gte, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_gte, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] >= mem[arg2]);
				if (arg1 == arg2) _cimg_mp_return(1);
				_cimg_mp_scalar2(mp_gte, arg1, arg2);
			}

		for (s = se2, ns = se1, ps = se3; s>ss; --s, --ns, --ps)
			if (*s == '<' && *ns != '<' && *ps != '<' && level[s - expr._data] == clevel) { // Less than ('<')
				_cimg_mp_op("Operator '<'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 1, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_lt, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_lt, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_lt, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1]<mem[arg2]);
				if (arg1 == arg2) _cimg_mp_return(0);
				_cimg_mp_scalar2(mp_lt, arg1, arg2);
			}

		for (s = se2, ns = se1, ps = se3; s>ss; --s, --ns, --ps)
			if (*s == '>' && *ns != '>' && *ps != '>' && level[s - expr._data] == clevel) { // Greather than ('>')
				_cimg_mp_op("Operator '>'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 1, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_gt, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_gt, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_gt, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1]>mem[arg2]);
				if (arg1 == arg2) _cimg_mp_return(0);
				_cimg_mp_scalar2(mp_gt, arg1, arg2);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '<' && *ns == '<' && level[s - expr._data] == clevel) { // Left bit shift ('<<')
				_cimg_mp_op("Operator '<<'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2))
					_cimg_mp_vector2_vv(mp_bitwise_left_shift, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) {
					if (!arg2) _cimg_mp_return(arg1);
					_cimg_mp_vector2_vs(mp_bitwise_left_shift, arg1, arg2);
				}
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2))
					_cimg_mp_vector2_sv(mp_bitwise_left_shift, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
					_cimg_mp_constant((longT)mem[arg1] << (unsigned int)mem[arg2]);
				if (!arg1) _cimg_mp_return(0);
				if (!arg2) _cimg_mp_return(arg1);
				_cimg_mp_scalar2(mp_bitwise_left_shift, arg1, arg2);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '>' && *ns == '>' && level[s - expr._data] == clevel) { // Right bit shift ('>>')
				_cimg_mp_op("Operator '>>'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2))
					_cimg_mp_vector2_vv(mp_bitwise_right_shift, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) {
					if (!arg2) _cimg_mp_return(arg1);
					_cimg_mp_vector2_vs(mp_bitwise_right_shift, arg1, arg2);
				}
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2))
					_cimg_mp_vector2_sv(mp_bitwise_right_shift, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
					_cimg_mp_constant((longT)mem[arg1] >> (unsigned int)mem[arg2]);
				if (!arg1) _cimg_mp_return(0);
				if (!arg2) _cimg_mp_return(arg1);
				_cimg_mp_scalar2(mp_bitwise_right_shift, arg1, arg2);
			}

		for (ns = se1, s = se2, ps = pexpr._data + (se3 - expr._data); s>ss; --ns, --s, --ps)
			if (*s == '+' && (*ns != '+' || ns != se1) && *ps != '-' && *ps != '+' && *ps != '*' && *ps != '/' && *ps != '%' &&
				*ps != '&' && *ps != '|' && *ps != '^' && *ps != '!' && *ps != '~' && *ps != '#' &&
				(*ps != 'e' || !(ps - pexpr._data>ss - expr._data && (*(ps - 1) == '.' || (*(ps - 1) >= '0' &&
					*(ps - 1) <= '9')))) &&
				level[s - expr._data] == clevel) { // Addition ('+')
				_cimg_mp_op("Operator '+'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 1, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (!arg2) _cimg_mp_return(arg1);
				if (!arg1) _cimg_mp_return(arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_add, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_add, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_add, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] + mem[arg2]);
				if (code) { // Try to spot linear case 'a*b + c'.
					CImg<ulongT> &pop = code.back();
					if (pop[0] == (ulongT)mp_mul && _cimg_mp_is_comp(pop[1]) && (pop[1] == arg1 || pop[1] == arg2)) {
						arg3 = (unsigned int)pop[1];
						arg4 = (unsigned int)pop[2];
						arg5 = (unsigned int)pop[3];
						code.remove();
						CImg<ulongT>::vector((ulongT)mp_linear_add, arg3, arg4, arg5, arg3 == arg2 ? arg1 : arg2).move_to(code);
						_cimg_mp_return(arg3);
					}
				}
				if (arg2 == 1) _cimg_mp_scalar1(mp_increment, arg1);
				if (arg1 == 1) _cimg_mp_scalar1(mp_increment, arg2);
				_cimg_mp_scalar2(mp_add, arg1, arg2);
			}

		for (ns = se1, s = se2, ps = pexpr._data + (se3 - expr._data); s>ss; --ns, --s, --ps)
			if (*s == '-' && (*ns != '-' || ns != se1) && *ps != '-' && *ps != '+' && *ps != '*' && *ps != '/' && *ps != '%' &&
				*ps != '&' && *ps != '|' && *ps != '^' && *ps != '!' && *ps != '~' && *ps != '#' &&
				(*ps != 'e' || !(ps - pexpr._data>ss - expr._data && (*(ps - 1) == '.' || (*(ps - 1) >= '0' &&
					*(ps - 1) <= '9')))) &&
				level[s - expr._data] == clevel) { // Subtraction ('-')
				_cimg_mp_op("Operator '-'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 1, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (!arg2) _cimg_mp_return(arg1);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_sub, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_sub, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) {
					if (!arg1) _cimg_mp_vector1_v(mp_minus, arg2);
					_cimg_mp_vector2_sv(mp_sub, arg1, arg2);
				}
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] - mem[arg2]);
				if (!arg1) _cimg_mp_scalar1(mp_minus, arg2);
				if (code) { // Try to spot linear cases 'a*b - c' and 'c - a*b'.
					CImg<ulongT> &pop = code.back();
					if (pop[0] == (ulongT)mp_mul && _cimg_mp_is_comp(pop[1]) && (pop[1] == arg1 || pop[1] == arg2)) {
						arg3 = (unsigned int)pop[1];
						arg4 = (unsigned int)pop[2];
						arg5 = (unsigned int)pop[3];
						code.remove();
						CImg<ulongT>::vector((ulongT)(arg3 == arg1 ? mp_linear_sub_left : mp_linear_sub_right),
							arg3, arg4, arg5, arg3 == arg1 ? arg2 : arg1).move_to(code);
						_cimg_mp_return(arg3);
					}
				}
				if (arg2 == 1) _cimg_mp_scalar1(mp_decrement, arg1);
				_cimg_mp_scalar2(mp_sub, arg1, arg2);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '*' && *ns == '*' && level[s - expr._data] == clevel) { // Complex multiplication ('**')
				_cimg_mp_op("Operator '**'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg1, 1, 3, 2);
				_cimg_mp_check_type(arg2, 2, 3, 2);
				if (arg2 == 1) _cimg_mp_return(arg1);
				if (arg1 == 1) _cimg_mp_return(arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) {
					pos = vector(2);
					CImg<ulongT>::vector((ulongT)mp_complex_mul, pos, arg1, arg2).move_to(code);
					_cimg_mp_return(pos);
				}
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_mul, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_mul, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] * mem[arg2]);
				if (!arg1 || !arg2) _cimg_mp_return(0);
				_cimg_mp_scalar2(mp_mul, arg1, arg2);
			}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '/' && *ns == '/' && level[s - expr._data] == clevel) { // Complex division ('//')
				_cimg_mp_op("Operator '//'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg1, 1, 3, 2);
				_cimg_mp_check_type(arg2, 2, 3, 2);
				if (arg2 == 1) _cimg_mp_return(arg1);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) {
					pos = vector(2);
					CImg<ulongT>::vector((ulongT)mp_complex_div_vv, pos, arg1, arg2).move_to(code);
					_cimg_mp_return(pos);
				}
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_div, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) {
					pos = vector(2);
					CImg<ulongT>::vector((ulongT)mp_complex_div_sv, pos, arg1, arg2).move_to(code);
					_cimg_mp_return(pos);
				}
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] / mem[arg2]);
				if (!arg1) _cimg_mp_return(0);
				_cimg_mp_scalar2(mp_div, arg1, arg2);
			}

		for (s = se2; s>ss; --s) if (*s == '*' && level[s - expr._data] == clevel) { // Multiplication ('*')
			_cimg_mp_op("Operator '*'");
			arg1 = compile(ss, s, depth1, 0);
			arg2 = compile(s + 1, se, depth1, 0);
			p2 = _cimg_mp_vector_size(arg2);
			if (p2>0 && _cimg_mp_vector_size(arg1) == p2*p2) { // Particular case of matrix multiplication
				pos = vector(p2);
				CImg<ulongT>::vector((ulongT)mp_matrix_mul, pos, arg1, arg2, p2, p2, 1).move_to(code);
				_cimg_mp_return(pos);
			}
			_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
			if (arg2 == 1) _cimg_mp_return(arg1);
			if (arg1 == 1) _cimg_mp_return(arg2);
			if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_mul, arg1, arg2);
			if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_mul, arg1, arg2);
			if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_mul, arg1, arg2);
			if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] * mem[arg2]);

			if (code) { // Try to spot double multiplication 'a*b*c'.
				CImg<ulongT> &pop = code.back();
				if (pop[0] == (ulongT)mp_mul && _cimg_mp_is_comp(pop[1]) && (pop[1] == arg1 || pop[1] == arg2)) {
					arg3 = (unsigned int)pop[1];
					arg4 = (unsigned int)pop[2];
					arg5 = (unsigned int)pop[3];
					code.remove();
					CImg<ulongT>::vector((ulongT)mp_mul2, arg3, arg4, arg5, arg3 == arg2 ? arg1 : arg2).move_to(code);
					_cimg_mp_return(arg3);
				}
			}
			if (!arg1 || !arg2) _cimg_mp_return(0);
			_cimg_mp_scalar2(mp_mul, arg1, arg2);
		}

		for (s = se2; s>ss; --s) if (*s == '/' && level[s - expr._data] == clevel) { // Division ('/')
			_cimg_mp_op("Operator '/'");
			arg1 = compile(ss, s, depth1, 0);
			arg2 = compile(s + 1, se, depth1, 0);
			_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
			if (arg2 == 1) _cimg_mp_return(arg1);
			if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_div, arg1, arg2);
			if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_div, arg1, arg2);
			if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_div, arg1, arg2);
			if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) _cimg_mp_constant(mem[arg1] / mem[arg2]);
			if (!arg1) _cimg_mp_return(0);
			_cimg_mp_scalar2(mp_div, arg1, arg2);
		}

		for (s = se2, ns = se1; s>ss; --s, --ns)
			if (*s == '%' && *ns != '^' && level[s - expr._data] == clevel) { // Modulo ('%')
				_cimg_mp_op("Operator '%'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 1, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_modulo, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_modulo, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_modulo, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
					_cimg_mp_constant(cimg::mod(mem[arg1], mem[arg2]));
				_cimg_mp_scalar2(mp_modulo, arg1, arg2);
			}

		if (se1>ss) {
			if (*ss == '+' && (*ss1 != '+' || (ss2<se && *ss2 >= '0' && *ss2 <= '9'))) { // Unary plus ('+')
				_cimg_mp_op("Operator '+'");
				_cimg_mp_return(compile(ss1, se, depth1, 0));
			}

			if (*ss == '-' && (*ss1 != '-' || (ss2<se && *ss2 >= '0' && *ss2 <= '9'))) { // Unary minus ('-')
				_cimg_mp_op("Operator '-'");
				arg1 = compile(ss1, se, depth1, 0);
				if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_minus, arg1);
				if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(-mem[arg1]);
				_cimg_mp_scalar1(mp_minus, arg1);
			}

			if (*ss == '!') { // Logical not ('!')
				_cimg_mp_op("Operator '!'");
				if (*ss1 == '!') { // '!!expr' optimized as 'bool(expr)'
					arg1 = compile(ss2, se, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_bool, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant((bool)mem[arg1]);
					_cimg_mp_scalar1(mp_bool, arg1);
				}
				arg1 = compile(ss1, se, depth1, 0);
				if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_logical_not, arg1);
				if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(!mem[arg1]);
				_cimg_mp_scalar1(mp_logical_not, arg1);
			}

			if (*ss == '~') { // Bitwise not ('~')
				_cimg_mp_op("Operator '~'");
				arg1 = compile(ss1, se, depth1, 0);
				if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_bitwise_not, arg1);
				if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(~(unsigned int)mem[arg1]);
				_cimg_mp_scalar1(mp_bitwise_not, arg1);
			}
		}

		for (s = se3, ns = se2; s>ss; --s, --ns)
			if (*s == '^' && *ns == '^' && level[s - expr._data] == clevel) { // Complex power ('^^')
				_cimg_mp_op("Operator '^^'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 2, se, depth1, 0);
				_cimg_mp_check_type(arg1, 1, 3, 2);
				_cimg_mp_check_type(arg2, 2, 3, 2);
				if (arg2 == 1) _cimg_mp_return(arg1);
				pos = vector(2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) {
					CImg<ulongT>::vector((ulongT)mp_complex_pow_vv, pos, arg1, arg2).move_to(code);
					_cimg_mp_return(pos);
				}
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) {
					CImg<ulongT>::vector((ulongT)mp_complex_pow_vs, pos, arg1, arg2).move_to(code);
					_cimg_mp_return(pos);
				}
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) {
					CImg<ulongT>::vector((ulongT)mp_complex_pow_sv, pos, arg1, arg2).move_to(code);
					_cimg_mp_return(pos);
				}
				CImg<ulongT>::vector((ulongT)mp_complex_pow_ss, pos, arg1, arg2).move_to(code);
				_cimg_mp_return(pos);
			}

		for (s = se2; s>ss; --s)
			if (*s == '^' && level[s - expr._data] == clevel) { // Power ('^')
				_cimg_mp_op("Operator '^'");
				arg1 = compile(ss, s, depth1, 0);
				arg2 = compile(s + 1, se, depth1, 0);
				_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
				if (arg2 == 1) _cimg_mp_return(arg1);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_pow, arg1, arg2);
				if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_pow, arg1, arg2);
				if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_pow, arg1, arg2);
				if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
					_cimg_mp_constant(std::pow(mem[arg1], mem[arg2]));
				switch (arg2) {
				case 0: _cimg_mp_return(1);
				case 2: _cimg_mp_scalar1(mp_sqr, arg1);
				case 3: _cimg_mp_scalar1(mp_pow3, arg1);
				case 4: _cimg_mp_scalar1(mp_pow4, arg1);
				default:
					if (_cimg_mp_is_constant(arg2)) {
						if (mem[arg2] == 0.5) { _cimg_mp_scalar1(mp_sqrt, arg1); }
						else if (mem[arg2] == 0.25) { _cimg_mp_scalar1(mp_pow0_25, arg1); }
					}
					_cimg_mp_scalar2(mp_pow, arg1, arg2);
				}
			}

		// Percentage computation.
		if (*se1 == '%') {
			arg1 = compile(ss, se1, depth1, 0);
			arg2 = _cimg_mp_is_constant(arg1) ? 0 : constant(100);
			if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector2_vs(mp_div, arg1, arg2);
			if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(mem[arg1] / 100);
			_cimg_mp_scalar2(mp_div, arg1, arg2);
		}

		is_sth = ss1<se1 && (*ss == '+' || *ss == '-') && *ss1 == *ss; // is pre-?
		if (is_sth || (se2>ss && (*se1 == '+' || *se1 == '-') && *se2 == *se1)) { // Pre/post-decrement and increment
			if ((is_sth && *ss == '+') || (!is_sth && *se1 == '+')) {
				_cimg_mp_op("Operator '++'");
				op = mp_self_increment;
			}
			else {
				_cimg_mp_op("Operator '--'");
				op = mp_self_decrement;
			}
			ref.assign(7);
			arg1 = is_sth ? compile(ss2, se, depth1, ref) : compile(ss, se2, depth1, ref); // Variable slot

																						   // Apply operator on a copy to prevent modifying a constant or a variable.
			if (*ref && (_cimg_mp_is_constant(arg1) || _cimg_mp_is_vector(arg1) || _cimg_mp_is_variable(arg1))) {
				if (_cimg_mp_is_vector(arg1)) arg1 = vector_copy(arg1);
				else arg1 = scalar1(mp_copy, arg1);
			}

			if (is_sth) pos = arg1; // Determine return indice, depending on pre/post action
			else {
				if (_cimg_mp_is_vector(arg1)) pos = vector_copy(arg1);
				else pos = scalar1(mp_copy, arg1);
			}

			if (*ref == 1) { // Vector value (scalar): V[k]++
				arg3 = ref[1]; // Vector slot
				arg4 = ref[2]; // Index
				if (is_sth && p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
				CImg<ulongT>::vector((ulongT)op, arg1, 1).move_to(code);
				CImg<ulongT>::vector((ulongT)mp_vector_set_off, arg1, arg3, (ulongT)_cimg_mp_vector_size(arg3), arg4, arg1).
					move_to(code);
				_cimg_mp_return(pos);
			}

			if (*ref == 2) { // Image value (scalar): i/j[_#ind,off]++
				is_parallelizable = false;
				p1 = ref[1]; // Index
				is_relative = (bool)ref[2];
				arg3 = ref[3]; // Offset
				if (is_sth && p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
				CImg<ulongT>::vector((ulongT)op, arg1).move_to(code);
				if (p1 != ~0U) {
					if (!listout) _cimg_mp_return(pos);
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_joff : mp_list_set_ioff),
						arg1, p1, arg3).move_to(code);
				}
				else {
					if (!imgout) _cimg_mp_return(pos);
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_joff : mp_set_ioff),
						arg1, arg3).move_to(code);
				}
				_cimg_mp_return(pos);
			}

			if (*ref == 3) { // Image value (scalar): i/j(_#ind,_x,_y,_z,_c)++
				is_parallelizable = false;
				p1 = ref[1]; // Index
				is_relative = (bool)ref[2];
				arg3 = ref[3]; // X
				arg4 = ref[4]; // Y
				arg5 = ref[5]; // Z
				arg6 = ref[6]; // C
				if (is_sth && p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
				CImg<ulongT>::vector((ulongT)op, arg1).move_to(code);
				if (p1 != ~0U) {
					if (!listout) _cimg_mp_return(pos);
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_jxyzc : mp_list_set_ixyzc),
						arg1, p1, arg3, arg4, arg5, arg6).move_to(code);
				}
				else {
					if (!imgout) _cimg_mp_return(pos);
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_jxyzc : mp_set_ixyzc),
						arg1, arg3, arg4, arg5, arg6).move_to(code);
				}
				_cimg_mp_return(pos);
			}

			if (*ref == 4) { // Image value (vector): I/J[_#ind,off]++
				is_parallelizable = false;
				p1 = ref[1]; // Index
				is_relative = (bool)ref[2];
				arg3 = ref[3]; // Offset
				if (is_sth && p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
				self_vector_s(arg1, op == mp_self_increment ? mp_self_add : mp_self_sub, 1);
				if (p1 != ~0U) {
					if (!listout) _cimg_mp_return(pos);
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Joff_v : mp_list_set_Ioff_v),
						arg1, p1, arg3, _cimg_mp_vector_size(arg1)).move_to(code);
				}
				else {
					if (!imgout) _cimg_mp_return(pos);
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Joff_v : mp_set_Ioff_v),
						arg1, arg3, _cimg_mp_vector_size(arg1)).move_to(code);
				}
				_cimg_mp_return(pos);
			}

			if (*ref == 5) { // Image value (vector): I/J(_#ind,_x,_y,_z,_c)++
				is_parallelizable = false;
				p1 = ref[1]; // Index
				is_relative = (bool)ref[2];
				arg3 = ref[3]; // X
				arg4 = ref[4]; // Y
				arg5 = ref[5]; // Z
				if (is_sth && p_ref) std::memcpy(p_ref, ref, ref._width * sizeof(unsigned int));
				self_vector_s(arg1, op == mp_self_increment ? mp_self_add : mp_self_sub, 1);
				if (p1 != ~0U) {
					if (!listout) _cimg_mp_return(pos);
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_set_Jxyz_v : mp_list_set_Ixyz_v),
						arg1, p1, arg3, arg4, arg5, _cimg_mp_vector_size(arg1)).move_to(code);
				}
				else {
					if (!imgout) _cimg_mp_return(pos);
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_set_Jxyz_v : mp_set_Ixyz_v),
						arg1, arg3, arg4, arg5, _cimg_mp_vector_size(arg1)).move_to(code);
				}
				_cimg_mp_return(pos);
			}

			if (_cimg_mp_is_vector(arg1)) { // Vector variable: V++
				self_vector_s(arg1, op == mp_self_increment ? mp_self_add : mp_self_sub, 1);
				_cimg_mp_return(pos);
			}

			if (_cimg_mp_is_variable(arg1)) { // Scalar variable: s++
				CImg<ulongT>::vector((ulongT)op, arg1).move_to(code);
				_cimg_mp_return(pos);
			}

			if (is_sth) variable_name.assign(ss2, (unsigned int)(se - ss1));
			else variable_name.assign(ss, (unsigned int)(se1 - ss));
			variable_name.back() = 0;
			cimg::strpare(variable_name, false, true);
			*se = saved_char;
			cimg::strellipsize(variable_name, 64);
			s0 = ss - 4>expr._data ? ss - 4 : expr._data;
			cimg::strellipsize(s0, 64);
			throw CImgArgumentException("[" cimg_appname "_math_parser] "
				"CImg<%s>::%s: %s: Invalid %slvalue '%s', "
				"in expression '%s%s%s'.",
				pixel_type(), _cimg_mp_calling_function, s_op,
				_cimg_mp_is_constant(arg1) ? "const " : "",
				variable_name._data,
				s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
		}

		// Array-like access to vectors and  image values 'i/j/I/J[_#ind,offset,_boundary]' and 'vector[offset]'.
		if (*se1 == ']' && *ss != '[') {
			_cimg_mp_op("Value accessor '[]'");
			is_relative = *ss == 'j' || *ss == 'J';
			s0 = s1 = std::strchr(ss, '['); do { --s1; } while ((signed char)*s1 <= ' '); cimg::swap(*s0, *++s1);

			if ((*ss == 'I' || *ss == 'J') && *ss1 == '[' &&
				(reserved_label[*ss] == ~0U || !_cimg_mp_is_vector(reserved_label[*ss]))) { // Image value as a vector
				if (*ss2 == '#') { // Index specified
					s0 = ss3; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
					p1 = compile(ss3, s0++, depth1, 0);
					_cimg_mp_check_list(false);
				}
				else { p1 = ~0U; s0 = ss2; }
				s1 = s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
				p2 = 1 + (p1 != ~0U);
				arg1 = compile(s0, s1, depth1, 0); // Offset
				_cimg_mp_check_type(arg1, p2, 1, 0);
				arg2 = ~0U;
				if (s1<se1) {
					arg2 = compile(++s1, se1, depth1, 0); // Boundary
					_cimg_mp_check_type(arg2, p2 + 1, 1, 0);
				}
				if (p_ref && arg2 == ~0U) {
					*p_ref = 4;
					p_ref[1] = p1;
					p_ref[2] = (unsigned int)is_relative;
					p_ref[3] = arg1;
					if (p1 != ~0U && _cimg_mp_is_comp(p1)) memtype[p1] = -2; // Prevent from being used in further optimization
					if (_cimg_mp_is_comp(arg1)) memtype[arg1] = -2;
				}
				p2 = ~0U; // 'p2' must be the dimension of the vector-valued operand if any
				if (p1 == ~0U) p2 = imgin._spectrum;
				else if (_cimg_mp_is_constant(p1)) {
					p3 = (unsigned int)cimg::mod((int)mem[p1], listin.width());
					p2 = listin[p3]._spectrum;
				}
				_cimg_mp_check_vector0(p2);
				pos = vector(p2);
				if (p1 != ~0U) {
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_Joff : mp_list_Ioff),
						pos, p1, arg1, arg2 == ~0U ? _cimg_mp_boundary : arg2, p2).move_to(code);
				}
				else {
					need_input_copy = true;
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_Joff : mp_Ioff),
						pos, arg1, arg2 == ~0U ? _cimg_mp_boundary : arg2, p2).move_to(code);
				}
				_cimg_mp_return(pos);
			}

			if ((*ss == 'i' || *ss == 'j') && *ss1 == '[' &&
				(reserved_label[*ss] == ~0U || !_cimg_mp_is_vector(reserved_label[*ss]))) { // Image value as a scalar
				if (*ss2 == '#') { // Index specified
					s0 = ss3; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
					p1 = compile(ss3, s0++, depth1, 0);
				}
				else { p1 = ~0U; s0 = ss2; }
				s1 = s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
				arg1 = compile(s0, s1, depth1, 0); // Offset
				arg2 = s1<se1 ? compile(++s1, se1, depth1, 0) : ~0U; // Boundary
				if (p_ref && arg2 == ~0U) {
					*p_ref = 2;
					p_ref[1] = p1;
					p_ref[2] = (unsigned int)is_relative;
					p_ref[3] = arg1;
					if (p1 != ~0U && _cimg_mp_is_comp(p1)) memtype[p1] = -2; // Prevent from being used in further optimization
					if (_cimg_mp_is_comp(arg1)) memtype[arg1] = -2;
				}
				if (p1 != ~0U) {
					if (!listin) _cimg_mp_return(0);
					pos = scalar3(is_relative ? mp_list_joff : mp_list_ioff, p1, arg1, arg2 == ~0U ? _cimg_mp_boundary : arg2);
				}
				else {
					if (!imgin) _cimg_mp_return(0);
					need_input_copy = true;
					pos = scalar2(is_relative ? mp_joff : mp_ioff, arg1, arg2 == ~0U ? _cimg_mp_boundary : arg2);
				}
				memtype[pos] = -2; // Prevent from being used in further optimization
				_cimg_mp_return(pos);
			}

			s0 = se1; while (s0>ss && (*s0 != '[' || level[s0 - expr._data] != clevel)) --s0;
			if (s0>ss) { // Vector value
				arg1 = compile(ss, s0, depth1, 0);
				if (_cimg_mp_is_scalar(arg1)) {
					variable_name.assign(ss, (unsigned int)(s0 - ss + 1)).back() = 0;
					*se = saved_char;
					cimg::strellipsize(variable_name, 64);
					s0 = ss - 4>expr._data ? ss - 4 : expr._data;
					cimg::strellipsize(s0, 64);
					throw CImgArgumentException("[" cimg_appname "_math_parser] "
						"CImg<%s>::%s: %s: Array brackets used on non-vector variable '%s', "
						"in expression '%s%s%s'.",
						pixel_type(), _cimg_mp_calling_function, s_op,
						variable_name._data,
						s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");

				}
				s1 = s0 + 1; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;

				if (s1<se1) { // Two arguments -> sub-vector extraction
					p1 = _cimg_mp_vector_size(arg1);
					arg2 = compile(++s0, s1, depth1, 0); // Starting indice
					arg3 = compile(++s1, se1, depth1, 0); // Length
					_cimg_mp_check_constant(arg3, 2, 3);
					arg3 = (unsigned int)mem[arg3];
					pos = vector(arg3);
					CImg<ulongT>::vector((ulongT)mp_vector_crop, pos, arg1, p1, arg2, arg3).move_to(code);
					_cimg_mp_return(pos);
				}

				// One argument -> vector value reference
				arg2 = compile(++s0, se1, depth1, 0);
				if (_cimg_mp_is_constant(arg2)) { // Constant index
					nb = (int)mem[arg2];
					if (nb >= 0 && nb<(int)_cimg_mp_vector_size(arg1)) _cimg_mp_return(arg1 + 1 + nb);
					variable_name.assign(ss, (unsigned int)(s0 - ss)).back() = 0;
					*se = saved_char;
					cimg::strellipsize(variable_name, 64);
					s0 = ss - 4>expr._data ? ss - 4 : expr._data;
					cimg::strellipsize(s0, 64);
					throw CImgArgumentException("[" cimg_appname "_math_parser] "
						"CImg<%s>::%s: Out-of-bounds reference '%s[%d]' "
						"(vector '%s' has dimension %u), "
						"in expression '%s%s%s'.",
						pixel_type(), _cimg_mp_calling_function,
						variable_name._data, nb,
						variable_name._data, _cimg_mp_vector_size(arg1),
						s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
				}
				if (p_ref) {
					*p_ref = 1;
					p_ref[1] = arg1;
					p_ref[2] = arg2;
					if (_cimg_mp_is_comp(arg2)) memtype[arg2] = -2; // Prevent from being used in further optimization
				}
				pos = scalar3(mp_vector_off, arg1, _cimg_mp_vector_size(arg1), arg2);
				memtype[pos] = -2; // Prevent from being used in further optimization
				_cimg_mp_return(pos);
			}
		}

		// Look for a function call, an access to image value, or a parenthesis.
		if (*se1 == ')') {
			if (*ss == '(') _cimg_mp_return(compile(ss1, se1, depth1, p_ref)); // Simple parentheses
			_cimg_mp_op("Value accessor '()'");
			is_relative = *ss == 'j' || *ss == 'J';
			s0 = s1 = std::strchr(ss, '('); do { --s1; } while ((signed char)*s1 <= ' '); cimg::swap(*s0, *++s1);

			// I/J(_#ind,_x,_y,_z,_interpolation,_boundary_conditions)
			if ((*ss == 'I' || *ss == 'J') && *ss1 == '(') { // Image value as scalar
				if (*ss2 == '#') { // Index specified
					s0 = ss3; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
					p1 = compile(ss3, s0++, depth1, 0);
					_cimg_mp_check_list(false);
				}
				else { p1 = ~0U; s0 = ss2; }
				arg1 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_x;
				arg2 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_y;
				arg3 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_z;
				arg4 = arg5 = ~0U;
				if (s0<se1) {
					s1 = s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(s0, s1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) { // Coordinates specified as a vector
						p2 = _cimg_mp_vector_size(arg1);
						++arg1;
						if (p2>1) {
							arg2 = arg1 + 1;
							if (p2>2) arg3 = arg2 + 1;
						}
						if (s1<se1) {
							s2 = ++s1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
							arg4 = compile(s1, s2, depth1, 0);
							arg5 = s2<se1 ? compile(++s2, se1, depth1, 0) : ~0U;
						}
					}
					else if (s1<se1) {
						s2 = ++s1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
						arg2 = compile(s1, s2, depth1, 0);
						if (s2<se1) {
							s3 = ++s2; while (s3<se1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
							arg3 = compile(s2, s3, depth1, 0);
							if (s3<se1) {
								s2 = ++s3; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
								arg4 = compile(s3, s2, depth1, 0);
								arg5 = s2<se1 ? compile(++s2, se1, depth1, 0) : ~0U;
							}
						}
					}
				}
				if (p_ref && arg4 == ~0U && arg5 == ~0U) {
					*p_ref = 5;
					p_ref[1] = p1;
					p_ref[2] = (unsigned int)is_relative;
					p_ref[3] = arg1;
					p_ref[4] = arg2;
					p_ref[5] = arg3;
					if (p1 != ~0U && _cimg_mp_is_comp(p1)) memtype[p1] = -2; // Prevent from being used in further optimization
					if (_cimg_mp_is_comp(arg1)) memtype[arg1] = -2;
					if (_cimg_mp_is_comp(arg2)) memtype[arg2] = -2;
					if (_cimg_mp_is_comp(arg3)) memtype[arg3] = -2;
				}
				p2 = ~0U; // 'p2' must be the dimension of the vector-valued operand if any
				if (p1 == ~0U) p2 = imgin._spectrum;
				else if (_cimg_mp_is_constant(p1)) {
					p3 = (unsigned int)cimg::mod((int)mem[p1], listin.width());
					p2 = listin[p3]._spectrum;
				}
				_cimg_mp_check_vector0(p2);
				pos = vector(p2);
				if (p1 != ~0U)
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_list_Jxyz : mp_list_Ixyz),
						pos, p1, arg1, arg2, arg3,
						arg4 == ~0U ? _cimg_mp_interpolation : arg4,
						arg5 == ~0U ? _cimg_mp_boundary : arg5, p2).move_to(code);
				else {
					need_input_copy = true;
					CImg<ulongT>::vector((ulongT)(is_relative ? mp_Jxyz : mp_Ixyz),
						pos, arg1, arg2, arg3,
						arg4 == ~0U ? _cimg_mp_interpolation : arg4,
						arg5 == ~0U ? _cimg_mp_boundary : arg5, p2).move_to(code);
				}
				_cimg_mp_return(pos);
			}

			// i/j(_#ind,_x,_y,_z,_c,_interpolation,_boundary_conditions)
			if ((*ss == 'i' || *ss == 'j') && *ss1 == '(') { // Image value as scalar
				if (*ss2 == '#') { // Index specified
					s0 = ss3; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
					p1 = compile(ss3, s0++, depth1, 0);
				}
				else { p1 = ~0U; s0 = ss2; }
				arg1 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_x;
				arg2 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_y;
				arg3 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_z;
				arg4 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_c;
				arg5 = arg6 = ~0U;
				if (s0<se1) {
					s1 = s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(s0, s1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) { // Coordinates specified as a vector
						p2 = _cimg_mp_vector_size(arg1);
						++arg1;
						if (p2>1) {
							arg2 = arg1 + 1;
							if (p2>2) {
								arg3 = arg2 + 1;
								if (p2>3) arg4 = arg3 + 1;
							}
						}
						if (s1<se1) {
							s2 = ++s1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
							arg5 = compile(s1, s2, depth1, 0);
							arg6 = s2<se1 ? compile(++s2, se1, depth1, 0) : ~0U;
						}
					}
					else if (s1<se1) {
						s2 = ++s1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
						arg2 = compile(s1, s2, depth1, 0);
						if (s2<se1) {
							s3 = ++s2; while (s3<se1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
							arg3 = compile(s2, s3, depth1, 0);
							if (s3<se1) {
								s2 = ++s3; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
								arg4 = compile(s3, s2, depth1, 0);
								if (s2<se1) {
									s3 = ++s2; while (s3<se1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
									arg5 = compile(s2, s3, depth1, 0);
									arg6 = s3<se1 ? compile(++s3, se1, depth1, 0) : ~0U;
								}
							}
						}
					}
				}
				if (p_ref && arg5 == ~0U && arg6 == ~0U) {
					*p_ref = 3;
					p_ref[1] = p1;
					p_ref[2] = (unsigned int)is_relative;
					p_ref[3] = arg1;
					p_ref[4] = arg2;
					p_ref[5] = arg3;
					p_ref[6] = arg4;
					if (p1 != ~0U && _cimg_mp_is_comp(p1)) memtype[p1] = -2; // Prevent from being used in further optimization
					if (_cimg_mp_is_comp(arg1)) memtype[arg1] = -2;
					if (_cimg_mp_is_comp(arg2)) memtype[arg2] = -2;
					if (_cimg_mp_is_comp(arg3)) memtype[arg3] = -2;
					if (_cimg_mp_is_comp(arg4)) memtype[arg4] = -2;
				}

				if (p1 != ~0U) {
					if (!listin) _cimg_mp_return(0);
					pos = scalar7(is_relative ? mp_list_jxyzc : mp_list_ixyzc,
						p1, arg1, arg2, arg3, arg4,
						arg5 == ~0U ? _cimg_mp_interpolation : arg5,
						arg6 == ~0U ? _cimg_mp_boundary : arg6);
				}
				else {
					if (!imgin) _cimg_mp_return(0);
					need_input_copy = true;
					pos = scalar6(is_relative ? mp_jxyzc : mp_ixyzc,
						arg1, arg2, arg3, arg4,
						arg5 == ~0U ? _cimg_mp_interpolation : arg5,
						arg6 == ~0U ? _cimg_mp_boundary : arg6);
				}
				memtype[pos] = -2; // Prevent from being used in further optimization
				_cimg_mp_return(pos);
			}

			// Mathematical functions.
			switch (*ss) {

			case '_':
				if (*ss1 == '(') // Skip arguments
					_cimg_mp_return_nan();
				break;

			case 'a':
				if (!std::strncmp(ss, "abs(", 4)) { // Absolute value
					_cimg_mp_op("Function 'abs()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_abs, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::abs(mem[arg1]));
					_cimg_mp_scalar1(mp_abs, arg1);
				}

				if (!std::strncmp(ss, "acos(", 5)) { // Arccos
					_cimg_mp_op("Function 'acos()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_acos, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::acos(mem[arg1]));
					_cimg_mp_scalar1(mp_acos, arg1);
				}

				if (!std::strncmp(ss, "arg(", 4)) { // Nth argument
					_cimg_mp_op("Function 'arg()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss4, s1, depth1, 0);
					_cimg_mp_check_type(arg1, 1, 1, 0);
					s2 = ++s1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					arg2 = compile(s1, s2, depth1, 0);
					p2 = _cimg_mp_vector_size(arg2);
					p3 = 3;
					CImg<ulongT>::vector((ulongT)mp_arg, 0, 0, p2, arg1, arg2).move_to(_opcode);
					for (s = ++s2; s<se; ++s) {
						ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
							(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
						arg3 = compile(s, ns, depth1, 0);
						_cimg_mp_check_type(arg3, p3, p2 ? 2 : 1, p2);
						CImg<ulongT>::vector(arg3).move_to(_opcode);
						++p3;
						s = ns;
					}
					(_opcode>'y').move_to(opcode);
					opcode[2] = opcode._height;
					if (_cimg_mp_is_constant(arg1)) {
						p3 -= 1; // Number of args
						arg1 = (unsigned int)(mem[arg1]<0 ? mem[arg1] + p3 : mem[arg1]);
						if (arg1<p3) _cimg_mp_return(opcode[4 + arg1]);
						if (p2) {
							pos = vector(p2);
							std::memset(&mem[pos] + 1, 0, p2 * sizeof(double));
							_cimg_mp_return(pos);
						}
						else _cimg_mp_return(0);
					}
					pos = opcode[1] = p2 ? vector(p2) : scalar();
					opcode.move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "asin(", 5)) { // Arcsin
					_cimg_mp_op("Function 'asin()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_asin, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::asin(mem[arg1]));
					_cimg_mp_scalar1(mp_asin, arg1);
				}

				if (!std::strncmp(ss, "atan(", 5)) { // Arctan
					_cimg_mp_op("Function 'atan()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_atan, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::atan(mem[arg1]));
					_cimg_mp_scalar1(mp_atan, arg1);
				}

				if (!std::strncmp(ss, "atan2(", 6)) { // Arctan2
					_cimg_mp_op("Function 'atan2()'");
					s1 = ss6; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss6, s1, depth1, 0);
					arg2 = compile(++s1, se1, depth1, 0);
					_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
					if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_atan2, arg1, arg2);
					if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_atan2, arg1, arg2);
					if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_atan2, arg1, arg2);
					if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
						_cimg_mp_constant(std::atan2(mem[arg1], mem[arg2]));
					_cimg_mp_scalar2(mp_atan2, arg1, arg2);
				}
				break;

			case 'b':
				if (!std::strncmp(ss, "bool(", 5)) { // Boolean cast
					_cimg_mp_op("Function 'bool()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_bool, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant((bool)mem[arg1]);
					_cimg_mp_scalar1(mp_bool, arg1);
				}

				if (!std::strncmp(ss, "break(", 6)) { // Complex absolute value
					if (pexpr[se2 - expr._data] == '(') { // no arguments?
						CImg<ulongT>::vector((ulongT)mp_break, _cimg_mp_slot_nan).move_to(code);
						_cimg_mp_return_nan();
					}
				}

				if (!std::strncmp(ss, "breakpoint(", 11)) { // Break point (for abort test)
					_cimg_mp_op("Function 'breakpoint()'");
					if (pexpr[se2 - expr._data] == '(') { // no arguments?
						CImg<ulongT>::vector((ulongT)mp_breakpoint, _cimg_mp_slot_nan).move_to(code);
						_cimg_mp_return_nan();
					}
				}
				break;

			case 'c':
				if ((*ss1 == 'm' || *ss1 == 'M') && *ss2 == '(') { // cm(), cM()
					_cimg_mp_op(*ss1 == 'm' ? "Function 'cm()'" : "Function 'cM()'");
					if (*ss3 == '#') { p1 = compile(ss4, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss3 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)(*ss1 == 'm' ? mp_image_cm : mp_image_cM), pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "cabs(", 5)) { // Complex absolute value
					_cimg_mp_op("Function 'cabs()'");
					arg1 = compile(ss5, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 0, 2, 2);
					_cimg_mp_scalar2(mp_complex_abs, arg1 + 1, arg1 + 2);
				}

				if (!std::strncmp(ss, "carg(", 5)) { // Complex argument
					_cimg_mp_op("Function 'carg()'");
					arg1 = compile(ss5, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 0, 2, 2);
					_cimg_mp_scalar2(mp_atan2, arg1 + 2, arg1 + 1);
				}

				if (!std::strncmp(ss, "cbrt(", 5)) { // Cubic root
					_cimg_mp_op("Function 'cbrt()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_cbrt, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::cbrt(mem[arg1]));
					_cimg_mp_scalar1(mp_cbrt, arg1);
				}

				if (!std::strncmp(ss, "cconj(", 6)) { // Complex conjugate
					_cimg_mp_op("Function 'cconj()'");
					arg1 = compile(ss6, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 0, 2, 2);
					pos = vector(2);
					CImg<ulongT>::vector((ulongT)mp_complex_conj, pos, arg1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "ceil(", 5)) { // Ceil
					_cimg_mp_op("Function 'ceil()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_ceil, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::ceil(mem[arg1]));
					_cimg_mp_scalar1(mp_ceil, arg1);
				}

				if (!std::strncmp(ss, "cexp(", 5)) { // Complex exponential
					_cimg_mp_op("Function 'cexp()'");
					arg1 = compile(ss5, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 0, 2, 2);
					pos = vector(2);
					CImg<ulongT>::vector((ulongT)mp_complex_exp, pos, arg1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "clog(", 5)) { // Complex logarithm
					_cimg_mp_op("Function 'clog()'");
					arg1 = compile(ss5, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 0, 2, 2);
					pos = vector(2);
					CImg<ulongT>::vector((ulongT)mp_complex_log, pos, arg1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "continue(", 9)) { // Complex absolute value
					if (pexpr[se2 - expr._data] == '(') { // no arguments?
						CImg<ulongT>::vector((ulongT)mp_continue, _cimg_mp_slot_nan).move_to(code);
						_cimg_mp_return_nan();
					}
				}

				if (!std::strncmp(ss, "copy(", 5)) { // Memory copy
					_cimg_mp_op("Function 'copy()'");
					ref.assign(14);
					s1 = ss5; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = p1 = compile(ss5, s1, depth1, ref);
					s2 = ++s1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					arg2 = compile(s1, s2, depth1, ref._data + 7);
					arg3 = ~0U; arg4 = arg5 = arg6 = 1;
					if (s2<se1) {
						s3 = ++s2; while (s3<se1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
						arg3 = compile(s2, s3, depth1, 0);
						if (s3<se1) {
							s1 = ++s3; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
							arg4 = compile(s3, s1, depth1, 0);
							if (s1<se1) {
								s2 = ++s1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
								arg5 = compile(s1, s2, depth1, 0);
								arg6 = s2<se1 ? compile(++s2, se1, depth1, 0) : 1;
							}
						}
					}
					if (_cimg_mp_is_vector(arg1) && !ref[0]) ++arg1;
					if (_cimg_mp_is_vector(arg2)) {
						if (arg3 == ~0U) arg3 = _cimg_mp_vector_size(arg2);
						if (!ref[7]) ++arg2;
					}
					if (arg3 == ~0U) arg3 = 1;
					_cimg_mp_check_type(arg3, 3, 1, 0);
					_cimg_mp_check_type(arg4, 4, 1, 0);
					_cimg_mp_check_type(arg5, 5, 1, 0);
					_cimg_mp_check_type(arg6, 5, 1, 0);
					CImg<ulongT>(1, 22).move_to(code);
					code.back().get_shared_rows(0, 7).fill((ulongT)mp_memcopy, p1, arg1, arg2, arg3, arg4, arg5, arg6);
					code.back().get_shared_rows(8, 21).fill(ref);
					_cimg_mp_return(p1);
				}

				if (!std::strncmp(ss, "cos(", 4)) { // Cosine
					_cimg_mp_op("Function 'cos()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_cos, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::cos(mem[arg1]));
					_cimg_mp_scalar1(mp_cos, arg1);
				}

				if (!std::strncmp(ss, "cosh(", 5)) { // Hyperbolic cosine
					_cimg_mp_op("Function 'cosh()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_cosh, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::cosh(mem[arg1]));
					_cimg_mp_scalar1(mp_cosh, arg1);
				}

				if (!std::strncmp(ss, "crop(", 5)) { // Image crop
					_cimg_mp_op("Function 'crop()'");
					if (*ss5 == '#') { // Index specified
						s0 = ss6; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
						p1 = compile(ss6, s0++, depth1, 0);
						_cimg_mp_check_list(false);
					}
					else { p1 = ~0U; s0 = ss5; need_input_copy = true; }
					pos = 0;
					is_sth = false; // Coordinates specified as a vector?
					if (ss5<se1) for (s = s0; s<se; ++s, ++pos) {
						ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
							(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
						arg1 = compile(s, ns, depth1, 0);
						if (!pos && _cimg_mp_is_vector(arg1)) { // Coordinates specified as a vector
							opcode = CImg<ulongT>::sequence(_cimg_mp_vector_size(arg1), arg1 + 1,
								arg1 + (ulongT)_cimg_mp_vector_size(arg1));
							opcode.resize(1, std::min(opcode._height, 4U), 1, 1, 0).move_to(_opcode);
							is_sth = true;
						}
						else {
							_cimg_mp_check_type(arg1, pos + 1, 1, 0);
							CImg<ulongT>::vector(arg1).move_to(_opcode);
						}
						s = ns;
					}
					(_opcode>'y').move_to(opcode);

					arg1 = 0; arg2 = (p1 != ~0U);
					switch (opcode._height) {
					case 0: case 1:
						CImg<ulongT>::vector(0, 0, 0, 0, ~0U, ~0U, ~0U, ~0U, 0).move_to(opcode);
						break;
					case 2:
						CImg<ulongT>::vector(*opcode, 0, 0, 0, opcode[1], ~0U, ~0U, ~0U, _cimg_mp_boundary).move_to(opcode);
						arg1 = arg2 ? 3 : 2;
						break;
					case 3:
						CImg<ulongT>::vector(*opcode, 0, 0, 0, opcode[1], ~0U, ~0U, ~0U, opcode[2]).move_to(opcode);
						arg1 = arg2 ? 3 : 2;
						break;
					case 4:
						CImg<ulongT>::vector(*opcode, opcode[1], 0, 0, opcode[2], opcode[3], ~0U, ~0U, _cimg_mp_boundary).
							move_to(opcode);
						arg1 = (is_sth ? 2 : 1) + arg2;
						break;
					case 5:
						CImg<ulongT>::vector(*opcode, opcode[1], 0, 0, opcode[2], opcode[3], ~0U, ~0U, opcode[4]).
							move_to(opcode);
						arg1 = (is_sth ? 2 : 1) + arg2;
						break;
					case 6:
						CImg<ulongT>::vector(*opcode, opcode[1], opcode[2], 0, opcode[3], opcode[4], opcode[5], ~0U,
							_cimg_mp_boundary).move_to(opcode);
						arg1 = (is_sth ? 2 : 4) + arg2;
						break;
					case 7:
						CImg<ulongT>::vector(*opcode, opcode[1], opcode[2], 0, opcode[3], opcode[4], opcode[5], ~0U,
							opcode[6]).move_to(opcode);
						arg1 = (is_sth ? 2 : 4) + arg2;
						break;
					case 8:
						CImg<ulongT>::vector(*opcode, opcode[1], opcode[2], opcode[3], opcode[4], opcode[5], opcode[6],
							opcode[7], _cimg_mp_boundary).move_to(opcode);
						arg1 = (is_sth ? 2 : 5) + arg2;
						break;
					case 9:
						arg1 = (is_sth ? 2 : 5) + arg2;
						break;
					default: // Error -> too much arguments
						*se = saved_char;
						s0 = ss - 4>expr._data ? ss - 4 : expr._data;
						cimg::strellipsize(s0, 64);
						throw CImgArgumentException("[" cimg_appname "_math_parser] "
							"CImg<%s>::%s: %s: Too much arguments specified, "
							"in expression '%s%s%s'.",
							pixel_type(), _cimg_mp_calling_function, s_op,
							s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
					}

					_cimg_mp_check_type((unsigned int)*opcode, arg2 + 1, 1, 0);
					_cimg_mp_check_type((unsigned int)opcode[1], arg2 + 1 + (is_sth ? 0 : 1), 1, 0);
					_cimg_mp_check_type((unsigned int)opcode[2], arg2 + 1 + (is_sth ? 0 : 2), 1, 0);
					_cimg_mp_check_type((unsigned int)opcode[3], arg2 + 1 + (is_sth ? 0 : 3), 1, 0);
					if (opcode[4] != (ulongT)~0U) {
						_cimg_mp_check_constant((unsigned int)opcode[4], arg1, 3);
						opcode[4] = (ulongT)mem[opcode[4]];
					}
					if (opcode[5] != (ulongT)~0U) {
						_cimg_mp_check_constant((unsigned int)opcode[5], arg1 + 1, 3);
						opcode[5] = (ulongT)mem[opcode[5]];
					}
					if (opcode[6] != (ulongT)~0U) {
						_cimg_mp_check_constant((unsigned int)opcode[6], arg1 + 2, 3);
						opcode[6] = (ulongT)mem[opcode[6]];
					}
					if (opcode[7] != (ulongT)~0U) {
						_cimg_mp_check_constant((unsigned int)opcode[7], arg1 + 3, 3);
						opcode[7] = (ulongT)mem[opcode[7]];
					}
					_cimg_mp_check_type((unsigned int)opcode[8], arg1 + 4, 1, 0);

					if (opcode[4] == (ulongT)~0U || opcode[5] == (ulongT)~0U ||
						opcode[6] == (ulongT)~0U || opcode[7] == (ulongT)~0U) {
						if (p1 != ~0U) {
							_cimg_mp_check_constant(p1, 1, 1);
							p1 = (unsigned int)cimg::mod((int)mem[p1], listin.width());
						}
						const CImg<T> &img = p1 != ~0U ? listin[p1] : imgin;
						if (!img) {
							*se = saved_char;
							s0 = ss - 4>expr._data ? ss - 4 : expr._data;
							cimg::strellipsize(s0, 64);
							throw CImgArgumentException("[" cimg_appname "_math_parser] "
								"CImg<%s>::%s: %s: Cannot crop empty image when "
								"some xyzc-coordinates are unspecified, in expression '%s%s%s'.",
								pixel_type(), _cimg_mp_calling_function, s_op,
								s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
						}
						if (opcode[4] == (ulongT)~0U) opcode[4] = (ulongT)img._width;
						if (opcode[5] == (ulongT)~0U) opcode[5] = (ulongT)img._height;
						if (opcode[6] == (ulongT)~0U) opcode[6] = (ulongT)img._depth;
						if (opcode[7] == (ulongT)~0U) opcode[7] = (ulongT)img._spectrum;
					}

					pos = vector((unsigned int)(opcode[4] * opcode[5] * opcode[6] * opcode[7]));
					CImg<ulongT>::vector((ulongT)mp_image_crop,
						pos, p1,
						*opcode, opcode[1], opcode[2], opcode[3],
						opcode[4], opcode[5], opcode[6], opcode[7],
						opcode[8]).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "cross(", 6)) { // Cross product
					_cimg_mp_op("Function 'cross()'");
					s1 = ss6; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss6, s1, depth1, 0);
					arg2 = compile(++s1, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 1, 2, 3);
					_cimg_mp_check_type(arg2, 2, 2, 3);
					pos = vector(3);
					CImg<ulongT>::vector((ulongT)mp_cross, pos, arg1, arg2).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "cut(", 4)) { // Cut
					_cimg_mp_op("Function 'cut()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss4, s1, depth1, 0);
					s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					arg2 = compile(++s1, s2, depth1, 0);
					arg3 = compile(++s2, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector3_vss(mp_cut, arg1, arg2, arg3);
					if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2) && _cimg_mp_is_constant(arg3)) {
						val = mem[arg1];
						val1 = mem[arg2];
						val2 = mem[arg3];
						_cimg_mp_constant(val<val1 ? val1 : val>val2 ? val2 : val);
					}
					_cimg_mp_scalar3(mp_cut, arg1, arg2, arg3);
				}
				break;

			case 'd':
				if (*ss1 == '(') { // Image depth
					_cimg_mp_op("Function 'd()'");
					if (*ss2 == '#') { p1 = compile(ss3, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss2 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)mp_image_d, pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "date(", 5)) { // Date and file date
					_cimg_mp_op("Function 'date()'");
					s1 = ss5; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = 0;
					is_sth = s1 != se1; // is_fdate
					if (s1 == se1 && ss5 != se1 && // Exactly one argument
						(cimg_sscanf(ss5, "%u%c", &arg1, &sep) != 2 || sep != ')')) is_sth = true;
					if (is_sth) {
						if (cimg_sscanf(ss5, "%u%c", &arg1, &sep) != 2 || sep != ',') { arg1 = 0; s1 = ss4; }
						*se1 = 0; val = (double)cimg::fdate(++s1, arg1); *se1 = ')';
					}
					else val = (double)cimg::date(arg1);
					_cimg_mp_constant(val);
				}

				if (!std::strncmp(ss, "debug(", 6)) { // Print debug info
					_cimg_mp_op("Function 'debug()'");
					p1 = code._width;
					arg1 = compile(ss6, se1, depth1, p_ref);
					*se1 = 0;
					variable_name.assign(CImg<charT>::string(ss6, true, true).unroll('y'), true);
					cimg::strpare(variable_name, false, true);
					((CImg<ulongT>::vector((ulongT)mp_debug, arg1, 0, code._width - p1),
						variable_name)>'y').move_to(opcode);
					opcode[2] = opcode._height;
					opcode.move_to(code, p1);
					*se1 = ')';
					_cimg_mp_return(arg1);
				}

				if (!std::strncmp(ss, "display(", 8)) { // Display memory, vector or image
					_cimg_mp_op("Function 'display()'");
					if (pexpr[se2 - expr._data] == '(') { // no arguments?
						CImg<ulongT>::vector((ulongT)mp_display_memory, _cimg_mp_slot_nan).move_to(code);
						_cimg_mp_return_nan();
					}
					if (*ss8 != '#') { // Vector
						s1 = ss8; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
						arg1 = compile(ss8, s1, depth1, 0);
						arg2 = 0; arg3 = arg4 = arg5 = 1;
						if (s1<se1) {
							s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
							arg2 = compile(s1 + 1, s2, depth1, 0);
							if (s2<se1) {
								s3 = ++s2; while (s3<se1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
								arg3 = compile(s2, s3, depth1, 0);
								if (s3<se1) {
									s2 = ++s3; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
									arg4 = compile(s3, s2, depth1, 0);
									arg5 = s2<se1 ? compile(++s2, se1, depth1, 0) : 0;
								}
							}
						}
						_cimg_mp_check_type(arg2, 2, 1, 0);
						_cimg_mp_check_type(arg3, 3, 1, 0);
						_cimg_mp_check_type(arg4, 4, 1, 0);
						_cimg_mp_check_type(arg5, 5, 1, 0);

						c1 = *s1; *s1 = 0;
						variable_name.assign(CImg<charT>::string(ss8, true, true).unroll('y'), true);
						cimg::strpare(variable_name, false, true);
						if (_cimg_mp_is_vector(arg1))
							((CImg<ulongT>::vector((ulongT)mp_vector_print, arg1, 0, (ulongT)_cimg_mp_vector_size(arg1), 0),
								variable_name)>'y').move_to(opcode);
						else
							((CImg<ulongT>::vector((ulongT)mp_print, arg1, 0, 0),
								variable_name)>'y').move_to(opcode);
						opcode[2] = opcode._height;
						opcode.move_to(code);

						((CImg<ulongT>::vector((ulongT)mp_display, arg1, 0, (ulongT)_cimg_mp_vector_size(arg1),
							arg2, arg3, arg4, arg5),
							variable_name)>'y').move_to(opcode);
						opcode[2] = opcode._height;
						opcode.move_to(code);
						*s1 = c1;
						_cimg_mp_return(arg1);

					}
					else { // Image
						p1 = compile(ss8 + 1, se1, depth1, 0);
						_cimg_mp_check_list(true);
						CImg<ulongT>::vector((ulongT)mp_image_display, _cimg_mp_slot_nan, p1).move_to(code);
						_cimg_mp_return_nan();
					}
				}

				if (!std::strncmp(ss, "det(", 4)) { // Matrix determinant
					_cimg_mp_op("Function 'det()'");
					arg1 = compile(ss4, se1, depth1, 0);
					_cimg_mp_check_matrix_square(arg1, 1);
					p1 = (unsigned int)std::sqrt((float)_cimg_mp_vector_size(arg1));
					_cimg_mp_scalar2(mp_det, arg1, p1);
				}

				if (!std::strncmp(ss, "diag(", 5)) { // Diagonal matrix
					_cimg_mp_op("Function 'diag()'");
					arg1 = compile(ss5, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 1, 2, 0);
					p1 = _cimg_mp_vector_size(arg1);
					pos = vector(p1*p1);
					CImg<ulongT>::vector((ulongT)mp_diag, pos, arg1, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "dot(", 4)) { // Dot product
					_cimg_mp_op("Function 'dot()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss4, s1, depth1, 0);
					arg2 = compile(++s1, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 1, 2, 0);
					_cimg_mp_check_type(arg2, 2, 2, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_scalar3(mp_dot, arg1, arg2, _cimg_mp_vector_size(arg1));
					_cimg_mp_scalar2(mp_mul, arg1, arg2);
				}

				if (!std::strncmp(ss, "do(", 3) || !std::strncmp(ss, "dowhile(", 8)) { // Do..while
					_cimg_mp_op("Function 'dowhile()'");
					s0 = *ss2 == '(' ? ss3 : ss8;
					s1 = s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = code._width;
					arg6 = mempos;
					p1 = compile(s0, s1, depth1, 0); // Body
					arg2 = code._width;
					p2 = s1<se1 ? compile(++s1, se1, depth1, 0) : p1; // Condition
					_cimg_mp_check_type(p2, 2, 1, 0);
					CImg<ulongT>::vector((ulongT)mp_dowhile, p1, p2, arg2 - arg1, code._width - arg2, _cimg_mp_vector_size(p1),
						p1 >= arg6 && !_cimg_mp_is_constant(p1),
						p2 >= arg6 && !_cimg_mp_is_constant(p2)).move_to(code, arg1);
					_cimg_mp_return(p1);
				}

				if (!std::strncmp(ss, "draw(", 5)) { // Draw image
					is_parallelizable = false;
					_cimg_mp_op("Function 'draw()'");
					if (*ss5 == '#') { // Index specified
						s0 = ss6; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
						p1 = compile(ss6, s0++, depth1, 0);
						_cimg_mp_check_list(true);
					}
					else { p1 = ~0U; s0 = ss5; }
					s1 = s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(s0, s1, depth1, 0);
					arg2 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_x;
					arg3 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_y;
					arg4 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_z;
					arg5 = is_relative ? 0U : (unsigned int)_cimg_mp_slot_c;
					s0 = se1;
					if (s1<se1) {
						s0 = s1 + 1; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
						arg2 = compile(++s1, s0, depth1, 0);
						if (_cimg_mp_is_vector(arg2)) { // Coordinates specified as a vector
							p2 = _cimg_mp_vector_size(arg2);
							++arg2;
							if (p2>1) {
								arg3 = arg2 + 1;
								if (p2>2) {
									arg4 = arg3 + 1;
									if (p2>3) arg5 = arg4 + 1;
								}
							}
							++s0;
							is_sth = true;
						}
						else {
							if (s0<se1) {
								is_sth = p1 != ~0U;
								s1 = s0 + 1; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
								arg3 = compile(++s0, s1, depth1, 0);
								_cimg_mp_check_type(arg3, is_sth ? 4 : 3, 1, 0);
								if (s1<se1) {
									s0 = s1 + 1; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
									arg4 = compile(++s1, s0, depth1, 0);
									_cimg_mp_check_type(arg4, is_sth ? 5 : 4, 1, 0);
									if (s0<se1) {
										s1 = s0 + 1; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
										arg5 = compile(++s0, s1, depth1, 0);
										_cimg_mp_check_type(arg5, is_sth ? 6 : 5, 1, 0);
										s0 = ++s1;
									}
								}
							}
							is_sth = false;
						}
					}

					CImg<ulongT>::vector((ulongT)mp_image_draw, arg1, (ulongT)_cimg_mp_vector_size(arg1), p1, arg2, arg3, arg4, arg5,
						0, 0, 0, 0, 1, (ulongT)~0U, 0, 1).move_to(opcode);

					arg2 = arg3 = arg4 = arg5 = ~0U;
					p2 = p1 != ~0U ? 0 : 1;
					if (s0<se1) {
						s1 = s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
						arg2 = compile(s0, s1, depth1, 0);
						_cimg_mp_check_type(arg2, p2 + (is_sth ? 3 : 6), 1, 0);
						if (s1<se1) {
							s0 = s1 + 1; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
							arg3 = compile(++s1, s0, depth1, 0);
							_cimg_mp_check_type(arg3, p2 + (is_sth ? 4 : 7), 1, 0);
							if (s0<se1) {
								s1 = s0 + 1; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
								arg4 = compile(++s0, s1, depth1, 0);
								_cimg_mp_check_type(arg4, p2 + (is_sth ? 5 : 8), 1, 0);
								if (s1<se1) {
									s0 = s1 + 1; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
									arg5 = compile(++s1, s0, depth1, 0);
									_cimg_mp_check_type(arg5, p2 + (is_sth ? 6 : 9), 1, 0);
								}
							}
						}
					}
					if (s0<s1) s0 = s1;

					opcode[8] = (ulongT)arg2;
					opcode[9] = (ulongT)arg3;
					opcode[10] = (ulongT)arg4;
					opcode[11] = (ulongT)arg5;

					if (s0<se1) {
						s1 = s0 + 1; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
						arg6 = compile(++s0, s1, depth1, 0);
						_cimg_mp_check_type(arg6, 0, 1, 0);
						opcode[12] = arg6;
						if (s1<se1) {
							s0 = s1 + 1; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
							p2 = compile(++s1, s0, depth1, 0);
							_cimg_mp_check_type(p2, 0, 2, 0);
							opcode[13] = p2;
							opcode[14] = _cimg_mp_vector_size(p2);
							p3 = s0<se1 ? compile(++s0, se1, depth1, 0) : 1;
							_cimg_mp_check_type(p3, 0, 1, 0);
							opcode[15] = p3;
						}
					}
					opcode.move_to(code);
					_cimg_mp_return(arg1);
				}
				break;

			case 'e':
				if (!std::strncmp(ss, "eig(", 4)) { // Matrix eigenvalues/eigenvector
					_cimg_mp_op("Function 'eig()'");
					arg1 = compile(ss4, se1, depth1, 0);
					_cimg_mp_check_matrix_square(arg1, 1);
					p1 = (unsigned int)std::sqrt((float)_cimg_mp_vector_size(arg1));
					pos = vector((p1 + 1)*p1);
					CImg<ulongT>::vector((ulongT)mp_matrix_eig, pos, arg1, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "end(", 4)) { // End
					_cimg_mp_op("Function 'end()'");
					code.swap(code_end);
					compile(ss4, se1, depth1, p_ref);
					code.swap(code_end);
					_cimg_mp_return_nan();
				}

				if (!std::strncmp(ss, "exp(", 4)) { // Exponential
					_cimg_mp_op("Function 'exp()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_exp, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::exp(mem[arg1]));
					_cimg_mp_scalar1(mp_exp, arg1);
				}

				if (!std::strncmp(ss, "eye(", 4)) { // Identity matrix
					_cimg_mp_op("Function 'eye()'");
					arg1 = compile(ss4, se1, depth1, 0);
					_cimg_mp_check_constant(arg1, 1, 3);
					p1 = (unsigned int)mem[arg1];
					pos = vector(p1*p1);
					CImg<ulongT>::vector((ulongT)mp_eye, pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}
				break;

			case 'f':
				if (!std::strncmp(ss, "fact(", 5)) { // Factorial
					_cimg_mp_op("Function 'fact()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_factorial, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::factorial(mem[arg1]));
					_cimg_mp_scalar1(mp_factorial, arg1);
				}

				if (!std::strncmp(ss, "fibo(", 5)) { // Fibonacci
					_cimg_mp_op("Function 'fibo()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_fibonacci, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::fibonacci(mem[arg1]));
					_cimg_mp_scalar1(mp_fibonacci, arg1);
				}

				if (!std::strncmp(ss, "find(", 5)) { // Find
					_cimg_mp_op("Function 'find()'");

					// First argument: data to look at.
					s0 = ss5; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
					if (*ss5 == '#') { // Index specified
						p1 = compile(ss6, s0, depth1, 0);
						_cimg_mp_check_list(false);
						arg1 = ~0U;
					}
					else { // Vector specified
						arg1 = compile(ss5, s0, depth1, 0);
						_cimg_mp_check_type(arg1, 1, 2, 0);
						p1 = ~0U;
					}

					// Second argument: data to find.
					s1 = ++s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg2 = compile(s0, s1, depth1, 0);

					// Third and fourth arguments: search direction and starting index.
					arg3 = 1; arg4 = _cimg_mp_slot_nan;
					if (s1<se1) {
						s0 = s1 + 1; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
						arg3 = compile(++s1, s0, depth1, 0);
						_cimg_mp_check_type(arg3, 3, 1, 0);
						if (s0<se1) {
							arg4 = compile(++s0, se1, depth1, 0);
							_cimg_mp_check_type(arg4, 4, 1, 0);
						}
					}
					if (p1 != ~0U) {
						if (_cimg_mp_is_vector(arg2))
							_cimg_mp_scalar5(mp_list_find_seq, p1, arg2, _cimg_mp_vector_size(arg2), arg3, arg4);
						_cimg_mp_scalar4(mp_list_find, p1, arg2, arg3, arg4);
					}
					if (_cimg_mp_is_vector(arg2))
						_cimg_mp_scalar6(mp_find_seq, arg1, _cimg_mp_vector_size(arg1), arg2, _cimg_mp_vector_size(arg2), arg3, arg4);
					_cimg_mp_scalar5(mp_find, arg1, _cimg_mp_vector_size(arg1), arg2, arg3, arg4);
				}

				if (*ss1 == 'o' && *ss2 == 'r' && *ss3 == '(') { // For loop
					_cimg_mp_op("Function 'for()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					s3 = s2 + 1; while (s3<se1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
					arg1 = code._width;
					p1 = compile(ss4, s1, depth1, 0); // Init
					arg2 = code._width;
					p2 = compile(++s1, s2, depth1, 0); // Cond
					arg3 = code._width;
					arg6 = mempos;
					if (s3<se1) { // Body + post
						p3 = compile(s3 + 1, se1, depth1, 0); // Body
						arg4 = code._width;
						pos = compile(++s2, s3, depth1, 0); // Post
					}
					else {
						p3 = compile(++s2, se1, depth1, 0); // Body only
						arg4 = pos = code._width;
					}
					_cimg_mp_check_type(p2, 2, 1, 0);
					arg5 = _cimg_mp_vector_size(pos);
					CImg<ulongT>::vector((ulongT)mp_for, p3, (ulongT)_cimg_mp_vector_size(p3), p2, arg2 - arg1, arg3 - arg2,
						arg4 - arg3, code._width - arg4,
						p3 >= arg6 && !_cimg_mp_is_constant(p3),
						p2 >= arg6 && !_cimg_mp_is_constant(p2)).move_to(code, arg1);
					_cimg_mp_return(p3);
				}

				if (!std::strncmp(ss, "floor(", 6)) { // Floor
					_cimg_mp_op("Function 'floor()'");
					arg1 = compile(ss6, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_floor, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::floor(mem[arg1]));
					_cimg_mp_scalar1(mp_floor, arg1);
				}
				break;

			case 'g':
				if (!std::strncmp(ss, "gauss(", 6)) { // Gaussian function
					_cimg_mp_op("Function 'gauss()'");
					s1 = ss6; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss6, s1, depth1, 0);
					arg2 = s1<se1 ? compile(++s1, se1, depth1, 0) : 1;
					_cimg_mp_check_type(arg2, 2, 1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector2_vs(mp_gauss, arg1, arg2);
					if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2)) {
						val1 = mem[arg1];
						val2 = mem[arg2];
						_cimg_mp_constant(std::exp(-val1*val1 / (2 * val2*val2)) / std::sqrt(2 * val2*val2*cimg::PI));
					}
					_cimg_mp_scalar2(mp_gauss, arg1, arg2);
				}
				break;

			case 'h':
				if (*ss1 == '(') { // Image height
					_cimg_mp_op("Function 'h()'");
					if (*ss2 == '#') { p1 = compile(ss3, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss2 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)mp_image_h, pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

			case 'i':
				if ((*ss1 == 'm' || *ss1 == 'M' || *ss1 == 'a' || *ss1 == 'v' || *ss1 == 's' || *ss1 == 'p' || *ss1 == 'c') &&
					*ss2 == '(') { // im(), iM(), ia(), iv(), is(), ip(), ic()
					_cimg_mp_op(*ss1 == 'm' ? "Function 'im()'" :
						*ss1 == 'M' ? "Function 'iM()'" :
						*ss1 == 'a' ? "Function 'ia()'" :
						*ss1 == 'v' ? "Function 'iv()'" :
						*ss1 == 's' ? "Function 'is()'" :
						*ss1 == 'p' ? "Function 'ip()'" :
						"Function 'ic()");
					if (*ss3 == '#') { p1 = compile(ss4, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss3 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)(*ss1 == 'm' ? mp_image_im :
						*ss1 == 'M' ? mp_image_iM :
						*ss1 == 'a' ? mp_image_ia :
						*ss1 == 'v' ? mp_image_iv :
						*ss1 == 's' ? mp_image_is :
						*ss1 == 'p' ? mp_image_ip :
						mp_image_ic), pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (*ss1 == 'f' && *ss2 == '(') { // If..then[..else.]
					_cimg_mp_op("Function 'if()'");
					s1 = ss3; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					arg1 = compile(ss3, s1, depth1, 0);
					_cimg_mp_check_type(arg1, 1, 1, 0);
					if (_cimg_mp_is_constant(arg1)) {
						if ((bool)mem[arg1]) return compile(++s1, s2, depth1, 0);
						else return s2<se1 ? compile(++s2, se1, depth1, 0) : 0;
					}
					p2 = code._width;
					arg2 = compile(++s1, s2, depth1, 0);
					p3 = code._width;
					arg3 = s2<se1 ? compile(++s2, se1, depth1, 0) : _cimg_mp_is_vector(arg2) ? vector(_cimg_mp_vector_size(arg2), 0) : 0;
					_cimg_mp_check_type(arg3, 3, _cimg_mp_is_vector(arg2) ? 2 : 1, _cimg_mp_vector_size(arg2));
					arg4 = _cimg_mp_vector_size(arg2);
					if (arg4) pos = vector(arg4); else pos = scalar();
					CImg<ulongT>::vector((ulongT)mp_if, pos, arg1, arg2, arg3,
						p3 - p2, code._width - p3, arg4).move_to(code, p2);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "init(", 5)) { // Init
					_cimg_mp_op("Function 'init()'");
					code.swap(code_init);
					arg1 = compile(ss5, se1, depth1, p_ref);
					code.swap(code_init);
					_cimg_mp_return(arg1);
				}

				if (!std::strncmp(ss, "int(", 4)) { // Integer cast
					_cimg_mp_op("Function 'int()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_int, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant((longT)mem[arg1]);
					_cimg_mp_scalar1(mp_int, arg1);
				}

				if (!std::strncmp(ss, "inv(", 4)) { // Matrix/scalar inversion
					_cimg_mp_op("Function 'inv()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) {
						_cimg_mp_check_matrix_square(arg1, 1);
						p1 = (unsigned int)std::sqrt((float)_cimg_mp_vector_size(arg1));
						pos = vector(p1*p1);
						CImg<ulongT>::vector((ulongT)mp_matrix_inv, pos, arg1, p1).move_to(code);
						_cimg_mp_return(pos);
					}
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(1 / mem[arg1]);
					_cimg_mp_scalar2(mp_div, 1, arg1);
				}

				if (*ss1 == 's') { // Family of 'is_?()' functions

					if (!std::strncmp(ss, "isbool(", 7)) { // Is boolean?
						_cimg_mp_op("Function 'isbool()'");
						if (ss7 == se1) _cimg_mp_return(0);
						arg1 = compile(ss7, se1, depth1, 0);
						if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_isbool, arg1);
						if (_cimg_mp_is_constant(arg1)) _cimg_mp_return(mem[arg1] == 0.0 || mem[arg1] == 1.0);
						_cimg_mp_scalar1(mp_isbool, arg1);
					}

					if (!std::strncmp(ss, "isdir(", 6)) { // Is directory?
						_cimg_mp_op("Function 'isdir()'");
						*se1 = 0;
						is_sth = cimg::is_directory(ss6);
						*se1 = ')';
						_cimg_mp_return(is_sth ? 1U : 0U);
					}

					if (!std::strncmp(ss, "isfile(", 7)) { // Is file?
						_cimg_mp_op("Function 'isfile()'");
						*se1 = 0;
						is_sth = cimg::is_file(ss7);
						*se1 = ')';
						_cimg_mp_return(is_sth ? 1U : 0U);
					}

					if (!std::strncmp(ss, "isin(", 5)) { // Is in sequence/vector?
						if (ss5 >= se1) _cimg_mp_return(0);
						_cimg_mp_op("Function 'isin()'");
						pos = scalar();
						CImg<ulongT>::vector((ulongT)mp_isin, pos, 0).move_to(_opcode);
						for (s = ss5; s<se; ++s) {
							ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
								(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
							arg1 = compile(s, ns, depth1, 0);
							if (_cimg_mp_is_vector(arg1))
								CImg<ulongT>::sequence(_cimg_mp_vector_size(arg1), arg1 + 1,
									arg1 + (ulongT)_cimg_mp_vector_size(arg1)).
								move_to(_opcode);
							else CImg<ulongT>::vector(arg1).move_to(_opcode);
							s = ns;
						}
						(_opcode>'y').move_to(opcode);
						opcode[2] = opcode._height;
						opcode.move_to(code);
						_cimg_mp_return(pos);
					}

					if (!std::strncmp(ss, "isinf(", 6)) { // Is infinite?
						_cimg_mp_op("Function 'isinf()'");
						if (ss6 == se1) _cimg_mp_return(0);
						arg1 = compile(ss6, se1, depth1, 0);
						if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_isinf, arg1);
						if (_cimg_mp_is_constant(arg1)) _cimg_mp_return((unsigned int)cimg::type<double>::is_inf(mem[arg1]));
						_cimg_mp_scalar1(mp_isinf, arg1);
					}

					if (!std::strncmp(ss, "isint(", 6)) { // Is integer?
						_cimg_mp_op("Function 'isint()'");
						if (ss6 == se1) _cimg_mp_return(0);
						arg1 = compile(ss6, se1, depth1, 0);
						if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_isint, arg1);
						if (_cimg_mp_is_constant(arg1)) _cimg_mp_return((unsigned int)(cimg::mod(mem[arg1], 1.0) == 0));
						_cimg_mp_scalar1(mp_isint, arg1);
					}

					if (!std::strncmp(ss, "isnan(", 6)) { // Is NaN?
						_cimg_mp_op("Function 'isnan()'");
						if (ss6 == se1) _cimg_mp_return(0);
						arg1 = compile(ss6, se1, depth1, 0);
						if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_isnan, arg1);
						if (_cimg_mp_is_constant(arg1)) _cimg_mp_return((unsigned int)cimg::type<double>::is_nan(mem[arg1]));
						_cimg_mp_scalar1(mp_isnan, arg1);
					}

					if (!std::strncmp(ss, "isval(", 6)) { // Is value?
						_cimg_mp_op("Function 'isval()'");
						val = 0;
						if (cimg_sscanf(ss6, "%lf%c%c", &val, &sep, &end) == 2 && sep == ')') _cimg_mp_return(1);
						_cimg_mp_return(0);
					}

				}
				break;

			case 'l':
				if (!std::strncmp(ss, "log(", 4)) { // Natural logarithm
					_cimg_mp_op("Function 'log()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_log, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::log(mem[arg1]));
					_cimg_mp_scalar1(mp_log, arg1);
				}

				if (!std::strncmp(ss, "log2(", 5)) { // Base-2 logarithm
					_cimg_mp_op("Function 'log2()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_log2, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::log2(mem[arg1]));
					_cimg_mp_scalar1(mp_log2, arg1);
				}

				if (!std::strncmp(ss, "log10(", 6)) { // Base-10 logarithm
					_cimg_mp_op("Function 'log10()'");
					arg1 = compile(ss6, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_log10, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::log10(mem[arg1]));
					_cimg_mp_scalar1(mp_log10, arg1);
				}

				if (!std::strncmp(ss, "lowercase(", 10)) { // Lower case
					_cimg_mp_op("Function 'lowercase()'");
					arg1 = compile(ss + 10, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_lowercase, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::lowercase(mem[arg1]));
					_cimg_mp_scalar1(mp_lowercase, arg1);
				}
				break;

			case 'm':
				if (!std::strncmp(ss, "mul(", 4)) { // Matrix multiplication
					_cimg_mp_op("Function 'mul()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss4, s1, depth1, 0);
					s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					arg2 = compile(++s1, s2, depth1, 0);
					arg3 = s2<se1 ? compile(++s2, se1, depth1, 0) : 1;
					_cimg_mp_check_type(arg1, 1, 2, 0);
					_cimg_mp_check_type(arg2, 2, 2, 0);
					_cimg_mp_check_constant(arg3, 3, 3);
					p1 = _cimg_mp_vector_size(arg1);
					p2 = _cimg_mp_vector_size(arg2);
					p3 = (unsigned int)mem[arg3];
					arg5 = p2 / p3;
					arg4 = p1 / arg5;
					if (arg4*arg5 != p1 || arg5*p3 != p2) {
						*se = saved_char;
						s0 = ss - 4>expr._data ? ss - 4 : expr._data;
						cimg::strellipsize(s0, 64);
						throw CImgArgumentException("[" cimg_appname "_math_parser] "
							"CImg<%s>::%s: %s: Types of first and second arguments ('%s' and '%s') "
							"do not match with third argument 'nb_colsB=%u', "
							"in expression '%s%s%s'.",
							pixel_type(), _cimg_mp_calling_function, s_op,
							s_type(arg1)._data, s_type(arg2)._data, p3,
							s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
					}
					pos = vector(arg4*p3);
					CImg<ulongT>::vector((ulongT)mp_matrix_mul, pos, arg1, arg2, arg4, arg5, p3).move_to(code);
					_cimg_mp_return(pos);
				}
				break;

			case 'n':
				if (!std::strncmp(ss, "narg(", 5)) { // Number of arguments
					_cimg_mp_op("Function 'narg()'");
					if (ss5 >= se1) _cimg_mp_return(0);
					arg1 = 0;
					for (s = ss5; s<se; ++s) {
						ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
							(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
						++arg1; s = ns;
					}
					_cimg_mp_constant(arg1);
				}

				if ((cimg_sscanf(ss, "norm%u%c", &(arg1 = ~0U), &sep) == 2 && sep == '(') ||
					!std::strncmp(ss, "norminf(", 8) || !std::strncmp(ss, "norm(", 5) ||
					(!std::strncmp(ss, "norm", 4) && ss5<se1 && (s = std::strchr(ss5, '(')) != 0)) { // Lp norm
					_cimg_mp_op("Function 'normP()'");
					if (*ss4 == '(') { arg1 = 2; s = ss5; }
					else if (*ss4 == 'i' && *ss5 == 'n' && *ss6 == 'f' && *ss7 == '(') { arg1 = ~0U; s = ss8; }
					else if (arg1 == ~0U) {
						arg1 = compile(ss4, s++, depth1, 0);
						_cimg_mp_check_constant(arg1, 0, 2);
						arg1 = (unsigned int)mem[arg1];
					}
					else s = std::strchr(ss4, '(') + 1;
					pos = scalar();
					switch (arg1) {
					case 0:
						CImg<ulongT>::vector((ulongT)mp_norm0, pos, 0).move_to(_opcode); break;
					case 1:
						CImg<ulongT>::vector((ulongT)mp_norm1, pos, 0).move_to(_opcode); break;
					case 2:
						CImg<ulongT>::vector((ulongT)mp_norm2, pos, 0).move_to(_opcode); break;
					case ~0U:
						CImg<ulongT>::vector((ulongT)mp_norminf, pos, 0).move_to(_opcode); break;
					default:
						CImg<ulongT>::vector((ulongT)mp_normp, pos, 0, (ulongT)(arg1 == ~0U ? -1 : (int)arg1)).
							move_to(_opcode);
					}
					for (; s<se; ++s) {
						ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
							(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
						arg2 = compile(s, ns, depth1, 0);
						if (_cimg_mp_is_vector(arg2))
							CImg<ulongT>::sequence(_cimg_mp_vector_size(arg2), arg2 + 1,
								arg2 + (ulongT)_cimg_mp_vector_size(arg2)).
							move_to(_opcode);
						else CImg<ulongT>::vector(arg2).move_to(_opcode);
						s = ns;
					}

					(_opcode>'y').move_to(opcode);
					if (arg1>0 && opcode._height == 4) // Special case with one argument and p>=1
						_cimg_mp_scalar1(mp_abs, opcode[3]);
					opcode[2] = opcode._height;
					opcode.move_to(code);
					_cimg_mp_return(pos);
				}
				break;

			case 'p':
				if (!std::strncmp(ss, "permut(", 7)) { // Number of permutations
					_cimg_mp_op("Function 'permut()'");
					s1 = ss7; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					arg1 = compile(ss7, s1, depth1, 0);
					arg2 = compile(++s1, s2, depth1, 0);
					arg3 = compile(++s2, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 1, 1, 0);
					_cimg_mp_check_type(arg2, 2, 1, 0);
					_cimg_mp_check_type(arg3, 3, 1, 0);
					if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2) && _cimg_mp_is_constant(arg3))
						_cimg_mp_constant(cimg::permutations(mem[arg1], mem[arg2], (bool)mem[arg3]));
					_cimg_mp_scalar3(mp_permutations, arg1, arg2, arg3);
				}

				if (!std::strncmp(ss, "pseudoinv(", 10)) { // Matrix/scalar pseudo-inversion
					_cimg_mp_op("Function 'pseudoinv()'");
					s1 = ss + 10; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss + 10, s1, depth1, 0);
					arg2 = s1<se1 ? compile(++s1, se1, depth1, 0) : 1;
					_cimg_mp_check_type(arg1, 1, 2, 0);
					_cimg_mp_check_constant(arg2, 2, 3);
					p1 = _cimg_mp_vector_size(arg1);
					p2 = (unsigned int)mem[arg2];
					p3 = p1 / p2;
					if (p3*p2 != p1) {
						*se = saved_char;
						s0 = ss - 4>expr._data ? ss - 4 : expr._data;
						cimg::strellipsize(s0, 64);
						throw CImgArgumentException("[" cimg_appname "_math_parser] "
							"CImg<%s>::%s: %s: Type of first argument ('%s') "
							"does not match with second argument 'nb_colsA=%u', "
							"in expression '%s%s%s'.",
							pixel_type(), _cimg_mp_calling_function, s_op,
							s_type(arg1)._data, p2,
							s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
					}
					pos = vector(p1);
					CImg<ulongT>::vector((ulongT)mp_matrix_pseudoinv, pos, arg1, p2, p3).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "print(", 6) || !std::strncmp(ss, "prints(", 7)) { // Print expressions
					is_sth = ss[5] == 's'; // is prints()
					_cimg_mp_op(is_sth ? "Function 'prints()'" : "Function 'print()'");
					s0 = is_sth ? ss7 : ss6;
					if (*s0 != '#' || is_sth) { // Regular expression
						for (s = s0; s<se; ++s) {
							ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
								(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
							pos = compile(s, ns, depth1, p_ref);
							c1 = *ns; *ns = 0;
							variable_name.assign(CImg<charT>::string(s, true, true).unroll('y'), true);
							cimg::strpare(variable_name, false, true);
							if (_cimg_mp_is_vector(pos)) // Vector
								((CImg<ulongT>::vector((ulongT)mp_vector_print, pos, 0, (ulongT)_cimg_mp_vector_size(pos), is_sth ? 1 : 0),
									variable_name)>'y').move_to(opcode);
							else // Scalar
								((CImg<ulongT>::vector((ulongT)mp_print, pos, 0, is_sth ? 1 : 0),
									variable_name)>'y').move_to(opcode);
							opcode[2] = opcode._height;
							opcode.move_to(code);
							*ns = c1; s = ns;
						}
						_cimg_mp_return(pos);
					}
					else { // Image
						p1 = compile(ss7, se1, depth1, 0);
						_cimg_mp_check_list(true);
						CImg<ulongT>::vector((ulongT)mp_image_print, _cimg_mp_slot_nan, p1).move_to(code);
						_cimg_mp_return_nan();
					}
				}
				break;

			case 'r':
				if (!std::strncmp(ss, "resize(", 7)) { // Vector or image resize
					_cimg_mp_op("Function 'resize()'");
					if (*ss7 != '#') { // Vector
						s1 = ss7; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
						arg1 = compile(ss7, s1, depth1, 0);
						s2 = ++s1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
						arg2 = compile(s1, s2, depth1, 0);
						arg3 = 1;
						arg4 = 0;
						if (s2<se1) {
							s1 = ++s2; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
							arg3 = compile(s2, s1, depth1, 0);
							arg4 = s1<se1 ? compile(++s1, se1, depth1, 0) : 0;
						}
						_cimg_mp_check_constant(arg2, 2, 3);
						arg2 = (unsigned int)mem[arg2];
						_cimg_mp_check_type(arg3, 3, 1, 0);
						_cimg_mp_check_type(arg4, 4, 1, 0);
						pos = vector(arg2);
						CImg<ulongT>::vector((ulongT)mp_vector_resize, pos, arg2, arg1, (ulongT)_cimg_mp_vector_size(arg1),
							arg3, arg4).move_to(code);
						_cimg_mp_return(pos);

					}
					else { // Image
						is_parallelizable = false;
						s0 = ss8; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
						p1 = compile(ss8, s0++, depth1, 0);
						_cimg_mp_check_list(true);
						CImg<ulongT>::vector((ulongT)mp_image_resize, _cimg_mp_slot_nan, p1, ~0U, ~0U, ~0U, ~0U, 1, 0, 0, 0, 0, 0).
							move_to(opcode);
						pos = 0;
						for (s = s0; s<se && pos<10; ++s) {
							ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
								(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
							arg1 = compile(s, ns, depth1, 0);
							_cimg_mp_check_type(arg1, pos + 2, 1, 0);
							opcode[pos + 3] = arg1;
							s = ns;
							++pos;
						}
						if (pos<1 || pos>10) {
							*se = saved_char;
							s0 = ss - 4>expr._data ? ss - 4 : expr._data;
							cimg::strellipsize(s0, 64);
							throw CImgArgumentException("[" cimg_appname "_math_parser] "
								"CImg<%s>::%s: %s: %s arguments, in expression '%s%s%s'.",
								pixel_type(), _cimg_mp_calling_function, s_op,
								pos<1 ? "Missing" : "Too much",
								s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
						}
						opcode.move_to(code);
						_cimg_mp_return_nan();
					}
				}

				if (!std::strncmp(ss, "reverse(", 8)) { // Vector reverse
					_cimg_mp_op("Function 'reverse()'");
					arg1 = compile(ss8, se1, depth1, 0);
					if (!_cimg_mp_is_vector(arg1)) _cimg_mp_return(arg1);
					p1 = _cimg_mp_vector_size(arg1);
					pos = vector(p1);
					CImg<ulongT>::vector((ulongT)mp_vector_reverse, pos, arg1, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "rol(", 4) || !std::strncmp(ss, "ror(", 4)) { // Bitwise rotation
					_cimg_mp_op(ss[2] == 'l' ? "Function 'rol()'" : "Function 'ror()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss4, s1, depth1, 0);
					arg2 = s1<se1 ? compile(++s1, se1, depth1, 0) : 1;
					_cimg_mp_check_type(arg2, 2, 1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector2_vs(*ss2 == 'l' ? mp_rol : mp_ror, arg1, arg2);
					if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
						_cimg_mp_constant(*ss2 == 'l' ? cimg::rol(mem[arg1], (unsigned int)mem[arg2]) :
							cimg::ror(mem[arg1], (unsigned int)mem[arg2]));
					_cimg_mp_scalar2(*ss2 == 'l' ? mp_rol : mp_ror, arg1, arg2);
				}

				if (!std::strncmp(ss, "rot(", 4)) { // 2d/3d rotation matrix
					_cimg_mp_op("Function 'rot()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss4, s1, depth1, 0);
					if (s1<se1) { // 3d rotation
						_cimg_mp_check_type(arg1, 1, 3, 3);
						is_sth = false; // Is coordinates as vector?
						if (_cimg_mp_is_vector(arg1)) { // Coordinates specified as a vector
							is_sth = true;
							p2 = _cimg_mp_vector_size(arg1);
							++arg1;
							arg2 = arg3 = 0;
							if (p2>1) {
								arg2 = arg1 + 1;
								if (p2>2) arg3 = arg2 + 1;
							}
							arg4 = compile(++s1, se1, depth1, 0);
						}
						else {
							s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
							arg2 = compile(++s1, s2, depth1, 0);
							s3 = s2 + 1; while (s3<se1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
							arg3 = compile(++s2, s3, depth1, 0);
							arg4 = compile(++s3, se1, depth1, 0);
							_cimg_mp_check_type(arg2, 2, 1, 0);
							_cimg_mp_check_type(arg3, 3, 1, 0);
						}
						_cimg_mp_check_type(arg4, is_sth ? 2 : 4, 1, 0);
						pos = vector(9);
						CImg<ulongT>::vector((ulongT)mp_rot3d, pos, arg1, arg2, arg3, arg4).move_to(code);
					}
					else { // 2d rotation
						_cimg_mp_check_type(arg1, 1, 1, 0);
						pos = vector(4);
						CImg<ulongT>::vector((ulongT)mp_rot2d, pos, arg1).move_to(code);
					}
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "round(", 6)) { // Value rounding
					_cimg_mp_op("Function 'round()'");
					s1 = ss6; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss6, s1, depth1, 0);
					arg2 = 1;
					arg3 = 0;
					if (s1<se1) {
						s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
						arg2 = compile(++s1, s2, depth1, 0);
						arg3 = s2<se1 ? compile(++s2, se1, depth1, 0) : 0;
					}
					_cimg_mp_check_type(arg2, 2, 1, 0);
					_cimg_mp_check_type(arg3, 3, 1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector3_vss(mp_round, arg1, arg2, arg3);
					if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2) && _cimg_mp_is_constant(arg3))
						_cimg_mp_constant(cimg::round(mem[arg1], mem[arg2], (int)mem[arg3]));
					_cimg_mp_scalar3(mp_round, arg1, arg2, arg3);
				}
				break;

			case 's':
				if (*ss1 == '(') { // Image spectrum
					_cimg_mp_op("Function 's()'");
					if (*ss2 == '#') { p1 = compile(ss3, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss2 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)mp_image_s, pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "same(", 5)) { // Test if operands have the same values
					_cimg_mp_op("Function 'same()'");
					s1 = ss5; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss5, s1, depth1, 0);
					s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					arg2 = compile(++s1, s2, depth1, 0);
					arg3 = 11;
					arg4 = 1;
					if (s2<se1) {
						s3 = s2 + 1; while (s3<se1 && (*s3 != ',' || level[s3 - expr._data] != clevel1)) ++s3;
						arg3 = compile(++s2, s3, depth1, 0);
						_cimg_mp_check_type(arg3, 3, 1, 0);
						arg4 = s3<se1 ? compile(++s3, se1, depth1, 0) : 1;
					}
					p1 = _cimg_mp_vector_size(arg1);
					p2 = _cimg_mp_vector_size(arg2);
					_cimg_mp_scalar6(mp_vector_eq, arg1, p1, arg2, p2, arg3, arg4);
				}

				if (!std::strncmp(ss, "shift(", 6)) { // Shift vector
					_cimg_mp_op("Function 'shift()'");
					s1 = ss6; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss6, s1, depth1, 0);
					arg2 = 1; arg3 = 0;
					if (s1<se1) {
						s0 = ++s1; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
						arg2 = compile(s1, s0, depth1, 0);
						arg3 = s0<se1 ? compile(++s0, se1, depth1, 0) : 0;
					}
					_cimg_mp_check_type(arg1, 1, 2, 0);
					_cimg_mp_check_type(arg2, 2, 1, 0);
					_cimg_mp_check_type(arg3, 3, 1, 0);
					p1 = _cimg_mp_vector_size(arg1);
					pos = vector(p1);
					CImg<ulongT>::vector((ulongT)mp_shift, pos, arg1, p1, arg2, arg3).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "sign(", 5)) { // Sign
					_cimg_mp_op("Function 'sign()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_sign, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::sign(mem[arg1]));
					_cimg_mp_scalar1(mp_sign, arg1);
				}

				if (!std::strncmp(ss, "sin(", 4)) { // Sine
					_cimg_mp_op("Function 'sin()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_sin, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::sin(mem[arg1]));
					_cimg_mp_scalar1(mp_sin, arg1);
				}

				if (!std::strncmp(ss, "sinc(", 5)) { // Sine cardinal
					_cimg_mp_op("Function 'sinc()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_sinc, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::sinc(mem[arg1]));
					_cimg_mp_scalar1(mp_sinc, arg1);
				}

				if (!std::strncmp(ss, "single(", 7)) { // Force single thread execution
					_cimg_mp_op("Function 'single()'");
					p1 = code._width;
					arg1 = compile(ss7, se1, depth1, p_ref);
					CImg<ulongT>::vector((ulongT)mp_single, arg1, code._width - p1).move_to(code, p1);
					_cimg_mp_return(arg1);
				}

				if (!std::strncmp(ss, "sinh(", 5)) { // Hyperbolic sine
					_cimg_mp_op("Function 'sinh()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_sinh, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::sinh(mem[arg1]));
					_cimg_mp_scalar1(mp_sinh, arg1);
				}

				if (!std::strncmp(ss, "size(", 5)) { // Vector size.
					_cimg_mp_op("Function 'size()'");
					arg1 = compile(ss5, se1, depth1, 0);
					_cimg_mp_constant(_cimg_mp_is_scalar(arg1) ? 0 : _cimg_mp_vector_size(arg1));
				}

				if (!std::strncmp(ss, "solve(", 6)) { // Solve linear system
					_cimg_mp_op("Function 'solve()'");
					s1 = ss6; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss6, s1, depth1, 0);
					s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
					arg2 = compile(++s1, s2, depth1, 0);
					arg3 = s2<se1 ? compile(++s2, se1, depth1, 0) : 1;
					_cimg_mp_check_type(arg1, 1, 2, 0);
					_cimg_mp_check_type(arg2, 2, 2, 0);
					_cimg_mp_check_constant(arg3, 3, 3);
					p1 = _cimg_mp_vector_size(arg1);
					p2 = _cimg_mp_vector_size(arg2);
					p3 = (unsigned int)mem[arg3];
					arg5 = p2 / p3;
					arg4 = p1 / arg5;
					if (arg4*arg5 != p1 || arg5*p3 != p2) {
						*se = saved_char;
						s0 = ss - 4>expr._data ? ss - 4 : expr._data;
						cimg::strellipsize(s0, 64);
						throw CImgArgumentException("[" cimg_appname "_math_parser] "
							"CImg<%s>::%s: %s: Types of first and second arguments ('%s' and '%s') "
							"do not match with third argument 'nb_colsB=%u', "
							"in expression '%s%s%s'.",
							pixel_type(), _cimg_mp_calling_function, s_op,
							s_type(arg1)._data, s_type(arg2)._data, p3,
							s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
					}
					pos = vector(arg4*p3);
					CImg<ulongT>::vector((ulongT)mp_solve, pos, arg1, arg2, arg4, arg5, p3).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "sort(", 5)) { // Sort vector
					_cimg_mp_op("Function 'sort()'");
					if (*ss5 != '#') { // Vector
						s1 = ss5; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
						arg1 = compile(ss5, s1, depth1, 0);
						arg2 = arg3 = 1;
						if (s1<se1) {
							s0 = ++s1; while (s0<se1 && (*s0 != ',' || level[s0 - expr._data] != clevel1)) ++s0;
							arg2 = compile(s1, s0, depth1, 0);
							arg3 = s0<se1 ? compile(++s0, se1, depth1, 0) : 1;
						}
						_cimg_mp_check_type(arg1, 1, 2, 0);
						_cimg_mp_check_type(arg2, 2, 1, 0);
						_cimg_mp_check_constant(arg3, 3, 3);
						arg3 = (unsigned int)mem[arg3];
						p1 = _cimg_mp_vector_size(arg1);
						if (p1%arg3) {
							*se = saved_char;
							s0 = ss - 4>expr._data ? ss - 4 : expr._data;
							cimg::strellipsize(s0, 64);
							throw CImgArgumentException("[" cimg_appname "_math_parser] "
								"CImg<%s>::%s: %s: Invalid specified chunk size (%u) for first argument "
								"('%s'), in expression '%s%s%s'.",
								pixel_type(), _cimg_mp_calling_function, s_op,
								arg3, s_type(arg1)._data,
								s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
						}
						pos = vector(p1);
						CImg<ulongT>::vector((ulongT)mp_sort, pos, arg1, p1, arg2, arg3).move_to(code);
						_cimg_mp_return(pos);

					}
					else { // Image
						s1 = ss6; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
						p1 = compile(ss6, s1, depth1, 0);
						arg1 = 1;
						arg2 = constant(-1.0);
						if (s1<se1) {
							s2 = s1 + 1; while (s2<se1 && (*s2 != ',' || level[s2 - expr._data] != clevel1)) ++s2;
							arg1 = compile(++s1, s2, depth1, 0);
							if (s2<se1) arg2 = compile(++s2, se1, depth1, 0);
						}
						_cimg_mp_check_type(arg1, 2, 1, 0);
						_cimg_mp_check_type(arg2, 3, 1, 0);
						_cimg_mp_check_list(true);
						CImg<ulongT>::vector((ulongT)mp_image_sort, _cimg_mp_slot_nan, p1, arg1, arg2).move_to(code);
						_cimg_mp_return_nan();
					}
				}

				if (!std::strncmp(ss, "sqr(", 4)) { // Square
					_cimg_mp_op("Function 'sqr()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_sqr, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::sqr(mem[arg1]));
					_cimg_mp_scalar1(mp_sqr, arg1);
				}

				if (!std::strncmp(ss, "sqrt(", 5)) { // Square root
					_cimg_mp_op("Function 'sqrt()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_sqrt, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::sqrt(mem[arg1]));
					_cimg_mp_scalar1(mp_sqrt, arg1);
				}

				if (!std::strncmp(ss, "srand(", 6)) { // Set RNG seed
					_cimg_mp_op("Function 'srand()'");
					arg1 = ss6<se1 ? compile(ss6, se1, depth1, 0) : ~0U;
					if (arg1 != ~0U) { _cimg_mp_check_type(arg1, 1, 1, 0); _cimg_mp_scalar1(mp_srand, arg1); }
					_cimg_mp_scalar0(mp_srand0);
				}

				if (!std::strncmp(ss, "stod(", 5)) { // String to double
					_cimg_mp_op("Function 'stod()'");
					s1 = ss5; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss5, s1, depth1, 0);
					arg2 = s1<se1 ? compile(++s1, se1, depth1, 0) : 0;
					_cimg_mp_check_type(arg2, 2, 1, 0);
					p1 = _cimg_mp_vector_size(arg1);
					_cimg_mp_scalar3(mp_stod, arg1, p1, arg2);
				}

				if (!std::strncmp(ss, "svd(", 4)) { // Matrix SVD
					_cimg_mp_op("Function 'svd()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss4, s1, depth1, 0);
					arg2 = s1<se1 ? compile(++s1, se1, depth1, 0) : 1;
					_cimg_mp_check_type(arg1, 1, 2, 0);
					_cimg_mp_check_constant(arg2, 2, 3);
					p1 = _cimg_mp_vector_size(arg1);
					p2 = (unsigned int)mem[arg2];
					p3 = p1 / p2;
					if (p3*p2 != p1) {
						*se = saved_char;
						s0 = ss - 4>expr._data ? ss - 4 : expr._data;
						cimg::strellipsize(s0, 64);
						throw CImgArgumentException("[" cimg_appname "_math_parser] "
							"CImg<%s>::%s: %s: Type of first argument ('%s') "
							"does not match with second argument 'nb_colsA=%u', "
							"in expression '%s%s%s'.",
							pixel_type(), _cimg_mp_calling_function, s_op,
							s_type(arg1)._data, p2,
							s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
					}
					pos = vector(p1 + p2 + p2*p2);
					CImg<ulongT>::vector((ulongT)mp_matrix_svd, pos, arg1, p2, p3).move_to(code);
					_cimg_mp_return(pos);
				}
				break;

			case 't':
				if (!std::strncmp(ss, "tan(", 4)) { // Tangent
					_cimg_mp_op("Function 'tan()'");
					arg1 = compile(ss4, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_tan, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::tan(mem[arg1]));
					_cimg_mp_scalar1(mp_tan, arg1);
				}

				if (!std::strncmp(ss, "tanh(", 5)) { // Hyperbolic tangent
					_cimg_mp_op("Function 'tanh()'");
					arg1 = compile(ss5, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_tanh, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(std::tanh(mem[arg1]));
					_cimg_mp_scalar1(mp_tanh, arg1);
				}

				if (!std::strncmp(ss, "trace(", 6)) { // Matrix trace
					_cimg_mp_op("Function 'trace()'");
					arg1 = compile(ss6, se1, depth1, 0);
					_cimg_mp_check_matrix_square(arg1, 1);
					p1 = (unsigned int)std::sqrt((float)_cimg_mp_vector_size(arg1));
					_cimg_mp_scalar2(mp_trace, arg1, p1);
				}

				if (!std::strncmp(ss, "transp(", 7)) { // Matrix transpose
					_cimg_mp_op("Function 'transp()'");
					s1 = ss7; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss7, s1, depth1, 0);
					arg2 = compile(++s1, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 1, 2, 0);
					_cimg_mp_check_constant(arg2, 2, 3);
					p1 = _cimg_mp_vector_size(arg1);
					p2 = (unsigned int)mem[arg2];
					p3 = p1 / p2;
					if (p2*p3 != p1) {
						*se = saved_char;
						s0 = ss - 4>expr._data ? ss - 4 : expr._data;
						cimg::strellipsize(s0, 64);
						throw CImgArgumentException("[" cimg_appname "_math_parser] "
							"CImg<%s>::%s: %s: Size of first argument ('%s') does not match "
							"second argument 'nb_cols=%u', in expression '%s%s%s'.",
							pixel_type(), _cimg_mp_calling_function, s_op,
							s_type(arg1)._data, p2,
							s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
					}
					pos = vector(p3*p2);
					CImg<ulongT>::vector((ulongT)mp_transp, pos, arg1, p2, p3).move_to(code);
					_cimg_mp_return(pos);
				}
				break;

			case 'u':
				if (*ss1 == '(') { // Random value with uniform distribution
					_cimg_mp_op("Function 'u()'");
					if (*ss2 == ')') _cimg_mp_scalar2(mp_u, 0, 1);
					s1 = ss2; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss2, s1, depth1, 0);
					if (s1<se1) arg2 = compile(++s1, se1, depth1, 0); else { arg2 = arg1; arg1 = 0; }
					_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
					if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_u, arg1, arg2);
					if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_u, arg1, arg2);
					if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_u, arg1, arg2);
					_cimg_mp_scalar2(mp_u, arg1, arg2);
				}

				if (!std::strncmp(ss, "unref(", 6)) { // Un-reference variable
					_cimg_mp_op("Function 'unref()'");
					arg1 = ~0U;
					for (s0 = ss6; s0<se1; s0 = s1) {
						if (s0>ss6 && *s0 == ',') ++s0;
						s1 = s0; while (s1<se1 && *s1 != ',') ++s1;
						c1 = *s1;
						if (s1>s0) {
							*s1 = 0;
							arg2 = arg3 = ~0U;
							if (s0[0] == 'w' && s0[1] == 'h' && !s0[2]) arg1 = reserved_label[arg3 = 0];
							else if (s0[0] == 'w' && s0[1] == 'h' && s0[2] == 'd' && !s0[3]) arg1 = reserved_label[arg3 = 1];
							else if (s0[0] == 'w' && s0[1] == 'h' && s0[2] == 'd' && s0[3] == 's' && !s0[4])
								arg1 = reserved_label[arg3 = 2];
							else if (s0[0] == 'p' && s0[1] == 'i' && !s0[2]) arg1 = reserved_label[arg3 = 3];
							else if (s0[0] == 'i' && s0[1] == 'm' && !s0[2]) arg1 = reserved_label[arg3 = 4];
							else if (s0[0] == 'i' && s0[1] == 'M' && !s0[2]) arg1 = reserved_label[arg3 = 5];
							else if (s0[0] == 'i' && s0[1] == 'a' && !s0[2]) arg1 = reserved_label[arg3 = 6];
							else if (s0[0] == 'i' && s0[1] == 'v' && !s0[2]) arg1 = reserved_label[arg3 = 7];
							else if (s0[0] == 'i' && s0[1] == 's' && !s0[2]) arg1 = reserved_label[arg3 = 8];
							else if (s0[0] == 'i' && s0[1] == 'p' && !s0[2]) arg1 = reserved_label[arg3 = 9];
							else if (s0[0] == 'i' && s0[1] == 'c' && !s0[2]) arg1 = reserved_label[arg3 = 10];
							else if (s0[0] == 'x' && s0[1] == 'm' && !s0[2]) arg1 = reserved_label[arg3 = 11];
							else if (s0[0] == 'y' && s0[1] == 'm' && !s0[2]) arg1 = reserved_label[arg3 = 12];
							else if (s0[0] == 'z' && s0[1] == 'm' && !s0[2]) arg1 = reserved_label[arg3 = 13];
							else if (s0[0] == 'c' && s0[1] == 'm' && !s0[2]) arg1 = reserved_label[arg3 = 14];
							else if (s0[0] == 'x' && s0[1] == 'M' && !s0[2]) arg1 = reserved_label[arg3 = 15];
							else if (s0[0] == 'y' && s0[1] == 'M' && !s0[2]) arg1 = reserved_label[arg3 = 16];
							else if (s0[0] == 'z' && s0[1] == 'M' && !s0[2]) arg1 = reserved_label[arg3 = 17];
							else if (s0[0] == 'c' && s0[1] == 'M' && !s0[2]) arg1 = reserved_label[arg3 = 18];
							else if (s0[0] == 'i' && s0[1] >= '0' && s0[1] <= '9' && !s0[2])
								arg1 = reserved_label[arg3 = 19 + s0[1] - '0'];
							else if (!std::strcmp(s0, "interpolation")) arg1 = reserved_label[arg3 = 29];
							else if (!std::strcmp(s0, "boundary")) arg1 = reserved_label[arg3 = 30];
							else if (s0[1]) { // Multi-char variable
								cimglist_for(variable_def, i) if (!std::strcmp(s0, variable_def[i])) {
									arg1 = variable_pos[i]; arg2 = i; break;
								}
							}
							else arg1 = reserved_label[arg3 = *s0]; // Single-char variable

							if (arg1 != ~0U) {
								if (arg2 == ~0U) { if (arg3 != ~0U) reserved_label[arg3] = ~0U; }
								else {
									variable_def.remove(arg2);
									if (arg2<variable_pos._width - 1)
										std::memmove(variable_pos._data + arg2, variable_pos._data + arg2 + 1,
											sizeof(uintT)*(variable_pos._width - arg2 - 1));
									--variable_pos._width;
								}
							}
							*s1 = c1;
						}
						else compile(s0, s1, depth1, 0); // Will throw a 'missing argument' exception
					}
					_cimg_mp_return(arg1 != ~0U ? arg1 : _cimg_mp_slot_nan); // Return value of last specified variable.
				}

				if (!std::strncmp(ss, "uppercase(", 10)) { // Upper case
					_cimg_mp_op("Function 'uppercase()'");
					arg1 = compile(ss + 10, se1, depth1, 0);
					if (_cimg_mp_is_vector(arg1)) _cimg_mp_vector1_v(mp_uppercase, arg1);
					if (_cimg_mp_is_constant(arg1)) _cimg_mp_constant(cimg::uppercase(mem[arg1]));
					_cimg_mp_scalar1(mp_uppercase, arg1);
				}
				break;

			case 'v':
				if ((cimg_sscanf(ss, "vector%u%c", &(arg1 = ~0U), &sep) == 2 && sep == '(' && arg1>0) ||
					!std::strncmp(ss, "vector(", 7) ||
					(!std::strncmp(ss, "vector", 6) && ss7<se1 && (s = std::strchr(ss7, '(')) != 0)) { // Vector
					_cimg_mp_op("Function 'vector()'");
					arg2 = 0; // Number of specified values.
					if (arg1 == ~0U && *ss6 != '(') {
						arg1 = compile(ss6, s++, depth1, 0);
						_cimg_mp_check_constant(arg1, 0, 3);
						arg1 = (unsigned int)mem[arg1];
					}
					else s = std::strchr(ss6, '(') + 1;

					if (s<se1 || arg1 == ~0U) for (; s<se; ++s) {
						ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
							(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
						arg3 = compile(s, ns, depth1, 0);
						if (_cimg_mp_is_vector(arg3)) {
							arg4 = _cimg_mp_vector_size(arg3);
							CImg<ulongT>::sequence(arg4, arg3 + 1, arg3 + arg4).move_to(_opcode);
							arg2 += arg4;
						}
						else { CImg<ulongT>::vector(arg3).move_to(_opcode); ++arg2; }
						s = ns;
					}
					if (arg1 == ~0U) arg1 = arg2;
					_cimg_mp_check_vector0(arg1);
					pos = vector(arg1);
					_opcode.insert(CImg<ulongT>::vector((ulongT)mp_vector_init, pos, 0, arg1), 0);
					(_opcode>'y').move_to(opcode);
					opcode[2] = opcode._height;
					opcode.move_to(code);
					_cimg_mp_return(pos);
				}
				break;

			case 'w':
				if (*ss1 == '(') { // Image width
					_cimg_mp_op("Function 'w()'");
					if (*ss2 == '#') { p1 = compile(ss3, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss2 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)mp_image_w, pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (*ss1 == 'h' && *ss2 == '(') { // Image width*height
					_cimg_mp_op("Function 'wh()'");
					if (*ss3 == '#') { p1 = compile(ss4, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss3 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)mp_image_wh, pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (*ss1 == 'h' && *ss2 == 'd' && *ss3 == '(') { // Image width*height*depth
					_cimg_mp_op("Function 'whd()'");
					if (*ss4 == '#') { p1 = compile(ss5, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss4 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)mp_image_whd, pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (*ss1 == 'h' && *ss2 == 'd' && *ss3 == 's' && *ss4 == '(') { // Image width*height*depth*spectrum
					_cimg_mp_op("Function 'whds()'");
					if (*ss5 == '#') { p1 = compile(ss6, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss5 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)mp_image_whds, pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "while(", 6) || !std::strncmp(ss, "whiledo(", 8)) { // While...do
					_cimg_mp_op("Function 'whiledo()'");
					s0 = *ss5 == '(' ? ss6 : ss8;
					s1 = s0; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					p1 = code._width;
					arg1 = compile(s0, s1, depth1, 0);
					p2 = code._width;
					arg6 = mempos;
					pos = compile(++s1, se1, depth1, 0);
					_cimg_mp_check_type(arg1, 1, 1, 0);
					arg2 = _cimg_mp_vector_size(pos);
					CImg<ulongT>::vector((ulongT)mp_whiledo, pos, arg1, p2 - p1, code._width - p2, arg2,
						pos >= arg6 && !_cimg_mp_is_constant(pos),
						arg1 >= arg6 && !_cimg_mp_is_constant(arg1)).move_to(code, p1);
					_cimg_mp_return(pos);
				}
				break;

			case 'x':
				if ((*ss1 == 'm' || *ss1 == 'M') && *ss2 == '(') { // xm(), xM()
					_cimg_mp_op(*ss1 == 'm' ? "Function 'xm()'" : "Function 'xM()'");
					if (*ss3 == '#') { p1 = compile(ss4, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss3 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)(*ss1 == 'm' ? mp_image_xm : mp_image_xM), pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}

				if (!std::strncmp(ss, "xor(", 4)) { // Xor
					_cimg_mp_op("Function 'xor()'");
					s1 = ss4; while (s1<se1 && (*s1 != ',' || level[s1 - expr._data] != clevel1)) ++s1;
					arg1 = compile(ss4, s1, depth1, 0);
					arg2 = compile(++s1, se1, depth1, 0);
					_cimg_mp_check_type(arg2, 2, 3, _cimg_mp_vector_size(arg1));
					if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_vv(mp_bitwise_xor, arg1, arg2);
					if (_cimg_mp_is_vector(arg1) && _cimg_mp_is_scalar(arg2)) _cimg_mp_vector2_vs(mp_bitwise_xor, arg1, arg2);
					if (_cimg_mp_is_scalar(arg1) && _cimg_mp_is_vector(arg2)) _cimg_mp_vector2_sv(mp_bitwise_xor, arg1, arg2);
					if (_cimg_mp_is_constant(arg1) && _cimg_mp_is_constant(arg2))
						_cimg_mp_constant((longT)mem[arg1] ^ (longT)mem[arg2]);
					_cimg_mp_scalar2(mp_bitwise_xor, arg1, arg2);
				}
				break;

			case 'y':
				if ((*ss1 == 'm' || *ss1 == 'M') && *ss2 == '(') { // ym(), yM()
					_cimg_mp_op(*ss1 == 'm' ? "Function 'ym()'" : "Function 'yM()'");
					if (*ss3 == '#') { p1 = compile(ss4, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss3 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)(*ss1 == 'm' ? mp_image_ym : mp_image_yM), pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}
				break;

			case 'z':
				if ((*ss1 == 'm' || *ss1 == 'M') && *ss2 == '(') { // zm(), zM()
					_cimg_mp_op(*ss1 == 'm' ? "Function 'zm()'" : "Function 'zM()'");
					if (*ss3 == '#') { p1 = compile(ss4, se1, depth1, 0); _cimg_mp_check_list(false); } // Index specified
					else { if (ss3 != se1) break; p1 = ~0U; }
					pos = scalar();
					CImg<ulongT>::vector((ulongT)(*ss1 == 'm' ? mp_image_zm : mp_image_zM), pos, p1).move_to(code);
					_cimg_mp_return(pos);
				}
				break;

			}

			if (!std::strncmp(ss, "min(", 4) || !std::strncmp(ss, "max(", 4) ||
				!std::strncmp(ss, "med(", 4) || !std::strncmp(ss, "kth(", 4) ||
				!std::strncmp(ss, "sum(", 4) ||
				!std::strncmp(ss, "std(", 4) || !std::strncmp(ss, "variance(", 9) ||
				!std::strncmp(ss, "prod(", 5) || !std::strncmp(ss, "mean(", 5) ||
				!std::strncmp(ss, "argmin(", 7) || !std::strncmp(ss, "argmax(", 7)) { // Multi-argument functions
				_cimg_mp_op(*ss == 'a' ? (ss[3] == '(' ? "Function 'arg()'" : ss[4] == 'i' ? "Function 'argmin()'" :
					"Function 'argmax()'") :
					*ss == 's' ? (ss[1] == 'u' ? "Function 'sum()'" : "Function 'std()'") :
					*ss == 'k' ? "Function 'kth()'" :
					*ss == 'p' ? "Function 'prod()'" :
					*ss == 'v' ? "Function 'variance()'" :
					ss[1] == 'i' ? "Function 'min()'" :
					ss[1] == 'a' ? "Function 'max()'" :
					ss[2] == 'a' ? "Function 'mean()'" : "Function 'med()'");
				op = *ss == 'a' ? (ss[3] == '(' ? mp_arg : ss[4] == 'i' ? mp_argmin : mp_argmax) :
					*ss == 's' ? (ss[1] == 'u' ? mp_sum : mp_std) :
					*ss == 'k' ? mp_kth :
					*ss == 'p' ? mp_prod :
					*ss == 'v' ? mp_variance :
					ss[1] == 'i' ? mp_min :
					ss[1] == 'a' ? mp_max :
					ss[2] == 'a' ? mp_mean :
					mp_median;
				is_sth = true; // Tell if all arguments are constant
				pos = scalar();
				CImg<ulongT>::vector((ulongT)op, pos, 0).move_to(_opcode);
				for (s = std::strchr(ss, '(') + 1; s<se; ++s) {
					ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
						(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
					arg2 = compile(s, ns, depth1, 0);
					if (_cimg_mp_is_vector(arg2))
						CImg<ulongT>::sequence(_cimg_mp_vector_size(arg2), arg2 + 1,
							arg2 + (ulongT)_cimg_mp_vector_size(arg2)).
						move_to(_opcode);
					else CImg<ulongT>::vector(arg2).move_to(_opcode);
					is_sth &= _cimg_mp_is_constant(arg2);
					s = ns;
				}
				(_opcode>'y').move_to(opcode);
				opcode[2] = opcode._height;
				if (is_sth) _cimg_mp_constant(op(*this));
				opcode.move_to(code);
				_cimg_mp_return(pos);
			}

			// No corresponding built-in function -> Look for a user-defined macro call.
			s0 = strchr(ss, '(');
			if (s0) {
				variable_name.assign(ss, (unsigned int)(s0 - ss + 1)).back() = 0;

				// Count number of specified arguments.
				p1 = 0;
				for (s = s0 + 1; s <= se1; ++p1, s = ns + 1) {
					while (*s && (signed char)*s <= ' ') ++s;
					if (*s == ')' && !p1) break;
					ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
						(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
				}

				arg3 = 0; // Number of possible name matches
				cimglist_for(macro_def, l) if (!std::strcmp(macro_def[l], variable_name) && ++arg3 &&
					macro_def[l].back() == (char)p1) {
					p2 = (unsigned int)macro_def[l].back(); // Number of required arguments
					CImg<charT> _expr = macro_body[l]; // Expression to be substituted

					p1 = 1; // Indice of current parsed argument
					for (s = s0 + 1; s <= se1; ++p1, s = ns + 1) { // Parse function arguments
						while (*s && (signed char)*s <= ' ') ++s;
						if (*s == ')' && p1 == 1) break; // Function has no arguments
						if (p1>p2) { ++p1; break; }
						ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
							(*ns != ')' || level[ns - expr._data] != clevel)) ++ns;
						variable_name.assign(s, (unsigned int)(ns - s + 1)).back() = 0; // Argument to write
						arg2 = 0;
						cimg_forX(_expr, k) {
							if (_expr[k] == (char)p1) { // Perform argument substitution
								arg1 = _expr._width;
								_expr.resize(arg1 + variable_name._width - 2, 1, 1, 1, 0);
								std::memmove(_expr._data + k + variable_name._width - 1, _expr._data + k + 1, arg1 - k - 1);
								std::memcpy(_expr._data + k, variable_name, variable_name._width - 1);
								k += variable_name._width - 2;
							}
							++arg2;
						}
					}

					// Recompute 'pexpr' and 'level' for evaluating substituted expression.
					CImg<charT> _pexpr(_expr._width);
					ns = _pexpr._data;
					for (ps = _expr._data, c1 = ' '; *ps; ++ps) {
						if ((signed char)*ps>' ') c1 = *ps;
						*(ns++) = c1;
					}
					*ns = 0;

					CImg<uintT> _level = get_level(_expr);
					expr.swap(_expr);
					pexpr.swap(_pexpr);
					level.swap(_level);
					s0 = user_macro;
					user_macro = macro_def[l];
					pos = compile(expr._data, expr._data + expr._width - 1, depth1, p_ref);
					user_macro = s0;
					level.swap(_level);
					pexpr.swap(_pexpr);
					expr.swap(_expr);
					_cimg_mp_return(pos);
				}

				if (arg3) { // Macro name matched but number of arguments does not
					CImg<uintT> sig_nargs(arg3);
					arg1 = 0;
					cimglist_for(macro_def, l) if (!std::strcmp(macro_def[l], variable_name))
						sig_nargs[arg1++] = (unsigned int)macro_def[l].back();
					*se = saved_char;
					cimg::strellipsize(variable_name, 64);
					s0 = ss - 4>expr._data ? ss - 4 : expr._data;
					cimg::strellipsize(s0, 64);
					if (sig_nargs._width>1) {
						sig_nargs.sort();
						arg1 = sig_nargs.back();
						--sig_nargs._width;
						throw CImgArgumentException("[" cimg_appname "_math_parser] "
							"CImg<%s>::%s: Function '%s()': Number of specified arguments (%u) "
							"does not match macro declaration (defined for %s or %u arguments), "
							"in expression '%s%s%s'.",
							pixel_type(), _cimg_mp_calling_function, variable_name._data,
							p1, sig_nargs.value_string()._data, arg1,
							s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
					}
					else
						throw CImgArgumentException("[" cimg_appname "_math_parser] "
							"CImg<%s>::%s: Function '%s()': Number of specified arguments (%u) "
							"does not match macro declaration (defined for %u argument%s), "
							"in expression '%s%s%s'.",
							pixel_type(), _cimg_mp_calling_function, variable_name._data,
							p1, *sig_nargs, *sig_nargs != 1 ? "s" : "",
							s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
				}
			}
		} // if (se1==')')

		  // Char / string initializer.
		if (*se1 == '\'' &&
			((se1>ss && *ss == '\'') ||
			(se1>ss1 && *ss == '_' && *ss1 == '\''))) {
			if (*ss == '_') { _cimg_mp_op("Char initializer"); s1 = ss2; }
			else { _cimg_mp_op("String initializer"); s1 = ss1; }
			arg1 = (unsigned int)(se1 - s1); // Original string length.
			if (arg1) {
				CImg<charT>(s1, arg1 + 1).move_to(variable_name).back() = 0;
				cimg::strunescape(variable_name);
				arg1 = (unsigned int)std::strlen(variable_name);
			}
			if (!arg1) _cimg_mp_return(0); // Empty string -> 0
			if (*ss == '_') {
				if (arg1 == 1) _cimg_mp_constant(*variable_name);
				*se = saved_char;
				cimg::strellipsize(variable_name, 64);
				s0 = ss - 4>expr._data ? ss - 4 : expr._data;
				cimg::strellipsize(s0, 64);
				throw CImgArgumentException("[" cimg_appname "_math_parser] "
					"CImg<%s>::%s: %s: Literal %s contains more than one character, "
					"in expression '%s%s%s'.",
					pixel_type(), _cimg_mp_calling_function, s_op,
					ss1,
					s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
			}
			pos = vector(arg1);
			CImg<ulongT>::vector((ulongT)mp_string_init, pos, arg1).move_to(_opcode);
			CImg<ulongT>(1, arg1 / sizeof(ulongT) + (arg1 % sizeof(ulongT) ? 1 : 0)).move_to(_opcode);
			std::memcpy((char*)_opcode[1]._data, variable_name, arg1);
			(_opcode>'y').move_to(code);
			_cimg_mp_return(pos);
		}

		// Vector initializer [ ... ].
		if (*ss == '[' && *se1 == ']') {
			_cimg_mp_op("Vector initializer");
			s1 = ss1; while (s1<se2 && (signed char)*s1 <= ' ') ++s1;
			s2 = se2; while (s2>s1 && (signed char)*s2 <= ' ') --s2;
			if (s2>s1 && *s1 == '\'' && *s2 == '\'') { // Vector values provided as a string
				arg1 = (unsigned int)(s2 - s1 - 1); // Original string length.
				if (arg1) {
					CImg<charT>(s1 + 1, arg1 + 1).move_to(variable_name).back() = 0;
					cimg::strunescape(variable_name);
					arg1 = (unsigned int)std::strlen(variable_name);
				}
				if (!arg1) _cimg_mp_return(0); // Empty string -> 0
				pos = vector(arg1);
				CImg<ulongT>::vector((ulongT)mp_string_init, pos, arg1).move_to(_opcode);
				CImg<ulongT>(1, arg1 / sizeof(ulongT) + (arg1 % sizeof(ulongT) ? 1 : 0)).move_to(_opcode);
				std::memcpy((char*)_opcode[1]._data, variable_name, arg1);
				(_opcode>'y').move_to(code);
			}
			else { // Vector values provided as list of items
				arg1 = 0; // Number of specified values.
				if (*ss1 != ']') for (s = ss1; s<se; ++s) {
					ns = s; while (ns<se && (*ns != ',' || level[ns - expr._data] != clevel1) &&
						(*ns != ']' || level[ns - expr._data] != clevel)) ++ns;
					arg2 = compile(s, ns, depth1, 0);
					if (_cimg_mp_is_vector(arg2)) {
						arg3 = _cimg_mp_vector_size(arg2);
						CImg<ulongT>::sequence(arg3, arg2 + 1, arg2 + arg3).move_to(_opcode);
						arg1 += arg3;
					}
					else { CImg<ulongT>::vector(arg2).move_to(_opcode); ++arg1; }
					s = ns;
				}
				_cimg_mp_check_vector0(arg1);
				pos = vector(arg1);
				_opcode.insert(CImg<ulongT>::vector((ulongT)mp_vector_init, pos, 0, arg1), 0);
				(_opcode>'y').move_to(opcode);
				opcode[2] = opcode._height;
				opcode.move_to(code);
			}
			_cimg_mp_return(pos);
		}

		// Variables related to the input list of images.
		if (*ss1 == '#' && ss2<se) {
			arg1 = compile(ss2, se, depth1, 0);
			p1 = (unsigned int)(listin._width && _cimg_mp_is_constant(arg1) ? cimg::mod((int)mem[arg1], listin.width()) : ~0U);
			switch (*ss) {
			case 'w': // w#ind
				if (!listin) _cimg_mp_return(0);
				if (p1 != ~0U) _cimg_mp_constant(listin[p1]._width);
				_cimg_mp_scalar1(mp_list_width, arg1);
			case 'h': // h#ind
				if (!listin) _cimg_mp_return(0);
				if (p1 != ~0U) _cimg_mp_constant(listin[p1]._height);
				_cimg_mp_scalar1(mp_list_height, arg1);
			case 'd': // d#ind
				if (!listin) _cimg_mp_return(0);
				if (p1 != ~0U) _cimg_mp_constant(listin[p1]._depth);
				_cimg_mp_scalar1(mp_list_depth, arg1);
			case 'r': // r#ind
				if (!listin) _cimg_mp_return(0);
				if (p1 != ~0U) _cimg_mp_constant(listin[p1]._is_shared);
				_cimg_mp_scalar1(mp_list_is_shared, arg1);
			case 's': // s#ind
				if (!listin) _cimg_mp_return(0);
				if (p1 != ~0U) _cimg_mp_constant(listin[p1]._spectrum);
				_cimg_mp_scalar1(mp_list_spectrum, arg1);
			case 'i': // i#ind
				if (!listin) _cimg_mp_return(0);
				_cimg_mp_scalar7(mp_list_ixyzc, arg1, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, _cimg_mp_slot_c,
					0, _cimg_mp_boundary);
			case 'I': // I#ind
				p2 = p1 != ~0U ? listin[p1]._spectrum : listin._width ? ~0U : 0;
				_cimg_mp_check_vector0(p2);
				pos = vector(p2);
				CImg<ulongT>::vector((ulongT)mp_list_Joff, pos, p1, 0, 0, p2).move_to(code);
				_cimg_mp_return(pos);
			case 'R': // R#ind
				if (!listin) _cimg_mp_return(0);
				_cimg_mp_scalar7(mp_list_ixyzc, arg1, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, 0,
					0, _cimg_mp_boundary);
			case 'G': // G#ind
				if (!listin) _cimg_mp_return(0);
				_cimg_mp_scalar7(mp_list_ixyzc, arg1, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, 1,
					0, _cimg_mp_boundary);
			case 'B': // B#ind
				if (!listin) _cimg_mp_return(0);
				_cimg_mp_scalar7(mp_list_ixyzc, arg1, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, 2,
					0, _cimg_mp_boundary);
			case 'A': // A#ind
				if (!listin) _cimg_mp_return(0);
				_cimg_mp_scalar7(mp_list_ixyzc, arg1, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, 3,
					0, _cimg_mp_boundary);
			}
		}

		if (*ss1 && *ss2 == '#' && ss3<se) {
			arg1 = compile(ss3, se, depth1, 0);
			p1 = (unsigned int)(listin._width && _cimg_mp_is_constant(arg1) ? cimg::mod((int)mem[arg1], listin.width()) : ~0U);
			if (*ss == 'w' && *ss1 == 'h') { // wh#ind
				if (!listin) _cimg_mp_return(0);
				if (p1 != ~0U) _cimg_mp_constant(listin[p1]._width*listin[p1]._height);
				_cimg_mp_scalar1(mp_list_wh, arg1);
			}
			arg2 = ~0U;

			if (*ss == 'i') {
				if (*ss1 == 'c') { // ic#ind
					if (!listin) _cimg_mp_return(0);
					if (_cimg_mp_is_constant(arg1)) {
						if (!list_median) list_median.assign(listin._width);
						if (!list_median[p1]) CImg<doubleT>::vector(listin[p1].median()).move_to(list_median[p1]);
						_cimg_mp_constant(*list_median[p1]);
					}
					_cimg_mp_scalar1(mp_list_median, arg1);
				}
				if (*ss1 >= '0' && *ss1 <= '9') { // i0#ind...i9#ind
					if (!listin) _cimg_mp_return(0);
					_cimg_mp_scalar7(mp_list_ixyzc, arg1, _cimg_mp_slot_x, _cimg_mp_slot_y, _cimg_mp_slot_z, *ss1 - '0',
						0, _cimg_mp_boundary);
				}
				switch (*ss1) {
				case 'm': arg2 = 0; break; // im#ind
				case 'M': arg2 = 1; break; // iM#ind
				case 'a': arg2 = 2; break; // ia#ind
				case 'v': arg2 = 3; break; // iv#ind
				case 's': arg2 = 12; break; // is#ind
				case 'p': arg2 = 13; break; // ip#ind
				}
			}
			else if (*ss1 == 'm') switch (*ss) {
			case 'x': arg2 = 4; break; // xm#ind
			case 'y': arg2 = 5; break; // ym#ind
			case 'z': arg2 = 6; break; // zm#ind
			case 'c': arg2 = 7; break; // cm#ind
			}
			else if (*ss1 == 'M') switch (*ss) {
			case 'x': arg2 = 8; break; // xM#ind
			case 'y': arg2 = 9; break; // yM#ind
			case 'z': arg2 = 10; break; // zM#ind
			case 'c': arg2 = 11; break; // cM#ind
			}
			if (arg2 != ~0U) {
				if (!listin) _cimg_mp_return(0);
				if (_cimg_mp_is_constant(arg1)) {
					if (!list_stats) list_stats.assign(listin._width);
					if (!list_stats[p1]) list_stats[p1].assign(1, 14, 1, 1, 0).fill(listin[p1].get_stats(), false);
					_cimg_mp_constant(list_stats(p1, arg2));
				}
				_cimg_mp_scalar2(mp_list_stats, arg1, arg2);
			}
		}

		if (*ss == 'w' && *ss1 == 'h' && *ss2 == 'd' && *ss3 == '#' && ss4<se) { // whd#ind
			arg1 = compile(ss4, se, depth1, 0);
			if (!listin) _cimg_mp_return(0);
			p1 = (unsigned int)(_cimg_mp_is_constant(arg1) ? cimg::mod((int)mem[arg1], listin.width()) : ~0U);
			if (p1 != ~0U) _cimg_mp_constant(listin[p1]._width*listin[p1]._height*listin[p1]._depth);
			_cimg_mp_scalar1(mp_list_whd, arg1);
		}
		if (*ss == 'w' && *ss1 == 'h' && *ss2 == 'd' && *ss3 == 's' && *ss4 == '#' && ss5<se) { // whds#ind
			arg1 = compile(ss5, se, depth1, 0);
			if (!listin) _cimg_mp_return(0);
			p1 = (unsigned int)(_cimg_mp_is_constant(arg1) ? cimg::mod((int)mem[arg1], listin.width()) : ~0U);
			if (p1 != ~0U) _cimg_mp_constant(listin[p1]._width*listin[p1]._height*listin[p1]._depth*listin[p1]._spectrum);
			_cimg_mp_scalar1(mp_list_whds, arg1);
		}

		if (!std::strcmp(ss, "interpolation")) _cimg_mp_return(_cimg_mp_interpolation); // interpolation
		if (!std::strcmp(ss, "boundary")) _cimg_mp_return(_cimg_mp_boundary); // boundary

																			  // No known item found, assuming this is an already initialized variable.
		variable_name.assign(ss, (unsigned int)(se - ss + 1)).back() = 0;
		if (variable_name[1]) { // Multi-char variable
			cimglist_for(variable_def, i) if (!std::strcmp(variable_name, variable_def[i]))
				_cimg_mp_return(variable_pos[i]);
		}
		else if (reserved_label[*variable_name] != ~0U) // Single-char variable
			_cimg_mp_return(reserved_label[*variable_name]);

		// Reached an unknown item -> error.
		is_sth = true; // is_valid_variable_name
		if (*variable_name >= '0' && *variable_name <= '9') is_sth = false;
		else for (ns = variable_name._data; *ns; ++ns)
			if (!is_varchar(*ns)) { is_sth = false; break; }

		*se = saved_char;
		c1 = *se1;
		cimg::strellipsize(variable_name, 64);
		s0 = ss - 4>expr._data ? ss - 4 : expr._data;
		cimg::strellipsize(s0, 64);
		if (is_sth)
			throw CImgArgumentException("[" cimg_appname "_math_parser] "
				"CImg<%s>::%s: Undefined variable '%s' in expression '%s%s%s'.",
				pixel_type(), _cimg_mp_calling_function,
				variable_name._data,
				s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
		s1 = std::strchr(ss, '(');
		s_op = s1 && c1 == ')' ? "function call" : "item";
		throw CImgArgumentException("[" cimg_appname "_math_parser] "
			"CImg<%s>::%s: Unrecognized %s '%s' in expression '%s%s%s'.",
			pixel_type(), _cimg_mp_calling_function,
			s_op, variable_name._data,
			s0 != expr._data ? "..." : "", s0, se<&expr.back() ? "..." : "");
	}


		template<typename T> 
			template<typename tz, typename tc>
	CImg<T>& CImg<T>::draw_line(CImg<tz>& zbuffer,
		const int x0, const int y0, const float z0,
		const int x1, const int y1, const float z1,
		const CImg<tc>& texture,
		const int tx0, const int ty0,
		const int tx1, const int ty1,
		const float opacity = 1,
		const unsigned int pattern = ~0U, const bool init_hatch = true)
	{
		typedef typename cimg::superset<tz, float>::type tzfloat;
		if (is_empty() || z0 <= 0 || z1 <= 0) return *this;
		if (!is_sameXY(zbuffer))
			throw CImgArgumentException(_cimg_instance
				"draw_line(): Instance and specified Z-buffer (%u,%u,%u,%u,%p) have "
				"different dimensions.",
				cimg_instance,
				zbuffer._width, zbuffer._height, zbuffer._depth, zbuffer._spectrum, zbuffer._data);
		if (texture._depth>1 || texture._spectrum<_spectrum)
			throw CImgArgumentException(_cimg_instance
				"draw_line(): Invalid specified texture (%u,%u,%u,%u,%p).",
				cimg_instance,
				texture._width, texture._height, texture._depth, texture._spectrum, texture._data);
		if (is_overlapped(texture))
			return draw_line(zbuffer, x0, y0, z0, x1, y1, z1, +texture, tx0, ty0, tx1, ty1, opacity, pattern, init_hatch);
		static unsigned int hatch = ~0U - (~0U >> 1);
		if (init_hatch) hatch = ~0U - (~0U >> 1);
		const bool xdir = x0<x1, ydir = y0<y1;
		int
			nx0 = x0, nx1 = x1, ny0 = y0, ny1 = y1,
			&xleft = xdir ? nx0 : nx1, &yleft = xdir ? ny0 : ny1,
			&xright = xdir ? nx1 : nx0, &yright = xdir ? ny1 : ny0,
			&xup = ydir ? nx0 : nx1, &yup = ydir ? ny0 : ny1,
			&xdown = ydir ? nx1 : nx0, &ydown = ydir ? ny1 : ny0;
		float
			Tx0 = tx0 / z0, Tx1 = tx1 / z1,
			Ty0 = ty0 / z0, Ty1 = ty1 / z1,
			dtx = Tx1 - Tx0, dty = Ty1 - Ty0,
			tnx0 = Tx0, tnx1 = Tx1, tny0 = Ty0, tny1 = Ty1,
			&txleft = xdir ? tnx0 : tnx1, &tyleft = xdir ? tny0 : tny1,
			&txright = xdir ? tnx1 : tnx0, &tyright = xdir ? tny1 : tny0,
			&txup = ydir ? tnx0 : tnx1, &tyup = ydir ? tny0 : tny1,
			&txdown = ydir ? tnx1 : tnx0, &tydown = ydir ? tny1 : tny0;
		tzfloat
			Z0 = 1 / (tzfloat)z0, Z1 = 1 / (tzfloat)z1,
			dz = Z1 - Z0, nz0 = Z0, nz1 = Z1,
			&zleft = xdir ? nz0 : nz1,
			&zright = xdir ? nz1 : nz0,
			&zup = ydir ? nz0 : nz1,
			&zdown = ydir ? nz1 : nz0;
		if (xright<0 || xleft >= width()) return *this;
		if (xleft<0) {
			const float D = (float)xright - xleft;
			yleft -= (int)((float)xleft*((float)yright - yleft) / D);
			zleft -= (float)xleft*(zright - zleft) / D;
			txleft -= (float)xleft*(txright - txleft) / D;
			tyleft -= (float)xleft*(tyright - tyleft) / D;
			xleft = 0;
		}
		if (xright >= width()) {
			const float d = (float)xright - width(), D = (float)xright - xleft;
			yright -= (int)(d*((float)yright - yleft) / D);
			zright -= d*(zright - zleft) / D;
			txright -= d*(txright - txleft) / D;
			tyright -= d*(tyright - tyleft) / D;
			xright = width() - 1;
		}
		if (ydown<0 || yup >= height()) return *this;
		if (yup<0) {
			const float D = (float)ydown - yup;
			xup -= (int)((float)yup*((float)xdown - xup) / D);
			zup -= yup*(zdown - zup) / D;
			txup -= yup*(txdown - txup) / D;
			tyup -= yup*(tydown - tyup) / D;
			yup = 0;
		}
		if (ydown >= height()) {
			const float d = (float)ydown - height(), D = (float)ydown - yup;
			xdown -= (int)(d*((float)xdown - xup) / D);
			zdown -= d*(zdown - zup) / D;
			txdown -= d*(txdown - txup) / D;
			tydown -= d*(tydown - tyup) / D;
			ydown = height() - 1;
		}
		T *ptrd0 = data(nx0, ny0);
		tz *ptrz = zbuffer.data(nx0, ny0);
		int dx = xright - xleft, dy = ydown - yup;
		const bool steep = dy>dx;
		if (steep) cimg::swap(nx0, ny0, nx1, ny1, dx, dy);
		const longT
			offx = (longT)(nx0<nx1 ? 1 : -1)*(steep ? width() : 1),
			offy = (longT)(ny0<ny1 ? 1 : -1)*(steep ? 1 : width()),
			ndx = (longT)(dx>0 ? dx : 1);
		const ulongT
			whd = (ulongT)_width*_height*_depth,
			twh = (ulongT)texture._width*texture._height;

		if (opacity >= 1) {
			if (~pattern) for (int error = dx >> 1, x = 0; x <= dx; ++x) {
				if (pattern&hatch) {
					const tzfloat z = Z0 + x*dz / ndx;
					if (z >= (tzfloat)*ptrz) {
						*ptrz = (tz)z;
						const float tx = Tx0 + x*dtx / ndx, ty = Ty0 + x*dty / ndx;
						const tc *col = &texture._atXY((int)(tx / z), (int)(ty / z));
						T *ptrd = ptrd0;
						cimg_forC(*this, c) { *ptrd = (T)*col; ptrd += whd; col += twh; }
					}
				}
				hatch >>= 1; if (!hatch) hatch = ~0U - (~0U >> 1);
				ptrd0 += offx; ptrz += offx;
				if ((error -= dy)<0) { ptrd0 += offy; ptrz += offy; error += dx; }
			}
			else for (int error = dx >> 1, x = 0; x <= dx; ++x) {
				const tzfloat z = Z0 + x*dz / ndx;
				if (z >= (tzfloat)*ptrz) {
					*ptrz = (tz)z;
					const float tx = Tx0 + x*dtx / ndx, ty = Ty0 + x*dty / ndx;
					const tc *col = &texture._atXY((int)(tx / z), (int)(ty / z));
					T *ptrd = ptrd0;
					cimg_forC(*this, c) { *ptrd = (T)*col; ptrd += whd; col += twh; }
				}
				ptrd0 += offx; ptrz += offx;
				if ((error -= dy)<0) { ptrd0 += offy; ptrz += offy; error += dx; }
			}
		}
		else {
			const float nopacity = cimg::abs(opacity), copacity = 1 - std::max(opacity, 0.0f);
			if (~pattern) for (int error = dx >> 1, x = 0; x <= dx; ++x) {
				if (pattern&hatch) {
					const tzfloat z = Z0 + x*dz / ndx;
					if (z >= (tzfloat)*ptrz) {
						*ptrz = (tz)z;
						const float tx = Tx0 + x*dtx / ndx, ty = Ty0 + x*dty / ndx;
						const tc *col = &texture._atXY((int)(tx / z), (int)(ty / z));
						T *ptrd = ptrd0;
						cimg_forC(*this, c) { *ptrd = (T)(nopacity**col + *ptrd*copacity); ptrd += whd; col += twh; }
					}
				}
				hatch >>= 1; if (!hatch) hatch = ~0U - (~0U >> 1);
				ptrd0 += offx; ptrz += offx;
				if ((error -= dy)<0) { ptrd0 += offy; ptrz += offy; error += dx; }
			}
			else for (int error = dx >> 1, x = 0; x <= dx; ++x) {
				const tzfloat z = Z0 + x*dz / ndx;
				if (z >= (tzfloat)*ptrz) {
					*ptrz = (tz)z;
					const float tx = Tx0 + x*dtx / ndx, ty = Ty0 + x*dty / ndx;
					const tc *col = &texture._atXY((int)(tx / z), (int)(ty / z));
					T *ptrd = ptrd0;
					cimg_forC(*this, c) { *ptrd = (T)(nopacity**col + *ptrd*copacity); ptrd += whd; col += twh; }
				}
				ptrd0 += offx; ptrz += offx;
				if ((error -= dy)<0) { ptrd0 += offy; ptrz += offy; error += dx; }
			}
		}
		return *this;
	}

		template<typename T> 
			template<typename tc>
	CImg<T>& CImg<T>::draw_line(const int x0, const int y0,
		const int x1, const int y1,
		const CImg<tc>& texture,
		const int tx0, const int ty0,
		const int tx1, const int ty1,
		const float opacity = 1,
		const unsigned int pattern = ~0U, const bool init_hatch = true)
	{
		if (is_empty()) return *this;
		if (texture._depth>1 || texture._spectrum<_spectrum)
			throw CImgArgumentException(_cimg_instance
				"draw_line(): Invalid specified texture (%u,%u,%u,%u,%p).",
				cimg_instance,
				texture._width, texture._height, texture._depth, texture._spectrum, texture._data);
		if (is_overlapped(texture)) return draw_line(x0, y0, x1, y1, +texture, tx0, ty0, tx1, ty1, opacity, pattern, init_hatch);
		static unsigned int hatch = ~0U - (~0U >> 1);
		if (init_hatch) hatch = ~0U - (~0U >> 1);
		const bool xdir = x0<x1, ydir = y0<y1;
		int
			dtx = tx1 - tx0, dty = ty1 - ty0,
			nx0 = x0, nx1 = x1, ny0 = y0, ny1 = y1,
			tnx0 = tx0, tnx1 = tx1, tny0 = ty0, tny1 = ty1,
			&xleft = xdir ? nx0 : nx1, &yleft = xdir ? ny0 : ny1, &xright = xdir ? nx1 : nx0, &yright = xdir ? ny1 : ny0,
			&txleft = xdir ? tnx0 : tnx1, &tyleft = xdir ? tny0 : tny1, &txright = xdir ? tnx1 : tnx0, &tyright = xdir ? tny1 : tny0,
			&xup = ydir ? nx0 : nx1, &yup = ydir ? ny0 : ny1, &xdown = ydir ? nx1 : nx0, &ydown = ydir ? ny1 : ny0,
			&txup = ydir ? tnx0 : tnx1, &tyup = ydir ? tny0 : tny1, &txdown = ydir ? tnx1 : tnx0, &tydown = ydir ? tny1 : tny0;
		if (xright<0 || xleft >= width()) return *this;
		if (xleft<0) {
			const float D = (float)xright - xleft;
			yleft -= (int)((float)xleft*((float)yright - yleft) / D);
			txleft -= (int)((float)xleft*((float)txright - txleft) / D);
			tyleft -= (int)((float)xleft*((float)tyright - tyleft) / D);
			xleft = 0;
		}
		if (xright >= width()) {
			const float d = (float)xright - width(), D = (float)xright - xleft;
			yright -= (int)(d*((float)yright - yleft) / D);
			txright -= (int)(d*((float)txright - txleft) / D);
			tyright -= (int)(d*((float)tyright - tyleft) / D);
			xright = width() - 1;
		}
		if (ydown<0 || yup >= height()) return *this;
		if (yup<0) {
			const float D = (float)ydown - yup;
			xup -= (int)((float)yup*((float)xdown - xup) / D);
			txup -= (int)((float)yup*((float)txdown - txup) / D);
			tyup -= (int)((float)yup*((float)tydown - tyup) / D);
			yup = 0;
		}
		if (ydown >= height()) {
			const float d = (float)ydown - height(), D = (float)ydown - yup;
			xdown -= (int)(d*((float)xdown - xup) / D);
			txdown -= (int)(d*((float)txdown - txup) / D);
			tydown -= (int)(d*((float)tydown - tyup) / D);
			ydown = height() - 1;
		}
		T *ptrd0 = data(nx0, ny0);
		int dx = xright - xleft, dy = ydown - yup;
		const bool steep = dy>dx;
		if (steep) cimg::swap(nx0, ny0, nx1, ny1, dx, dy);
		const longT
			offx = (longT)(nx0<nx1 ? 1 : -1)*(steep ? width() : 1),
			offy = (longT)(ny0<ny1 ? 1 : -1)*(steep ? 1 : width()),
			ndx = (longT)(dx>0 ? dx : 1);
		const ulongT
			whd = (ulongT)_width*_height*_depth,
			twh = (ulongT)texture._width*texture._height;

		if (opacity >= 1) {
			if (~pattern) for (int error = dx >> 1, x = 0; x <= dx; ++x) {
				if (pattern&hatch) {
					T *ptrd = ptrd0;
					const int tx = tx0 + x*dtx / ndx, ty = ty0 + x*dty / ndx;
					const tc *col = &texture._atXY(tx, ty);
					cimg_forC(*this, c) { *ptrd = (T)*col; ptrd += whd; col += twh; }
				}
				hatch >>= 1; if (!hatch) hatch = ~0U - (~0U >> 1);
				ptrd0 += offx;
				if ((error -= dy)<0) { ptrd0 += offy; error += dx; }
			}
			else for (int error = dx >> 1, x = 0; x <= dx; ++x) {
				T *ptrd = ptrd0;
				const int tx = tx0 + x*dtx / ndx, ty = ty0 + x*dty / ndx;
				const tc *col = &texture._atXY(tx, ty);
				cimg_forC(*this, c) { *ptrd = (T)*col; ptrd += whd; col += twh; }
				ptrd0 += offx;
				if ((error -= dy)<0) { ptrd0 += offy; error += dx; }
			}
		}
		else {
			const float nopacity = cimg::abs(opacity), copacity = 1 - std::max(opacity, 0.0f);
			if (~pattern) for (int error = dx >> 1, x = 0; x <= dx; ++x) {
				T *ptrd = ptrd0;
				if (pattern&hatch) {
					const int tx = tx0 + x*dtx / ndx, ty = ty0 + x*dty / ndx;
					const tc *col = &texture._atXY(tx, ty);
					cimg_forC(*this, c) { *ptrd = (T)(nopacity**col + *ptrd*copacity); ptrd += whd; col += twh; }
				}
				hatch >>= 1; if (!hatch) hatch = ~0U - (~0U >> 1);
				ptrd0 += offx;
				if ((error -= dy)<0) { ptrd0 += offy; error += dx; }
			}
			else for (int error = dx >> 1, x = 0; x <= dx; ++x) {
				T *ptrd = ptrd0;
				const int tx = tx0 + x*dtx / ndx, ty = ty0 + x*dty / ndx;
				const tc *col = &texture._atXY(tx, ty);
				cimg_forC(*this, c) { *ptrd = (T)(nopacity**col + *ptrd*copacity); ptrd += whd; col += twh; }
				ptrd0 += offx;
				if ((error -= dy)<0) { ptrd0 += offy; error += dx; }
			}
		}
		return *this;
	}

	template<typename T> 
		template<typename t>
	CImg<t> CImg<T>::get_map(const CImg<t>& colormap, const unsigned int boundary_conditions = 0) const
	{
		if (_spectrum != 1 && colormap._spectrum != 1)
			throw CImgArgumentException(_cimg_instance
				"map(): Instance and specified colormap (%u,%u,%u,%u,%p) "
				"have incompatible dimensions.",
				cimg_instance,
				colormap._width, colormap._height, colormap._depth, colormap._spectrum, colormap._data);

		const ulongT
			whd = (ulongT)_width*_height*_depth,
			cwhd = (ulongT)colormap._width*colormap._height*colormap._depth,
			cwhd2 = 2 * cwhd;
		CImg<t> res(_width, _height, _depth, colormap._spectrum == 1 ? _spectrum : colormap._spectrum);
		switch (colormap._spectrum) {

		case 1: { // Optimized for scalars
			const T *ptrs = _data;
			switch (boundary_conditions) {
			case 3: // Mirror
				cimg_for(res, ptrd, t) {
					const ulongT ind = ((ulongT)*(ptrs++)) % cwhd2;
					*ptrd = colormap[ind<cwhd ? ind : cwhd2 - ind - 1];
				}
				break;
			case 2: // Periodic
				cimg_for(res, ptrd, t) {
					const ulongT ind = (ulongT)*(ptrs++);
					*ptrd = colormap[ind%cwhd];
				} break;
			case 1: // Neumann
				cimg_for(res, ptrd, t) {
					const longT ind = (longT)*(ptrs++);
					*ptrd = colormap[cimg::cut(ind, (longT)0, (longT)cwhd - 1)];
				} break;
			default: // Dirichlet
				cimg_for(res, ptrd, t) {
					const ulongT ind = (ulongT)*(ptrs++);
					*ptrd = ind<cwhd ? colormap[ind] : (t)0;
				}
			}
		} break;

		case 2: { // Optimized for 2d vectors.
			const t *const ptrp0 = colormap._data, *ptrp1 = ptrp0 + cwhd;
			t *ptrd0 = res._data, *ptrd1 = ptrd0 + whd;
			switch (boundary_conditions) {
			case 3: // Mirror
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT
						_ind = ((ulongT)*(ptrs++)) % cwhd2,
						ind = _ind<cwhd ? _ind : cwhd2 - _ind - 1;
					*(ptrd0++) = ptrp0[ind]; *(ptrd1++) = ptrp1[ind];
				}
				break;
			case 2: // Periodic
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT ind = ((ulongT)*(ptrs++)) % cwhd;
					*(ptrd0++) = ptrp0[ind]; *(ptrd1++) = ptrp1[ind];
				}
				break;
			case 1: // Neumann
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const longT ind = cimg::cut((longT)*(ptrs++), (longT)0, (longT)cwhd - 1);
					*(ptrd0++) = ptrp0[ind]; *(ptrd1++) = ptrp1[ind];
				}
				break;
			default: // Dirichlet
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT ind = (ulongT)*(ptrs++);
					const bool is_in = ind<cwhd;
					*(ptrd0++) = is_in ? ptrp0[ind] : (t)0; *(ptrd1++) = is_in ? ptrp1[ind] : (t)0;
				}
			}
		} break;

		case 3: { // Optimized for 3d vectors (colors).
			const t *const ptrp0 = colormap._data, *ptrp1 = ptrp0 + cwhd, *ptrp2 = ptrp1 + cwhd;
			t *ptrd0 = res._data, *ptrd1 = ptrd0 + whd, *ptrd2 = ptrd1 + whd;
			switch (boundary_conditions) {
			case 3: // Mirror
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT
						_ind = ((ulongT)*(ptrs++)) % cwhd2,
						ind = _ind<cwhd ? _ind : cwhd2 - _ind - 1;
					*(ptrd0++) = ptrp0[ind]; *(ptrd1++) = ptrp1[ind]; *(ptrd2++) = ptrp2[ind];
				} break;
			case 2: // Periodic
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT ind = ((ulongT)*(ptrs++)) % cwhd;
					*(ptrd0++) = ptrp0[ind]; *(ptrd1++) = ptrp1[ind]; *(ptrd2++) = ptrp2[ind];
				} break;
			case 1: // Neumann
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const longT ind = cimg::cut((longT)*(ptrs++), (longT)0, (longT)cwhd - 1);
					*(ptrd0++) = ptrp0[ind]; *(ptrd1++) = ptrp1[ind]; *(ptrd2++) = ptrp2[ind];
				} break;
			default: // Dirichlet
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT ind = (ulongT)*(ptrs++);
					const bool is_in = ind<cwhd;
					*(ptrd0++) = is_in ? ptrp0[ind] : (t)0; *(ptrd1++) = is_in ? ptrp1[ind] : (t)0; *(ptrd2++) = is_in ? ptrp2[ind] : (t)0;
				}
			}
		} break;

		default: { // Generic version.
			t *ptrd = res._data;
			switch (boundary_conditions) {
			case 3: // Mirror
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT
						_ind = ((ulongT)*(ptrs++)) % cwhd,
						ind = _ind<cwhd ? _ind : cwhd2 - _ind - 1;
					const t *ptrp = colormap._data + ind;
					t *_ptrd = ptrd++; cimg_forC(res, c) { *_ptrd = *ptrp; _ptrd += whd; ptrp += cwhd; }
				} break;
			case 2: // Periodic
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT ind = ((ulongT)*(ptrs++)) % cwhd;
					const t *ptrp = colormap._data + ind;
					t *_ptrd = ptrd++; cimg_forC(res, c) { *_ptrd = *ptrp; _ptrd += whd; ptrp += cwhd; }
				} break;
			case 1: // Neumann
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const longT ind = cimg::cut((longT)*(ptrs++), (longT)0, (longT)cwhd - 1);
					const t *ptrp = colormap._data + ind;
					t *_ptrd = ptrd++; cimg_forC(res, c) { *_ptrd = *ptrp; _ptrd += whd; ptrp += cwhd; }
				} break;
			default: // Dirichlet
				for (const T *ptrs = _data, *ptrs_end = ptrs + whd; ptrs<ptrs_end; ) {
					const ulongT ind = (ulongT)*(ptrs++);
					const bool is_in = ind<cwhd;
					if (is_in) {
						const t *ptrp = colormap._data + ind;
						t *_ptrd = ptrd++; cimg_forC(res, c) { *_ptrd = *ptrp; _ptrd += whd; ptrp += cwhd; }
					}
					else {
						t *_ptrd = ptrd++; cimg_forC(res, c) { *_ptrd = (t)0; _ptrd += whd; }
					}
				}
			}
		}
		}
		return res;
	}

	template<typename T> 
		template<typename t>
	CImg<typename CImg<t>::Tuint>
		CImg<T>::get_index(const CImg<t>& colormap, const float dithering = 1, const bool map_indexes = true) const
	{
		if (colormap._spectrum != _spectrum)
			throw CImgArgumentException(_cimg_instance
				"index(): Instance and specified colormap (%u,%u,%u,%u,%p) "
				"have incompatible dimensions.",
				cimg_instance,
				colormap._width, colormap._height, colormap._depth, colormap._spectrum, colormap._data);

		typedef typename CImg<t>::Tuint tuint;
		if (is_empty()) return CImg<tuint>();
		const ulongT
			whd = (ulongT)_width*_height*_depth,
			pwhd = (ulongT)colormap._width*colormap._height*colormap._depth;
		CImg<tuint> res(_width, _height, _depth, map_indexes ? _spectrum : 1);
		tuint *ptrd = res._data;
		if (dithering>0) { // Dithered versions.
			const float ndithering = cimg::cut(dithering, 0, 1) / 16;
			Tfloat valm = 0, valM = (Tfloat)max_min(valm);
			if (valm == valM && valm >= 0 && valM <= 255) { valm = 0; valM = 255; }
			CImg<Tfloat> cache = get_crop(-1, 0, 0, 0, _width, 1, 0, _spectrum - 1);
			Tfloat *cache_current = cache.data(1, 0, 0, 0), *cache_next = cache.data(1, 1, 0, 0);
			const ulongT cwhd = (ulongT)cache._width*cache._height*cache._depth;
			switch (_spectrum) {
			case 1: { // Optimized for scalars.
				cimg_forYZ(*this, y, z) {
					if (y<height() - 2) {
						Tfloat *ptrc0 = cache_next; const T *ptrs0 = data(0, y + 1, z, 0);
						cimg_forX(*this, x) *(ptrc0++) = (Tfloat)*(ptrs0++);
					}
					Tfloat *ptrs0 = cache_current, *ptrsn0 = cache_next;
					cimg_forX(*this, x) {
						const Tfloat _val0 = (Tfloat)*ptrs0, val0 = _val0<valm ? valm : _val0>valM ? valM : _val0;
						Tfloat distmin = cimg::type<Tfloat>::max(); const t *ptrmin0 = colormap._data;
						for (const t *ptrp0 = colormap._data, *ptrp_end = ptrp0 + pwhd; ptrp0<ptrp_end; ) {
							const Tfloat pval0 = (Tfloat)*(ptrp0++) - val0, dist = pval0*pval0;
							if (dist<distmin) { ptrmin0 = ptrp0 - 1; distmin = dist; }
						}
						const Tfloat err0 = ((*(ptrs0++) = val0) - (Tfloat)*ptrmin0)*ndithering;
						*ptrs0 += 7 * err0; *(ptrsn0 - 1) += 3 * err0; *(ptrsn0++) += 5 * err0; *ptrsn0 += err0;
						if (map_indexes) *(ptrd++) = (tuint)*ptrmin0; else *(ptrd++) = (tuint)(ptrmin0 - colormap._data);
					}
					cimg::swap(cache_current, cache_next);
				}
			} break;
			case 2: { // Optimized for 2d vectors.
				tuint *ptrd1 = ptrd + whd;
				cimg_forYZ(*this, y, z) {
					if (y<height() - 2) {
						Tfloat *ptrc0 = cache_next, *ptrc1 = ptrc0 + cwhd;
						const T *ptrs0 = data(0, y + 1, z, 0), *ptrs1 = ptrs0 + whd;
						cimg_forX(*this, x) { *(ptrc0++) = (Tfloat)*(ptrs0++); *(ptrc1++) = (Tfloat)*(ptrs1++); }
					}
					Tfloat
						*ptrs0 = cache_current, *ptrs1 = ptrs0 + cwhd,
						*ptrsn0 = cache_next, *ptrsn1 = ptrsn0 + cwhd;
					cimg_forX(*this, x) {
						const Tfloat
							_val0 = (Tfloat)*ptrs0, val0 = _val0<valm ? valm : _val0>valM ? valM : _val0,
							_val1 = (Tfloat)*ptrs1, val1 = _val1<valm ? valm : _val1>valM ? valM : _val1;
						Tfloat distmin = cimg::type<Tfloat>::max(); const t *ptrmin0 = colormap._data;
						for (const t *ptrp0 = colormap._data, *ptrp1 = ptrp0 + pwhd, *ptrp_end = ptrp1; ptrp0<ptrp_end; ) {
							const Tfloat
								pval0 = (Tfloat)*(ptrp0++) - val0, pval1 = (Tfloat)*(ptrp1++) - val1,
								dist = pval0*pval0 + pval1*pval1;
							if (dist<distmin) { ptrmin0 = ptrp0 - 1; distmin = dist; }
						}
						const t *const ptrmin1 = ptrmin0 + pwhd;
						const Tfloat
							err0 = ((*(ptrs0++) = val0) - (Tfloat)*ptrmin0)*ndithering,
							err1 = ((*(ptrs1++) = val1) - (Tfloat)*ptrmin1)*ndithering;
						*ptrs0 += 7 * err0; *ptrs1 += 7 * err1;
						*(ptrsn0 - 1) += 3 * err0; *(ptrsn1 - 1) += 3 * err1;
						*(ptrsn0++) += 5 * err0; *(ptrsn1++) += 5 * err1;
						*ptrsn0 += err0; *ptrsn1 += err1;
						if (map_indexes) { *(ptrd++) = (tuint)*ptrmin0; *(ptrd1++) = (tuint)*ptrmin1; }
						else *(ptrd++) = (tuint)(ptrmin0 - colormap._data);
					}
					cimg::swap(cache_current, cache_next);
				}
			} break;
			case 3: { // Optimized for 3d vectors (colors).
				tuint *ptrd1 = ptrd + whd, *ptrd2 = ptrd1 + whd;
				cimg_forYZ(*this, y, z) {
					if (y<height() - 2) {
						Tfloat *ptrc0 = cache_next, *ptrc1 = ptrc0 + cwhd, *ptrc2 = ptrc1 + cwhd;
						const T *ptrs0 = data(0, y + 1, z, 0), *ptrs1 = ptrs0 + whd, *ptrs2 = ptrs1 + whd;
						cimg_forX(*this, x) {
							*(ptrc0++) = (Tfloat)*(ptrs0++); *(ptrc1++) = (Tfloat)*(ptrs1++); *(ptrc2++) = (Tfloat)*(ptrs2++);
						}
					}
					Tfloat
						*ptrs0 = cache_current, *ptrs1 = ptrs0 + cwhd, *ptrs2 = ptrs1 + cwhd,
						*ptrsn0 = cache_next, *ptrsn1 = ptrsn0 + cwhd, *ptrsn2 = ptrsn1 + cwhd;
					cimg_forX(*this, x) {
						const Tfloat
							_val0 = (Tfloat)*ptrs0, val0 = _val0<valm ? valm : _val0>valM ? valM : _val0,
							_val1 = (Tfloat)*ptrs1, val1 = _val1<valm ? valm : _val1>valM ? valM : _val1,
							_val2 = (Tfloat)*ptrs2, val2 = _val2<valm ? valm : _val2>valM ? valM : _val2;
						Tfloat distmin = cimg::type<Tfloat>::max(); const t *ptrmin0 = colormap._data;
						for (const t *ptrp0 = colormap._data, *ptrp1 = ptrp0 + pwhd, *ptrp2 = ptrp1 + pwhd,
							*ptrp_end = ptrp1; ptrp0<ptrp_end; ) {
							const Tfloat
								pval0 = (Tfloat)*(ptrp0++) - val0,
								pval1 = (Tfloat)*(ptrp1++) - val1,
								pval2 = (Tfloat)*(ptrp2++) - val2,
								dist = pval0*pval0 + pval1*pval1 + pval2*pval2;
							if (dist<distmin) { ptrmin0 = ptrp0 - 1; distmin = dist; }
						}
						const t *const ptrmin1 = ptrmin0 + pwhd, *const ptrmin2 = ptrmin1 + pwhd;
						const Tfloat
							err0 = ((*(ptrs0++) = val0) - (Tfloat)*ptrmin0)*ndithering,
							err1 = ((*(ptrs1++) = val1) - (Tfloat)*ptrmin1)*ndithering,
							err2 = ((*(ptrs2++) = val2) - (Tfloat)*ptrmin2)*ndithering;

						*ptrs0 += 7 * err0; *ptrs1 += 7 * err1; *ptrs2 += 7 * err2;
						*(ptrsn0 - 1) += 3 * err0; *(ptrsn1 - 1) += 3 * err1; *(ptrsn2 - 1) += 3 * err2;
						*(ptrsn0++) += 5 * err0; *(ptrsn1++) += 5 * err1; *(ptrsn2++) += 5 * err2;
						*ptrsn0 += err0; *ptrsn1 += err1; *ptrsn2 += err2;

						if (map_indexes) {
							*(ptrd++) = (tuint)*ptrmin0; *(ptrd1++) = (tuint)*ptrmin1; *(ptrd2++) = (tuint)*ptrmin2;
						}
						else *(ptrd++) = (tuint)(ptrmin0 - colormap._data);
					}
					cimg::swap(cache_current, cache_next);
				}
			} break;
			default: // Generic version
				cimg_forYZ(*this, y, z) {
					if (y<height() - 2) {
						Tfloat *ptrc = cache_next;
						cimg_forC(*this, c) {
							Tfloat *_ptrc = ptrc; const T *_ptrs = data(0, y + 1, z, c);
							cimg_forX(*this, x) *(_ptrc++) = (Tfloat)*(_ptrs++);
							ptrc += cwhd;
						}
					}
					Tfloat *ptrs = cache_current, *ptrsn = cache_next;
					cimg_forX(*this, x) {
						Tfloat distmin = cimg::type<Tfloat>::max(); const t *ptrmin = colormap._data;
						for (const t *ptrp = colormap._data, *ptrp_end = ptrp + pwhd; ptrp<ptrp_end; ++ptrp) {
							Tfloat dist = 0; Tfloat *_ptrs = ptrs; const t *_ptrp = ptrp;
							cimg_forC(*this, c) {
								const Tfloat _val = *_ptrs, val = _val<valm ? valm : _val>valM ? valM : _val;
								dist += cimg::sqr((*_ptrs = val) - (Tfloat)*_ptrp); _ptrs += cwhd; _ptrp += pwhd;
							}
							if (dist<distmin) { ptrmin = ptrp; distmin = dist; }
						}
						const t *_ptrmin = ptrmin; Tfloat *_ptrs = ptrs++, *_ptrsn = (ptrsn++) - 1;
						cimg_forC(*this, c) {
							const Tfloat err = (*(_ptrs++) - (Tfloat)*_ptrmin)*ndithering;
							*_ptrs += 7 * err; *(_ptrsn++) += 3 * err; *(_ptrsn++) += 5 * err; *_ptrsn += err;
							_ptrmin += pwhd; _ptrs += cwhd - 1; _ptrsn += cwhd - 2;
						}
						if (map_indexes) {
							tuint *_ptrd = ptrd++;
							cimg_forC(*this, c) { *_ptrd = (tuint)*ptrmin; _ptrd += whd; ptrmin += pwhd; }
						}
						else *(ptrd++) = (tuint)(ptrmin - colormap._data);
					}
					cimg::swap(cache_current, cache_next);
				}
			}
		}
		else { // Non-dithered versions
			switch (_spectrum) {
			case 1: { // Optimized for scalars.
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(_width >= 64 && _height*_depth >= 16 && pwhd >= 16))
					cimg_forYZ(*this, y, z) {
					tuint *ptrd = res.data(0, y, z);
					for (const T *ptrs0 = data(0, y, z), *ptrs_end = ptrs0 + _width; ptrs0<ptrs_end; ) {
						const Tfloat val0 = (Tfloat)*(ptrs0++);
						Tfloat distmin = cimg::type<Tfloat>::max(); const t *ptrmin0 = colormap._data;
						for (const t *ptrp0 = colormap._data, *ptrp_end = ptrp0 + pwhd; ptrp0<ptrp_end; ) {
							const Tfloat pval0 = (Tfloat)*(ptrp0++) - val0, dist = pval0*pval0;
							if (dist<distmin) { ptrmin0 = ptrp0 - 1; distmin = dist; }
						}
						if (map_indexes) *(ptrd++) = (tuint)*ptrmin0; else *(ptrd++) = (tuint)(ptrmin0 - colormap._data);
					}
				}
			} break;
			case 2: { // Optimized for 2d vectors.
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(_width >= 64 && _height*_depth >= 16 && pwhd >= 16))
					cimg_forYZ(*this, y, z) {
					tuint *ptrd = res.data(0, y, z), *ptrd1 = ptrd + whd;
					for (const T *ptrs0 = data(0, y, z), *ptrs1 = ptrs0 + whd, *ptrs_end = ptrs0 + _width; ptrs0<ptrs_end; ) {
						const Tfloat val0 = (Tfloat)*(ptrs0++), val1 = (Tfloat)*(ptrs1++);
						Tfloat distmin = cimg::type<Tfloat>::max(); const t *ptrmin0 = colormap._data;
						for (const t *ptrp0 = colormap._data, *ptrp1 = ptrp0 + pwhd, *ptrp_end = ptrp1; ptrp0<ptrp_end; ) {
							const Tfloat
								pval0 = (Tfloat)*(ptrp0++) - val0, pval1 = (Tfloat)*(ptrp1++) - val1,
								dist = pval0*pval0 + pval1*pval1;
							if (dist<distmin) { ptrmin0 = ptrp0 - 1; distmin = dist; }
						}
						if (map_indexes) { *(ptrd++) = (tuint)*ptrmin0; *(ptrd1++) = (tuint)*(ptrmin0 + pwhd); }
						else *(ptrd++) = (tuint)(ptrmin0 - colormap._data);
					}
				}
			} break;
			case 3: { // Optimized for 3d vectors (colors).
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(_width >= 64 && _height*_depth >= 16 && pwhd >= 16))
					cimg_forYZ(*this, y, z) {
					tuint *ptrd = res.data(0, y, z), *ptrd1 = ptrd + whd, *ptrd2 = ptrd1 + whd;
					for (const T *ptrs0 = data(0, y, z), *ptrs1 = ptrs0 + whd, *ptrs2 = ptrs1 + whd,
						*ptrs_end = ptrs0 + _width; ptrs0<ptrs_end; ) {
						const Tfloat val0 = (Tfloat)*(ptrs0++), val1 = (Tfloat)*(ptrs1++), val2 = (Tfloat)*(ptrs2++);
						Tfloat distmin = cimg::type<Tfloat>::max(); const t *ptrmin0 = colormap._data;
						for (const t *ptrp0 = colormap._data, *ptrp1 = ptrp0 + pwhd, *ptrp2 = ptrp1 + pwhd,
							*ptrp_end = ptrp1; ptrp0<ptrp_end; ) {
							const Tfloat
								pval0 = (Tfloat)*(ptrp0++) - val0,
								pval1 = (Tfloat)*(ptrp1++) - val1,
								pval2 = (Tfloat)*(ptrp2++) - val2,
								dist = pval0*pval0 + pval1*pval1 + pval2*pval2;
							if (dist<distmin) { ptrmin0 = ptrp0 - 1; distmin = dist; }
						}
						if (map_indexes) {
							*(ptrd++) = (tuint)*ptrmin0;
							*(ptrd1++) = (tuint)*(ptrmin0 + pwhd);
							*(ptrd2++) = (tuint)*(ptrmin0 + 2 * pwhd);
						}
						else *(ptrd++) = (tuint)(ptrmin0 - colormap._data);
					}
				}
			} break;
			default: // Generic version.
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(_width >= 64 && _height*_depth >= 16 && pwhd >= 16))
					cimg_forYZ(*this, y, z) {
					tuint *ptrd = res.data(0, y, z);
					for (const T *ptrs = data(0, y, z), *ptrs_end = ptrs + _width; ptrs<ptrs_end; ++ptrs) {
						Tfloat distmin = cimg::type<Tfloat>::max(); const t *ptrmin = colormap._data;
						for (const t *ptrp = colormap._data, *ptrp_end = ptrp + pwhd; ptrp<ptrp_end; ++ptrp) {
							Tfloat dist = 0; const T *_ptrs = ptrs; const t *_ptrp = ptrp;
							cimg_forC(*this, c) { dist += cimg::sqr((Tfloat)*_ptrs - (Tfloat)*_ptrp); _ptrs += whd; _ptrp += pwhd; }
							if (dist<distmin) { ptrmin = ptrp; distmin = dist; }
						}
						if (map_indexes) {
							tuint *_ptrd = ptrd++;
							cimg_forC(*this, c) { *_ptrd = (tuint)*ptrmin; _ptrd += whd; ptrmin += pwhd; }
						}
						else *(ptrd++) = (tuint)(ptrmin - colormap._data);
					}
				}
			}
		}
		return res;
	}

	template<typename T> 
		template<typename t>
	CImg<T>& CImg<T>::blur_bilateral(const CImg<t>& guide,
		const float sigma_x, const float sigma_y,
		const float sigma_z, const float sigma_r,
		const float sampling_x, const float sampling_y,
		const float sampling_z, const float sampling_r)
	{
		if (!is_sameXYZ(guide))
			throw CImgArgumentException(_cimg_instance
				"blur_bilateral(): Invalid size for specified guide image (%u,%u,%u,%u,%p).",
				cimg_instance,
				guide._width, guide._height, guide._depth, guide._spectrum, guide._data);
		if (is_empty() || (!sigma_x && !sigma_y && !sigma_z)) return *this;
		T edge_min, edge_max = guide.max_min(edge_min);
		if (edge_min == edge_max) return blur(sigma_x, sigma_y, sigma_z);
		const float
			edge_delta = (float)(edge_max - edge_min),
			_sigma_x = sigma_x >= 0 ? sigma_x : -sigma_x*_width / 100,
			_sigma_y = sigma_y >= 0 ? sigma_y : -sigma_y*_height / 100,
			_sigma_z = sigma_z >= 0 ? sigma_z : -sigma_z*_depth / 100,
			_sigma_r = sigma_r >= 0 ? sigma_r : -sigma_r*(edge_max - edge_min) / 100,
			_sampling_x = sampling_x ? sampling_x : std::max(_sigma_x, 1.0f),
			_sampling_y = sampling_y ? sampling_y : std::max(_sigma_y, 1.0f),
			_sampling_z = sampling_z ? sampling_z : std::max(_sigma_z, 1.0f),
			_sampling_r = sampling_r ? sampling_r : std::max(_sigma_r, edge_delta / 256),
			derived_sigma_x = _sigma_x / _sampling_x,
			derived_sigma_y = _sigma_y / _sampling_y,
			derived_sigma_z = _sigma_z / _sampling_z,
			derived_sigma_r = _sigma_r / _sampling_r;
		const int
			padding_x = (int)(2 * derived_sigma_x) + 1,
			padding_y = (int)(2 * derived_sigma_y) + 1,
			padding_z = (int)(2 * derived_sigma_z) + 1,
			padding_r = (int)(2 * derived_sigma_r) + 1;
		const unsigned int
			bx = (unsigned int)((_width - 1) / _sampling_x + 1 + 2 * padding_x),
			by = (unsigned int)((_height - 1) / _sampling_y + 1 + 2 * padding_y),
			bz = (unsigned int)((_depth - 1) / _sampling_z + 1 + 2 * padding_z),
			br = (unsigned int)(edge_delta / _sampling_r + 1 + 2 * padding_r);
		if (bx>0 || by>0 || bz>0 || br>0) {
			const bool is_3d = (_depth>1);
			if (is_3d) { // 3d version of the algorithm
				CImg<floatT> bgrid(bx, by, bz, br), bgridw(bx, by, bz, br);
				cimg_forC(*this, c) {
					const CImg<t> _guide = guide.get_shared_channel(c%guide._spectrum);
					bgrid.fill(0); bgridw.fill(0);
					cimg_forXYZ(*this, x, y, z) {
						const T val = (*this)(x, y, z, c);
						const float edge = (float)_guide(x, y, z);
						const int
							X = (int)cimg::round(x / _sampling_x) + padding_x,
							Y = (int)cimg::round(y / _sampling_y) + padding_y,
							Z = (int)cimg::round(z / _sampling_z) + padding_z,
							R = (int)cimg::round((edge - edge_min) / _sampling_r) + padding_r;
						bgrid(X, Y, Z, R) += (float)val;
						bgridw(X, Y, Z, R) += 1;
					}
					bgrid.blur(derived_sigma_x, derived_sigma_y, derived_sigma_z, true).deriche(derived_sigma_r, 0, 'c', false);
					bgridw.blur(derived_sigma_x, derived_sigma_y, derived_sigma_z, true).deriche(derived_sigma_r, 0, 'c', false);

					cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(size() >= 4096))
						cimg_forXYZ(*this, x, y, z) {
						const float edge = (float)_guide(x, y, z);
						const float
							X = x / _sampling_x + padding_x,
							Y = y / _sampling_y + padding_y,
							Z = z / _sampling_z + padding_z,
							R = (edge - edge_min) / _sampling_r + padding_r;
						const float bval0 = bgrid._linear_atXYZC(X, Y, Z, R), bval1 = bgridw._linear_atXYZC(X, Y, Z, R);
						(*this)(x, y, z, c) = (T)(bval0 / bval1);
					}
				}
			}
			else { // 2d version of the algorithm
				CImg<floatT> bgrid(bx, by, br, 2);
				cimg_forC(*this, c) {
					const CImg<t> _guide = guide.get_shared_channel(c%guide._spectrum);
					bgrid.fill(0);
					cimg_forXY(*this, x, y) {
						const T val = (*this)(x, y, c);
						const float edge = (float)_guide(x, y);
						const int
							X = (int)cimg::round(x / _sampling_x) + padding_x,
							Y = (int)cimg::round(y / _sampling_y) + padding_y,
							R = (int)cimg::round((edge - edge_min) / _sampling_r) + padding_r;
						bgrid(X, Y, R, 0) += (float)val;
						bgrid(X, Y, R, 1) += 1;
					}
					bgrid.blur(derived_sigma_x, derived_sigma_y, 0, true).blur(0, 0, derived_sigma_r, false);

					cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(size() >= 4096))
						cimg_forXY(*this, x, y) {
						const float edge = (float)_guide(x, y);
						const float
							X = x / _sampling_x + padding_x,
							Y = y / _sampling_y + padding_y,
							R = (edge - edge_min) / _sampling_r + padding_r;
						const float bval0 = bgrid._linear_atXYZ(X, Y, R, 0), bval1 = bgrid._linear_atXYZ(X, Y, R, 1);
						(*this)(x, y, c) = (T)(bval0 / bval1);
					}
				}
			}
		}
		return *this;
	}

	template<typename T> 
		template<typename t>
	CImg<T>& CImg<T>::blur_anisotropic(const CImg<t>& G,
		const float amplitude = 60, const float dl = 0.8f, const float da = 30,
		const float gauss_prec = 2, const unsigned int interpolation_type = 0,
		const bool is_fast_approx = 1)
	{

		// Check arguments and init variables
		if (!is_sameXYZ(G) || (G._spectrum != 3 && G._spectrum != 6))
			throw CImgArgumentException(_cimg_instance
				"blur_anisotropic(): Invalid specified diffusion tensor field (%u,%u,%u,%u,%p).",
				cimg_instance,
				G._width, G._height, G._depth, G._spectrum, G._data);

		if (is_empty() || amplitude <= 0 || dl<0) return *this;
		const bool is_3d = (G._spectrum == 6);
		T val_min, val_max = max_min(val_min);
		cimg_abort_init;

		if (da <= 0) {  // Iterated oriented Laplacians
			CImg<Tfloat> velocity(_width, _height, _depth, _spectrum);
			for (unsigned int iteration = 0; iteration<(unsigned int)amplitude; ++iteration) {
				Tfloat *ptrd = velocity._data, veloc_max = 0;
				if (is_3d) // 3d version
					cimg_forC(*this, c) {
					cimg_abort_test();
					CImg_3x3x3(I, Tfloat);
					cimg_for3x3x3(*this, x, y, z, c, I, Tfloat) {
						const Tfloat
							ixx = Incc + Ipcc - 2 * Iccc,
							ixy = (Innc + Ippc - Inpc - Ipnc) / 4,
							ixz = (Incn + Ipcp - Incp - Ipcn) / 4,
							iyy = Icnc + Icpc - 2 * Iccc,
							iyz = (Icnn + Icpp - Icnp - Icpn) / 4,
							izz = Iccn + Iccp - 2 * Iccc,
							veloc = (Tfloat)(G(x, y, z, 0)*ixx + 2 * G(x, y, z, 1)*ixy + 2 * G(x, y, z, 2)*ixz +
								G(x, y, z, 3)*iyy + 2 * G(x, y, z, 4)*iyz + G(x, y, z, 5)*izz);
						*(ptrd++) = veloc;
						if (veloc>veloc_max) veloc_max = veloc; else if (-veloc>veloc_max) veloc_max = -veloc;
					}
				}
				else // 2d version
					cimg_forZC(*this, z, c) {
					cimg_abort_test();
					CImg_3x3(I, Tfloat);
					cimg_for3x3(*this, x, y, z, c, I, Tfloat) {
						const Tfloat
							ixx = Inc + Ipc - 2 * Icc,
							ixy = (Inn + Ipp - Inp - Ipn) / 4,
							iyy = Icn + Icp - 2 * Icc,
							veloc = (Tfloat)(G(x, y, 0, 0)*ixx + 2 * G(x, y, 0, 1)*ixy + G(x, y, 0, 2)*iyy);
						*(ptrd++) = veloc;
						if (veloc>veloc_max) veloc_max = veloc; else if (-veloc>veloc_max) veloc_max = -veloc;
					}
				}
				if (veloc_max>0) *this += (velocity *= dl / veloc_max);
			}
		}
		else { // LIC-based smoothing.
			const ulongT whd = (ulongT)_width*_height*_depth;
			const float sqrt2amplitude = (float)std::sqrt(2 * amplitude);
			const int dx1 = width() - 1, dy1 = height() - 1, dz1 = depth() - 1;
			CImg<Tfloat> res(_width, _height, _depth, _spectrum, 0), W(_width, _height, _depth, is_3d ? 4 : 3), val(_spectrum, 1, 1, 1, 0);
			int N = 0;
			if (is_3d) { // 3d version
				for (float phi = cimg::mod(180.0f, da) / 2.0f; phi <= 180; phi += da) {
					const float phir = (float)(phi*cimg::PI / 180), datmp = (float)(da / std::cos(phir)),
						da2 = datmp<1 ? 360.0f : datmp;
					for (float theta = 0; theta<360; (theta += da2), ++N) {
						const float
							thetar = (float)(theta*cimg::PI / 180),
							vx = (float)(std::cos(thetar)*std::cos(phir)),
							vy = (float)(std::sin(thetar)*std::cos(phir)),
							vz = (float)std::sin(phir);
						const t
							*pa = G.data(0, 0, 0, 0), *pb = G.data(0, 0, 0, 1), *pc = G.data(0, 0, 0, 2),
							*pd = G.data(0, 0, 0, 3), *pe = G.data(0, 0, 0, 4), *pf = G.data(0, 0, 0, 5);
						Tfloat *pd0 = W.data(0, 0, 0, 0), *pd1 = W.data(0, 0, 0, 1), *pd2 = W.data(0, 0, 0, 2), *pd3 = W.data(0, 0, 0, 3);
						cimg_forXYZ(G, xg, yg, zg) {
							const t a = *(pa++), b = *(pb++), c = *(pc++), d = *(pd++), e = *(pe++), f = *(pf++);
							const float
								u = (float)(a*vx + b*vy + c*vz),
								v = (float)(b*vx + d*vy + e*vz),
								w = (float)(c*vx + e*vy + f*vz),
								n = 1e-5f + cimg::hypot(u, v, w),
								dln = dl / n;
							*(pd0++) = (Tfloat)(u*dln);
							*(pd1++) = (Tfloat)(v*dln);
							*(pd2++) = (Tfloat)(w*dln);
							*(pd3++) = (Tfloat)n;
						}

						cimg_abort_test();
						cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(_width >= 256 && _height*_depth >= 2)
							firstprivate(val))
							cimg_forYZ(*this, y, z) cimg_abort_try2 {
							cimg_abort_test2();
							cimg_forX(*this, x) {
								val.fill(0);
								const float
									n = (float)W(x, y, z, 3),
									fsigma = (float)(n*sqrt2amplitude),
									fsigma2 = 2 * fsigma*fsigma,
									length = gauss_prec*fsigma;
								float
									S = 0,
									X = (float)x,
									Y = (float)y,
									Z = (float)z;
								switch (interpolation_type) {
								case 0: { // Nearest neighbor
									for (float l = 0; l<length && X >= 0 && X <= dx1 && Y >= 0 && Y <= dy1 && Z >= 0 && Z <= dz1; l += dl) {
										const int
											cx = (int)(X + 0.5f),
											cy = (int)(Y + 0.5f),
											cz = (int)(Z + 0.5f);
										const float
											u = (float)W(cx, cy, cz, 0),
											v = (float)W(cx, cy, cz, 1),
											w = (float)W(cx, cy, cz, 2);
										if (is_fast_approx) { cimg_forC(*this, c) val[c] += (Tfloat)(*this)(cx, cy, cz, c); ++S; }
										else {
											const float coef = (float)std::exp(-l*l / fsigma2);
											cimg_forC(*this, c) val[c] += (Tfloat)(coef*(*this)(cx, cy, cz, c));
											S += coef;
										}
										X += u; Y += v; Z += w;
									}
								} break;
								case 1: { // Linear interpolation
									for (float l = 0; l<length && X >= 0 && X <= dx1 && Y >= 0 && Y <= dy1 && Z >= 0 && Z <= dz1; l += dl) {
										const float
											u = (float)(W._linear_atXYZ(X, Y, Z, 0)),
											v = (float)(W._linear_atXYZ(X, Y, Z, 1)),
											w = (float)(W._linear_atXYZ(X, Y, Z, 2));
										if (is_fast_approx) { cimg_forC(*this, c) val[c] += (Tfloat)_linear_atXYZ(X, Y, Z, c); ++S; }
										else {
											const float coef = (float)std::exp(-l*l / fsigma2);
											cimg_forC(*this, c) val[c] += (Tfloat)(coef*_linear_atXYZ(X, Y, Z, c));
											S += coef;
										}
										X += u; Y += v; Z += w;
									}
								} break;
								default: { // 2nd order Runge Kutta
									for (float l = 0; l<length && X >= 0 && X <= dx1 && Y >= 0 && Y <= dy1 && Z >= 0 && Z <= dz1; l += dl) {
										const float
											u0 = (float)(0.5f*W._linear_atXYZ(X, Y, Z, 0)),
											v0 = (float)(0.5f*W._linear_atXYZ(X, Y, Z, 1)),
											w0 = (float)(0.5f*W._linear_atXYZ(X, Y, Z, 2)),
											u = (float)(W._linear_atXYZ(X + u0, Y + v0, Z + w0, 0)),
											v = (float)(W._linear_atXYZ(X + u0, Y + v0, Z + w0, 1)),
											w = (float)(W._linear_atXYZ(X + u0, Y + v0, Z + w0, 2));
										if (is_fast_approx) { cimg_forC(*this, c) val[c] += (Tfloat)_linear_atXYZ(X, Y, Z, c); ++S; }
										else {
											const float coef = (float)std::exp(-l*l / fsigma2);
											cimg_forC(*this, c) val[c] += (Tfloat)(coef*_linear_atXYZ(X, Y, Z, c));
											S += coef;
										}
										X += u; Y += v; Z += w;
									}
								} break;
								}
								Tfloat *ptrd = res.data(x, y, z);
								if (S>0) cimg_forC(res, c) { *ptrd += val[c] / S; ptrd += whd; }
								else cimg_forC(res, c) { *ptrd += (Tfloat)((*this)(x, y, z, c)); ptrd += whd; }
							}
						} cimg_abort_catch2()
					}
				}
			}
			else { // 2d LIC algorithm
				for (float theta = cimg::mod(360.0f, da) / 2.0f; theta<360; (theta += da), ++N) {
					const float thetar = (float)(theta*cimg::PI / 180),
						vx = (float)(std::cos(thetar)), vy = (float)(std::sin(thetar));
					const t *pa = G.data(0, 0, 0, 0), *pb = G.data(0, 0, 0, 1), *pc = G.data(0, 0, 0, 2);
					Tfloat *pd0 = W.data(0, 0, 0, 0), *pd1 = W.data(0, 0, 0, 1), *pd2 = W.data(0, 0, 0, 2);
					cimg_forXY(G, xg, yg) {
						const t a = *(pa++), b = *(pb++), c = *(pc++);
						const float
							u = (float)(a*vx + b*vy),
							v = (float)(b*vx + c*vy),
							n = std::max(1e-5f, cimg::hypot(u, v)),
							dln = dl / n;
						*(pd0++) = (Tfloat)(u*dln);
						*(pd1++) = (Tfloat)(v*dln);
						*(pd2++) = (Tfloat)n;
					}

					cimg_abort_test();
					cimg_pragma_openmp(parallel for cimg_openmp_if(_width >= 256 && _height >= 2) firstprivate(val))
						cimg_forY(*this, y) cimg_abort_try2 {
						cimg_abort_test2();
						cimg_forX(*this, x) {
							val.fill(0);
							const float
								n = (float)W(x, y, 0, 2),
								fsigma = (float)(n*sqrt2amplitude),
								fsigma2 = 2 * fsigma*fsigma,
								length = gauss_prec*fsigma;
							float
								S = 0,
								X = (float)x,
								Y = (float)y;
							switch (interpolation_type) {
							case 0: { // Nearest-neighbor
								for (float l = 0; l<length && X >= 0 && X <= dx1 && Y >= 0 && Y <= dy1; l += dl) {
									const int
										cx = (int)(X + 0.5f),
										cy = (int)(Y + 0.5f);
									const float
										u = (float)W(cx, cy, 0, 0),
										v = (float)W(cx, cy, 0, 1);
									if (is_fast_approx) { cimg_forC(*this, c) val[c] += (Tfloat)(*this)(cx, cy, 0, c); ++S; }
									else {
										const float coef = (float)std::exp(-l*l / fsigma2);
										cimg_forC(*this, c) val[c] += (Tfloat)(coef*(*this)(cx, cy, 0, c));
										S += coef;
									}
									X += u; Y += v;
								}
							} break;
							case 1: { // Linear interpolation
								for (float l = 0; l<length && X >= 0 && X <= dx1 && Y >= 0 && Y <= dy1; l += dl) {
									const float
										u = (float)(W._linear_atXY(X, Y, 0, 0)),
										v = (float)(W._linear_atXY(X, Y, 0, 1));
									if (is_fast_approx) { cimg_forC(*this, c) val[c] += (Tfloat)_linear_atXY(X, Y, 0, c); ++S; }
									else {
										const float coef = (float)std::exp(-l*l / fsigma2);
										cimg_forC(*this, c) val[c] += (Tfloat)(coef*_linear_atXY(X, Y, 0, c));
										S += coef;
									}
									X += u; Y += v;
								}
							} break;
							default: { // 2nd-order Runge-kutta interpolation
								for (float l = 0; l<length && X >= 0 && X <= dx1 && Y >= 0 && Y <= dy1; l += dl) {
									const float
										u0 = (float)(0.5f*W._linear_atXY(X, Y, 0, 0)),
										v0 = (float)(0.5f*W._linear_atXY(X, Y, 0, 1)),
										u = (float)(W._linear_atXY(X + u0, Y + v0, 0, 0)),
										v = (float)(W._linear_atXY(X + u0, Y + v0, 0, 1));
									if (is_fast_approx) { cimg_forC(*this, c) val[c] += (Tfloat)_linear_atXY(X, Y, 0, c); ++S; }
									else {
										const float coef = (float)std::exp(-l*l / fsigma2);
										cimg_forC(*this, c) val[c] += (Tfloat)(coef*_linear_atXY(X, Y, 0, c));
										S += coef;
									}
									X += u; Y += v;
								}
							}
							}
							Tfloat *ptrd = res.data(x, y);
							if (S>0) cimg_forC(res, c) { *ptrd += val[c] / S; ptrd += whd; }
							else cimg_forC(res, c) { *ptrd += (Tfloat)((*this)(x, y, 0, c)); ptrd += whd; }
						}
					} cimg_abort_catch2()
				}
			}
			const Tfloat *ptrs = res._data;
			cimg_for(*this, ptrd, T) {
				const Tfloat val = *(ptrs++) / N;
				*ptrd = val<val_min ? val_min : (val>val_max ? val_max : (T)val);
			}
		}
		cimg_abort_test();
		return *this;
	}
	template<typename T> 
		CImg<T>& CImg<T>::deriche(const float sigma, const unsigned int order = 0, const char axis = 'x',
			const bool boundary_conditions = true)
	{
#define _cimg_deriche_apply \
  CImg<Tfloat> Y(N); \
  Tfloat *ptrY = Y._data, yb = 0, yp = 0; \
  T xp = (T)0; \
  if (boundary_conditions) { xp = *ptrX; yb = yp = (Tfloat)(coefp*xp); } \
  for (int m = 0; m<N; ++m) { \
    const T xc = *ptrX; ptrX+=off; \
    const Tfloat yc = *(ptrY++) = (Tfloat)(a0*xc + a1*xp - b1*yp - b2*yb); \
    xp = xc; yb = yp; yp = yc; \
  } \
  T xn = (T)0, xa = (T)0; \
  Tfloat yn = 0, ya = 0; \
  if (boundary_conditions) { xn = xa = *(ptrX-off); yn = ya = (Tfloat)coefn*xn; } \
  for (int n = N - 1; n>=0; --n) { \
    const T xc = *(ptrX-=off); \
    const Tfloat yc = (Tfloat)(a2*xn + a3*xa - b1*yn - b2*ya); \
    xa = xn; xn = xc; ya = yn; yn = yc; \
    *ptrX = (T)(*(--ptrY)+yc); \
  }
		const char naxis = cimg::lowercase(axis);
		const float nsigma = sigma >= 0 ? sigma : -sigma*(naxis == 'x' ? _width : naxis == 'y' ? _height : naxis == 'z' ? _depth : _spectrum) / 100;
		if (is_empty() || (nsigma<0.1f && !order)) return *this;
		const float
			nnsigma = nsigma<0.1f ? 0.1f : nsigma,
			alpha = 1.695f / nnsigma,
			ema = (float)std::exp(-alpha),
			ema2 = (float)std::exp(-2 * alpha),
			b1 = -2 * ema,
			b2 = ema2;
		float a0 = 0, a1 = 0, a2 = 0, a3 = 0, coefp = 0, coefn = 0;
		switch (order) {
		case 0: {
			const float k = (1 - ema)*(1 - ema) / (1 + 2 * alpha*ema - ema2);
			a0 = k;
			a1 = k*(alpha - 1)*ema;
			a2 = k*(alpha + 1)*ema;
			a3 = -k*ema2;
		} break;
		case 1: {
			const float k = -(1 - ema)*(1 - ema)*(1 - ema) / (2 * (ema + 1)*ema);
			a0 = a3 = 0;
			a1 = k*ema;
			a2 = -a1;
		} break;
		case 2: {
			const float
				ea = (float)std::exp(-alpha),
				k = -(ema2 - 1) / (2 * alpha*ema),
				kn = (-2 * (-1 + 3 * ea - 3 * ea*ea + ea*ea*ea) / (3 * ea + 1 + 3 * ea*ea + ea*ea*ea));
			a0 = kn;
			a1 = -kn*(1 + k*alpha)*ema;
			a2 = kn*(1 - k*alpha)*ema;
			a3 = -kn*ema2;
		} break;
		default:
			throw CImgArgumentException(_cimg_instance
				"deriche(): Invalid specified filter order %u "
				"(should be { 0=smoothing | 1=1st-derivative | 2=2nd-derivative }).",
				cimg_instance,
				order);
		}
		coefp = (a0 + a1) / (1 + b1 + b2);
		coefn = (a2 + a3) / (1 + b1 + b2);
		switch (naxis) {
		case 'x': {
			const int N = width();
			const ulongT off = 1U;
			cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(_width >= 256 && _height*_depth*_spectrum >= 16))
				cimg_forYZC(*this, y, z, c) { T *ptrX = data(0, y, z, c); _cimg_deriche_apply; }
		} break;
		case 'y': {
			const int N = height();
			const ulongT off = (ulongT)_width;
			cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(_width >= 256 && _height*_depth*_spectrum >= 16))
				cimg_forXZC(*this, x, z, c) { T *ptrX = data(x, 0, z, c); _cimg_deriche_apply; }
		} break;
		case 'z': {
			const int N = depth();
			const ulongT off = (ulongT)_width*_height;
			cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(_width >= 256 && _height*_depth*_spectrum >= 16))
				cimg_forXYC(*this, x, y, c) { T *ptrX = data(x, y, 0, c); _cimg_deriche_apply; }
		} break;
		default: {
			const int N = spectrum();
			const ulongT off = (ulongT)_width*_height*_depth;
			cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(_width >= 256 && _height*_depth*_spectrum >= 16))
				cimg_forXYZ(*this, x, y, z) { T *ptrX = data(x, y, z, 0); _cimg_deriche_apply; }
		}
		}
		return *this;
	}

	template<typename T> 
		template<typename t>
	CImg<T>& CImg<T>::watershed(const CImg<t>& priority, const bool is_high_connectivity = false)
	{
#define _cimg_watershed_init(cond,X,Y,Z) \
      if (cond && !(*this)(X,Y,Z)) Q._priority_queue_insert(labels,sizeQ,priority(X,Y,Z),X,Y,Z,nb_seeds)

#define _cimg_watershed_propagate(cond,X,Y,Z) \
      if (cond) { \
        if ((*this)(X,Y,Z)) { \
          ns = labels(X,Y,Z) - 1; xs = seeds(ns,0); ys = seeds(ns,1); zs = seeds(ns,2); \
          d = cimg::sqr((float)x - xs) + cimg::sqr((float)y - ys) + cimg::sqr((float)z - zs); \
          if (d<dmin) { dmin = d; nmin = ns; label = (*this)(xs,ys,zs); } \
        } else Q._priority_queue_insert(labels,sizeQ,priority(X,Y,Z),X,Y,Z,n); \
      }

		if (is_empty()) return *this;
		if (!is_sameXYZ(priority))
			throw CImgArgumentException(_cimg_instance
				"watershed(): image instance and specified priority (%u,%u,%u,%u,%p) "
				"have different dimensions.",
				cimg_instance,
				priority._width, priority._height, priority._depth, priority._spectrum, priority._data);
		if (_spectrum != 1) {
			cimg_forC(*this, c)
				get_shared_channel(c).watershed(priority.get_shared_channel(c%priority._spectrum));
			return *this;
		}

		CImg<uintT> labels(_width, _height, _depth, 1, 0), seeds(64, 3);
		CImg<typename cimg::superset2<T, t, int>::type> Q;
		unsigned int sizeQ = 0;
		int px, nx, py, ny, pz, nz;
		bool is_px, is_nx, is_py, is_ny, is_pz, is_nz;
		const bool is_3d = _depth>1;

		// Find seed points and insert them in priority queue.
		unsigned int nb_seeds = 0;
		const T *ptrs = _data;
		cimg_forXYZ(*this, x, y, z) if (*(ptrs++)) { // 3d version
			if (nb_seeds >= seeds._width) seeds.resize(2 * seeds._width, 3, 1, 1, 0);
			seeds(nb_seeds, 0) = x; seeds(nb_seeds, 1) = y; seeds(nb_seeds++, 2) = z;
			px = x - 1; nx = x + 1;
			py = y - 1; ny = y + 1;
			pz = z - 1; nz = z + 1;
			is_px = px >= 0; is_nx = nx<width();
			is_py = py >= 0; is_ny = ny<height();
			is_pz = pz >= 0; is_nz = nz<depth();
			_cimg_watershed_init(is_px, px, y, z);
			_cimg_watershed_init(is_nx, nx, y, z);
			_cimg_watershed_init(is_py, x, py, z);
			_cimg_watershed_init(is_ny, x, ny, z);
			if (is_3d) {
				_cimg_watershed_init(is_pz, x, y, pz);
				_cimg_watershed_init(is_nz, x, y, nz);
			}
			if (is_high_connectivity) {
				_cimg_watershed_init(is_px && is_py, px, py, z);
				_cimg_watershed_init(is_nx && is_py, nx, py, z);
				_cimg_watershed_init(is_px && is_ny, px, ny, z);
				_cimg_watershed_init(is_nx && is_ny, nx, ny, z);
				if (is_3d) {
					_cimg_watershed_init(is_px && is_pz, px, y, pz);
					_cimg_watershed_init(is_nx && is_pz, nx, y, pz);
					_cimg_watershed_init(is_px && is_nz, px, y, nz);
					_cimg_watershed_init(is_nx && is_nz, nx, y, nz);
					_cimg_watershed_init(is_py && is_pz, x, py, pz);
					_cimg_watershed_init(is_ny && is_pz, x, ny, pz);
					_cimg_watershed_init(is_py && is_nz, x, py, nz);
					_cimg_watershed_init(is_ny && is_nz, x, ny, nz);
					_cimg_watershed_init(is_px && is_py && is_pz, px, py, pz);
					_cimg_watershed_init(is_nx && is_py && is_pz, nx, py, pz);
					_cimg_watershed_init(is_px && is_ny && is_pz, px, ny, pz);
					_cimg_watershed_init(is_nx && is_ny && is_pz, nx, ny, pz);
					_cimg_watershed_init(is_px && is_py && is_nz, px, py, nz);
					_cimg_watershed_init(is_nx && is_py && is_nz, nx, py, nz);
					_cimg_watershed_init(is_px && is_ny && is_nz, px, ny, nz);
					_cimg_watershed_init(is_nx && is_ny && is_nz, nx, ny, nz);
				}
			}
			labels(x, y, z) = nb_seeds;
		}

		// Start watershed computation.
		while (sizeQ) {

			// Get and remove point with maximal priority from the queue.
			const int x = (int)Q(0, 1), y = (int)Q(0, 2), z = (int)Q(0, 3);
			const unsigned int n = labels(x, y, z);
			px = x - 1; nx = x + 1;
			py = y - 1; ny = y + 1;
			pz = z - 1; nz = z + 1;
			is_px = px >= 0; is_nx = nx<width();
			is_py = py >= 0; is_ny = ny<height();
			is_pz = pz >= 0; is_nz = nz<depth();

			// Check labels of the neighbors.
			Q._priority_queue_remove(sizeQ);

			unsigned int xs, ys, zs, ns, nmin = 0;
			float d, dmin = cimg::type<float>::inf();
			T label = (T)0;
			_cimg_watershed_propagate(is_px, px, y, z);
			_cimg_watershed_propagate(is_nx, nx, y, z);
			_cimg_watershed_propagate(is_py, x, py, z);
			_cimg_watershed_propagate(is_ny, x, ny, z);
			if (is_3d) {
				_cimg_watershed_propagate(is_pz, x, y, pz);
				_cimg_watershed_propagate(is_nz, x, y, nz);
			}
			if (is_high_connectivity) {
				_cimg_watershed_propagate(is_px && is_py, px, py, z);
				_cimg_watershed_propagate(is_nx && is_py, nx, py, z);
				_cimg_watershed_propagate(is_px && is_ny, px, ny, z);
				_cimg_watershed_propagate(is_nx && is_ny, nx, ny, z);
				if (is_3d) {
					_cimg_watershed_propagate(is_px && is_pz, px, y, pz);
					_cimg_watershed_propagate(is_nx && is_pz, nx, y, pz);
					_cimg_watershed_propagate(is_px && is_nz, px, y, nz);
					_cimg_watershed_propagate(is_nx && is_nz, nx, y, nz);
					_cimg_watershed_propagate(is_py && is_pz, x, py, pz);
					_cimg_watershed_propagate(is_ny && is_pz, x, ny, pz);
					_cimg_watershed_propagate(is_py && is_nz, x, py, nz);
					_cimg_watershed_propagate(is_ny && is_nz, x, ny, nz);
					_cimg_watershed_propagate(is_px && is_py && is_pz, px, py, pz);
					_cimg_watershed_propagate(is_nx && is_py && is_pz, nx, py, pz);
					_cimg_watershed_propagate(is_px && is_ny && is_pz, px, ny, pz);
					_cimg_watershed_propagate(is_nx && is_ny && is_pz, nx, ny, pz);
					_cimg_watershed_propagate(is_px && is_py && is_nz, px, py, nz);
					_cimg_watershed_propagate(is_nx && is_py && is_nz, nx, py, nz);
					_cimg_watershed_propagate(is_px && is_ny && is_nz, px, ny, nz);
					_cimg_watershed_propagate(is_nx && is_ny && is_nz, nx, ny, nz);
				}
			}
			(*this)(x, y, z) = label;
			labels(x, y, z) = ++nmin;
		}
		return *this;
	}

	template<typename T> 
		template<typename t>
	const CImg<T>& CImg<T>::SVD(CImg<t>& U, CImg<t>& S, CImg<t>& V, const bool sorting = true,
		const unsigned int max_iteration = 40, const float lambda = 0) const
	{
		if (is_empty()) { U.assign(); S.assign(); V.assign(); }
		else {
			U = *this;
			if (lambda != 0) {
				const unsigned int delta = std::min(U._width, U._height);
				for (unsigned int i = 0; i<delta; ++i) U(i, i) = (t)(U(i, i) + lambda);
			}
			if (S.size()<_width) S.assign(1, _width);
			if (V._width<_width || V._height<_height) V.assign(_width, _width);
			CImg<t> rv1(_width);
			t anorm = 0, c, f, g = 0, h, s, scale = 0;
			int l = 0, nm = 0;

			cimg_forX(U, i) {
				l = i + 1; rv1[i] = scale*g; g = s = scale = 0;
				if (i<height()) {
					for (int k = i; k<height(); ++k) scale += cimg::abs(U(i, k));
					if (scale) {
						for (int k = i; k<height(); ++k) { U(i, k) /= scale; s += U(i, k)*U(i, k); }
						f = U(i, i); g = (t)((f >= 0 ? -1 : 1)*std::sqrt(s)); h = f*g - s; U(i, i) = f - g;
						for (int j = l; j<width(); ++j) {
							s = 0;
							for (int k = i; k<height(); ++k) s += U(i, k)*U(j, k);
							f = s / h;
							for (int k = i; k<height(); ++k) U(j, k) += f*U(i, k);
						}
						for (int k = i; k<height(); ++k) U(i, k) *= scale;
					}
				}
				S[i] = scale*g;

				g = s = scale = 0;
				if (i<height() && i != width() - 1) {
					for (int k = l; k<width(); ++k) scale += cimg::abs(U(k, i));
					if (scale) {
						for (int k = l; k<width(); ++k) { U(k, i) /= scale; s += U(k, i)*U(k, i); }
						f = U(l, i); g = (t)((f >= 0 ? -1 : 1)*std::sqrt(s)); h = f*g - s; U(l, i) = f - g;
						for (int k = l; k<width(); ++k) rv1[k] = U(k, i) / h;
						for (int j = l; j<height(); ++j) {
							s = 0;
							for (int k = l; k<width(); ++k) s += U(k, j)*U(k, i);
							for (int k = l; k<width(); ++k) U(k, j) += s*rv1[k];
						}
						for (int k = l; k<width(); ++k) U(k, i) *= scale;
					}
				}
				anorm = (t)std::max((float)anorm, (float)(cimg::abs(S[i]) + cimg::abs(rv1[i])));
			}

			for (int i = width() - 1; i >= 0; --i) {
				if (i<width() - 1) {
					if (g) {
						for (int j = l; j<width(); ++j) V(i, j) = (U(j, i) / U(l, i)) / g;
						for (int j = l; j<width(); ++j) {
							s = 0;
							for (int k = l; k<width(); ++k) s += U(k, i)*V(j, k);
							for (int k = l; k<width(); ++k) V(j, k) += s*V(i, k);
						}
					}
					for (int j = l; j<width(); ++j) V(j, i) = V(i, j) = (t)0.0;
				}
				V(i, i) = (t)1.0; g = rv1[i]; l = i;
			}

			for (int i = std::min(width(), height()) - 1; i >= 0; --i) {
				l = i + 1; g = S[i];
				for (int j = l; j<width(); ++j) U(j, i) = 0;
				if (g) {
					g = 1 / g;
					for (int j = l; j<width(); ++j) {
						s = 0; for (int k = l; k<height(); ++k) s += U(i, k)*U(j, k);
						f = (s / U(i, i))*g;
						for (int k = i; k<height(); ++k) U(j, k) += f*U(i, k);
					}
					for (int j = i; j<height(); ++j) U(i, j) *= g;
				}
				else for (int j = i; j<height(); ++j) U(i, j) = 0;
				++U(i, i);
			}

			for (int k = width() - 1; k >= 0; --k) {
				for (unsigned int its = 0; its<max_iteration; ++its) {
					bool flag = true;
					for (l = k; l >= 1; --l) {
						nm = l - 1;
						if ((cimg::abs(rv1[l]) + anorm) == anorm) { flag = false; break; }
						if ((cimg::abs(S[nm]) + anorm) == anorm) break;
					}
					if (flag) {
						c = 0; s = 1;
						for (int i = l; i <= k; ++i) {
							f = s*rv1[i]; rv1[i] = c*rv1[i];
							if ((cimg::abs(f) + anorm) == anorm) break;
							g = S[i]; h = cimg::_hypot(f, g); S[i] = h; h = 1 / h; c = g*h; s = -f*h;
							cimg_forY(U, j) { const t y = U(nm, j), z = U(i, j); U(nm, j) = y*c + z*s; U(i, j) = z*c - y*s; }
						}
					}

					const t z = S[k];
					if (l == k) { if (z<0) { S[k] = -z; cimg_forX(U, j) V(k, j) = -V(k, j); } break; }
					nm = k - 1;
					t x = S[l], y = S[nm];
					g = rv1[nm]; h = rv1[k];
					f = ((y - z)*(y + z) + (g - h)*(g + h)) / std::max((t)1e-25, 2 * h*y);
					g = cimg::_hypot(f, (t)1);
					f = ((x - z)*(x + z) + h*((y / (f + (f >= 0 ? g : -g))) - h)) / std::max((t)1e-25, x);
					c = s = 1;
					for (int j = l; j <= nm; ++j) {
						const int i = j + 1;
						g = rv1[i]; h = s*g; g = c*g;
						t y = S[i];
						t z = cimg::_hypot(f, h);
						rv1[j] = z; c = f / std::max((t)1e-25, z); s = h / std::max((t)1e-25, z);
						f = x*c + g*s; g = g*c - x*s; h = y*s; y *= c;
						cimg_forX(U, jj) { const t x = V(j, jj), z = V(i, jj); V(j, jj) = x*c + z*s; V(i, jj) = z*c - x*s; }
						z = cimg::_hypot(f, h); S[j] = z;
						if (z) { z = 1 / std::max((t)1e-25, z); c = f*z; s = h*z; }
						f = c*g + s*y; x = c*y - s*g;
						cimg_forY(U, jj) { const t y = U(j, jj); z = U(i, jj); U(j, jj) = y*c + z*s; U(i, jj) = z*c - y*s; }
					}
					rv1[l] = 0; rv1[k] = f; S[k] = x;
				}
			}

			if (sorting) {
				CImg<intT> permutations;
				CImg<t> tmp(_width);
				S.sort(permutations, false);
				cimg_forY(U, k) {
					cimg_forY(permutations, y) tmp(y) = U(permutations(y), k);
					std::memcpy(U.data(0, k), tmp._data, sizeof(t)*_width);
				}
				cimg_forY(V, k) {
					cimg_forY(permutations, y) tmp(y) = V(permutations(y), k);
					std::memcpy(V.data(0, k), tmp._data, sizeof(t)*_width);
				}
			}
		}
		return *this;
	}


	template<typename T>
		template<typename t>
	CImg<T>& CImg<T>::_quicksort(const long indm, const long indM, CImg<t>& permutations,
			const bool is_increasing, const bool is_permutations)
	{
		if (indm<indM) {
			const long mid = (indm + indM) / 2;
			if (is_increasing) {
				if ((*this)[indm]>(*this)[mid]) {
					cimg::swap((*this)[indm], (*this)[mid]);
					if (is_permutations) cimg::swap(permutations[indm], permutations[mid]);
				}
				if ((*this)[mid]>(*this)[indM]) {
					cimg::swap((*this)[indM], (*this)[mid]);
					if (is_permutations) cimg::swap(permutations[indM], permutations[mid]);
				}
				if ((*this)[indm]>(*this)[mid]) {
					cimg::swap((*this)[indm], (*this)[mid]);
					if (is_permutations) cimg::swap(permutations[indm], permutations[mid]);
				}
			}
			else {
				if ((*this)[indm]<(*this)[mid]) {
					cimg::swap((*this)[indm], (*this)[mid]);
					if (is_permutations) cimg::swap(permutations[indm], permutations[mid]);
				}
				if ((*this)[mid]<(*this)[indM]) {
					cimg::swap((*this)[indM], (*this)[mid]);
					if (is_permutations) cimg::swap(permutations[indM], permutations[mid]);
				}
				if ((*this)[indm]<(*this)[mid]) {
					cimg::swap((*this)[indm], (*this)[mid]);
					if (is_permutations) cimg::swap(permutations[indm], permutations[mid]);
				}
			}
			if (indM - indm >= 3) {
				const T pivot = (*this)[mid];
				long i = indm, j = indM;
				if (is_increasing) {
					do {
						while ((*this)[i]<pivot) ++i;
						while ((*this)[j]>pivot) --j;
						if (i <= j) {
							if (is_permutations) cimg::swap(permutations[i], permutations[j]);
							cimg::swap((*this)[i++], (*this)[j--]);
						}
					} while (i <= j);
				}
				else {
					do {
						while ((*this)[i]>pivot) ++i;
						while ((*this)[j]<pivot) --j;
						if (i <= j) {
							if (is_permutations) cimg::swap(permutations[i], permutations[j]);
							cimg::swap((*this)[i++], (*this)[j--]);
						}
					} while (i <= j);
				}
				if (indm<j) _quicksort(indm, j, permutations, is_increasing, is_permutations);
				if (i<indM) _quicksort(i, indM, permutations, is_increasing, is_permutations);
			}
		}
		return *this;
	}
		
	template<typename T>
		template<typename t>
		const CImg<T>& CImg<T>::symmetric_eigen(CImg<t>& val, CImg<t>& vec) const
	{
		if (is_empty()) { val.assign(); vec.assign(); }
		else {
#ifdef cimg_use_lapack
			char JOB = 'V', UPLO = 'U';
			int N = _width, LWORK = 4 * N, INFO;
			Tfloat
				*const lapA = new Tfloat[N*N],
				*const lapW = new Tfloat[N],
				*const WORK = new Tfloat[LWORK];
			cimg_forXY(*this, k, l) lapA[k*N + l] = (Tfloat)((*this)(k, l));
			cimg::syev(JOB, UPLO, N, lapA, lapW, WORK, LWORK, INFO);
			if (INFO)
				cimg::warn(_cimg_instance
					"symmetric_eigen(): LAPACK library function dsyev_() returned error code %d.",
					cimg_instance,
					INFO);

			val.assign(1, N);
			vec.assign(N, N);
			if (!INFO) {
				cimg_forY(val, i) val(i) = (T)lapW[N - 1 - i];
				cimg_forXY(vec, k, l) vec(k, l) = (T)(lapA[(N - 1 - k)*N + l]);
			}
			else { val.fill(0); vec.fill(0); }
			delete[] lapA; delete[] lapW; delete[] WORK;
#else
			if (_width != _height || _depth>1 || _spectrum>1)
				throw CImgInstanceException(_cimg_instance
					"eigen(): Instance is not a square matrix.",
					cimg_instance);

			val.assign(1, _width);
			if (vec._data) vec.assign(_width, _width);
			if (_width<3) {
				eigen(val, vec);
				if (_width == 2) { vec[1] = -vec[2]; vec[3] = vec[0]; } // Force orthogonality for 2x2 matrices.
				return *this;
			}
			CImg<t> V(_width, _width);
			Tfloat M = 0, m = (Tfloat)min_max(M), maxabs = cimg::max((Tfloat)1.0f, cimg::abs(m), cimg::abs(M));
			(CImg<Tfloat>(*this, false) /= maxabs).SVD(vec, val, V, false);
			if (maxabs != 1) val *= maxabs;

			bool is_ambiguous = false;
			float eig = 0;
			cimg_forY(val, p) {       // check for ambiguous cases.
				if (val[p]>eig) eig = (float)val[p];
				t scal = 0;
				cimg_forY(vec, y) scal += vec(p, y)*V(p, y);
				if (cimg::abs(scal)<0.9f) is_ambiguous = true;
				if (scal<0) val[p] = -val[p];
			}
			if (is_ambiguous) {
				++(eig *= 2);
				SVD(vec, val, V, false, 40, eig);
				val -= eig;
			}
			CImg<intT> permutations;  // sort eigenvalues in decreasing order
			CImg<t> tmp(_width);
			val.sort(permutations, false);
			cimg_forY(vec, k) {
				cimg_forY(permutations, y) tmp(y) = vec(permutations(y), k);
				std::memcpy(vec.data(0, k), tmp._data, sizeof(t)*_width);
			}
#endif
		}
		return *this;
	}
		template<typename T>
	CImg<T>& CImg<T>::invert(const bool use_LU = true)
	{
		if (_width != _height || _depth != 1 || _spectrum != 1)
			throw CImgInstanceException(_cimg_instance
				"invert(): Instance is not a square matrix.",
				cimg_instance);
#ifdef cimg_use_lapack
		int INFO = (int)use_LU, N = _width, LWORK = 4 * N, *const IPIV = new int[N];
		Tfloat
			*const lapA = new Tfloat[N*N],
			*const WORK = new Tfloat[LWORK];
		cimg_forXY(*this, k, l) lapA[k*N + l] = (Tfloat)((*this)(k, l));
		cimg::getrf(N, lapA, IPIV, INFO);
		if (INFO)
			cimg::warn(_cimg_instance
				"invert(): LAPACK function dgetrf_() returned error code %d.",
				cimg_instance,
				INFO);
		else {
			cimg::getri(N, lapA, IPIV, WORK, LWORK, INFO);
			if (INFO)
				cimg::warn(_cimg_instance
					"invert(): LAPACK function dgetri_() returned error code %d.",
					cimg_instance,
					INFO);
		}
		if (!INFO) cimg_forXY(*this, k, l) (*this)(k, l) = (T)(lapA[k*N + l]); else fill(0);
		delete[] IPIV; delete[] lapA; delete[] WORK;
#else
		const double dete = _width>3 ? -1.0 : det();
		if (dete != 0.0 && _width == 2) {
			const double
				a = _data[0], c = _data[1],
				b = _data[2], d = _data[3];
			_data[0] = (T)(d / dete); _data[1] = (T)(-c / dete);
			_data[2] = (T)(-b / dete); _data[3] = (T)(a / dete);
		}
		else if (dete != 0.0 && _width == 3) {
			const double
				a = _data[0], d = _data[1], g = _data[2],
				b = _data[3], e = _data[4], h = _data[5],
				c = _data[6], f = _data[7], i = _data[8];
			_data[0] = (T)((i*e - f*h) / dete), _data[1] = (T)((g*f - i*d) / dete), _data[2] = (T)((d*h - g*e) / dete);
			_data[3] = (T)((h*c - i*b) / dete), _data[4] = (T)((i*a - c*g) / dete), _data[5] = (T)((g*b - a*h) / dete);
			_data[6] = (T)((b*f - e*c) / dete), _data[7] = (T)((d*c - a*f) / dete), _data[8] = (T)((a*e - d*b) / dete);
		}
		else {
			if (use_LU) { // LU-based inverse computation
				CImg<Tfloat> A(*this), indx, col(1, _width);
				bool d;
				A._LU(indx, d);
				cimg_forX(*this, j) {
					col.fill(0);
					col(j) = 1;
					col._solve(A, indx);
					cimg_forX(*this, i) (*this)(j, i) = (T)col(i);
				}
			}
			else { // SVD-based inverse computation
				CImg<Tfloat> U(_width, _width), S(1, _width), V(_width, _width);
				SVD(U, S, V, false);
				U.transpose();
				cimg_forY(S, k) if (S[k] != 0) S[k] = 1 / S[k];
				S.diagonal();
				*this = V*S*U;
			}
		}
#endif
		return *this;
	}

		template<typename T>
		CImg<T>& CImg<T>::_load_bmp(std::FILE *const file, const char *const filename)
	{
		if (!file && !filename)
			throw CImgArgumentException(_cimg_instance
				"load_bmp(): Specified filename is (null).",
				cimg_instance);

		std::FILE *const nfile = file ? file : cimg::fopen(filename, "rb");
		CImg<ucharT> header(54);
		cimg::fread(header._data, 54, nfile);
		if (*header != 'B' || header[1] != 'M') {
			if (!file) cimg::fclose(nfile);
			throw CImgIOException(_cimg_instance
				"load_bmp(): Invalid BMP file '%s'.",
				cimg_instance,
				filename ? filename : "(FILE*)");
		}

		// Read header and pixel buffer
		int
			file_size = header[0x02] + (header[0x03] << 8) + (header[0x04] << 16) + (header[0x05] << 24),
			offset = header[0x0A] + (header[0x0B] << 8) + (header[0x0C] << 16) + (header[0x0D] << 24),
			header_size = header[0x0E] + (header[0x0F] << 8) + (header[0x10] << 16) + (header[0x11] << 24),
			dx = header[0x12] + (header[0x13] << 8) + (header[0x14] << 16) + (header[0x15] << 24),
			dy = header[0x16] + (header[0x17] << 8) + (header[0x18] << 16) + (header[0x19] << 24),
			compression = header[0x1E] + (header[0x1F] << 8) + (header[0x20] << 16) + (header[0x21] << 24),
			nb_colors = header[0x2E] + (header[0x2F] << 8) + (header[0x30] << 16) + (header[0x31] << 24),
			bpp = header[0x1C] + (header[0x1D] << 8);

		if (!file_size || file_size == offset) {
			cimg::fseek(nfile, 0, SEEK_END);
			file_size = (int)cimg::ftell(nfile);
			cimg::fseek(nfile, 54, SEEK_SET);
		}
		if (header_size>40) cimg::fseek(nfile, header_size - 40, SEEK_CUR);

		const int
			dx_bytes = (bpp == 1) ? (dx / 8 + (dx % 8 ? 1 : 0)) : ((bpp == 4) ? (dx / 2 + (dx % 2)) : (dx*bpp / 8)),
			align_bytes = (4 - dx_bytes % 4) % 4;
		const longT
			cimg_iobuffer = (longT)24 * 1024 * 1024,
			buf_size = std::min((longT)cimg::abs(dy)*(dx_bytes + align_bytes), (longT)file_size - offset);

		CImg<intT> colormap;
		if (bpp<16) { if (!nb_colors) nb_colors = 1 << bpp; }
		else nb_colors = 0;
		if (nb_colors) { colormap.assign(nb_colors); cimg::fread(colormap._data, nb_colors, nfile); }
		const int xoffset = offset - 14 - header_size - 4 * nb_colors;
		if (xoffset>0) cimg::fseek(nfile, xoffset, SEEK_CUR);

		CImg<ucharT> buffer;
		if (buf_size<cimg_iobuffer) {
			buffer.assign(cimg::abs(dy)*(dx_bytes + align_bytes), 1, 1, 1, 0);
			cimg::fread(buffer._data, buf_size, nfile);
		}
		else buffer.assign(dx_bytes + align_bytes);
		unsigned char *ptrs = buffer;

		// Decompress buffer (if necessary)
		if (compression) {
			if (file)
				throw CImgIOException(_cimg_instance
					"load_bmp(): Unable to load compressed data from '(*FILE)' inputs.",
					cimg_instance);
			else {
				if (!file) cimg::fclose(nfile);
				return load_other(filename);
			}
		}

		// Read pixel data
		assign(dx, cimg::abs(dy), 1, 3);
		switch (bpp) {
		case 1: { // Monochrome
			for (int y = height() - 1; y >= 0; --y) {
				if (buf_size >= cimg_iobuffer) {
					cimg::fread(ptrs = buffer._data, dx_bytes, nfile);
					cimg::fseek(nfile, align_bytes, SEEK_CUR);
				}
				unsigned char mask = 0x80, val = 0;
				cimg_forX(*this, x) {
					if (mask == 0x80) val = *(ptrs++);
					const unsigned char *col = (unsigned char*)(colormap._data + (val&mask ? 1 : 0));
					(*this)(x, y, 2) = (T)*(col++);
					(*this)(x, y, 1) = (T)*(col++);
					(*this)(x, y, 0) = (T)*(col++);
					mask = cimg::ror(mask);
				}
				ptrs += align_bytes;
			}
		} break;
		case 4: { // 16 colors
			for (int y = height() - 1; y >= 0; --y) {
				if (buf_size >= cimg_iobuffer) {
					cimg::fread(ptrs = buffer._data, dx_bytes, nfile);
					cimg::fseek(nfile, align_bytes, SEEK_CUR);
				}
				unsigned char mask = 0xF0, val = 0;
				cimg_forX(*this, x) {
					if (mask == 0xF0) val = *(ptrs++);
					const unsigned char color = (unsigned char)((mask<16) ? (val&mask) : ((val&mask) >> 4));
					const unsigned char *col = (unsigned char*)(colormap._data + color);
					(*this)(x, y, 2) = (T)*(col++);
					(*this)(x, y, 1) = (T)*(col++);
					(*this)(x, y, 0) = (T)*(col++);
					mask = cimg::ror(mask, 4);
				}
				ptrs += align_bytes;
			}
		} break;
		case 8: { //  256 colors
			for (int y = height() - 1; y >= 0; --y) {
				if (buf_size >= cimg_iobuffer) {
					cimg::fread(ptrs = buffer._data, dx_bytes, nfile);
					cimg::fseek(nfile, align_bytes, SEEK_CUR);
				}
				cimg_forX(*this, x) {
					const unsigned char *col = (unsigned char*)(colormap._data + *(ptrs++));
					(*this)(x, y, 2) = (T)*(col++);
					(*this)(x, y, 1) = (T)*(col++);
					(*this)(x, y, 0) = (T)*(col++);
				}
				ptrs += align_bytes;
			}
		} break;
		case 16: { // 16 bits colors
			for (int y = height() - 1; y >= 0; --y) {
				if (buf_size >= cimg_iobuffer) {
					cimg::fread(ptrs = buffer._data, dx_bytes, nfile);
					cimg::fseek(nfile, align_bytes, SEEK_CUR);
				}
				cimg_forX(*this, x) {
					const unsigned char c1 = *(ptrs++), c2 = *(ptrs++);
					const unsigned short col = (unsigned short)(c1 | (c2 << 8));
					(*this)(x, y, 2) = (T)(col & 0x1F);
					(*this)(x, y, 1) = (T)((col >> 5) & 0x1F);
					(*this)(x, y, 0) = (T)((col >> 10) & 0x1F);
				}
				ptrs += align_bytes;
			}
		} break;
		case 24: { // 24 bits colors
			for (int y = height() - 1; y >= 0; --y) {
				if (buf_size >= cimg_iobuffer) {
					cimg::fread(ptrs = buffer._data, dx_bytes, nfile);
					cimg::fseek(nfile, align_bytes, SEEK_CUR);
				}
				cimg_forX(*this, x) {
					(*this)(x, y, 2) = (T)*(ptrs++);
					(*this)(x, y, 1) = (T)*(ptrs++);
					(*this)(x, y, 0) = (T)*(ptrs++);
				}
				ptrs += align_bytes;
			}
		} break;
		case 32: { // 32 bits colors
			for (int y = height() - 1; y >= 0; --y) {
				if (buf_size >= cimg_iobuffer) {
					cimg::fread(ptrs = buffer._data, dx_bytes, nfile);
					cimg::fseek(nfile, align_bytes, SEEK_CUR);
				}
				cimg_forX(*this, x) {
					(*this)(x, y, 2) = (T)*(ptrs++);
					(*this)(x, y, 1) = (T)*(ptrs++);
					(*this)(x, y, 0) = (T)*(ptrs++);
					++ptrs;
				}
				ptrs += align_bytes;
			}
		} break;
		}
		if (dy<0) mirror('y');
		if (!file) cimg::fclose(nfile);
		return *this;
	}
		
		template<typename T>
		CImg<T>& CImg<T>::pow(const double p)
	{
		if (is_empty()) return *this;
		if (p == -4) {
			cimg_pragma_openmp(parallel for cimg_openmp_if(size() >= 32768))
				cimg_rof(*this, ptrd, T) { const T val = *ptrd; *ptrd = (T)(1.0 / (val*val*val*val)); }
			return *this;
		}
		if (p == -3) {
			cimg_pragma_openmp(parallel for cimg_openmp_if(size() >= 32768))
				cimg_rof(*this, ptrd, T) { const T val = *ptrd; *ptrd = (T)(1.0 / (val*val*val)); }
			return *this;
		}
		if (p == -2) {
			cimg_pragma_openmp(parallel for cimg_openmp_if(size() >= 32768))
				cimg_rof(*this, ptrd, T) { const T val = *ptrd; *ptrd = (T)(1.0 / (val*val)); }
			return *this;
		}
		if (p == -1) {
			cimg_pragma_openmp(parallel for cimg_openmp_if(size() >= 32768))
				cimg_rof(*this, ptrd, T) { const T val = *ptrd; *ptrd = (T)(1.0 / val); }
			return *this;
		}
		if (p == -0.5) {
			cimg_pragma_openmp(parallel for cimg_openmp_if(size() >= 8192))
				cimg_rof(*this, ptrd, T) { const T val = *ptrd; *ptrd = (T)(1 / std::sqrt((double)val)); }
			return *this;
		}
		if (p == 0) return fill((T)1);
		if (p == 0.25) return sqrt().sqrt();
		if (p == 0.5) return sqrt();
		if (p == 1) return *this;
		if (p == 2) return sqr();
		if (p == 3) {
			cimg_pragma_openmp(parallel for cimg_openmp_if(size() >= 262144))
				cimg_rof(*this, ptrd, T) { const T val = *ptrd; *ptrd = val*val*val; }
			return *this;
		}
		if (p == 4) {
			cimg_pragma_openmp(parallel for cimg_openmp_if(size() >= 131072))
				cimg_rof(*this, ptrd, T) { const T val = *ptrd; *ptrd = val*val*val*val; }
			return *this;
		}
		cimg_pragma_openmp(parallel for cimg_openmp_if(size() >= 1024))
			cimg_rof(*this, ptrd, T) *ptrd = (T)std::pow((double)*ptrd, p);
		return *this;
	}
		
	template<typename T>
		template<typename t>
	CImg<T> CImg<T>::get_discard(const CImg<t>& values, const char axis = 0) const
	{
		CImg<T> res;
		if (!values) return +*this;
		if (is_empty()) return res;
		const ulongT vsiz = values.size();
		const char _axis = cimg::lowercase(axis);
		ulongT j = 0;
		unsigned int k = 0;
		int i0 = 0;
		res.assign(width(), height(), depth(), spectrum());
		switch (_axis) {
		case 'x': {
			cimg_forX(*this, i) {
				if ((*this)(i) != (T)values[j]) {
					if (j) --i;
					res.draw_image(k, get_columns(i0, i));
					k += i - i0 + 1; i0 = i + 1; j = 0;
				}
				else { ++j; if (j >= vsiz) { j = 0; i0 = i + 1; } }
			}
			if (i0<width()) { res.draw_image(k, get_columns(i0, width() - 1)); k += width() - i0; }
			res.resize(k, -100, -100, -100, 0);
		} break;
		case 'y': {
			cimg_forY(*this, i) {
				if ((*this)(0, i) != (T)values[j]) {
					if (j) --i;
					res.draw_image(0, k, get_rows(i0, i));
					k += i - i0 + 1; i0 = i + 1; j = 0;
				}
				else { ++j; if (j >= vsiz) { j = 0; i0 = i + 1; } }
			}
			if (i0<height()) { res.draw_image(0, k, get_rows(i0, height() - 1)); k += height() - i0; }
			res.resize(-100, k, -100, -100, 0);
		} break;
		case 'z': {
			cimg_forZ(*this, i) {
				if ((*this)(0, 0, i) != (T)values[j]) {
					if (j) --i;
					res.draw_image(0, 0, k, get_slices(i0, i));
					k += i - i0 + 1; i0 = i + 1; j = 0;
				}
				else { ++j; if (j >= vsiz) { j = 0; i0 = i + 1; } }
			}
			if (i0<depth()) { res.draw_image(0, 0, k, get_slices(i0, height() - 1)); k += depth() - i0; }
			res.resize(-100, -100, k, -100, 0);
		} break;
		case 'c': {
			cimg_forC(*this, i) {
				if ((*this)(0, 0, 0, i) != (T)values[j]) {
					if (j) --i;
					res.draw_image(0, 0, 0, k, get_channels(i0, i));
					k += i - i0 + 1; i0 = i + 1; j = 0;
				}
				else { ++j; if (j >= vsiz) { j = 0; i0 = i + 1; } }
			}
			if (i0<spectrum()) { res.draw_image(0, 0, k, get_channels(i0, height() - 1)); k += spectrum() - i0; }
			res.resize(-100, -100, -100, k, 0);
		} break;
		default: {
			res.unroll('y');
			cimg_foroff(*this, i) {
				if ((*this)[i] != (T)values[j]) {
					if (j) --i;
					std::memcpy(res._data + k, _data + i0, (i - i0 + 1) * sizeof(T));
					k += i - i0 + 1; i0 = (int)i + 1; j = 0;
				}
				else { ++j; if (j >= vsiz) { j = 0; i0 = (int)i + 1; } }
			}
			const ulongT siz = size();
			if ((ulongT)i0<siz) { std::memcpy(res._data + k, _data + i0, (siz - i0) * sizeof(T)); k += siz - i0; }
			res.resize(1, k, 1, 1, 0);
		}
		}
		return res;
	}

	template<typename T>
		CImg<T>& CImg<T>::_fill(const char *const expression, const bool repeat_values, bool allow_formula,
			const CImgList<T> *const list_inputs, CImgList<T> *const list_outputs,
			const char *const calling_function, const CImg<T> *provides_copy)
	{
		if (is_empty() || !expression || !*expression) return *this;
		const unsigned int omode = cimg::exception_mode();
		cimg::exception_mode(0);
		CImg<charT> is_error;
		bool is_value_sequence = false;

		if (allow_formula) {

			// Try to pre-detect regular value sequence to avoid exception thrown by _cimg_math_parser.
			double value;
			char sep;
			const int err = cimg_sscanf(expression, "%lf %c", &value, &sep);
			if (err == 1 || (err == 2 && sep == ',')) {
				if (err == 1) return fill((T)value);
				else is_value_sequence = true;
			}

			// Try to fill values according to a formula.
			cimg_abort_init;
			if (!is_value_sequence) try {
				CImg<T> base = provides_copy ? provides_copy->get_shared() : get_shared();
				_cimg_math_parser mp(expression + (*expression == '>' || *expression == '<' ||
					*expression == '*' || *expression == ':'),
					calling_function, base, this, list_inputs, list_outputs, true);
				if (!provides_copy && expression && *expression != '>' && *expression != '<' && *expression != ':' &&
					mp.need_input_copy)
					base.assign().assign(*this); // Needs input copy

				bool do_in_parallel = false;
#ifdef cimg_use_openmp
				cimg_openmp_if(*expression == '*' || *expression == ':' ||
					(mp.is_parallelizable && _width >= 320 && _height*_depth*_spectrum >= 2))
					do_in_parallel = true;
#endif
				if (mp.result_dim) { // Vector-valued expression
					const unsigned int N = std::min(mp.result_dim, _spectrum);
					const ulongT whd = (ulongT)_width*_height*_depth;
					T *ptrd = *expression == '<' ? _data + _width*_height*_depth - 1 : _data;
					if (*expression == '<') {
						CImg<doubleT> res(1, mp.result_dim);
						cimg_rofYZ(*this, y, z) {
							cimg_abort_test();
							cimg_rofX(*this, x) {
								mp(x, y, z, 0, res._data);
								const double *ptrs = res._data;
								T *_ptrd = ptrd--; for (unsigned int n = N; n>0; --n) { *_ptrd = (T)(*ptrs++); _ptrd += whd; }
							}
						}
					}
					else if (*expression == '>' || !do_in_parallel) {
						CImg<doubleT> res(1, mp.result_dim);
						cimg_forYZ(*this, y, z) {
							cimg_abort_test();
							cimg_forX(*this, x) {
								mp(x, y, z, 0, res._data);
								const double *ptrs = res._data;
								T *_ptrd = ptrd++; for (unsigned int n = N; n>0; --n) { *_ptrd = (T)(*ptrs++); _ptrd += whd; }
							}
						}
					}
					else {
#ifdef cimg_use_openmp
						cimg_pragma_openmp(parallel)
						{
							_cimg_math_parser
								_mp = omp_get_thread_num() ? mp : _cimg_math_parser(),
								&lmp = omp_get_thread_num() ? _mp : mp;
							lmp.is_fill = true;
							cimg_pragma_openmp(for collapse(2))
								cimg_forYZ(*this, y, z) cimg_abort_try {
								cimg_abort_test();
								CImg<doubleT> res(1, lmp.result_dim);
								T *ptrd = data(0, y, z, 0);
								cimg_forX(*this, x) {
									lmp(x, y, z, 0, res._data);
									const double *ptrs = res._data;
									T *_ptrd = ptrd++; for (unsigned int n = N; n>0; --n) { *_ptrd = (T)(*ptrs++); _ptrd += whd; }
								}
							} cimg_abort_catch() cimg_abort_catch_fill()
						}
#endif
					}

				}
				else { // Scalar-valued expression
					T *ptrd = *expression == '<' ? end() - 1 : _data;
					if (*expression == '<')
						cimg_rofYZC(*this, y, z, c) { cimg_abort_test(); cimg_rofX(*this, x) *(ptrd--) = (T)mp(x, y, z, c); }
					else if (*expression == '>' || !do_in_parallel)
						cimg_forYZC(*this, y, z, c) { cimg_abort_test(); cimg_forX(*this, x) *(ptrd++) = (T)mp(x, y, z, c); }
					else {
#ifdef cimg_use_openmp
						cimg_pragma_openmp(parallel)
						{
							_cimg_math_parser
								_mp = omp_get_thread_num() ? mp : _cimg_math_parser(),
								&lmp = omp_get_thread_num() ? _mp : mp;
							lmp.is_fill = true;
							cimg_pragma_openmp(for collapse(3))
								cimg_forYZC(*this, y, z, c) cimg_abort_try {
								cimg_abort_test();
								T *ptrd = data(0, y, z, c);
								cimg_forX(*this, x) *ptrd++ = (T)lmp(x, y, z, c);
							} cimg_abort_catch() cimg_abort_catch_fill()
						}
#endif
					}
				}
				mp.end();
			}
			catch (CImgException& e) { CImg<charT>::string(e._message).move_to(is_error); }
		}

		// Try to fill values according to a value sequence.
		if (!allow_formula || is_value_sequence || is_error) {
			CImg<charT> item(256);
			char sep = 0;
			const char *nexpression = expression;
			ulongT nb = 0;
			const ulongT siz = size();
			T *ptrd = _data;
			for (double val = 0; *nexpression && nb<siz; ++nb) {
				sep = 0;
				const int err = cimg_sscanf(nexpression, "%255[ \n\t0-9.eEinfa+-]%c", item._data, &sep);
				if (err>0 && cimg_sscanf(item, "%lf", &val) == 1 && (sep == ',' || sep == ';' || err == 1)) {
					nexpression += std::strlen(item) + (err>1);
					*(ptrd++) = (T)val;
				}
				else break;
			}
			cimg::exception_mode(omode);
			if (nb<siz && (sep || *nexpression)) {
				if (is_error) throw CImgArgumentException("%s", is_error._data);
				else throw CImgArgumentException(_cimg_instance
					"%s(): Invalid sequence of filling values '%s'.",
					cimg_instance, calling_function, expression);
			}
			if (repeat_values && nb && nb<siz)
				for (T *ptrs = _data, *const ptre = _data + siz; ptrd<ptre; ++ptrs) *(ptrd++) = *ptrs;
		}

		cimg::exception_mode(omode);
		cimg_abort_test();
		return *this;
	}

	// [internal] Perform 3d rotation with arbitrary axis and angle.
	template<typename T>
	void CImg<T>::_rotate(CImg<T>& res, const CImg<Tfloat>& R,
		const unsigned int interpolation, const unsigned int boundary_conditions,
		const float w2, const float h2, const float d2,
		const float rw2, const float rh2, const float rd2) const
	{
		switch (boundary_conditions) {
		case 3: // Mirror
			switch (interpolation) {
			case 2: { // Cubic interpolation
				const float ww = 2.0f*width(), hh = 2.0f*height(), dd = 2.0f*depth();
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float
						xc = x - rw2, yc = y - rh2, zc = z - rd2,
						X = cimg::mod((float)(w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc), ww),
						Y = cimg::mod((float)(h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc), hh),
						Z = cimg::mod((float)(d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc), dd);
					cimg_forC(res, c) res(x, y, z, c) = cimg::type<T>::cut(_cubic_atXYZ(X<width() ? X : ww - X - 1,
						Y<height() ? Y : hh - Y - 1,
						Z<depth() ? Z : dd - Z - z, c));
				}
			} break;
			case 1: { // Linear interpolation
				const float ww = 2.0f*width(), hh = 2.0f*height(), dd = 2.0f*depth();
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float
						xc = x - rw2, yc = y - rh2, zc = z - rd2,
						X = cimg::mod((float)(w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc), ww),
						Y = cimg::mod((float)(h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc), hh),
						Z = cimg::mod((float)(d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc), dd);
					cimg_forC(res, c) res(x, y, z, c) = (T)_linear_atXYZ(X<width() ? X : ww - X - 1,
						Y<height() ? Y : hh - Y - 1,
						Z<depth() ? Z : dd - Z - 1, c);
				}
			} break;
			default: { // Nearest-neighbor interpolation
				const int ww = 2 * width(), hh = 2 * height(), dd = 2 * depth();
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float xc = x - rw2, yc = y - rh2, zc = z - rd2;
					const int
						X = cimg::mod((int)cimg::round(w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc), ww),
						Y = cimg::mod((int)cimg::round(h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc), hh),
						Z = cimg::mod((int)cimg::round(d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc), dd);
					cimg_forC(res, c) res(x, y, z, c) = (*this)(X<width() ? X : ww - X - 1,
						Y<height() ? Y : hh - Y - 1,
						Z<depth() ? Z : dd - Z - 1, c);
				}
			}
			} break;

		case 2: // Periodic
			switch (interpolation) {
			case 2: { // Cubic interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float
						xc = x - rw2, yc = y - rh2, zc = z - rd2,
						X = cimg::mod((float)(w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc), (float)width()),
						Y = cimg::mod((float)(h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc), (float)height()),
						Z = cimg::mod((float)(d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc), (float)depth());
					cimg_forC(res, c) res(x, y, z, c) = cimg::type<T>::cut(_cubic_atXYZ(X, Y, Z, c));
				}
			} break;
			case 1: { // Linear interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float
						xc = x - rw2, yc = y - rh2, zc = z - rd2,
						X = cimg::mod((float)(w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc), (float)width()),
						Y = cimg::mod((float)(h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc), (float)height()),
						Z = cimg::mod((float)(d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc), (float)depth());
					cimg_forC(res, c) res(x, y, z, c) = (T)_linear_atXYZ(X, Y, Z, c);
				}
			} break;
			default: { // Nearest-neighbor interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float xc = x - rw2, yc = y - rh2, zc = z - rd2;
					const int
						X = cimg::mod((int)cimg::round(w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc), width()),
						Y = cimg::mod((int)cimg::round(h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc), height()),
						Z = cimg::mod((int)cimg::round(d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc), depth());
					cimg_forC(res, c) res(x, y, z, c) = (*this)(X, Y, Z, c);
				}
			}
			} break;

		case 1: // Neumann
			switch (interpolation) {
			case 2: { // Cubic interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float
						xc = x - rw2, yc = y - rh2, zc = z - rd2,
						X = w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc,
						Y = h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc,
						Z = d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc;
					cimg_forC(res, c) res(x, y, z, c) = cimg::type<T>::cut(_cubic_atXYZ(X, Y, Z, c));
				}
			} break;
			case 1: { // Linear interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float
						xc = x - rw2, yc = y - rh2, zc = z - rd2,
						X = w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc,
						Y = h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc,
						Z = d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc;
					cimg_forC(res, c) res(x, y, z, c) = _linear_atXYZ(X, Y, Z, c);
				}
			} break;
			default: { // Nearest-neighbor interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float xc = x - rw2, yc = y - rh2, zc = z - rd2;
					const int
						X = (int)cimg::round(w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc),
						Y = (int)cimg::round(h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc),
						Z = (int)cimg::round(d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc);
					cimg_forC(res, c) res(x, y, z, c) = _atXYZ(X, Y, Z, c);
				}
			}
			} break;

		default: // Dirichlet
			switch (interpolation) {
			case 2: { // Cubic interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float
						xc = x - rw2, yc = y - rh2, zc = z - rd2,
						X = w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc,
						Y = h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc,
						Z = d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc;
					cimg_forC(res, c) res(x, y, z, c) = cimg::type<T>::cut(cubic_atXYZ(X, Y, Z, c, (T)0));
				}
			} break;
			case 1: { // Linear interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float
						xc = x - rw2, yc = y - rh2, zc = z - rd2,
						X = w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc,
						Y = h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc,
						Z = d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc;
					cimg_forC(res, c) res(x, y, z, c) = linear_atXYZ(X, Y, Z, c, (T)0);
				}
			} break;
			default: { // Nearest-neighbor interpolation
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(res.size() >= 2048))
					cimg_forXYZ(res, x, y, z) {
					const float xc = x - rw2, yc = y - rh2, zc = z - rd2;
					const int
						X = (int)cimg::round(w2 + R(0, 0)*xc + R(1, 0)*yc + R(2, 0)*zc),
						Y = (int)cimg::round(h2 + R(0, 1)*xc + R(1, 1)*yc + R(2, 1)*zc),
						Z = (int)cimg::round(d2 + R(0, 2)*xc + R(1, 2)*yc + R(2, 2)*zc);
					cimg_forC(res, c) res(x, y, z, c) = atXYZ(X, Y, Z, c, (T)0);
				}
			}
			} break;
		}
	}


	//! Warp image content by a warping field \newinstance
	template<typename T>
		template<typename t>
	CImg<T> CImg<T>::get_warp(const CImg<t>& warp, const unsigned int mode = 0,
		const unsigned int interpolation = 1, const unsigned int boundary_conditions = 0) const
	{
		if (is_empty() || !warp) return *this;
		if (mode && !is_sameXYZ(warp))
			throw CImgArgumentException(_cimg_instance
				"warp(): Instance and specified relative warping field (%u,%u,%u,%u,%p) "
				"have different XYZ dimensions.",
				cimg_instance,
				warp._width, warp._height, warp._depth, warp._spectrum, warp._data);

		CImg<T> res(warp._width, warp._height, warp._depth, _spectrum);

		if (warp._spectrum == 1) { // 1d warping
			if (mode >= 3) { // Forward-relative warp
				res.fill((T)0);
				if (interpolation >= 1) // Linear interpolation
					cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z); const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) res.set_linear_atX(*(ptrs++), x + (float)*(ptrs0++), y, z, c);
				}
				else // Nearest-neighbor interpolation
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z); const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) {
						const int X = x + (int)cimg::round(*(ptrs0++));
						if (X >= 0 && X<width()) res(X, y, z, c) = *(ptrs++);
					}
				}
			}
			else if (mode == 2) { // Forward-absolute warp
				res.fill((T)0);
				if (interpolation >= 1) // Linear interpolation
					cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z); const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) res.set_linear_atX(*(ptrs++), (float)*(ptrs0++), y, z, c);
				}
				else // Nearest-neighbor interpolation
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z); const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) {
						const int X = (int)cimg::round(*(ptrs0++));
						if (X >= 0 && X<width()) res(X, y, z, c) = *(ptrs++);
					}
				}
			}
			else if (mode == 1) { // Backward-relative warp
				if (interpolation == 2) // Cubic interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float mx = cimg::mod(x - (float)*(ptrs0++), w2);
								*(ptrd++) = (T)_cubic_atX(mx<width() ? mx : w2 - mx - 1, y, z, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atX(cimg::mod(x - (float)*(ptrs0++), (float)_width), y, z, c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atX(x - (float)*(ptrs0++), y, z, c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)cubic_atX(x - (float)*(ptrs0++), y, z, c, (T)0);
						}
					}
				else if (interpolation == 1) // Linear interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float mx = cimg::mod(x - (float)*(ptrs0++), w2);
								*(ptrd++) = (T)_linear_atX(mx<width() ? mx : w2 - mx - 1, y, z, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atX(cimg::mod(x - (float)*(ptrs0++), (float)_width), y, z, c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atX(x - (float)*(ptrs0++), y, z, c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)linear_atX(x - (float)*(ptrs0++), y, z, c, (T)0);
						}
					}
				else // Nearest-neighbor interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const int w2 = 2 * width();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const int mx = cimg::mod(x - (int)cimg::round(*(ptrs0++)), w2);
								*(ptrd++) = (*this)(mx<width() ? mx : w2 - mx - 1, y, z, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (*this)(cimg::mod(x - (int)cimg::round(*(ptrs0++)), (int)_width), y, z, c);
						}
						break;
					case 1: // Neumann
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = _atX(x - (int)*(ptrs0++), y, z, c);
						}
						break;
					default: // Dirichlet
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = atX(x - (int)*(ptrs0++), y, z, c, (T)0);
						}
					}
			}
			else { // Backward-absolute warp
				if (interpolation == 2) // Cubic interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float mx = cimg::mod((float)*(ptrs0++), w2);
								*(ptrd++) = (T)_cubic_atX(mx<width() ? mx : w2 - mx - 1, 0, 0, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atX(cimg::mod((float)*(ptrs0++), (float)_width), 0, 0, c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atX((float)*(ptrs0++), 0, 0, c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)cubic_atX((float)*(ptrs0++), 0, 0, c, (T)0);
						}
					}
				else if (interpolation == 1) // Linear interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float mx = cimg::mod((float)*(ptrs0++), w2);
								*(ptrd++) = (T)_linear_atX(mx<width() ? mx : w2 - mx - 1, 0, 0, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atX(cimg::mod((float)*(ptrs0++), (float)_width), 0, 0, c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atX((float)*(ptrs0++), 0, 0, c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)linear_atX((float)*(ptrs0++), 0, 0, c, (T)0);
						}
					}
				else // Nearest-neighbor interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const int w2 = 2 * width();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const int mx = cimg::mod((int)cimg::round(*(ptrs0++)), w2);
								*(ptrd++) = (*this)(mx<width() ? mx : w2 - mx - 1, 0, 0, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (*this)(cimg::mod((int)cimg::round(*(ptrs0++)), (int)_width), 0, 0, c);
						}
						break;
					case 1: // Neumann
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = _atX((int)*(ptrs0++), 0, 0, c);
						}
						break;
					default: // Dirichlet
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = atX((int)*(ptrs0++), 0, 0, c, (T)0);
						}
					}
			}

		}
		else if (warp._spectrum == 2) { // 2d warping
			if (mode >= 3) { // Forward-relative warp
				res.fill((T)0);
				if (interpolation >= 1) // Linear interpolation
					cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) res.set_linear_atXY(*(ptrs++), x + (float)*(ptrs0++), y + (float)*(ptrs1++), z, c);
				}
				else // Nearest-neighbor interpolation
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) {
						const int X = x + (int)cimg::round(*(ptrs0++)), Y = y + (int)cimg::round(*(ptrs1++));
						if (X >= 0 && X<width() && Y >= 0 && Y<height()) res(X, Y, z, c) = *(ptrs++);
					}
				}
			}
			else if (mode == 2) { // Forward-absolute warp
				res.fill((T)0);
				if (interpolation >= 1) // Linear interpolation
					cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) res.set_linear_atXY(*(ptrs++), (float)*(ptrs0++), (float)*(ptrs1++), z, c);
				}
				else // Nearest-neighbor interpolation
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) {
						const int X = (int)cimg::round(*(ptrs0++)), Y = (int)cimg::round(*(ptrs1++));
						if (X >= 0 && X<width() && Y >= 0 && Y<height()) res(X, Y, z, c) = *(ptrs++);
					}
				}
			}
			else if (mode == 1) { // Backward-relative warp
				if (interpolation == 2) // Cubic interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width(), h2 = 2.0f*height();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float
									mx = cimg::mod(x - (float)*(ptrs0++), w2),
									my = cimg::mod(y - (float)*(ptrs1++), h2);
								*(ptrd++) = (T)_cubic_atXY(mx<width() ? mx : w2 - mx - 1, my<height() ? my : h2 - my - 1, z, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atXY(cimg::mod(x - (float)*(ptrs0++), (float)_width),
								cimg::mod(y - (float)*(ptrs1++), (float)_height), z, c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atXY(x - (float)*(ptrs0++), y - (float)*(ptrs1++), z, c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)cubic_atXY(x - (float)*(ptrs0++), y - (float)*(ptrs1++), z, c, (T)0);
						}
					}
				else if (interpolation == 1) // Linear interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width(), h2 = 2.0f*height();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float
									mx = cimg::mod(x - (float)*(ptrs0++), w2),
									my = cimg::mod(y - (float)*(ptrs1++), h2);
								*(ptrd++) = (T)_linear_atXY(mx<width() ? mx : w2 - mx - 1, my<height() ? my : h2 - my - 1, z, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atXY(cimg::mod(x - (float)*(ptrs0++), (float)_width),
								cimg::mod(y - (float)*(ptrs1++), (float)_height), z, c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atXY(x - (float)*(ptrs0++), y - (float)*(ptrs1++), z, c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)linear_atXY(x - (float)*(ptrs0++), y - (float)*(ptrs1++), z, c, (T)0);
						}
					}
				else // Nearest-neighbor interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const int w2 = 2 * width(), h2 = 2 * height();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const int
									mx = cimg::mod(x - (int)cimg::round(*(ptrs0++)), w2),
									my = cimg::mod(y - (int)cimg::round(*(ptrs1++)), h2);
								*(ptrd++) = (*this)(mx<width() ? mx : w2 - mx - 1, my<height() ? my : h2 - my - 1, z, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (*this)(cimg::mod(x - (int)cimg::round(*(ptrs0++)), (int)_width),
								cimg::mod(y - (int)cimg::round(*(ptrs1++)), (int)_height), z, c);
						}
						break;
					case 1: // Neumann
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = _atXY(x - (int)*(ptrs0++), y - (int)*(ptrs1++), z, c);
						}
						break;
					default: // Dirichlet
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = atXY(x - (int)*(ptrs0++), y - (int)*(ptrs1++), z, c, (T)0);
						}
					}
			}
			else { // Backward-absolute warp
				if (interpolation == 2) // Cubic interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width(), h2 = 2.0f*height();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float
									mx = cimg::mod((float)*(ptrs0++), w2),
									my = cimg::mod((float)*(ptrs1++), h2);
								*(ptrd++) = (T)_cubic_atXY(mx<width() ? mx : w2 - mx - 1, my<height() ? my : h2 - my - 1, 0, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atXY(cimg::mod((float)*(ptrs0++), (float)_width),
								cimg::mod((float)*(ptrs1++), (float)_height), 0, c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atXY((float)*(ptrs0++), (float)*(ptrs1++), 0, c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)cubic_atXY((float)*(ptrs0++), (float)*(ptrs1++), 0, c, (T)0);
						}
					}
				else if (interpolation == 1) // Linear interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width(), h2 = 2.0f*height();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float
									mx = cimg::mod((float)*(ptrs0++), w2),
									my = cimg::mod((float)*(ptrs1++), h2);
								*(ptrd++) = (T)_linear_atXY(mx<width() ? mx : w2 - mx - 1, my<height() ? my : h2 - my - 1, 0, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atXY(cimg::mod((float)*(ptrs0++), (float)_width),
								cimg::mod((float)*(ptrs1++), (float)_height), 0, c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atXY((float)*(ptrs0++), (float)*(ptrs1++), 0, c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)linear_atXY((float)*(ptrs0++), (float)*(ptrs1++), 0, c, (T)0);
						}
					}
				else // Nearest-neighbor interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const int w2 = 2 * width(), h2 = 2 * height();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const int
									mx = cimg::mod((int)cimg::round(*(ptrs0++)), w2),
									my = cimg::mod((int)cimg::round(*(ptrs1++)), h2);
								*(ptrd++) = (*this)(mx<width() ? mx : w2 - mx - 1, my<height() ? my : h2 - my - 1, 0, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (*this)(cimg::mod((int)cimg::round(*(ptrs0++)), (int)_width),
								cimg::mod((int)cimg::round(*(ptrs1++)), (int)_height), 0, c);
						}
						break;
					case 1: // Neumann
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = _atXY((int)*(ptrs0++), (int)*(ptrs1++), 0, c);
						}
						break;
					default: // Dirichlet
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1); T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = atXY((int)*(ptrs0++), (int)*(ptrs1++), 0, c, (T)0);
						}
					}
			}

		}
		else { // 3d warping
			if (mode >= 3) { // Forward-relative warp
				res.fill((T)0);
				if (interpolation >= 1) // Linear interpolation
					cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
					const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) res.set_linear_atXYZ(*(ptrs++), x + (float)*(ptrs0++), y + (float)*(ptrs1++),
						z + (float)*(ptrs2++), c);
				}
				else // Nearest-neighbor interpolation
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
					const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) {
						const int
							X = x + (int)cimg::round(*(ptrs0++)),
							Y = y + (int)cimg::round(*(ptrs1++)),
							Z = z + (int)cimg::round(*(ptrs2++));
						if (X >= 0 && X<width() && Y >= 0 && Y<height() && Z >= 0 && Z<depth()) res(X, Y, Z, c) = *(ptrs++);
					}
				}
			}
			else if (mode == 2) { // Forward-absolute warp
				res.fill((T)0);
				if (interpolation >= 1) // Linear interpolation
					cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
					const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) res.set_linear_atXYZ(*(ptrs++), (float)*(ptrs0++), (float)*(ptrs1++), (float)*(ptrs2++), c);
				}
				else // Nearest-neighbor interpolation
					cimg_forYZC(res, y, z, c) {
					const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
					const T *ptrs = data(0, y, z, c);
					cimg_forX(res, x) {
						const int
							X = (int)cimg::round(*(ptrs0++)),
							Y = (int)cimg::round(*(ptrs1++)),
							Z = (int)cimg::round(*(ptrs2++));
						if (X >= 0 && X<width() && Y >= 0 && Y<height() && Z >= 0 && Z<depth()) res(X, Y, Z, c) = *(ptrs++);
					}
				}
			}
			else if (mode == 1) { // Backward-relative warp
				if (interpolation == 2) // Cubic interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width(), h2 = 2.0f*height(), d2 = 2.0f*depth();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float
									mx = cimg::mod(x - (float)*(ptrs0++), w2),
									my = cimg::mod(y - (float)*(ptrs1++), h2),
									mz = cimg::mod(z - (float)*(ptrs2++), d2);
								*(ptrd++) = (T)_cubic_atXYZ(mx<width() ? mx : w2 - mx - 1,
									my<height() ? my : h2 - my - 1,
									mz<depth() ? mz : d2 - mz - 1, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atXYZ(cimg::mod(x - (float)*(ptrs0++), (float)_width),
								cimg::mod(y - (float)*(ptrs1++), (float)_height),
								cimg::mod(z - (float)*(ptrs2++), (float)_depth), c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x)
								*(ptrd++) = (T)_cubic_atXYZ(x - (float)*(ptrs0++), y - (float)*(ptrs1++), z - (float)*(ptrs2++), c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x)
								*(ptrd++) = (T)cubic_atXYZ(x - (float)*(ptrs0++), y - (float)*(ptrs1++), z - (float)*(ptrs2++), c, (T)0);
						}
					}
				else if (interpolation == 1) // Linear interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width(), h2 = 2.0f*height(), d2 = 2.0f*depth();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float
									mx = cimg::mod(x - (float)*(ptrs0++), w2),
									my = cimg::mod(y - (float)*(ptrs1++), h2),
									mz = cimg::mod(z - (float)*(ptrs2++), d2);
								*(ptrd++) = (T)_linear_atXYZ(mx<width() ? mx : w2 - mx - 1,
									my<height() ? my : h2 - my - 1,
									mz<depth() ? mz : d2 - mz - 1, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atXYZ(cimg::mod(x - (float)*(ptrs0++), (float)_width),
								cimg::mod(y - (float)*(ptrs1++), (float)_height),
								cimg::mod(z - (float)*(ptrs2++), (float)_depth), c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x)
								*(ptrd++) = (T)_linear_atXYZ(x - (float)*(ptrs0++), y - (float)*(ptrs1++), z - (float)*(ptrs2++), c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x)
								*(ptrd++) = (T)linear_atXYZ(x - (float)*(ptrs0++), y - (float)*(ptrs1++), z - (float)*(ptrs2++), c, (T)0);
						}
					}
				else // Nearest neighbor interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const int w2 = 2 * width(), h2 = 2 * height(), d2 = 2 * depth();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const int
									mx = cimg::mod(x - (int)cimg::round(*(ptrs0++)), w2),
									my = cimg::mod(y - (int)cimg::round(*(ptrs1++)), h2),
									mz = cimg::mod(z - (int)cimg::round(*(ptrs2++)), d2);
								*(ptrd++) = (*this)(mx<width() ? mx : w2 - mx - 1,
									my<height() ? my : h2 - my - 1,
									mz<depth() ? mz : d2 - mz - 1, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (*this)(cimg::mod(x - (int)cimg::round(*(ptrs0++)), (int)_width),
								cimg::mod(y - (int)cimg::round(*(ptrs1++)), (int)_height),
								cimg::mod(z - (int)cimg::round(*(ptrs2++)), (int)_depth), c);
						}
						break;
					case 1: // Neumann
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = _atXYZ(x - (int)*(ptrs0++), y - (int)*(ptrs1++), z - (int)*(ptrs2++), c);
						}
						break;
					default: // Dirichlet
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = atXYZ(x - (int)*(ptrs0++), y - (int)*(ptrs1++), z - (int)*(ptrs2++), c, (T)0);
						}
					}
			}
			else { // Backward-absolute warp
				if (interpolation == 2) // Cubic interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width(), h2 = 2.0f*height(), d2 = 2.0f*depth();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float
									mx = cimg::mod((float)*(ptrs0++), w2),
									my = cimg::mod((float)*(ptrs1++), h2),
									mz = cimg::mod((float)*(ptrs2++), d2);
								*(ptrd++) = (T)_cubic_atXYZ(mx<width() ? mx : w2 - mx - 1,
									my<height() ? my : h2 - my - 1,
									mz<depth() ? mz : d2 - mz - 1, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atXYZ(cimg::mod((float)*(ptrs0++), (float)_width),
								cimg::mod((float)*(ptrs1++), (float)_height),
								cimg::mod((float)*(ptrs2++), (float)_depth), c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_cubic_atXYZ((float)*(ptrs0++), (float)*(ptrs1++), (float)*(ptrs2++), c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)cubic_atXYZ((float)*(ptrs0++), (float)*(ptrs1++), (float)*(ptrs2++),
								c, (T)0);
						}
					}
				else if (interpolation == 1) // Linear interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const float w2 = 2.0f*width(), h2 = 2.0f*height(), d2 = 2.0f*depth();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const float
									mx = cimg::mod((float)*(ptrs0++), w2),
									my = cimg::mod((float)*(ptrs1++), h2),
									mz = cimg::mod((float)*(ptrs2++), d2);
								*(ptrd++) = (T)_linear_atXYZ(mx<width() ? mx : w2 - mx - 1,
									my<height() ? my : h2 - my - 1,
									mz<depth() ? mz : d2 - mz - 1, c);
							}
						}
					} break;
					case 2:// Periodic
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atXYZ(cimg::mod((float)*(ptrs0++), (float)_width),
								cimg::mod((float)*(ptrs1++), (float)_height),
								cimg::mod((float)*(ptrs2++), (float)_depth), c);
						}
						break;
					case 1: // Neumann
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)_linear_atXYZ((float)*(ptrs0++), (float)*(ptrs1++), (float)*(ptrs2++), c);
						}
						break;
					default: // Dirichlet
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 1048576))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (T)linear_atXYZ((float)*(ptrs0++), (float)*(ptrs1++), (float)*(ptrs2++),
								c, (T)0);
						}
					}
				else // Nearest-neighbor interpolation
					switch (boundary_conditions) {
					case 3: { // Mirror
						const int w2 = 2 * width(), h2 = 2 * height(), d2 = 2 * depth();
						cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 4096))
							cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) {
								const int
									mx = cimg::mod((int)cimg::round(*(ptrs0++)), w2),
									my = cimg::mod((int)cimg::round(*(ptrs1++)), h2),
									mz = cimg::mod((int)cimg::round(*(ptrs2++)), d2);
								*(ptrd++) = (*this)(mx<width() ? mx : w2 - mx - 1,
									my<height() ? my : h2 - my - 1,
									mz<depth() ? mz : d2 - mz - 1, c);
							}
						}
					} break;
					case 2: // Periodic
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = (*this)(cimg::mod((int)cimg::round(*(ptrs0++)), (int)_width),
								cimg::mod((int)cimg::round(*(ptrs1++)), (int)_height),
								cimg::mod((int)cimg::round(*(ptrs2++)), (int)_depth), c);
						}
						break;
					case 1: // Neumann
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = _atXYZ((int)*(ptrs0++), (int)*(ptrs1++), (int)*(ptrs2++), c);
						}
						break;
					default: // Dirichlet
						cimg_forYZC(res, y, z, c) {
							const t *ptrs0 = warp.data(0, y, z, 0), *ptrs1 = warp.data(0, y, z, 1), *ptrs2 = warp.data(0, y, z, 2);
							T *ptrd = res.data(0, y, z, c);
							cimg_forX(res, x) *(ptrd++) = atXYZ((int)*(ptrs0++), (int)*(ptrs1++), (int)*(ptrs2++), c, (T)0);
						}
					}
			}
		}
		return res;
	}

	//! Crop image region \newinstance.
	template<typename T>
	CImg<T> CImg<T>::get_crop(const int x0, const int y0, const int z0, const int c0,
		const int x1, const int y1, const int z1, const int c1,
		const unsigned int boundary_conditions)const
	{
		if (is_empty())
			throw CImgInstanceException(_cimg_instance
				"crop(): Empty instance.",
				cimg_instance);
		const int
			nx0 = x0<x1 ? x0 : x1, nx1 = x0^x1^nx0,
			ny0 = y0<y1 ? y0 : y1, ny1 = y0^y1^ny0,
			nz0 = z0<z1 ? z0 : z1, nz1 = z0^z1^nz0,
			nc0 = c0<c1 ? c0 : c1, nc1 = c0^c1^nc0;
		CImg<T> res(1U + nx1 - nx0, 1U + ny1 - ny0, 1U + nz1 - nz0, 1U + nc1 - nc0);
		if (nx0<0 || nx1 >= width() || ny0<0 || ny1 >= height() || nz0<0 || nz1 >= depth() || nc0<0 || nc1 >= spectrum())
			switch (boundary_conditions) {
			case 3: { // Mirror
				const int w2 = 2 * width(), h2 = 2 * height(), d2 = 2 * depth(), s2 = 2 * spectrum();
				cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(_width >= 16 && _height*_depth*_spectrum >= 4))
					cimg_forXYZC(res, x, y, z, c) {
					const int
						mx = cimg::mod(nx0 + x, w2),
						my = cimg::mod(ny0 + y, h2),
						mz = cimg::mod(nz0 + z, d2),
						mc = cimg::mod(nc0 + c, s2);
					res(x, y, z, c) = (*this)(mx<width() ? mx : w2 - mx - 1,
						my<height() ? my : h2 - my - 1,
						mz<depth() ? mz : d2 - mz - 1,
						mc<spectrum() ? mc : s2 - mc - 1);
				}
			} break;
			case 2: { // Periodic
				cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(_width >= 16 && _height*_depth*_spectrum >= 4))
					cimg_forXYZC(res, x, y, z, c) {
					res(x, y, z, c) = (*this)(cimg::mod(nx0 + x, width()), cimg::mod(ny0 + y, height()),
						cimg::mod(nz0 + z, depth()), cimg::mod(nc0 + c, spectrum()));
				}
			} break;
			case 1: // Neumann
				cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(_width >= 16 && _height*_depth*_spectrum >= 4))
					cimg_forXYZC(res, x, y, z, c) res(x, y, z, c) = _atXYZC(nx0 + x, ny0 + y, nz0 + z, nc0 + c);
				break;
			default: // Dirichlet
				res.fill((T)0).draw_image(-nx0, -ny0, -nz0, -nc0, *this);
			}
		else res.draw_image(-nx0, -ny0, -nz0, -nc0, *this);
		return res;
	}


		//! Sharpen image.
		/**
		\param amplitude Sharpening amplitude
		\param sharpen_type Select sharpening method. Can be <tt>{ false=inverse diffusion | true=shock filters }</tt>.
		\param edge Edge threshold (shock filters only).
		\param alpha Gradient smoothness (shock filters only).
		\param sigma Tensor smoothness (shock filters only).
		**/
		template<typename T>
		CImg<T>& CImg<T>::sharpen(const float amplitude, const bool sharpen_type, const float edge,
			const float alpha, const float sigma) {
		if (is_empty()) return *this;
		T val_min, val_max = max_min(val_min);
		const float nedge = edge / 2;
		CImg<Tfloat> velocity(_width, _height, _depth, _spectrum), _veloc_max(_spectrum);

		if (_depth>1) { // 3d
			if (sharpen_type) { // Shock filters.
				CImg<Tfloat> G = (alpha>0 ? get_blur(alpha).get_structure_tensors() : get_structure_tensors());
				if (sigma>0) G.blur(sigma);
				cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(_width >= 32 && _height*_depth >= 16))
					cimg_forYZ(G, y, z) {
					Tfloat *ptrG0 = G.data(0, y, z, 0), *ptrG1 = G.data(0, y, z, 1),
						*ptrG2 = G.data(0, y, z, 2), *ptrG3 = G.data(0, y, z, 3);
					CImg<Tfloat> val, vec;
					cimg_forX(G, x) {
						G.get_tensor_at(x, y, z).symmetric_eigen(val, vec);
						if (val[0]<0) val[0] = 0;
						if (val[1]<0) val[1] = 0;
						if (val[2]<0) val[2] = 0;
						*(ptrG0++) = vec(0, 0);
						*(ptrG1++) = vec(0, 1);
						*(ptrG2++) = vec(0, 2);
						*(ptrG3++) = 1 - (Tfloat)std::pow(1 + val[0] + val[1] + val[2], -(Tfloat)nedge);
					}
				}
				cimg_pragma_openmp(parallel for cimg_openmp_if(_width*_height*_depth >= 512 && _spectrum >= 2))
					cimg_forC(*this, c) {
					Tfloat *ptrd = velocity.data(0, 0, 0, c), veloc_max = 0;
					CImg_3x3x3(I, Tfloat);
					cimg_for3x3x3(*this, x, y, z, c, I, Tfloat) {
						const Tfloat
							u = G(x, y, z, 0),
							v = G(x, y, z, 1),
							w = G(x, y, z, 2),
							amp = G(x, y, z, 3),
							ixx = Incc + Ipcc - 2 * Iccc,
							ixy = (Innc + Ippc - Inpc - Ipnc) / 4,
							ixz = (Incn + Ipcp - Incp - Ipcn) / 4,
							iyy = Icnc + Icpc - 2 * Iccc,
							iyz = (Icnn + Icpp - Icnp - Icpn) / 4,
							izz = Iccn + Iccp - 2 * Iccc,
							ixf = Incc - Iccc,
							ixb = Iccc - Ipcc,
							iyf = Icnc - Iccc,
							iyb = Iccc - Icpc,
							izf = Iccn - Iccc,
							izb = Iccc - Iccp,
							itt = u*u*ixx + v*v*iyy + w*w*izz + 2 * u*v*ixy + 2 * u*w*ixz + 2 * v*w*iyz,
							it = u*cimg::minmod(ixf, ixb) + v*cimg::minmod(iyf, iyb) + w*cimg::minmod(izf, izb),
							veloc = -amp*cimg::sign(itt)*cimg::abs(it);
						*(ptrd++) = veloc;
						if (veloc>veloc_max) veloc_max = veloc; else if (-veloc>veloc_max) veloc_max = -veloc;
					}
					_veloc_max[c] = veloc_max;
				}
			}
			else  // Inverse diffusion.
				cimg_forC(*this, c) {
				Tfloat *ptrd = velocity.data(0, 0, 0, c), veloc_max = 0;
				CImg_3x3x3(I, Tfloat);
				cimg_for3x3x3(*this, x, y, z, c, I, Tfloat) {
					const Tfloat veloc = -Ipcc - Incc - Icpc - Icnc - Iccp - Iccn + 6 * Iccc;
					*(ptrd++) = veloc;
					if (veloc>veloc_max) veloc_max = veloc; else if (-veloc>veloc_max) veloc_max = -veloc;
				}
				_veloc_max[c] = veloc_max;
			}
		}
		else { // 2d.
			if (sharpen_type) { // Shock filters.
				CImg<Tfloat> G = (alpha>0 ? get_blur(alpha).get_structure_tensors() : get_structure_tensors());
				if (sigma>0) G.blur(sigma);
				cimg_pragma_openmp(parallel for cimg_openmp_if(_width >= 32 && _height >= 16))
					cimg_forY(G, y) {
					CImg<Tfloat> val, vec;
					Tfloat *ptrG0 = G.data(0, y, 0, 0), *ptrG1 = G.data(0, y, 0, 1), *ptrG2 = G.data(0, y, 0, 2);
					cimg_forX(G, x) {
						G.get_tensor_at(x, y).symmetric_eigen(val, vec);
						if (val[0]<0) val[0] = 0;
						if (val[1]<0) val[1] = 0;
						*(ptrG0++) = vec(0, 0);
						*(ptrG1++) = vec(0, 1);
						*(ptrG2++) = 1 - (Tfloat)std::pow(1 + val[0] + val[1], -(Tfloat)nedge);
					}
				}
				cimg_pragma_openmp(parallel for cimg_openmp_if(_width*_height >= 512 && _spectrum >= 2))
					cimg_forC(*this, c) {
					Tfloat *ptrd = velocity.data(0, 0, 0, c), veloc_max = 0;
					CImg_3x3(I, Tfloat);
					cimg_for3x3(*this, x, y, 0, c, I, Tfloat) {
						const Tfloat
							u = G(x, y, 0),
							v = G(x, y, 1),
							amp = G(x, y, 2),
							ixx = Inc + Ipc - 2 * Icc,
							ixy = (Inn + Ipp - Inp - Ipn) / 4,
							iyy = Icn + Icp - 2 * Icc,
							ixf = Inc - Icc,
							ixb = Icc - Ipc,
							iyf = Icn - Icc,
							iyb = Icc - Icp,
							itt = u*u*ixx + v*v*iyy + 2 * u*v*ixy,
							it = u*cimg::minmod(ixf, ixb) + v*cimg::minmod(iyf, iyb),
							veloc = -amp*cimg::sign(itt)*cimg::abs(it);
						*(ptrd++) = veloc;
						if (veloc>veloc_max) veloc_max = veloc; else if (-veloc>veloc_max) veloc_max = -veloc;
					}
					_veloc_max[c] = veloc_max;
				}
			}
			else // Inverse diffusion.
				cimg_forC(*this, c) {
				Tfloat *ptrd = velocity.data(0, 0, 0, c), veloc_max = 0;
				CImg_3x3(I, Tfloat);
				cimg_for3x3(*this, x, y, 0, c, I, Tfloat) {
					const Tfloat veloc = -Ipc - Inc - Icp - Icn + 4 * Icc;
					*(ptrd++) = veloc;
					if (veloc>veloc_max) veloc_max = veloc; else if (-veloc>veloc_max) veloc_max = -veloc;
				}
				_veloc_max[c] = veloc_max;
			}
		}
		const Tfloat veloc_max = _veloc_max.max();
		if (veloc_max <= 0) return *this;
		return ((velocity *= amplitude / veloc_max) += *this).cut(val_min, val_max).move_to(*this);
	}

	 //! Compute field of diffusion tensors for edge-preserving smoothing.
	 /**
	 \param sharpness Sharpness
	 \param anisotropy Anisotropy
	 \param alpha Standard deviation of the gradient blur.
	 \param sigma Standard deviation of the structure tensor blur.
	 \param is_sqrt Tells if the square root of the tensor field is computed instead.
	 **/
	template<typename T>
	 CImg<T>& CImg<T>::diffusion_tensors(const float sharpness, const float anisotropy,
		 const float alpha, const float sigma, const bool is_sqrt) {
	 CImg<Tfloat> res;
	 const float
		 nsharpness = std::max(sharpness, 1e-5f),
		 power1 = (is_sqrt ? 0.5f : 1)*nsharpness,
		 power2 = power1 / (1e-7f + 1 - anisotropy);
	 blur(alpha).normalize(0, (T)255);

	 if (_depth>1) { // 3d
		 get_structure_tensors().move_to(res).blur(sigma);
		 cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if(_width >= 256 && _height*_depth >= 256))
			 cimg_forYZ(*this, y, z) {
			 Tfloat
				 *ptrd0 = res.data(0, y, z, 0), *ptrd1 = res.data(0, y, z, 1), *ptrd2 = res.data(0, y, z, 2),
				 *ptrd3 = res.data(0, y, z, 3), *ptrd4 = res.data(0, y, z, 4), *ptrd5 = res.data(0, y, z, 5);
			 CImg<floatT> val(3), vec(3, 3);
			 cimg_forX(*this, x) {
				 res.get_tensor_at(x, y, z).symmetric_eigen(val, vec);
				 const float
					 _l1 = val[2], _l2 = val[1], _l3 = val[0],
					 l1 = _l1>0 ? _l1 : 0, l2 = _l2>0 ? _l2 : 0, l3 = _l3>0 ? _l3 : 0,
					 ux = vec(0, 0), uy = vec(0, 1), uz = vec(0, 2),
					 vx = vec(1, 0), vy = vec(1, 1), vz = vec(1, 2),
					 wx = vec(2, 0), wy = vec(2, 1), wz = vec(2, 2),
					 n1 = (float)std::pow(1 + l1 + l2 + l3, -power1),
					 n2 = (float)std::pow(1 + l1 + l2 + l3, -power2);
				 *(ptrd0++) = n1*(ux*ux + vx*vx) + n2*wx*wx;
				 *(ptrd1++) = n1*(ux*uy + vx*vy) + n2*wx*wy;
				 *(ptrd2++) = n1*(ux*uz + vx*vz) + n2*wx*wz;
				 *(ptrd3++) = n1*(uy*uy + vy*vy) + n2*wy*wy;
				 *(ptrd4++) = n1*(uy*uz + vy*vz) + n2*wy*wz;
				 *(ptrd5++) = n1*(uz*uz + vz*vz) + n2*wz*wz;
			 }
		 }
	 }
	 else { // for 2d images
		 get_structure_tensors().move_to(res).blur(sigma);
		 cimg_pragma_openmp(parallel for cimg_openmp_if(_width >= 256 && _height >= 256))
			 cimg_forY(*this, y) {
			 Tfloat *ptrd0 = res.data(0, y, 0, 0), *ptrd1 = res.data(0, y, 0, 1), *ptrd2 = res.data(0, y, 0, 2);
			 CImg<floatT> val(2), vec(2, 2);
			 cimg_forX(*this, x) {
				 res.get_tensor_at(x, y).symmetric_eigen(val, vec);
				 const float
					 _l1 = val[1], _l2 = val[0],
					 l1 = _l1>0 ? _l1 : 0, l2 = _l2>0 ? _l2 : 0,
					 ux = vec(1, 0), uy = vec(1, 1),
					 vx = vec(0, 0), vy = vec(0, 1),
					 n1 = (float)std::pow(1 + l1 + l2, -power1),
					 n2 = (float)std::pow(1 + l1 + l2, -power2);
				 *(ptrd0++) = n1*ux*ux + n2*vx*vx;
				 *(ptrd1++) = n1*ux*uy + n2*vx*vy;
				 *(ptrd2++) = n1*uy*uy + n2*vy*vy;
			 }
		 }
	 }
	 return res.move_to(*this);
 }

	 //! Draw a quadratic Mandelbrot or Julia 2d fractal.
	 /**
	 \param x0 X-coordinate of the upper-left pixel.
	 \param y0 Y-coordinate of the upper-left pixel.
	 \param x1 X-coordinate of the lower-right pixel.
	 \param y1 Y-coordinate of the lower-right pixel.
	 \param colormap Colormap.
	 \param opacity Drawing opacity.
	 \param z0r Real part of the upper-left fractal vertex.
	 \param z0i Imaginary part of the upper-left fractal vertex.
	 \param z1r Real part of the lower-right fractal vertex.
	 \param z1i Imaginary part of the lower-right fractal vertex.
	 \param iteration_max Maximum number of iterations for each estimated point.
	 \param is_normalized_iteration Tells if iterations are normalized.
	 \param is_julia_set Tells if the Mandelbrot or Julia set is rendered.
	 \param param_r Real part of the Julia set parameter.
	 \param param_i Imaginary part of the Julia set parameter.
	 \note Fractal rendering is done by the Escape Time Algorithm.
	 **/
	 template<typename T> 
		template<typename tc>
	 CImg<T>& CImg<T>::draw_mandelbrot(const int x0, const int y0, const int x1, const int y1,
		 const CImg<tc>& colormap, const float opacity = 1,
		 const double z0r = -2, const double z0i = -2, const double z1r = 2, const double z1i = 2,
		 const unsigned int iteration_max = 255,
		 const bool is_normalized_iteration = false,
		 const bool is_julia_set = false,
		 const double param_r = 0, const double param_i = 0)
	 {
		 if (is_empty()) return *this;
		 CImg<tc> palette;
		 if (colormap) palette.assign(colormap._data, colormap.size() / colormap._spectrum, 1, 1, colormap._spectrum, true);
		 if (palette && palette._spectrum != _spectrum)
			 throw CImgArgumentException(_cimg_instance
				 "draw_mandelbrot(): Instance and specified colormap (%u,%u,%u,%u,%p) have "
				 "incompatible dimensions.",
				 cimg_instance,
				 colormap._width, colormap._height, colormap._depth, colormap._spectrum, colormap._data);

		 const float nopacity = cimg::abs(opacity), copacity = 1 - std::max(opacity, 0.0f), ln2 = (float)std::log(2.0);
		 const int
			 _x0 = cimg::cut(x0, 0, width() - 1),
			 _y0 = cimg::cut(y0, 0, height() - 1),
			 _x1 = cimg::cut(x1, 0, width() - 1),
			 _y1 = cimg::cut(y1, 0, height() - 1);

		 cimg_pragma_openmp(parallel for collapse(2) cimg_openmp_if((1 + _x1 - _x0)*(1 + _y1 - _y0) >= 2048))
			 for (int q = _y0; q <= _y1; ++q)
				 for (int p = _x0; p <= _x1; ++p) {
					 unsigned int iteration = 0;
					 const double x = z0r + p*(z1r - z0r) / _width, y = z0i + q*(z1i - z0i) / _height;
					 double zr, zi, cr, ci;
					 if (is_julia_set) { zr = x; zi = y; cr = param_r; ci = param_i; }
					 else { zr = param_r; zi = param_i; cr = x; ci = y; }
					 for (iteration = 1; zr*zr + zi*zi <= 4 && iteration <= iteration_max; ++iteration) {
						 const double temp = zr*zr - zi*zi + cr;
						 zi = 2 * zr*zi + ci;
						 zr = temp;
					 }
					 if (iteration>iteration_max) {
						 if (palette) {
							 if (opacity >= 1) cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)palette(0, c);
							 else cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)(palette(0, c)*nopacity + (*this)(p, q, 0, c)*copacity);
						 }
						 else {
							 if (opacity >= 1) cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)0;
							 else cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)((*this)(p, q, 0, c)*copacity);
						 }
					 }
					 else if (is_normalized_iteration) {
						 const float
							 normz = (float)cimg::abs(zr*zr + zi*zi),
							 niteration = (float)(iteration + 1 - std::log(std::log(normz)) / ln2);
						 if (palette) {
							 if (opacity >= 1) cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)palette._linear_atX(niteration, c);
							 else cimg_forC(*this, c)
								 (*this)(p, q, 0, c) = (T)(palette._linear_atX(niteration, c)*nopacity + (*this)(p, q, 0, c)*copacity);
						 }
						 else {
							 if (opacity >= 1) cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)niteration;
							 else cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)(niteration*nopacity + (*this)(p, q, 0, c)*copacity);
						 }
					 }
					 else {
						 if (palette) {
							 if (opacity >= 1) cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)palette._atX(iteration, c);
							 else cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)(palette(iteration, c)*nopacity + (*this)(p, q, 0, c)*copacity);
						 }
						 else {
							 if (opacity >= 1) cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)iteration;
							 else cimg_forC(*this, c) (*this)(p, q, 0, c) = (T)(iteration*nopacity + (*this)(p, q, 0, c)*copacity);
						 }
					 }
				 }
		 return *this;
	 }


	 //! Resize image to new dimensions \newinstance.
	 template<typename T>
	 CImg<T> CImg<T>::get_resize(const int size_x, const int size_y = -100,
		 const int size_z = -100, const int size_c = -100,
		 const int interpolation_type = 1, const unsigned int boundary_conditions = 0,
		 const float centering_x = 0, const float centering_y = 0,
		 const float centering_z = 0, const float centering_c = 0) const
	 {
		 if (centering_x<0 || centering_x>1 || centering_y<0 || centering_y>1 ||
			 centering_z<0 || centering_z>1 || centering_c<0 || centering_c>1)
			 throw CImgArgumentException(_cimg_instance
				 "resize(): Specified centering arguments (%g,%g,%g,%g) are outside range [0,1].",
				 cimg_instance,
				 centering_x, centering_y, centering_z, centering_c);

		 if (!size_x || !size_y || !size_z || !size_c) return CImg<T>();
		 const unsigned int
			 sx = std::max(1U, (unsigned int)(size_x >= 0 ? size_x : -size_x*width() / 100)),
			 sy = std::max(1U, (unsigned int)(size_y >= 0 ? size_y : -size_y*height() / 100)),
			 sz = std::max(1U, (unsigned int)(size_z >= 0 ? size_z : -size_z*depth() / 100)),
			 sc = std::max(1U, (unsigned int)(size_c >= 0 ? size_c : -size_c*spectrum() / 100));
		 if (sx == _width && sy == _height && sz == _depth && sc == _spectrum) return +*this;
		 if (is_empty()) return CImg<T>(sx, sy, sz, sc, (T)0);
		 CImg<T> res;
		 switch (interpolation_type) {

			 // Raw resizing.
			 //
		 case -1:
			 std::memcpy(res.assign(sx, sy, sz, sc, (T)0)._data, _data, sizeof(T)*std::min(size(), (ulongT)sx*sy*sz*sc));
			 break;

			 // No interpolation.
			 //
		 case 0: {
			 const int
				 xc = (int)(centering_x*((int)sx - width())),
				 yc = (int)(centering_y*((int)sy - height())),
				 zc = (int)(centering_z*((int)sz - depth())),
				 cc = (int)(centering_c*((int)sc - spectrum()));

			 switch (boundary_conditions) {
			 case 3: { // Mirror
				 res.assign(sx, sy, sz, sc);
				 const int w2 = 2 * width(), h2 = 2 * height(), d2 = 2 * depth(), s2 = 2 * spectrum();
				 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 65536))
					 cimg_forXYZC(res, x, y, z, c) {
					 const int
						 mx = cimg::mod(x - xc, w2), my = cimg::mod(y - yc, h2),
						 mz = cimg::mod(z - zc, d2), mc = cimg::mod(c - cc, s2);
					 res(x, y, z, c) = (*this)(mx<width() ? mx : w2 - mx - 1,
						 my<height() ? my : h2 - my - 1,
						 mz<depth() ? mz : d2 - mz - 1,
						 mc<spectrum() ? mc : s2 - mc - 1);
				 }
			 } break;
			 case 2: { // Periodic
				 res.assign(sx, sy, sz, sc);
				 const int
					 x0 = ((int)xc%width()) - width(),
					 y0 = ((int)yc%height()) - height(),
					 z0 = ((int)zc%depth()) - depth(),
					 c0 = ((int)cc%spectrum()) - spectrum(),
					 dx = width(), dy = height(), dz = depth(), dc = spectrum();
				 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(res.size() >= 65536))
					 for (int c = c0; c<(int)sc; c += dc)
						 for (int z = z0; z<(int)sz; z += dz)
							 for (int y = y0; y<(int)sy; y += dy)
								 for (int x = x0; x<(int)sx; x += dx)
									 res.draw_image(x, y, z, c, *this);
			 } break;
			 case 1: { // Neumann
				 res.assign(sx, sy, sz, sc).draw_image(xc, yc, zc, cc, *this);
				 CImg<T> sprite;
				 if (xc>0) {  // X-backward
					 res.get_crop(xc, yc, zc, cc, xc, yc + height() - 1, zc + depth() - 1, cc + spectrum() - 1).move_to(sprite);
					 for (int x = xc - 1; x >= 0; --x) res.draw_image(x, yc, zc, cc, sprite);
				 }
				 if (xc + width()<(int)sx) { // X-forward
					 res.get_crop(xc + width() - 1, yc, zc, cc, xc + width() - 1, yc + height() - 1,
						 zc + depth() - 1, cc + spectrum() - 1).move_to(sprite);
					 for (int x = xc + width(); x<(int)sx; ++x) res.draw_image(x, yc, zc, cc, sprite);
				 }
				 if (yc>0) {  // Y-backward
					 res.get_crop(0, yc, zc, cc, sx - 1, yc, zc + depth() - 1, cc + spectrum() - 1).move_to(sprite);
					 for (int y = yc - 1; y >= 0; --y) res.draw_image(0, y, zc, cc, sprite);
				 }
				 if (yc + height()<(int)sy) { // Y-forward
					 res.get_crop(0, yc + height() - 1, zc, cc, sx - 1, yc + height() - 1,
						 zc + depth() - 1, cc + spectrum() - 1).move_to(sprite);
					 for (int y = yc + height(); y<(int)sy; ++y) res.draw_image(0, y, zc, cc, sprite);
				 }
				 if (zc>0) {  // Z-backward
					 res.get_crop(0, 0, zc, cc, sx - 1, sy - 1, zc, cc + spectrum() - 1).move_to(sprite);
					 for (int z = zc - 1; z >= 0; --z) res.draw_image(0, 0, z, cc, sprite);
				 }
				 if (zc + depth()<(int)sz) { // Z-forward
					 res.get_crop(0, 0, zc + depth() - 1, cc, sx - 1, sy - 1, zc + depth() - 1, cc + spectrum() - 1).move_to(sprite);
					 for (int z = zc + depth(); z<(int)sz; ++z) res.draw_image(0, 0, z, cc, sprite);
				 }
				 if (cc>0) {  // C-backward
					 res.get_crop(0, 0, 0, cc, sx - 1, sy - 1, sz - 1, cc).move_to(sprite);
					 for (int c = cc - 1; c >= 0; --c) res.draw_image(0, 0, 0, c, sprite);
				 }
				 if (cc + spectrum()<(int)sc) { // C-forward
					 res.get_crop(0, 0, 0, cc + spectrum() - 1, sx - 1, sy - 1, sz - 1, cc + spectrum() - 1).move_to(sprite);
					 for (int c = cc + spectrum(); c<(int)sc; ++c) res.draw_image(0, 0, 0, c, sprite);
				 }
			 } break;
			 default: // Dirichlet
				 res.assign(sx, sy, sz, sc, (T)0).draw_image(xc, yc, zc, cc, *this);
			 }
			 break;
		 } break;

			 // Nearest neighbor interpolation.
			 //
		 case 1: {
			 res.assign(sx, sy, sz, sc);
			 CImg<ulongT> off_x(sx), off_y(sy + 1), off_z(sz + 1), off_c(sc + 1);
			 const ulongT
				 wh = (ulongT)_width*_height,
				 whd = (ulongT)_width*_height*_depth,
				 sxy = (ulongT)sx*sy,
				 sxyz = (ulongT)sx*sy*sz;
			 if (sx == _width) off_x.fill(1);
			 else {
				 ulongT *poff_x = off_x._data, curr = 0;
				 cimg_forX(res, x) {
					 const ulongT old = curr;
					 curr = (ulongT)((x + 1.0)*_width / sx);
					 *(poff_x++) = curr - old;
				 }
			 }
			 if (sy == _height) off_y.fill(_width);
			 else {
				 ulongT *poff_y = off_y._data, curr = 0;
				 cimg_forY(res, y) {
					 const ulongT old = curr;
					 curr = (ulongT)((y + 1.0)*_height / sy);
					 *(poff_y++) = _width*(curr - old);
				 }
				 *poff_y = 0;
			 }
			 if (sz == _depth) off_z.fill(wh);
			 else {
				 ulongT *poff_z = off_z._data, curr = 0;
				 cimg_forZ(res, z) {
					 const ulongT old = curr;
					 curr = (ulongT)((z + 1.0)*_depth / sz);
					 *(poff_z++) = wh*(curr - old);
				 }
				 *poff_z = 0;
			 }
			 if (sc == _spectrum) off_c.fill(whd);
			 else {
				 ulongT *poff_c = off_c._data, curr = 0;
				 cimg_forC(res, c) {
					 const ulongT old = curr;
					 curr = (ulongT)((c + 1.0)*_spectrum / sc);
					 *(poff_c++) = whd*(curr - old);
				 }
				 *poff_c = 0;
			 }

			 T *ptrd = res._data;
			 const T* ptrc = _data;
			 const ulongT *poff_c = off_c._data;
			 for (unsigned int c = 0; c<sc; ) {
				 const T *ptrz = ptrc;
				 const ulongT *poff_z = off_z._data;
				 for (unsigned int z = 0; z<sz; ) {
					 const T *ptry = ptrz;
					 const ulongT *poff_y = off_y._data;
					 for (unsigned int y = 0; y<sy; ) {
						 const T *ptrx = ptry;
						 const ulongT *poff_x = off_x._data;
						 cimg_forX(res, x) { *(ptrd++) = *ptrx; ptrx += *(poff_x++); }
						 ++y;
						 ulongT dy = *(poff_y++);
						 for (; !dy && y<dy; std::memcpy(ptrd, ptrd - sx, sizeof(T)*sx), ++y, ptrd += sx, dy = *(poff_y++)) {}
						 ptry += dy;
					 }
					 ++z;
					 ulongT dz = *(poff_z++);
					 for (; !dz && z<dz; std::memcpy(ptrd, ptrd - sxy, sizeof(T)*sxy), ++z, ptrd += sxy, dz = *(poff_z++)) {}
					 ptrz += dz;
				 }
				 ++c;
				 ulongT dc = *(poff_c++);
				 for (; !dc && c<dc; std::memcpy(ptrd, ptrd - sxyz, sizeof(T)*sxyz), ++c, ptrd += sxyz, dc = *(poff_c++)) {}
				 ptrc += dc;
			 }
		 } break;

			 // Moving average.
			 //
		 case 2: {
			 bool instance_first = true;
			 if (sx != _width) {
				 CImg<Tfloat> tmp(sx, _height, _depth, _spectrum, 0);
				 for (unsigned int a = _width*sx, b = _width, c = sx, s = 0, t = 0; a; ) {
					 const unsigned int d = std::min(b, c);
					 a -= d; b -= d; c -= d;
					 cimg_forYZC(tmp, y, z, v) tmp(t, y, z, v) += (Tfloat)(*this)(s, y, z, v)*d;
					 if (!b) {
						 cimg_forYZC(tmp, y, z, v) tmp(t, y, z, v) /= _width;
						 ++t;
						 b = _width;
					 }
					 if (!c) { ++s; c = sx; }
				 }
				 tmp.move_to(res);
				 instance_first = false;
			 }
			 if (sy != _height) {
				 CImg<Tfloat> tmp(sx, sy, _depth, _spectrum, 0);
				 for (unsigned int a = _height*sy, b = _height, c = sy, s = 0, t = 0; a; ) {
					 const unsigned int d = std::min(b, c);
					 a -= d; b -= d; c -= d;
					 if (instance_first)
						 cimg_forXZC(tmp, x, z, v) tmp(x, t, z, v) += (Tfloat)(*this)(x, s, z, v)*d;
					 else
						 cimg_forXZC(tmp, x, z, v) tmp(x, t, z, v) += (Tfloat)res(x, s, z, v)*d;
					 if (!b) {
						 cimg_forXZC(tmp, x, z, v) tmp(x, t, z, v) /= _height;
						 ++t;
						 b = _height;
					 }
					 if (!c) { ++s; c = sy; }
				 }
				 tmp.move_to(res);
				 instance_first = false;
			 }
			 if (sz != _depth) {
				 CImg<Tfloat> tmp(sx, sy, sz, _spectrum, 0);
				 for (unsigned int a = _depth*sz, b = _depth, c = sz, s = 0, t = 0; a; ) {
					 const unsigned int d = std::min(b, c);
					 a -= d; b -= d; c -= d;
					 if (instance_first)
						 cimg_forXYC(tmp, x, y, v) tmp(x, y, t, v) += (Tfloat)(*this)(x, y, s, v)*d;
					 else
						 cimg_forXYC(tmp, x, y, v) tmp(x, y, t, v) += (Tfloat)res(x, y, s, v)*d;
					 if (!b) {
						 cimg_forXYC(tmp, x, y, v) tmp(x, y, t, v) /= _depth;
						 ++t;
						 b = _depth;
					 }
					 if (!c) { ++s; c = sz; }
				 }
				 tmp.move_to(res);
				 instance_first = false;
			 }
			 if (sc != _spectrum) {
				 CImg<Tfloat> tmp(sx, sy, sz, sc, 0);
				 for (unsigned int a = _spectrum*sc, b = _spectrum, c = sc, s = 0, t = 0; a; ) {
					 const unsigned int d = std::min(b, c);
					 a -= d; b -= d; c -= d;
					 if (instance_first)
						 cimg_forXYZ(tmp, x, y, z) tmp(x, y, z, t) += (Tfloat)(*this)(x, y, z, s)*d;
					 else
						 cimg_forXYZ(tmp, x, y, z) tmp(x, y, z, t) += (Tfloat)res(x, y, z, s)*d;
					 if (!b) {
						 cimg_forXYZ(tmp, x, y, z) tmp(x, y, z, t) /= _spectrum;
						 ++t;
						 b = _spectrum;
					 }
					 if (!c) { ++s; c = sc; }
				 }
				 tmp.move_to(res);
				 instance_first = false;
			 }
		 } break;

			 // Linear interpolation.
			 //
		 case 3: {
			 CImg<uintT> off(cimg::max(sx, sy, sz, sc));
			 CImg<doubleT> foff(off._width);
			 CImg<T> resx, resy, resz, resc;
			 double curr, old;

			 if (sx != _width) {
				 if (_width == 1) get_resize(sx, _height, _depth, _spectrum, 1).move_to(resx);
				 else if (_width>sx) get_resize(sx, _height, _depth, _spectrum, 2).move_to(resx);
				 else {
					 const double fx = (!boundary_conditions && sx>_width) ? (sx>1 ? (_width - 1.0) / (sx - 1) : 0) :
						 (double)_width / sx;
					 resx.assign(sx, _height, _depth, _spectrum);
					 curr = old = 0;
					 unsigned int *poff = off._data;
					 double *pfoff = foff._data;
					 cimg_forX(resx, x) {
						 *(pfoff++) = curr - (unsigned int)curr;
						 old = curr;
						 curr = std::min(width() - 1.0, curr + fx);
						 *(poff++) = (unsigned int)curr - (unsigned int)old;
					 }
					 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resx.size() >= 65536))
						 cimg_forYZC(resx, y, z, c) {
						 const T *ptrs = data(0, y, z, c), *const ptrsmax = ptrs + _width - 1;
						 T *ptrd = resx.data(0, y, z, c);
						 const unsigned int *poff = off._data;
						 const double *pfoff = foff._data;
						 cimg_forX(resx, x) {
							 const double alpha = *(pfoff++);
							 const T val1 = *ptrs, val2 = ptrs<ptrsmax ? *(ptrs + 1) : val1;
							 *(ptrd++) = (T)((1 - alpha)*val1 + alpha*val2);
							 ptrs += *(poff++);
						 }
					 }
				 }
			 }
			 else resx.assign(*this, true);

			 if (sy != _height) {
				 if (_height == 1) resx.get_resize(sx, sy, _depth, _spectrum, 1).move_to(resy);
				 else {
					 if (_height>sy) resx.get_resize(sx, sy, _depth, _spectrum, 2).move_to(resy);
					 else {
						 const double fy = (!boundary_conditions && sy>_height) ? (sy>1 ? (_height - 1.0) / (sy - 1) : 0) :
							 (double)_height / sy;
						 resy.assign(sx, sy, _depth, _spectrum);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forY(resy, y) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(height() - 1.0, curr + fy);
							 *(poff++) = sx*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resy.size() >= 65536))
							 cimg_forXZC(resy, x, z, c) {
							 const T *ptrs = resx.data(x, 0, z, c), *const ptrsmax = ptrs + (_height - 1)*sx;
							 T *ptrd = resy.data(x, 0, z, c);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forY(resy, y) {
								 const double alpha = *(pfoff++);
								 const T val1 = *ptrs, val2 = ptrs<ptrsmax ? *(ptrs + sx) : val1;
								 *ptrd = (T)((1 - alpha)*val1 + alpha*val2);
								 ptrd += sx;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resx.assign();
			 }
			 else resy.assign(resx, true);

			 if (sz != _depth) {
				 if (_depth == 1) resy.get_resize(sx, sy, sz, _spectrum, 1).move_to(resz);
				 else {
					 if (_depth>sz) resy.get_resize(sx, sy, sz, _spectrum, 2).move_to(resz);
					 else {
						 const double fz = (!boundary_conditions && sz>_depth) ? (sz>1 ? (_depth - 1.0) / (sz - 1) : 0) :
							 (double)_depth / sz;
						 const unsigned int sxy = sx*sy;
						 resz.assign(sx, sy, sz, _spectrum);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forZ(resz, z) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(depth() - 1.0, curr + fz);
							 *(poff++) = sxy*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resz.size() >= 65536))
							 cimg_forXYC(resz, x, y, c) {
							 const T *ptrs = resy.data(x, y, 0, c), *const ptrsmax = ptrs + (_depth - 1)*sxy;
							 T *ptrd = resz.data(x, y, 0, c);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forZ(resz, z) {
								 const double alpha = *(pfoff++);
								 const T val1 = *ptrs, val2 = ptrs<ptrsmax ? *(ptrs + sxy) : val1;
								 *ptrd = (T)((1 - alpha)*val1 + alpha*val2);
								 ptrd += sxy;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resy.assign();
			 }
			 else resz.assign(resy, true);

			 if (sc != _spectrum) {
				 if (_spectrum == 1) resz.get_resize(sx, sy, sz, sc, 1).move_to(resc);
				 else {
					 if (_spectrum>sc) resz.get_resize(sx, sy, sz, sc, 2).move_to(resc);
					 else {
						 const double fc = (!boundary_conditions && sc>_spectrum) ? (sc>1 ? (_spectrum - 1.0) / (sc - 1) : 0) :
							 (double)_spectrum / sc;
						 const unsigned int sxyz = sx*sy*sz;
						 resc.assign(sx, sy, sz, sc);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forC(resc, c) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(spectrum() - 1.0, curr + fc);
							 *(poff++) = sxyz*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resc.size() >= 65536))
							 cimg_forXYZ(resc, x, y, z) {
							 const T *ptrs = resz.data(x, y, z, 0), *const ptrsmax = ptrs + (_spectrum - 1)*sxyz;
							 T *ptrd = resc.data(x, y, z, 0);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forC(resc, c) {
								 const double alpha = *(pfoff++);
								 const T val1 = *ptrs, val2 = ptrs<ptrsmax ? *(ptrs + sxyz) : val1;
								 *ptrd = (T)((1 - alpha)*val1 + alpha*val2);
								 ptrd += sxyz;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resz.assign();
			 }
			 else resc.assign(resz, true);
			 return resc._is_shared ? (resz._is_shared ? (resy._is_shared ? (resx._is_shared ? (+(*this)) : resx) : resy) : resz) : resc;
		 } break;

			 // Grid interpolation.
			 //
		 case 4: {
			 CImg<T> resx, resy, resz, resc;
			 if (sx != _width) {
				 if (sx<_width) get_resize(sx, _height, _depth, _spectrum, 1).move_to(resx);
				 else {
					 resx.assign(sx, _height, _depth, _spectrum, (T)0);
					 const int dx = (int)(2 * sx), dy = 2 * width();
					 int err = (int)(dy + centering_x*(sx*dy / width() - dy)), xs = 0;
					 cimg_forX(resx, x) if ((err -= dy) <= 0) {
						 cimg_forYZC(resx, y, z, c) resx(x, y, z, c) = (*this)(xs, y, z, c);
						 ++xs;
						 err += dx;
					 }
				 }
			 }
			 else resx.assign(*this, true);

			 if (sy != _height) {
				 if (sy<_height) resx.get_resize(sx, sy, _depth, _spectrum, 1).move_to(resy);
				 else {
					 resy.assign(sx, sy, _depth, _spectrum, (T)0);
					 const int dx = (int)(2 * sy), dy = 2 * height();
					 int err = (int)(dy + centering_y*(sy*dy / height() - dy)), ys = 0;
					 cimg_forY(resy, y) if ((err -= dy) <= 0) {
						 cimg_forXZC(resy, x, z, c) resy(x, y, z, c) = resx(x, ys, z, c);
						 ++ys;
						 err += dx;
					 }
				 }
				 resx.assign();
			 }
			 else resy.assign(resx, true);

			 if (sz != _depth) {
				 if (sz<_depth) resy.get_resize(sx, sy, sz, _spectrum, 1).move_to(resz);
				 else {
					 resz.assign(sx, sy, sz, _spectrum, (T)0);
					 const int dx = (int)(2 * sz), dy = 2 * depth();
					 int err = (int)(dy + centering_z*(sz*dy / depth() - dy)), zs = 0;
					 cimg_forZ(resz, z) if ((err -= dy) <= 0) {
						 cimg_forXYC(resz, x, y, c) resz(x, y, z, c) = resy(x, y, zs, c);
						 ++zs;
						 err += dx;
					 }
				 }
				 resy.assign();
			 }
			 else resz.assign(resy, true);

			 if (sc != _spectrum) {
				 if (sc<_spectrum) resz.get_resize(sx, sy, sz, sc, 1).move_to(resc);
				 else {
					 resc.assign(sx, sy, sz, sc, (T)0);
					 const int dx = (int)(2 * sc), dy = 2 * spectrum();
					 int err = (int)(dy + centering_c*(sc*dy / spectrum() - dy)), cs = 0;
					 cimg_forC(resc, c) if ((err -= dy) <= 0) {
						 cimg_forXYZ(resc, x, y, z) resc(x, y, z, c) = resz(x, y, z, cs);
						 ++cs;
						 err += dx;
					 }
				 }
				 resz.assign();
			 }
			 else resc.assign(resz, true);

			 return resc._is_shared ? (resz._is_shared ? (resy._is_shared ? (resx._is_shared ? (+(*this)) : resx) : resy) : resz) : resc;
		 } break;

			 // Cubic interpolation.
			 //
		 case 5: {
			 const Tfloat vmin = (Tfloat)cimg::type<T>::min(), vmax = (Tfloat)cimg::type<T>::max();
			 CImg<uintT> off(cimg::max(sx, sy, sz, sc));
			 CImg<doubleT> foff(off._width);
			 CImg<T> resx, resy, resz, resc;
			 double curr, old;

			 if (sx != _width) {
				 if (_width == 1) get_resize(sx, _height, _depth, _spectrum, 1).move_to(resx);
				 else {
					 if (_width>sx) get_resize(sx, _height, _depth, _spectrum, 2).move_to(resx);
					 else {
						 const double fx = (!boundary_conditions && sx>_width) ? (sx>1 ? (_width - 1.0) / (sx - 1) : 0) :
							 (double)_width / sx;
						 resx.assign(sx, _height, _depth, _spectrum);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forX(resx, x) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(width() - 1.0, curr + fx);
							 *(poff++) = (unsigned int)curr - (unsigned int)old;
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resx.size() >= 65536))
							 cimg_forYZC(resx, y, z, c) {
							 const T *const ptrs0 = data(0, y, z, c), *ptrs = ptrs0, *const ptrsmax = ptrs + (_width - 2);
							 T *ptrd = resx.data(0, y, z, c);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forX(resx, x) {
								 const double
									 t = *(pfoff++),
									 val1 = (double)*ptrs,
									 val0 = ptrs>ptrs0 ? (double)*(ptrs - 1) : val1,
									 val2 = ptrs <= ptrsmax ? (double)*(ptrs + 1) : val1,
									 val3 = ptrs<ptrsmax ? (double)*(ptrs + 2) : val2,
									 val = val1 + 0.5f*(t*(-val0 + val2) + t*t*(2 * val0 - 5 * val1 + 4 * val2 - val3) +
										 t*t*t*(-val0 + 3 * val1 - 3 * val2 + val3));
								 *(ptrd++) = (T)(val<vmin ? vmin : val>vmax ? vmax : val);
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
			 }
			 else resx.assign(*this, true);

			 if (sy != _height) {
				 if (_height == 1) resx.get_resize(sx, sy, _depth, _spectrum, 1).move_to(resy);
				 else {
					 if (_height>sy) resx.get_resize(sx, sy, _depth, _spectrum, 2).move_to(resy);
					 else {
						 const double fy = (!boundary_conditions && sy>_height) ? (sy>1 ? (_height - 1.0) / (sy - 1) : 0) :
							 (double)_height / sy;
						 resy.assign(sx, sy, _depth, _spectrum);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forY(resy, y) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(height() - 1.0, curr + fy);
							 *(poff++) = sx*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resy.size() >= 65536))
							 cimg_forXZC(resy, x, z, c) {
							 const T *const ptrs0 = resx.data(x, 0, z, c), *ptrs = ptrs0, *const ptrsmax = ptrs + (_height - 2)*sx;
							 T *ptrd = resy.data(x, 0, z, c);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forY(resy, y) {
								 const double
									 t = *(pfoff++),
									 val1 = (double)*ptrs,
									 val0 = ptrs>ptrs0 ? (double)*(ptrs - sx) : val1,
									 val2 = ptrs <= ptrsmax ? (double)*(ptrs + sx) : val1,
									 val3 = ptrs<ptrsmax ? (double)*(ptrs + 2 * sx) : val2,
									 val = val1 + 0.5f*(t*(-val0 + val2) + t*t*(2 * val0 - 5 * val1 + 4 * val2 - val3) +
										 t*t*t*(-val0 + 3 * val1 - 3 * val2 + val3));
								 *ptrd = (T)(val<vmin ? vmin : val>vmax ? vmax : val);
								 ptrd += sx;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resx.assign();
			 }
			 else resy.assign(resx, true);

			 if (sz != _depth) {
				 if (_depth == 1) resy.get_resize(sx, sy, sz, _spectrum, 1).move_to(resz);
				 else {
					 if (_depth>sz) resy.get_resize(sx, sy, sz, _spectrum, 2).move_to(resz);
					 else {
						 const double fz = (!boundary_conditions && sz>_depth) ? (sz>1 ? (_depth - 1.0) / (sz - 1) : 0) :
							 (double)_depth / sz;
						 const unsigned int sxy = sx*sy;
						 resz.assign(sx, sy, sz, _spectrum);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forZ(resz, z) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(depth() - 1.0, curr + fz);
							 *(poff++) = sxy*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resz.size() >= 65536))
							 cimg_forXYC(resz, x, y, c) {
							 const T *const ptrs0 = resy.data(x, y, 0, c), *ptrs = ptrs0, *const ptrsmax = ptrs + (_depth - 2)*sxy;
							 T *ptrd = resz.data(x, y, 0, c);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forZ(resz, z) {
								 const double
									 t = *(pfoff++),
									 val1 = (double)*ptrs,
									 val0 = ptrs>ptrs0 ? (double)*(ptrs - sxy) : val1,
									 val2 = ptrs <= ptrsmax ? (double)*(ptrs + sxy) : val1,
									 val3 = ptrs<ptrsmax ? (double)*(ptrs + 2 * sxy) : val2,
									 val = val1 + 0.5f*(t*(-val0 + val2) + t*t*(2 * val0 - 5 * val1 + 4 * val2 - val3) +
										 t*t*t*(-val0 + 3 * val1 - 3 * val2 + val3));
								 *ptrd = (T)(val<vmin ? vmin : val>vmax ? vmax : val);
								 ptrd += sxy;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resy.assign();
			 }
			 else resz.assign(resy, true);

			 if (sc != _spectrum) {
				 if (_spectrum == 1) resz.get_resize(sx, sy, sz, sc, 1).move_to(resc);
				 else {
					 if (_spectrum>sc) resz.get_resize(sx, sy, sz, sc, 2).move_to(resc);
					 else {
						 const double fc = (!boundary_conditions && sc>_spectrum) ? (sc>1 ? (_spectrum - 1.0) / (sc - 1) : 0) :
							 (double)_spectrum / sc;
						 const unsigned int sxyz = sx*sy*sz;
						 resc.assign(sx, sy, sz, sc);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forC(resc, c) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(spectrum() - 1.0, curr + fc);
							 *(poff++) = sxyz*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resc.size() >= 65536))
							 cimg_forXYZ(resc, x, y, z) {
							 const T *const ptrs0 = resz.data(x, y, z, 0), *ptrs = ptrs0, *const ptrsmax = ptrs + (_spectrum - 2)*sxyz;
							 T *ptrd = resc.data(x, y, z, 0);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forC(resc, c) {
								 const double
									 t = *(pfoff++),
									 val1 = (double)*ptrs,
									 val0 = ptrs>ptrs0 ? (double)*(ptrs - sxyz) : val1,
									 val2 = ptrs <= ptrsmax ? (double)*(ptrs + sxyz) : val1,
									 val3 = ptrs<ptrsmax ? (double)*(ptrs + 2 * sxyz) : val2,
									 val = val1 + 0.5f*(t*(-val0 + val2) + t*t*(2 * val0 - 5 * val1 + 4 * val2 - val3) +
										 t*t*t*(-val0 + 3 * val1 - 3 * val2 + val3));
								 *ptrd = (T)(val<vmin ? vmin : val>vmax ? vmax : val);
								 ptrd += sxyz;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resz.assign();
			 }
			 else resc.assign(resz, true);

			 return resc._is_shared ? (resz._is_shared ? (resy._is_shared ? (resx._is_shared ? (+(*this)) : resx) : resy) : resz) : resc;
		 } break;

			 // Lanczos interpolation.
			 //
		 case 6: {
			 const double vmin = (double)cimg::type<T>::min(), vmax = (double)cimg::type<T>::max();
			 CImg<uintT> off(cimg::max(sx, sy, sz, sc));
			 CImg<doubleT> foff(off._width);
			 CImg<T> resx, resy, resz, resc;
			 double curr, old;

			 if (sx != _width) {
				 if (_width == 1) get_resize(sx, _height, _depth, _spectrum, 1).move_to(resx);
				 else {
					 if (_width>sx) get_resize(sx, _height, _depth, _spectrum, 2).move_to(resx);
					 else {
						 const double fx = (!boundary_conditions && sx>_width) ? (sx>1 ? (_width - 1.0) / (sx - 1) : 0) :
							 (double)_width / sx;
						 resx.assign(sx, _height, _depth, _spectrum);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forX(resx, x) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(width() - 1.0, curr + fx);
							 *(poff++) = (unsigned int)curr - (unsigned int)old;
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resx.size() >= 65536))
							 cimg_forYZC(resx, y, z, c) {
							 const T *const ptrs0 = data(0, y, z, c), *ptrs = ptrs0, *const ptrsmin = ptrs0 + 1,
								 *const ptrsmax = ptrs0 + (_width - 2);
							 T *ptrd = resx.data(0, y, z, c);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forX(resx, x) {
								 const double
									 t = *(pfoff++),
									 w0 = _cimg_lanczos(t + 2),
									 w1 = _cimg_lanczos(t + 1),
									 w2 = _cimg_lanczos(t),
									 w3 = _cimg_lanczos(t - 1),
									 w4 = _cimg_lanczos(t - 2),
									 val2 = (double)*ptrs,
									 val1 = ptrs >= ptrsmin ? (double)*(ptrs - 1) : val2,
									 val0 = ptrs>ptrsmin ? (double)*(ptrs - 2) : val1,
									 val3 = ptrs <= ptrsmax ? (double)*(ptrs + 1) : val2,
									 val4 = ptrs<ptrsmax ? (double)*(ptrs + 2) : val3,
									 val = (val0*w0 + val1*w1 + val2*w2 + val3*w3 + val4*w4) / (w1 + w2 + w3 + w4);
								 *(ptrd++) = (T)(val<vmin ? vmin : val>vmax ? vmax : val);
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
			 }
			 else resx.assign(*this, true);

			 if (sy != _height) {
				 if (_height == 1) resx.get_resize(sx, sy, _depth, _spectrum, 1).move_to(resy);
				 else {
					 if (_height>sy) resx.get_resize(sx, sy, _depth, _spectrum, 2).move_to(resy);
					 else {
						 const double fy = (!boundary_conditions && sy>_height) ? (sy>1 ? (_height - 1.0) / (sy - 1) : 0) :
							 (double)_height / sy;
						 resy.assign(sx, sy, _depth, _spectrum);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forY(resy, y) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(height() - 1.0, curr + fy);
							 *(poff++) = sx*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resy.size() >= 65536))
							 cimg_forXZC(resy, x, z, c) {
							 const T *const ptrs0 = resx.data(x, 0, z, c), *ptrs = ptrs0, *const ptrsmin = ptrs0 + sx,
								 *const ptrsmax = ptrs0 + (_height - 2)*sx;
							 T *ptrd = resy.data(x, 0, z, c);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forY(resy, y) {
								 const double
									 t = *(pfoff++),
									 w0 = _cimg_lanczos(t + 2),
									 w1 = _cimg_lanczos(t + 1),
									 w2 = _cimg_lanczos(t),
									 w3 = _cimg_lanczos(t - 1),
									 w4 = _cimg_lanczos(t - 2),
									 val2 = (double)*ptrs,
									 val1 = ptrs >= ptrsmin ? (double)*(ptrs - sx) : val2,
									 val0 = ptrs>ptrsmin ? (double)*(ptrs - 2 * sx) : val1,
									 val3 = ptrs <= ptrsmax ? (double)*(ptrs + sx) : val2,
									 val4 = ptrs<ptrsmax ? (double)*(ptrs + 2 * sx) : val3,
									 val = (val0*w0 + val1*w1 + val2*w2 + val3*w3 + val4*w4) / (w1 + w2 + w3 + w4);
								 *ptrd = (T)(val<vmin ? vmin : val>vmax ? vmax : val);
								 ptrd += sx;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resx.assign();
			 }
			 else resy.assign(resx, true);

			 if (sz != _depth) {
				 if (_depth == 1) resy.get_resize(sx, sy, sz, _spectrum, 1).move_to(resz);
				 else {
					 if (_depth>sz) resy.get_resize(sx, sy, sz, _spectrum, 2).move_to(resz);
					 else {
						 const double fz = (!boundary_conditions && sz>_depth) ? (sz>1 ? (_depth - 1.0) / (sz - 1) : 0) :
							 (double)_depth / sz;
						 const unsigned int sxy = sx*sy;
						 resz.assign(sx, sy, sz, _spectrum);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forZ(resz, z) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(depth() - 1.0, curr + fz);
							 *(poff++) = sxy*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resz.size() >= 65536))
							 cimg_forXYC(resz, x, y, c) {
							 const T *const ptrs0 = resy.data(x, y, 0, c), *ptrs = ptrs0, *const ptrsmin = ptrs0 + sxy,
								 *const ptrsmax = ptrs0 + (_depth - 2)*sxy;
							 T *ptrd = resz.data(x, y, 0, c);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forZ(resz, z) {
								 const double
									 t = *(pfoff++),
									 w0 = _cimg_lanczos(t + 2),
									 w1 = _cimg_lanczos(t + 1),
									 w2 = _cimg_lanczos(t),
									 w3 = _cimg_lanczos(t - 1),
									 w4 = _cimg_lanczos(t - 2),
									 val2 = (double)*ptrs,
									 val1 = ptrs >= ptrsmin ? (double)*(ptrs - sxy) : val2,
									 val0 = ptrs>ptrsmin ? (double)*(ptrs - 2 * sxy) : val1,
									 val3 = ptrs <= ptrsmax ? (double)*(ptrs + sxy) : val2,
									 val4 = ptrs<ptrsmax ? (double)*(ptrs + 2 * sxy) : val3,
									 val = (val0*w0 + val1*w1 + val2*w2 + val3*w3 + val4*w4) / (w1 + w2 + w3 + w4);
								 *ptrd = (T)(val<vmin ? vmin : val>vmax ? vmax : val);
								 ptrd += sxy;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resy.assign();
			 }
			 else resz.assign(resy, true);

			 if (sc != _spectrum) {
				 if (_spectrum == 1) resz.get_resize(sx, sy, sz, sc, 1).move_to(resc);
				 else {
					 if (_spectrum>sc) resz.get_resize(sx, sy, sz, sc, 2).move_to(resc);
					 else {
						 const double fc = (!boundary_conditions && sc>_spectrum) ? (sc>1 ? (_spectrum - 1.0) / (sc - 1) : 0) :
							 (double)_spectrum / sc;
						 const unsigned int sxyz = sx*sy*sz;
						 resc.assign(sx, sy, sz, sc);
						 curr = old = 0;
						 unsigned int *poff = off._data;
						 double *pfoff = foff._data;
						 cimg_forC(resc, c) {
							 *(pfoff++) = curr - (unsigned int)curr;
							 old = curr;
							 curr = std::min(spectrum() - 1.0, curr + fc);
							 *(poff++) = sxyz*((unsigned int)curr - (unsigned int)old);
						 }
						 cimg_pragma_openmp(parallel for collapse(3) cimg_openmp_if(resc.size() >= 65536))
							 cimg_forXYZ(resc, x, y, z) {
							 const T *const ptrs0 = resz.data(x, y, z, 0), *ptrs = ptrs0, *const ptrsmin = ptrs0 + sxyz,
								 *const ptrsmax = ptrs + (_spectrum - 2)*sxyz;
							 T *ptrd = resc.data(x, y, z, 0);
							 const unsigned int *poff = off._data;
							 const double *pfoff = foff._data;
							 cimg_forC(resc, c) {
								 const double
									 t = *(pfoff++),
									 w0 = _cimg_lanczos(t + 2),
									 w1 = _cimg_lanczos(t + 1),
									 w2 = _cimg_lanczos(t),
									 w3 = _cimg_lanczos(t - 1),
									 w4 = _cimg_lanczos(t - 2),
									 val2 = (double)*ptrs,
									 val1 = ptrs >= ptrsmin ? (double)*(ptrs - sxyz) : val2,
									 val0 = ptrs>ptrsmin ? (double)*(ptrs - 2 * sxyz) : val1,
									 val3 = ptrs <= ptrsmax ? (double)*(ptrs + sxyz) : val2,
									 val4 = ptrs<ptrsmax ? (double)*(ptrs + 2 * sxyz) : val3,
									 val = (val0*w0 + val1*w1 + val2*w2 + val3*w3 + val4*w4) / (w1 + w2 + w3 + w4);
								 *ptrd = (T)(val<vmin ? vmin : val>vmax ? vmax : val);
								 ptrd += sxyz;
								 ptrs += *(poff++);
							 }
						 }
					 }
				 }
				 resz.assign();
			 }
			 else resc.assign(resz, true);

			 return resc._is_shared ? (resz._is_shared ? (resy._is_shared ? (resx._is_shared ? (+(*this)) : resx) : resy) : resz) : resc;
		 } break;

			 // Unknow interpolation.
			 //
		 default:
			 throw CImgArgumentException(_cimg_instance
				 "resize(): Invalid specified interpolation %d "
				 "(should be { -1=raw | 0=none | 1=nearest | 2=average | 3=linear | 4=grid | "
				 "5=cubic | 6=lanczos }).",
				 cimg_instance,
				 interpolation_type);
		 }
		 return res;
	 }

}//cimg_library_suffixed

#endif // !_CLASS_CIMG_HPP
