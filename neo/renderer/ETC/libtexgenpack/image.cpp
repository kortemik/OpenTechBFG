/*
    image.c -- part of texgenpack, a texture compressor using fgen.

    texgenpack -- a genetic algorithm texture compressor.
    Copyright 2013 Harm Hanemaaijer

    This file is part of texgenpack.

    texgenpack is free software: you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    texgenpack is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with texgenpack.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <malloc.h>
#include "libtexgenpack.h"
#include "libtexgenpack_internal.h"
#include "decode.h"
#include "packing.h"

namespace texgen {

	// Load image file or texture file. In the latter case, the texture is decoded into an image.

	bool load_image(const Options *opt, const char *filename, int filetype, Image *image) {
		if (filetype == FILE_TYPE_ASTC) {
			return decompress_astc_file(opt, filename, image);
		}
		else
		if (filetype & FILE_TYPE_TEXTURE_BIT) {
			Texture texture;
			switch (filetype) {
			case FILE_TYPE_PKM :
				if (!load_pkm_file(filename, &texture))
					return false;
				break;
			case FILE_TYPE_KTX :
				if (load_ktx_file(filename, 1, &texture, opt) < 0)
					return false;
				break;
			case FILE_TYPE_DDS :
				if (load_dds_file(filename, 1, &texture, opt) < 0)
					return false;
				break;
			case FILE_TYPE_ASTC :
				if (!load_astc_file(filename, &texture))
					return false;
				break;
			default :
				return false;
			}
			const char *texture_type_str = texture_type_text(texture.type);
			if (!texture_type_str) {
				destroy_texture(&texture);
				return false;
			}
			if (!set_texture_decoding_function(opt, &texture, NULL) || !convert_texture_to_image(opt, &texture, image)) {
				destroy_texture(&texture);
				return false;
			}
			destroy_texture(&texture);
		}
		else {
			switch (filetype) {
			case FILE_TYPE_PNG :
				if (!load_png_file(filename, image))
					return false;
				break;
			default :
				return false;
			}
			// Optionally convert to half-float format.
			if (opt->half_float)
				convert_image_to_half_float(image);
		}
		return true;
	}

	// Load multiple mipmaps from a texture file. image must be an array big enough to hold max_images.
	// Return the number of mipmap images.

	int load_mipmap_images(const Options *opt, const char *filename, int filetype, int max_images, Image *image) {
		if (filetype & FILE_TYPE_TEXTURE_BIT) {
			Texture *texture = (Texture *)alloca(sizeof(Texture) * max_images);
			int n;
			switch (filetype) {
			case FILE_TYPE_KTX :
				n = load_ktx_file(filename, max_images, &texture[0], opt);
				if (n == -1)
					return -1;
				break;
			case FILE_TYPE_DDS :
				n = load_dds_file(filename, max_images, &texture[0], opt);
				if (n == -1)
					return -1;
				break;
			case FILE_TYPE_PKM :
				if (!load_pkm_file(filename, &texture[0]))
					return -1;
				n = 1;
				break;
			case FILE_TYPE_ASTC :
				if (!load_astc_file(filename, &texture[0]))
					return -1;
				n = 1;
				break;
			}
			const char *texture_type_str = texture_type_text(texture[0].type);
			if (!texture_type_str) {
				for (int i = 0; i < n; i++) {
					destroy_texture(&texture[i]);
				}
				return -1;
			}
			bool working = true;
			for (int i = 0; i < n && working; i++) {
				working = set_texture_decoding_function(opt, &texture[i], NULL);
			}
			bool success = working;
			for (int i = 0; i < n; i++) {
				if (working)
					success = convert_texture_to_image(opt, &texture[i], &image[i]);
				destroy_texture(&texture[i]);
			}
			return working ? n : -1;
		}
		return -1;
	}

	// Save image file.

	bool save_image(Image *image, const char *filename, int filetype) {
		switch (filetype) {
		case FILE_TYPE_PNG :
			return save_png_file(image, filename);
		default :
			return false;
		}
	}

	// Load multiple mipmaps from a texture file. texture must be an array big enough to hold max_texture.
	// Return the number of mipmap images.

	int load_texture(const Options *opt, const char *filename, int filetype, int max_mipmaps, Texture *texture) {
		if (filetype & FILE_TYPE_TEXTURE_BIT) {
			int n;
			switch (filetype) {
			case FILE_TYPE_KTX :
				n = load_ktx_file(filename, max_mipmaps, &texture[0], opt);
				if (n == -1)
					return -1;
				break;
			case FILE_TYPE_DDS :
				n = load_dds_file(filename, max_mipmaps, &texture[0], opt);
				if (n == -1)
					return -1;
				break;
			case FILE_TYPE_PKM :
				if (!load_pkm_file(filename, &texture[0]))
					return -1;
				n = 1;
				break;
			case FILE_TYPE_ASTC :
				if (!load_astc_file(filename, &texture[0]))
					return -1;
				n = 1;
				break;
			}
			const char *texture_type_str = texture_type_text(texture[0].type);
			if (!texture_type_str)
				return -1;
			bool working = true;
			for (int i = 0; i < n && working; i++)
				working = set_texture_decoding_function(opt, &texture[i], NULL);
			if (!working)
				for (int i = 0; i < n && working; i++) {
					destroy_texture(&texture[i]);
				}
			return working ? n : -1;
		}
		return -1;
	}

	// Save texture file.

	bool save_texture(const Options *opt, Texture *texture, int nu_mipmaps, const char *filename, int filetype) {
		switch (filetype) {
		case FILE_TYPE_PKM :
			if (texture->type != TEXTURE_TYPE_ETC1) {
				return false;
			}
			return save_pkm_file(texture, filename);
		case FILE_TYPE_KTX :
			if (!texture->info->ktx_support) {
				return false;
			}
			return save_ktx_file(texture, nu_mipmaps, filename, opt);
		case FILE_TYPE_DDS :
			if (!texture->info->dds_support) {
				return false;
			}
			return save_dds_file(texture, nu_mipmaps, filename);
		case FILE_TYPE_ASTC :
			if (!(texture->type & TEXTURE_TYPE_ASTC_BIT)) {
				return false;
			}
			return save_astc_file(texture, filename);
		default:
			return false;
		}
	}

	// Compare images. Must be the same size. Returns the RMSE. New version using block comparison functions.

	double compare_images(const Options *opt, Image *image1, Image *image2) {
		int compare_alpha = 0;
		if (image1->alpha_bits > 0 && image2->alpha_bits > 0)
			compare_alpha = 1;
		if (image1->is_half_float ^ image2->is_half_float) {
			// Allow exception for the case that image1 is half-float and image2 is in regular format.
			if (!(image1->is_half_float && image2->bits_per_component == 8)) {
				return 0;
			}
		}
		if (image1->bits_per_component != image2->bits_per_component) {
			if (!(image1->is_half_float && image2->bits_per_component == 8) &&
			!(image1->bits_per_component == 16 && image2->bits_per_component == 8)) {
				return 0;
			}
		}
		int nu_components = image1->nu_components;
		if (image2->nu_components < image1->nu_components)
			nu_components = image2->nu_components;
		TextureComparisonFunction compare_func;
		if (image1->bits_per_component == 16) {
			if (image1->is_half_float && image2->is_half_float) {
				calculate_half_float_table();
				if (opt->hdr) {
					calculate_gamma_corrected_half_float_table();
					if (compare_alpha)
						compare_func = compare_block_4x4_rgba_half_float_hdr;
					else
						compare_func = compare_block_4x4_rgb_half_float_hdr;
				}
				else
					if (compare_alpha)
						compare_func = compare_block_4x4_rgba_half_float;
					else
						compare_func = compare_block_4x4_rgb_half_float;
			}
			else
			if (image1->is_half_float && image2->bits_per_component == 8) {
				// Comparison of half-float image with RGB(A)8 image.
				calculate_normalized_float_table();
				calculate_half_float_table();
				if (compare_alpha)
					compare_func = compare_block_4x4_rgba8_with_half_float;
				else
					compare_func = compare_block_4x4_rgb8_with_half_float;
			}
			else
 			if (image2->bits_per_component == 8)
				if (image1->is_signed)
					compare_func = compare_block_4x4_signed_8_bit_components_with_16_bit;
				else
					compare_func = compare_block_4x4_8_bit_components_with_16_bit;
			else
			if (image1->is_signed && image2->is_signed)
				if (image1->nu_components == 2)
					compare_func = compare_block_4x4_rg16_signed;
				else
					compare_func = compare_block_4x4_r16_signed;
			else
				if (image1->nu_components == 2)
					compare_func = compare_block_4x4_rg16;
				else
					compare_func = compare_block_4x4_r16;
		}
		else	// 8-bit components.
			if (nu_components >= 3)
				if (nu_components == 4)
					compare_func = compare_block_4x4_rgba;
				else
					compare_func = compare_block_4x4_rgb;
			else
				if (image1->is_signed)
					compare_func = compare_block_4x4_signed_8_bit_components;
				else
					compare_func = compare_block_4x4_8_bit_components;
		BlockUserData block_user_data;
		block_user_data.flags = 0;
		block_user_data.image_pixels = image1->pixels;
		if (image1->is_half_float)
			block_user_data.image_rowstride = image1->extended_width * 8;
		else
			block_user_data.image_rowstride = image1->extended_width * 4;
		Texture texture;
		texture.width = image1->width;
		texture.height = image1->height;
		block_user_data.texture = &texture;
		double error = 0;
		for (int y = 0; y < image1->height; y += 4)
			for (int x = 0; x < image1->width; x += 4) {
				int w;
				if (x + 4 > image1->width)
					w = image1->width - x;
				else
					w = 4;
				int h;
				if (y + 4 > image1->height)
					h = image1->height - y;
				else
					h = 4;
				block_user_data.x_offset = x;
				block_user_data.y_offset = y;
				unsigned int image_buffer[32];
				if (image2->is_half_float)
					for (int i = 0; i < h; i++)
						memcpy(&image_buffer[i * 8], &image2->pixels[((y + i) * image2->extended_width +
							x) * 2], w * 8);
				else
					for (int i = 0; i < h; i++)
						memcpy(&image_buffer[i * 4], &image2->pixels[(y + i) * image2->extended_width +
							x], w * 4);
				error += 1 / compare_func(image_buffer, &block_user_data, NULL);
			}
		int n = image1->height * image1->width;
		return sqrt((double)error / n);
	}

	// Destroy a texture.

	void destroy_texture(Texture *texture) {
		free(texture->pixels);
	}

	// Destroy an image.

	void destroy_image(Image *image) {
		free(image->pixels);
	}

	// Clone an image.

	void clone_image(Image *image1, Image *image2) {
		*image2 = *image1;
		int size;
		if (image1->is_half_float)
			size = image1->height * image1->extended_width * 8;
		else
			size = image1->height * image1->extended_width * 4;
		image2->pixels = (unsigned int *)malloc(size);
		memcpy(image2->pixels, image1->pixels, size);
	}

	// Fill image with black and alpha of 1.

	void clear_image(Image *image) {
		if (image->is_half_float) {
			uint16_t h[2];
			float f[2];
			f[0] = 0;
			f[1] = 1.0;
			singles2halfp(&h[0], &f[0], 2);
			int n =  image->extended_width * image->height;
			for (int i = 0; i < n; i++) {
				*(uint64_t *)&image->pixels[i * 2] = pack_rgba16(h[0], h[0], h[0], h[1]);
			}
			return;
		}
		int n =  image->extended_width * image->height;
		if (image->bits_per_component == 16)
			for (int i = 0; i < n; i++)
				image->pixels[i] = 0;
		else
		if (image->is_signed) {
			// Signed 8-bit components.
			uint32_t pixel = pack_r((uint8_t)(char)- 127);
			if (image->nu_components == 2)
				pixel |= pack_g((uint8_t)(char)- 127);
			for (int i = 0; i < n; i++)
				image->pixels[i] = pixel;
		}
		else
			for (int i = 0; i < n; i++)
				image->pixels[i] = pack_rgb_alpha_0xff(0, 0, 0);
	}

	// Convert a texture to an image. The image's pixels will be allocated. The image will have extended_width and
	// extended_height to block boundaries, but the width and height are that of the original texture.

	bool convert_texture_to_image(const Options *opt, Texture *texture, Image *image) {
		if (texture->type & TEXTURE_TYPE_ASTC_BIT) {
			return convert_astc_texture_to_image(opt, texture, image);
		}
		int n = (texture->extended_height / texture->block_height) * (texture->extended_width / texture->block_width);
		image->width = texture->width;
		image->height = texture->height;
		image->extended_width = texture->extended_width;
		image->extended_height = texture->extended_height;
		int bpp = 4;
		if (texture->type & TEXTURE_TYPE_HALF_FLOAT_BIT)
			bpp = 8;	// 64-bit pixels
		image->pixels = (unsigned int *)malloc(n * texture->block_width * texture->block_height * bpp);
		image->alpha_bits = 0;
		if (texture->type & TEXTURE_TYPE_ALPHA_BIT) {
			if (texture->type & TEXTURE_TYPE_HALF_FLOAT_BIT)
				image->alpha_bits = 16;
			else
				image->alpha_bits = 8;
		}
		image->nu_components = texture->info->nu_components;
		image->bits_per_component = 8;
		if (texture->type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT)
			image->bits_per_component = 16;
		image->is_half_float = 0;
		if (texture->type & TEXTURE_TYPE_HALF_FLOAT_BIT) {
			image->is_half_float = 1;
		}
		if (texture->type & TEXTURE_TYPE_SIGNED_BIT)
			image->is_signed = 1;
		else
			image->is_signed = 0;
		image->srgb = 0;
		unsigned int *buffer = (unsigned int *)alloca(sizeof(unsigned int) * texture->block_width * texture->block_height * (bpp / 4));
		int flags = 0;
		if (texture->type & TEXTURE_TYPE_ETC_BIT) {
			flags = ETC2_MODE_ALLOWED_ALL;
			if (opt->allowed_modes_etc2 != - 1)
				flags = opt->allowed_modes_etc2;
		}
		if (texture->type == TEXTURE_TYPE_BPTC_FLOAT || texture->type == TEXTURE_TYPE_BPTC_SIGNED_FLOAT)
			flags = BPTC_FLOAT_MODE_ALLOWED_ALL;
		for (int y = 0; y < texture->extended_height; y += texture->block_height)
			for (int x = 0; x < texture->extended_width; x += texture->block_width) {
				unsigned char *bitstring = (unsigned char *)&texture->pixels[
					((y / texture->block_height) * (texture->extended_width / texture->block_width) +
					(x / texture->block_width)) * (texture->info->internal_bits_per_block / 32)];
				int r = texture->decoding_function(bitstring, buffer, flags);
				if (r == 0)
					// If the block mode is not allowed, display a black block.
					memset(buffer, 0, 16 * bpp);
				for (int i = 0; i < texture->block_height; i++)
					memcpy(&image->pixels[((y + i) * image->extended_width + x) * (bpp / 4)],
						&buffer[i * texture->block_width * (bpp / 4)], texture->block_width * bpp);
			}
		if (image->alpha_bits >= 8)
			check_1bit_alpha(image);
		return true;
	}

	// Pad the borders of the image (the area beyond width x height to extended_width x extended_heigth).

	void pad_image_borders(Image *image) {
		int bpp = 4;
		if (image->is_half_float)
			bpp = 8;
		// Pad the borders by extending the nearest pixel.
		for (int y = 0; y < image->height; y++)
			for (int x = image->width; x < image->extended_width; x++)
				image->pixels[(y * image->extended_width  + x) * (bpp / 4)] =
					image->pixels[(y * image->extended_width + image->width - 1) * (bpp / 4)];
		for (int y = image->height; y < image->extended_height; y++)
			for (int x = 0; x < image->extended_width; x++)
				image->pixels[(y * image->extended_width + x)* (bpp / 4)] = image->pixels[
					((image->height - 1) * image->extended_width + x) * (bpp / 4)];
	}


	// Check whether the image contains only 0x00 (transparent) or 0xFF (opaque) alpha. If so, set the
	// number of alpha bits to 1.

	void check_1bit_alpha(Image *image) {
		if (image->is_half_float) {
			uint16_t hf[2];
			float f[2];
			f[0] = 0;
			f[1] = 1.0;
			singles2halfp(&hf[0], &f[0], 2);
			for (int y = 0; y < image->height; y++)
				for (int x = 0; x < image->width; x++) {
					uint64_t pixel = *(uint64_t *)&image->pixels[(y * image->extended_width  + x) * 2];
					uint16_t a = pixel64_get_a16(pixel);
					if (a != hf[0] && a != hf[1])
						return;
				}
			image->alpha_bits = 1;
			return;
		}
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				unsigned int pixel = image->pixels[y * image->extended_width  + x];
				int a = pixel_get_a(pixel);
				if (a != 0 && a != 0xFF)
					return;
			}
		image->alpha_bits = 1;
	}

	// Flip an image vertically.

	void flip_image_vertical(Image *image) {
		int bpp = 4;
		if (image->is_half_float)
			bpp = 8;
		unsigned int *pixels = (unsigned int *)malloc(image->width * image->height * bpp);
		for (int y = 0; y < image->height; y++)
			memcpy(&pixels[y * image->width * (bpp / 4)], &image->pixels[
				(image->height - 1 - y) * image->extended_width * (bpp / 4)], image->width * bpp);
		for (int y = 0; y < image->height; y++)
			memcpy(&image->pixels[y * image->extended_width * (bpp /4)], &pixels[y * image->width * (bpp / 4)],
				image->width * bpp);
		free(pixels);
		pad_image_borders(image);
	}

	// RGB to and from sRGB conversion.

	static int component_srgb_to_rgb(int c) {
		double cs = (double)c / 255.0;
		double cl;
		if (cs <= 0.04045)
			cl = cs / 12.92;
		else
			cl = pow((cs + 0.055) / 1.055, 2.4);
		return floor(255.0 * cl + 0.5);
	}

	bool convert_image_from_srgb_to_rgb(Image *source_image, Image *dest_image) {
		dest_image->width = source_image->width;
		dest_image->height = source_image->height;
		dest_image->extended_width = source_image->extended_width;
		dest_image->extended_height = source_image->extended_height;
		dest_image->alpha_bits = source_image->alpha_bits;
		dest_image->nu_components = source_image->nu_components;
		dest_image->bits_per_component = source_image->bits_per_component;
		dest_image->pixels = (unsigned int *)malloc(dest_image->extended_width * dest_image->extended_height * 4);
		dest_image->srgb = 0;
		dest_image->is_half_float = 0;
		dest_image->is_signed = 0;
		if (source_image->is_half_float) {
			return false;
		}
		for (int y = 0; y < source_image->height; y++)
			for (int x = 0; x < source_image->width; x++) {
				unsigned int pixel = source_image->pixels[y * source_image->extended_width  + x];
				int r = pixel_get_r(pixel);
				int g = pixel_get_g(pixel);
				int b = pixel_get_b(pixel);
				int a = pixel_get_a(pixel);
				r = component_srgb_to_rgb(r);
				g = component_srgb_to_rgb(g);
				b = component_srgb_to_rgb(b);
				dest_image->pixels[y * dest_image->extended_width + x] = pack_rgba(r, g, b, a);
			}
		return true;
	}

	static int component_rgb_to_srgb(int c) {
		double cl  = (double)c / 255.0;
		double cs;
		if (cl < 0.0031308) {
			cs = 12.92 * cl;
		} else {
			cs = 1.055 * pow(cl, 0.41666) - 0.055;
		}
		return floor(255.0 * cs + 0.5);
	}

	bool convert_image_from_rgb_to_srgb(Image *source_image, Image *dest_image) {
		dest_image->width = source_image->width;
		dest_image->height = source_image->height;
		dest_image->extended_width = source_image->extended_width;
		dest_image->extended_height = source_image->extended_height;
		dest_image->alpha_bits = source_image->alpha_bits;
		dest_image->nu_components = source_image->nu_components;
		dest_image->bits_per_component = source_image->bits_per_component;
		dest_image->pixels = (unsigned int *)malloc(dest_image->extended_width * dest_image->extended_height * 4);
		dest_image->srgb = 1;
		dest_image->is_half_float = 0;
		dest_image->is_signed = 0;
		if (source_image->is_half_float) {
			return false;
		}
		for (int y = 0; y < source_image->height; y++)
			for (int x = 0; x < source_image->width; x++) {
				unsigned int pixel = source_image->pixels[y * source_image->extended_width  + x];
				int r = pixel_get_r(pixel);
				int g = pixel_get_g(pixel);
				int b = pixel_get_b(pixel);
				int a = pixel_get_a(pixel);
				r = component_rgb_to_srgb(r);
				g = component_rgb_to_srgb(g);
				b = component_rgb_to_srgb(b);
				dest_image->pixels[y * dest_image->extended_width + x] = pack_rgba(r, g, b, a);
			}
		return true;
	}

	bool copy_image_to_uncompressed_texture(const Options *opt, Image *image, int texture_type, Texture *texture) {
		texture->info = match_texture_type(texture_type);
		texture->width = image->width;
		texture->height = image->height;
		texture->extended_width = image->width;
		texture->extended_height = image->height;
		texture->type = texture_type;
		texture->block_width = 1;
		texture->block_height = 1;
		texture->bits_per_block = texture->info->bits_per_block;
		if (!set_texture_decoding_function(opt, texture, NULL))
			return false;
		if (image->bits_per_component == 8 && !(image->is_signed) && (texture_type & TEXTURE_TYPE_HALF_FLOAT_BIT)) {
			// Convert image from 8-bit unsigned format to normalized half-float texture.
			Image cloned_image;
			clone_image(image, &cloned_image);
			convert_image_to_half_float(&cloned_image);
			bool ret = copy_image_to_uncompressed_texture(opt, &cloned_image, texture_type, texture);
			destroy_image(&cloned_image);
			return ret;
		}
		if (image->is_half_float && (texture_type & TEXTURE_TYPE_HALF_FLOAT_BIT)) {
			texture->pixels = (unsigned int *)malloc(image->height * image->width * 8);
			for (int y = 0; y < image->height; y++)
				memcpy(&texture->pixels[y * texture->width * 2], &image->pixels[y * image->extended_width * 2],
					image->width * 8);
			return true;
		}
		if (image->is_half_float && !(texture_type & TEXTURE_TYPE_HALF_FLOAT_BIT) && !(texture_type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT)) {
			// Convert image from normalized half-float format to 8 bits per component, signed or unsigned
			// texture format.
			Image cloned_image;
			clone_image(image, &cloned_image);
			bool ret = convert_image_from_half_float(&cloned_image, 0, 1.0, 1.0);
			if (ret) {
				// Convert to signed if necessary.
				if (texture_type & TEXTURE_TYPE_SIGNED_BIT)
					convert_image_to_8_bit_format(&cloned_image, cloned_image.nu_components, 1);
				ret = copy_image_to_uncompressed_texture(opt, &cloned_image, texture_type, texture);
			}
			destroy_image(&cloned_image);
			return ret;
		}
		if (image->is_half_float) {
			return false;
		}
		if (image->bits_per_component == 16) {
			if (!(texture_type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT)) {
				// Convert image from signed or unsigned 16-bit format to texture with signed or
				// unsigned 8-bit format.
				Image cloned_image;
				clone_image(image, &cloned_image);
				if (!(texture_type & TEXTURE_TYPE_SIGNED_BIT))
					// Convert to unsigned 8-bit format.
					convert_image_to_8_bit_format(&cloned_image, cloned_image.nu_components, 0);
				else
					// Convert to signed 8-bit format.
					convert_image_to_8_bit_format(&cloned_image, cloned_image.nu_components, 1);
				bool ret = copy_image_to_uncompressed_texture(opt, &cloned_image, texture_type, texture);
				destroy_image(&cloned_image);
				return ret;
			}
			else
			if ((texture_type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT) && !(texture_type & TEXTURE_TYPE_HALF_FLOAT_BIT))
			{
				// Convert image with signed or unsigned 16-bit format to texture with signed or
				// unsigned 16-bit format.
				if ((image->is_signed && (texture_type & TEXTURE_TYPE_SIGNED_BIT)) ||
				(!image->is_signed && !(texture_type & TEXTURE_TYPE_SIGNED_BIT)))
					// No conversion necessary.
					goto copy;
				Image cloned_image;
				clone_image(image, &cloned_image);
				if (!(texture_type & TEXTURE_TYPE_SIGNED_BIT))
					// Convert to unsigned 8-bit format.
					convert_image_to_16_bit_format(&cloned_image, cloned_image.nu_components, 0);
				else
					// Convert to signed 8-bit format.
					convert_image_to_16_bit_format(&cloned_image, cloned_image.nu_components, 1);
				bool ret = copy_image_to_uncompressed_texture(opt, &cloned_image, texture_type, texture);
				destroy_image(&cloned_image);
				return ret;
			}
			else
			if (texture_type & TEXTURE_TYPE_HALF_FLOAT_BIT) {
				if (image->is_signed) {
					return false;
				}
				Image cloned_image;
				clone_image(image, &cloned_image);
				convert_image_to_half_float(&cloned_image);
				bool ret = copy_image_to_uncompressed_texture(opt, &cloned_image, texture_type, texture);
				destroy_image(&cloned_image);
				return ret;
			}
		}
		if (image->bits_per_component == 8) {
			if (!(texture_type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT) && (texture_type & TEXTURE_TYPE_SIGNED_BIT)) {
				// Convert image from signed or unsigned 8-bit format to texture with signed 8-bit format.
				if (image->is_signed)
					// No conversion necessary.
					goto copy;
				Image cloned_image;
				clone_image(image, &cloned_image);
				// Convert to signed 8-bit format.
				convert_image_to_8_bit_format(&cloned_image, cloned_image.nu_components, 1);
				bool ret = copy_image_to_uncompressed_texture(opt, &cloned_image, texture_type, texture);
				destroy_image(&cloned_image);
				return ret;
			}
			else
			if ((texture_type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT) && !(texture_type & TEXTURE_TYPE_HALF_FLOAT_BIT))
			{
				// Convert image with signed or unsigned 8-bit format to texture with signed or unsigned
				// 16-bit format.
				Image cloned_image;
				clone_image(image, &cloned_image);
				if (!(texture_type & TEXTURE_TYPE_SIGNED_BIT))
					// Convert to unsigned 8-bit format.
					convert_image_to_16_bit_format(&cloned_image, cloned_image.nu_components, 0);
				else
					// Convert to signed 8-bit format.
					convert_image_to_16_bit_format(&cloned_image, cloned_image.nu_components, 1);
				bool ret = copy_image_to_uncompressed_texture(opt, &cloned_image, texture_type, texture);
				destroy_image(&cloned_image);
				return ret;
			}
		}
	copy :
		texture->pixels = (unsigned int *)malloc(image->height * image->width * 4);
		if (texture_type == TEXTURE_TYPE_UNCOMPRESSED_ARGB8) {
			for (int y = 0; y < image->height; y++)
				for (int x = 0; x < image->width; x++) {
					uint32_t pixel = image->pixels[y * image->extended_width + x];
					texture->pixels[y * texture->extended_width + x] = pack_rgba(pixel_get_a(pixel),
						pixel_get_r(pixel), pixel_get_g(pixel), pixel_get_b(pixel));
				}
			return true;
		}
		if (image->bits_per_component == 8 && image->nu_components >= 3 && (texture->info->red_mask != 0xFF ||
		texture->info->green_mask != 0xFF00 || texture->info->blue_mask != 0xFF0000)) {
			return false;
		}
		if (texture->info->internal_bits_per_block != 32) {
			return false;
		}
		for (int y = 0; y < image->height; y++)
			memcpy(&texture->pixels[y * texture->width], &image->pixels[y * image->extended_width],
				image->width * 4);
		return true;
	}

	void convert_image_to_half_float(Image *image) {
		Image new_image;
		new_image = *image;	// Copy fields.
		new_image.is_half_float = 1;
		new_image.bits_per_component = 16;
		new_image.pixels = (unsigned int *)malloc(image->extended_width * image->extended_height * 8);
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				float f[4];
				if (image->bits_per_component == 16) {
					f[0] = ((double)pixel_get_r16(image->pixels[y * image->extended_width + x])) / 65535.0;
					f[1] = f[2] = f[3] = 0;
					if (image->nu_components >= 2)
						f[1] = ((double)pixel_get_g16(image->pixels[y * image->extended_width + x]))
							/ 65535.0;
				}
				else {
					f[0] = ((double)pixel_get_r(image->pixels[y * image->extended_width + x])) / 255.0;
					f[1] = f[2] = f[3] = 0;
					if (image->nu_components >= 2) {
						f[1] = ((double)pixel_get_g(image->pixels[y * image->extended_width + x]))
							/ 255.0;
						if (image->nu_components >= 3) {
							f[2] = ((double)pixel_get_b(image->pixels[y * image->extended_width + x]))
								/ 255.0;
							if (image->nu_components == 4)
								f[3] = ((double)pixel_get_a(image->pixels[
									y * image->extended_width + x])) / 255.0;
							else
								f[3] = 1.0;
						}
					}
				}
				singles2halfp(&new_image.pixels[(y * new_image.extended_width + x) * 2], &f[0], 4);
			}
		free(image->pixels);
		*image = new_image;
	}

	static float clamp_0to1(float x) {
		if (x < 0)
			return 0;
		if (x > 1.0)
			return 1.0;
		return x;
	}

	void calculate_image_dynamic_range(Image *image, float *range_min_out, float *range_max_out) {
		calculate_half_float_table();
		const float *half_float_table = get_half_float_table();
		float range_min = FLT_MAX;
		float range_max = FLT_MIN;
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				uint16_t *pix = (uint16_t *)&image->pixels[(y * image->extended_width + x) * 2];
				float f[4];
				f[0] = half_float_table[pix[0]];
				f[1] = half_float_table[pix[1]];
				f[2] = half_float_table[pix[2]];
				f[3] = half_float_table[pix[3]];
				for (int i = 0; i < 3; i++) {
					if (f[i] < range_min)
						range_min = f[i];
					if (f[i] > range_max)
						range_max = f[i];
				}
				if (image->alpha_bits > 0) {
					if (f[3] < range_min)
						range_min = f[3];
					if (f[3] > range_max)
						range_max = f[3];
				}
			}
		*range_min_out = range_min;
		*range_max_out = range_max;
	}

	bool convert_image_from_half_float(Image *image, float _range_min, float _range_max, float gamma) {
		Image new_image;
		new_image = *image;	// Copy fields.
		new_image.is_half_float = 0;
		new_image.bits_per_component = 8;
		new_image.pixels = (unsigned int *)malloc(image->extended_width * image->extended_height * 4);
		if (image->alpha_bits == 8)
			new_image.alpha_bits = 16;
		float range_min, range_max;
		calculate_half_float_table();
		if (gamma >= 0.99 && gamma <= 1.01) {
			range_min = _range_min;
			range_max = _range_max;
		}
		else
		if (gamma >= 2.19 && gamma <= 2.21) {
			calculate_gamma_corrected_half_float_table();
			if (_range_min < 0)
				range_min = - powf(- _range_min, 1 / 2.2);
			else
				range_min = powf(_range_min, 1 / 2.2);
			if (_range_max < 0)
				range_max = - powf(- _range_max, 1 / 2.2);
			else
				range_max = powf(_range_max, 1 / 2.2);
		}
		else {
			return false;
		}
		const float *half_float_table = get_half_float_table();
		const float *gamma_corrected_half_float_table = get_gamma_corrected_half_float_table();
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				uint16_t *pix = (uint16_t *)&image->pixels[(y * image->extended_width + x) * 2];
				float f[4];
				if (gamma == 1.0) {
					f[0] = half_float_table[pix[0]];
					f[1] = half_float_table[pix[1]];
					f[2] = half_float_table[pix[2]];
					f[3] = half_float_table[pix[3]];
					if (image->nu_components < 2) {
						f[1] = 0;
						if (image->nu_components < 3)
							f[2] = 0;
					}
				}
				else {
					f[0] = gamma_corrected_half_float_table[pix[0]];
					f[1] = gamma_corrected_half_float_table[pix[1]];
					f[2] = gamma_corrected_half_float_table[pix[2]];
					f[3] = half_float_table[pix[3]];
				}
				int r = floor(clamp_0to1((f[0] - range_min) / (range_max - range_min)) * 255 + 0.5);
				int g = floor(clamp_0to1((f[1] - range_min) / (range_max - range_min)) * 255 + 0.5);
				int b = floor(clamp_0to1((f[2] - range_min) / (range_max - range_min)) * 255 + 0.5);
				if (image->alpha_bits > 0) {
					// Alpha is assumed to be linear value in the range 0 to 1.0.
					int a = floor(clamp_0to1(f[3]) * 255 + 0.5);
					new_image.pixels[y * new_image.extended_width + x] = pack_rgba(r, g, b, a);
				}
				else
					new_image.pixels[y * new_image.extended_width + x] = pack_rgb_alpha_0xff(r, g, b);
			}
		free(image->pixels);
		*image = new_image;
		return true;
	}

	void extend_half_float_image_to_rgb(Image *image) {
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				uint64_t pixel64 = *(uint64_t *)&image->pixels[(y * image->extended_width + x) * 2];
				uint16_t h[1];
				float f[1];
				f[0] = 0;
				singles2halfp(&h[0], &f[0], 1);
				int r = pixel64_get_r16(pixel64);
				int g = pixel64_get_g16(pixel64);
				int b = pixel64_get_b16(pixel64);
				if (image->nu_components < 2)
					g = h[0];
				if (image->nu_components < 3)
					b = h[0];
				*(uint64_t *)&image->pixels[(y * image->extended_width + x) * 2] = pack_rgb16(r, g, b);
			}
		image->nu_components = 3;
	}

	void print_image_info(Image *image, FILE* file) {
	//	fprintf(file, "Image pixels: 0x%016lX\n", image->pixels);
		fprintf(file, "Image size: %d x %d\n", image->width, image->height);
		fprintf(file, "Image extended size: %d x %d\n", image->extended_width, image->extended_height);
		fprintf(file, "Image alpha bits: %d\n", image->alpha_bits);
		fprintf(file, "Image number of components: %d\n", image->nu_components);
		fprintf(file, "Image bits per component: %d\n", image->bits_per_component);
		fprintf(file, "Components are signed: %d\n", image->is_signed);
		fprintf(file, "Image is srgb: %d\n", image->srgb);
		fprintf(file, "Image is half-float: %d\n", image->is_half_float);
	}

	static void fill_alpha_with_one(Image *image) {
		int n = image->extended_height * image->extended_width;
		if (image->is_half_float) {
			uint16_t hf[1];
			float f[1];
			f[0] = 1.0;
			singles2halfp(&hf[0], &f[0], 1);
			for (int i = 0; i < n; i++) {
				uint64_t pixel = *(uint64_t *)&image->pixels[i * 2];
				*(uint64_t *)&image->pixels[i * 2] = pack_rgba16(pixel64_get_r16(pixel),
					pixel64_get_g16(pixel), pixel64_get_b16(pixel), hf[0]);
			}
		}
		else {
			for (int i = 0; i < n; i++) {
				uint32_t pixel = image->pixels[i];
				image->pixels[i] = pixel | pack_a(0xFF);
			}
		}
	}

	void remove_alpha_from_image(Image *image) {
		image->alpha_bits = 0;
		image->nu_components--;
		fill_alpha_with_one(image);
	}

	void add_alpha_to_image(Image *image) {
		if (image->is_half_float)
			image->alpha_bits = 16;
		else
			image->alpha_bits = 1;
		image->nu_components++;
		fill_alpha_with_one(image);
	}

	void convert_image_from_16_bit_format(Image *image) {
		Image new_image;
		new_image = *image;	// Copy fields.
		new_image.bits_per_component = 8;
		new_image.nu_components = 3;
		new_image.is_signed = 0;
		new_image.pixels = (unsigned int *)malloc(image->extended_width * image->extended_height * 4);
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				uint32_t pixel = image->pixels[y * image->extended_width + x];
				int r, g, b;
				if (image->is_signed) {
					// Map from [-32768, 32767] to [0, 255]
					r = (pixel_get_signed_r16(pixel) + 32768 + 127) * 255 / 65535;
				}
				else
					// Map from [0, 65535] to [0, 255]
					r = (pixel_get_r16(pixel) + 127) * 255 / 65535;
				if (image->nu_components > 1) {
					if (image->is_signed)
						g = (pixel_get_signed_g16(pixel) + 32768 + 127) * 255 / 65535;
					else
						g = (pixel_get_g16(pixel) + 127) * 255 / 65535;
					b = 0;
				}
				else {
					// If there's only one component, display it as grayscale.
					g = r;
					b = r;
				}
				new_image.pixels[y * new_image.extended_width + x] = pack_rgb_alpha_0xff(r, g, b);
			}
		free(image->pixels);
		*image = new_image;
	}

	void convert_image_to_16_bit_format(Image *image, int nu_components, int signed_format) {
		Image new_image;
		new_image = *image;	// Copy fields.
		new_image.bits_per_component = 16;
		new_image.nu_components = nu_components;
		new_image.alpha_bits = 0;
		if (signed_format)
			new_image.is_signed = 1;
		else
			new_image.is_signed = 0;
		new_image.pixels = (unsigned int *)malloc(image->extended_width * image->extended_height * 4);
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				uint32_t pixel = image->pixels[y * image->extended_width + x];
				uint32_t new_pixel;
				if (image->bits_per_component == 8) {
					if (signed_format) {
						if (!image->is_signed) {
							// Map from [0, 255] to [-32768, 32767].
							new_pixel = pack_r16((uint16_t)(int16_t)((pixel_get_r(pixel) * 65535
								/ 255) - 32768));
							if (nu_components >= 2)
								new_pixel |= pack_g16((uint16_t)(int16_t)((pixel_get_g(pixel)
									* 65535 / 255) - 32768));
						}
						else {
							// Map from [-128, 127] to [-32768, 32767].
							new_pixel = pack_r16((uint16_t)(int16_t)(
								((pixel_get_signed_r8(pixel) + 128) * 65535 / 255) - 32768));
							if (nu_components >= 2)
								new_pixel |= pack_g16((uint16_t)(int16_t)(
									((pixel_get_signed_g8(pixel) + 128) * 65535 / 255)
									- 32768));
						}
					}
					else {
						if (!image->is_signed) {
							// Map from [0, 255] to [0, 65535].
							new_pixel = pack_r16(pixel_get_r(pixel) * 65535 / 255);
							if (nu_components >= 2)
								new_pixel |= pack_g16(pixel_get_g(pixel) * 65535 / 255);
						}
						else {
							// Map from [-128, 127] to [0, 65535].
							new_pixel = pack_r16((uint16_t)(int16_t)(
								(pixel_get_signed_r8(pixel) + 128) * 65535 / 255));
							if (nu_components >= 2)
								new_pixel |= pack_g16((uint16_t)(int16_t)(
									(pixel_get_signed_g8(pixel) + 128) * 65535 / 255));

						}
					}
				}
				else {
					// If the format is already 16-bit and the right signedness, copy the pixel.
					if ((image->is_signed && signed_format) || (!image->is_signed && !signed_format))
						new_pixel = pixel;
					else
					// 16-bit signed to unsigned and vice versa.
					if (signed_format) {
						// 16-bit unsigned to signed.
						new_pixel = pack_r16((uint16_t)(int16_t)(pixel_get_r16(pixel) - 32768));
						if (nu_components >= 2)
							new_pixel |= pack_g16((uint16_t)(int16_t)(pixel_get_g16(pixel) - 32768));
					}
					else {
						// 16-bit signed to unsigned.
						new_pixel = pack_r16(pixel_get_signed_r16(pixel) + 32768);
						if (nu_components >= 2)
							new_pixel |= pack_g16(pixel_get_signed_g16(pixel) + 32768);
					}
				}
				new_image.pixels[y * new_image.extended_width + x] = new_pixel;
			}
		free(image->pixels);
		*image = new_image;
	}

	void convert_image_from_8_bit_format(Image *image) {
		Image new_image;
		new_image = *image;	// Copy fields.
		new_image.nu_components = 3;
		new_image.is_signed = 0;
		new_image.pixels = (unsigned int *)malloc(image->extended_width * image->extended_height * 4);
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				uint8_t *pix = (uint8_t *)&image->pixels[y * image->extended_width + x];
				int r, g, b;
				if (image->is_signed)
					// Map [-128, 127] to [0, 255].
					r = (int)(*(int8_t *)&pix[0]) + 128;
				else
					r = pix[0];
				if (image->nu_components > 1) {
					if (image->is_signed) {
						g = (int)(*(int8_t *)&pix[1]) + 128;
					}
					else
						g = pix[1];
					b = 0;
				}
				else {
					// If there's only one component, display it as grayscale.
					g = r;
					b = r;
				}
				new_image.pixels[y * new_image.extended_width + x] = pack_rgb_alpha_0xff(r, g, b);
			}
		free(image->pixels);
		*image = new_image;
	}

	void convert_image_to_8_bit_format(Image *image, int nu_components, int signed_format) {
		Image new_image;
		new_image = *image;	// Copy fields.
		new_image.nu_components = nu_components;
		new_image.alpha_bits = 0;
		new_image.bits_per_component = 8;
		if (signed_format)
			new_image.is_signed = 1;
		else
			new_image.is_signed = 0;
		new_image.pixels = (unsigned int *)malloc(image->extended_width * image->extended_height * 4);
		for (int y = 0; y < image->height; y++)
			for (int x = 0; x < image->width; x++) {
				uint32_t pixel = image->pixels[y * image->extended_width + x];
				uint32_t new_pixel;
				if (image->bits_per_component == 16) {
					// Convert from 16-bit format to 8-bit format.
					if (!signed_format) {
						if (!image->is_signed) {
							// Map from [0, 65535] to [0, 255], rounding correctly.
							new_pixel = pack_r((pixel_get_r16(pixel) + 127) * 255 / 65535);
							if (nu_components >= 2)
								new_pixel |= pack_g((pixel_get_g16(pixel) + 127) * 255 / 65535);
						}
						else {
							// Map from [-32767, 32768] to [0, 255].
							new_pixel = pack_r((pixel_get_signed_r16(pixel) + 32768 + 127) *
								255 / 65535);
							if (nu_components >= 2)
								new_pixel |= pack_g((pixel_get_signed_g16(pixel) + 32768 +  127) *
									255 / 65535);
						}
					}
					else {
						if (image->is_signed) {
							// Map from [-32768, 32767] to [-128, 127].
							new_pixel = pack_r((uint8_t)(char)(
								(pixel_get_signed_r16(pixel) + 32768 + 127) * 255 / 65535
								- 128
								));
							if (nu_components >= 2)
								new_pixel |= pack_g((uint8_t)(char)(
									(pixel_get_signed_g16(pixel) + 32768 + 127) * 255 / 65535
									- 128
									));
						}
						else {
							// Map from [0, 65535] to [-128, 127].
							new_pixel = pack_r((uint8_t)(char)(
								(pixel_get_r16(pixel) + 127) * 255 / 65535 - 128
								));
							if (nu_components >= 2)
								new_pixel = pack_g((uint8_t)(char)(
									(pixel_get_g16(pixel) + 127) * 255 / 65535 - 128
									));
						}
					}
				}
				else
				if ((!image->is_signed && !signed_format)) {
					// Convert from 8-bit or RGB format, losing any extra components.
					new_pixel = pack_r(pixel_get_r(pixel));
					if (nu_components >= 2)
						new_pixel |= pack_g(pixel_get_g(pixel));
				}
				else
				if (image->is_signed && signed_format)
					// No conversion necessary.
					new_pixel = pixel;
				else {
					if (signed_format) {
						// Map from [0, 255] to [-128, 127].
						new_pixel = pack_r((uint8_t)(char)(pixel_get_r(pixel) - 128));
						if (nu_components >= 2)
							new_pixel |= pack_g((uint8_t)(char)(pixel_get_g(pixel) - 128));
					}
					else {
						// Map from [-128, 127] to [0, 255].
						new_pixel = pack_r(pixel_get_signed_r8(pixel) + 128);
						if (nu_components >= 2)
							new_pixel |= pack_g(pixel_get_signed_g8(pixel) + 128);
					}
				}
				new_image.pixels[y * new_image.extended_width + x] = new_pixel;
			}
		free(image->pixels);
		*image = new_image;
	}

}
