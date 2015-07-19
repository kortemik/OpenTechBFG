/*
    decode.h -- part of texgenpack, a texture compressor using fgen.

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


// Flags values for the decoding functions.

#define ETC_MODE_ALLOWED_INDIVIDUAL	1
#define ETC_MODE_ALLOWED_DIFFERENTIAL	2
#define ETC2_MODE_ALLOWED_T		4
#define ETC2_MODE_ALLOWED_H		8
#define ETC2_MODE_ALLOWED_PLANAR	16
#define ETC_MODE_ALLOWED_ALL		3
#define ETC2_MODE_ALLOWED_ALL		31
#define BPTC_FLOAT_MODE_ALLOWED_ALL	0x3FFF
#define ENCODE_BIT			0x10000

// Functions defined in etc2.c.

// Draw (decompress) a 64-bit 4x4 pixel block.
int draw_block4x4_etc1(const unsigned char *bitstring, unsigned int *image_buffer, int modes_allowed);
int draw_block4x4_etc2_rgb8(const unsigned char *bitstring, unsigned int *image_buffer, int modes_allowed);
int draw_block4x4_etc2_eac(const unsigned char *bitstring, unsigned int *image_buffer, int modes_allowed);
int draw_block4x4_etc2_punchthrough(const unsigned char *bitstring, unsigned int *image_buffer, int modes_allowed);
int draw_block4x4_r11_eac(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_rg11_eac(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_signed_r11_eac(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_signed_rg11_eac(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
// Return ETC2 mode number from 0 to 4.
int block4x4_etc2_rgb8_get_mode(const unsigned char *bitstring);
// "Manual" optimization function.
void optimize_block_etc2_punchthrough(unsigned char *bitstring, unsigned char *alpha_values);

// Functions defined in dxtc.c.

int draw_block4x4_dxt1(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_dxt1a(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_dxt3(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_dxt5(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
void optimize_block_dxt3(unsigned char *bitstring, unsigned char *alpha_values);

// Functions defined in astc.c

int draw_block_rgba_astc(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
void convert_astc_texture_to_image(Texture *texture, Image *image);
void decompress_astc_file(const char *filename, Image *image);
void compress_image_to_astc_texture(Image *image, int texture_type, Texture *exture);
int match_astc_block_size(int w, int h);
int get_astc_block_size_width(int astc_block_type);
int get_astc_block_size_height(int astc_block_type);

// Functions defined in bptc.c

int draw_block4x4_bptc(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_bptc_float(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_bptc_signed_float(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int block4x4_bptc_float_get_mode(const unsigned char *bitstring);

// Functions defined in rgtc.c

int draw_block4x4_rgtc1(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_signed_rgtc1(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_rgtc2(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_signed_rgtc2(const unsigned char *bitstring, unsigned int *image_buffer, int flags);

// Function defined in texture.c.

int draw_block4x4_uncompressed(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_argb8(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_uncompressed_rgb_half_float(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_uncompressed_rgba_half_float(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_uncompressed_r_half_float(const unsigned char *bitstring, unsigned int *image_buffer, int flags);
int draw_block4x4_uncompressed_rg_half_float(const unsigned char *bitstring, unsigned int *image_buffer, int flags);

