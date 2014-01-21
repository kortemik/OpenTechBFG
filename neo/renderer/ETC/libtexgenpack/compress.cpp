/*
    compress.c -- part of texgenpack, a texture compressor using fgen.

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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "../fgen/fgen.h"
#include "libtexgenpack.h"
#include "libtexgenpack_internal.h"
#include "decode.h"
#include "packing.h"

namespace texgen {

	typedef struct {
		int nu_generations;
		int population_size;
		int nu_islands;
		float mutation_probability;
		float crossover_probability;
		CompressCallbackFunction compress_callback_func;
		double rmse_threshold;
	} CompressOptions;

	typedef struct {
		BlockUserData *block;
		CompressOptions *compOpt;
	} CompressFGenUserData;

	static void compress_with_single_population(Image *image, Texture *Texture, const Options *opt, CompressOptions *compOpt);
	static void compress_with_archipelago(Image *image, Texture *texture, const Options *opt, CompressOptions *compOpt);
	static void compress_multiple_blocks_concurrently(Image *image, Texture *texture, const Options *opt, CompressOptions *compOpt);
	static void set_alpha_pixels(Image *image, int x, int y, int w, int h, unsigned char *alpha_pixels);
	static void optimize_alpha(Image *image, Texture *texture);
	static double get_rmse_threshold(Texture *texture, int speed, Image *image, const Options *opt);

	// Compress an image into a texture.

	bool compress_image(const Options *opt, Image *image, int texture_type, CompressCallbackFunction callback_func, Texture *texture,
		int genetic_parameters, float mutation_prob, float crossover_prob) {
		texture->info = match_texture_type(texture_type);
		if (texture_type & TEXTURE_TYPE_UNCOMPRESSED_BIT) {
			return copy_image_to_uncompressed_texture(opt, image, texture_type, texture);
		}
		if (texture_type & TEXTURE_TYPE_ASTC_BIT) {
			return compress_image_to_astc_texture(opt, image, texture_type, texture);
		}
		if ((texture_type & TEXTURE_TYPE_HALF_FLOAT_BIT) && !image->is_half_float) {
			return false;
		}
		texture->width = image->width;
		texture->height = image->height;
		texture->bits_per_block = texture->info->internal_bits_per_block;
		texture->type = texture_type;
		texture->block_width = texture->info->block_width;
		texture->block_height = texture->info->block_height;
		texture->extended_width = ((texture->width + texture->block_width - 1) / texture->block_width) *
			texture->block_width;
		texture->extended_height = ((texture->height + texture->block_height - 1) / texture->block_height)
			* texture->block_height;
		texture->pixels = (unsigned int *)malloc((texture->extended_height / texture->block_height) *
			(texture->extended_width / texture->block_width) * (texture->bits_per_block / 8));
		if (!set_texture_decoding_function(opt, texture, image)) {
			free(texture->pixels);
			texture->pixels = NULL;
			return false;
		}
		if ((texture_type & TEXTURE_TYPE_HALF_FLOAT_BIT) || image->is_half_float)
			calculate_half_float_table();
		if ((texture_type == TEXTURE_TYPE_BPTC_FLOAT || texture_type == TEXTURE_TYPE_BPTC_SIGNED_FLOAT) && opt->hdr)
			calculate_gamma_corrected_half_float_table();
		if (image->is_half_float)
			calculate_normalized_float_table();
		CompressOptions compOpt;
		memset(&compOpt, 0, sizeof(CompressOptions));
		compOpt.rmse_threshold = get_rmse_threshold(texture, opt->speed, image, opt);

		compOpt.compress_callback_func = callback_func;
		if (genetic_parameters) {
			compOpt.mutation_probability = mutation_prob;
			compOpt.crossover_probability = crossover_prob;
		}
		else {
			// Emperically determined mutation probability.
			if (texture_type & TEXTURE_TYPE_128BIT_BIT)
			if (texture_type == TEXTURE_TYPE_BPTC_FLOAT)
				// bptc_float format needs higher mutation probability.
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.016;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.014;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.018;
			else	// SPEED_SLOW
				compOpt.mutation_probability = 0.018;
			else
			if (texture_type == TEXTURE_TYPE_BPTC)
				// bptc
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.010;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.011;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.012;
			else	// SPEED_SLOW
				compOpt.mutation_probability = 0.012;
			else
			if (texture_type == TEXTURE_TYPE_DXT5)
				// dxt5
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.020;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.017;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.018;
			else	// SPEED_SLOW
				compOpt.mutation_probability = 0.018;
			else
				// Other 128-bit block texture types.
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.015;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.013;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.015;
			else	// SPEED_SLOW
				compOpt.mutation_probability = 0.015;
			else	// 64-bit texture formats.
			if (texture_type == TEXTURE_TYPE_ETC1)
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.027;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.023;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.024;
			else	// SPEED_SLOW
				compOpt.mutation_probability = 0.025;
			else
			if (texture_type == TEXTURE_TYPE_ETC2_RGB8)
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.023;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.022;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.024;
			else	// SPEED_SLOW
				compOpt.mutation_probability = 0.025;
			else
			if (texture_type == TEXTURE_TYPE_DXT1)
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.028;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.025;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.026;
			else	// SPEED_SLOW
				compOpt.mutation_probability = 0.026;
			else
			if (texture_type == TEXTURE_TYPE_R11_EAC)
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.029;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.028;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.026;
			else
				compOpt.mutation_probability = 0.027;
			else	// Other 64-bit texture formats.
			if (opt->speed == SPEED_ULTRA)
				compOpt.mutation_probability = 0.025;
			else
			if (opt->speed == SPEED_FAST)
				compOpt.mutation_probability = 0.024;
			else
			if (opt->speed == SPEED_MEDIUM)
				compOpt.mutation_probability = 0.025;
			else
				compOpt.mutation_probability = 0.026;
			compOpt.crossover_probability = 0.7;
		}
		if (opt->speed == SPEED_FAST) {
			compOpt.population_size = 64;
			compOpt.nu_generations = 200;
			compOpt.nu_islands = 4;
			compress_with_archipelago(image, texture, opt, &compOpt);
		}
		else
		if (opt->speed == SPEED_MEDIUM) {
			compOpt.population_size = 128;
			compOpt.nu_generations = 200;
			compOpt.nu_islands = 8;
			compress_with_archipelago(image, texture, opt, &compOpt);
		}
		else
		if (opt->speed == SPEED_SLOW) {
			compOpt.population_size = 128;
			compOpt.nu_generations = 500;
			compOpt.nu_islands = 16;
			compress_with_archipelago(image, texture, opt, &compOpt);
		}
		else
		if (opt->speed == SPEED_ULTRA) {
			compOpt.population_size = 256;
			compOpt.nu_generations = 100;
			compOpt.nu_islands = 8;
			if (opt->max_threads != -1 && opt->max_threads < 8) {
				printf("Option --ultra not compatible with max threads setting (need >= 8).\n");
				exit(1);
			}
			compress_multiple_blocks_concurrently(image, texture, opt, &compOpt);
		}

		// Optionally post-process the texture to optimize the alpha values.
		//	optimize_alpha(image, texture);

		return true;
	}


	// The fitness function of the genetic algorithm.

	static double calculate_fitness(const FgenPopulation *pop, const unsigned char *bitstring) {
		unsigned int image_buffer[32];	// 16 required for regular pixels, 32 for 64-bit pixel formats like half floats.
		BlockUserData *user_data = ((CompressFGenUserData *)pop->user_data)->block;
		int flags = user_data->flags;
		int r = user_data->texture->decoding_function(bitstring, image_buffer, flags);
		if (r == 0) {
			//		printf("Invalid block.\n");
			return 0;	// Fitness is zero for invalid blocks.
		}
		return user_data->texture->comparison_function(image_buffer, user_data, NULL);
	}

	// The generation callback function of the genetic algorithm.

	static void generation_callback(FgenPopulation *pop, int generation) {
		if (generation == 0)
			return;
		if (generation >= ((CompressFGenUserData *)pop->user_data)->compOpt->nu_generations * 2) {
			fgen_signal_stop(pop);
			return;
		}
		// Adaptive, if the fitness is above the threshold, stop, otherwise go on for another nu_generations
		// generations.
		FgenIndividual *best = fgen_best_individual_of_population(pop);
		double rmse = sqrt((1.0 / best->fitness) / 16);
		if (rmse < ((CompressFGenUserData *)pop->user_data)->compOpt->rmse_threshold)
			fgen_signal_stop(pop);
	}

	// Custom seeding function for 128-bit formats for archipelagos where each island is compressing the same block.

	static void seed_128bit(FgenPopulation *pop, unsigned char *bitstring) {
		BlockUserData *user_data = ((CompressFGenUserData *)pop->user_data)->block;
		FgenRNG *rng = fgen_get_rng(pop);
		int r = fgen_random_8(rng);
		// The chance of seeding with an already calculated neighbour block should be chosen carefully.
		// A too high probability results in less diversity in the archipelago.
		int factor;
		if (((CompressFGenUserData *)pop->user_data)->compOpt->population_size == 128)
			factor = 1;
		else	// population_size == 64
			factor = 2;
		if (r < 2 * factor && user_data->x_offset > 0) {
			// Seed with solution to the left with chance 1/128th (1/64th if population size is 64).
			int compressed_block_index = (user_data->y_offset / user_data->texture->block_height) *
				(user_data->texture->extended_width / user_data->texture->block_width) +
				(user_data->x_offset / user_data->texture->block_width) - 1;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 4];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 4 + 1];
			*(unsigned int *)&bitstring[8] = user_data->texture->pixels[compressed_block_index * 4 + 2];
			*(unsigned int *)&bitstring[12] = user_data->texture->pixels[compressed_block_index * 4 + 3];
			goto end;
		}
		if (r < 4 * factor && user_data->y_offset > 0) {
			// Seed with solution above with chance 1/128th (1/64th for x == 0).
			// 1/64th if population size is 64.
			int compressed_block_index = (user_data->y_offset / user_data->texture->block_height - 1) *
				(user_data->texture->extended_width / user_data->texture->block_width) +
				user_data->x_offset / user_data->texture->block_width;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 4];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 4 + 1];
			*(unsigned int *)&bitstring[8] = user_data->texture->pixels[compressed_block_index * 4 + 2];
			*(unsigned int *)&bitstring[12] = user_data->texture->pixels[compressed_block_index * 4 + 3];
			goto end;
		}
		if (r < 6 * factor && (user_data->x_offset > 0 || user_data->y_offset > 0)) {
			// Seed with a random already calculated solution with chance 1/128th.
			// 1/64th if population size is 64.
			int x, y;
			if (user_data->y_offset == 0) {
				x = fgen_random_n(rng, user_data->x_offset / user_data->texture->block_width);
				y = 0;
			}
			else {
				int i = fgen_random_n(rng, (user_data->y_offset / user_data->texture->block_height) *
					(user_data->texture->extended_width / user_data->texture->block_width)
					+ user_data->x_offset / user_data->texture->block_width);
				x = i % (user_data->texture->extended_width / user_data->texture->block_width);
				y = i / (user_data->texture->extended_width / user_data->texture->block_width);
			}
			int compressed_block_index = y * (user_data->texture->extended_width / user_data->texture->block_width) + x;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 4];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 4 + 1];
			*(unsigned int *)&bitstring[8] = user_data->texture->pixels[compressed_block_index * 4 + 2];
			*(unsigned int *)&bitstring[12] = user_data->texture->pixels[compressed_block_index * 4 + 3];
			goto end;
		}
		fgen_seed_random(pop, bitstring);
	end:
		if (user_data->texture->type == TEXTURE_TYPE_DXT3)
			optimize_block_dxt3(bitstring, user_data->alpha_pixels);
	}

	// Seeding function for archipelagos where each island is compressing the same block.

	static void seed(FgenPopulation *pop, unsigned char *bitstring) {
		BlockUserData *user_data = ((CompressFGenUserData *)pop->user_data)->block;
		if (user_data->texture->bits_per_block == 128) {
			seed_128bit(pop, bitstring);
			return;
		}
		FgenRNG *rng = fgen_get_rng(pop);
		int r = fgen_random_8(rng);
		// The chance of seeding with an already calculated neighbour block should be chosen carefully.
		// A too high probability results in less diversity in the archipelago.
		int factor;
		if (((CompressFGenUserData *)pop->user_data)->compOpt->population_size == 128)
			factor = 1;
		else	// population_size == 64
			factor = 2;
		if (r < 2 * factor && user_data->x_offset > 0) {
			// Seed with solution to the left with chance 1/128th.
			int compressed_block_index = (user_data->y_offset / user_data->texture->block_height) *
				(user_data->texture->extended_width / user_data->texture->block_width) +
				(user_data->x_offset / user_data->texture->block_width) - 1;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 2];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 2 + 1];
			goto end;
		}
		if (r < 4 * factor && user_data->y_offset > 0) {
			// Seed with solution above with chance 1/128th (1/64th for x == 0).
			int compressed_block_index = (user_data->y_offset / user_data->texture->block_height - 1) *
				(user_data->texture->extended_width / user_data->texture->block_width) +
				user_data->x_offset / user_data->texture->block_width;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 2];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 2 + 1];
			goto end;
		}
		if (r < 6 * factor && (user_data->x_offset > 0 || user_data->y_offset > 0)) {
			// Seed with a random already calculated solution with chance 1/128th.
			int x, y;
			if (user_data->y_offset == 0) {
				x = fgen_random_n(rng, user_data->x_offset / user_data->texture->block_width);
				y = 0;
			}
			else {
				int i = fgen_random_n(rng, (user_data->y_offset / user_data->texture->block_height) *
					(user_data->texture->extended_width / user_data->texture->block_width)
					+ user_data->x_offset / user_data->texture->block_width);
				x = i % (user_data->texture->extended_width / user_data->texture->block_width);
				y = i / (user_data->texture->extended_width / user_data->texture->block_width);
			}
			int compressed_block_index = y * (user_data->texture->extended_width / user_data->texture->block_width) + x;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 2];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 2 + 1];
			goto end;
		}
		fgen_seed_random(pop, bitstring);
	end:
		if (user_data->texture->type == TEXTURE_TYPE_ETC2_PUNCHTHROUGH)
			optimize_block_etc2_punchthrough(bitstring, user_data->alpha_pixels);
	}

	// 128-bit seeding function for archipelagos where each island is compressing a different block (--ultra setting).
	// Population size is assumed to be 256.

	static void seed2_128bit(FgenPopulation *pop, unsigned char *bitstring) {
		BlockUserData *user_data = ((CompressFGenUserData *)pop->user_data)->block;
		FgenRNG *rng = fgen_get_rng(pop);
		int r = fgen_random_8(rng);
		// The chance of seeding with an already calculated neighbour block should be chosen carefully.
		// A too high probability results in less diversity in the archipelago.
		if (r < 3 && user_data->y_offset > 0) {
			// Seed with solution above with chance 3/256th.
			int compressed_block_index = (user_data->y_offset / user_data->texture->block_height - 1) *
				(user_data->texture->extended_width / user_data->texture->block_width) +
				user_data->x_offset / user_data->texture->block_width;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 4];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 4 + 1];
			*(unsigned int *)&bitstring[8] = user_data->texture->pixels[compressed_block_index * 4 + 2];
			*(unsigned int *)&bitstring[12] = user_data->texture->pixels[compressed_block_index * 4 + 3];
			goto end;
		}
		if (r < 6 && user_data->y_offset > 0) {
			// Seed with a random already calculated solution from the area above with chance 3/256th
			int x, y;
			int i = fgen_random_n(rng, (user_data->y_offset / user_data->texture->block_height) *
				(user_data->texture->extended_width / user_data->texture->block_width));
			x = i % (user_data->texture->extended_width / user_data->texture->block_width);
			y = i / (user_data->texture->extended_width / user_data->texture->block_width);
			int compressed_block_index = y * (user_data->texture->extended_width / user_data->texture->block_width) + x;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 4];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 4 + 1];
			*(unsigned int *)&bitstring[8] = user_data->texture->pixels[compressed_block_index * 4 + 2];
			*(unsigned int *)&bitstring[12] = user_data->texture->pixels[compressed_block_index * 4 + 3];
			goto end;
		}
		fgen_seed_random(pop, bitstring);
	end:;
		if (user_data->texture->type == TEXTURE_TYPE_DXT3)
			optimize_block_dxt3(bitstring, user_data->alpha_pixels);
	}

	// Seeding function for archipelagos where each island is compressing a different block (--ultra setting).

	static void seed2(FgenPopulation *pop, unsigned char *bitstring) {
		BlockUserData *user_data = ((CompressFGenUserData *)pop->user_data)->block;
		if (user_data->texture->bits_per_block == 128) {
			seed2_128bit(pop, bitstring);
			return;
		}
		FgenRNG *rng = fgen_get_rng(pop);
		int r = fgen_random_8(rng);
		// The chance of seeding with an already calculated neighbour block should be chosen carefully.
		// A too high probability results in less diversity in the archipelago.
		if (r < 3 && user_data->y_offset > 0) {
			// Seed with solution above with chance 3/256th
			int compressed_block_index = (user_data->y_offset / 4 - 1) * (user_data->texture->extended_width /
				user_data->texture->block_width) + user_data->x_offset / user_data->texture->block_width;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 2];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 2 + 1];
			goto end;
		}
		if (r < 6 && user_data->y_offset > 0) {
			// Seed with a random already calculated solution from the area above with chance 3/256th
			int x, y;
			int i = fgen_random_n(rng, (user_data->y_offset / user_data->texture->block_height) *
				(user_data->texture->extended_width / user_data->texture->block_width));
			x = i % (user_data->texture->extended_width / user_data->texture->block_width);
			y = i / (user_data->texture->extended_width / user_data->texture->block_width);
			int compressed_block_index = y * (user_data->texture->extended_width / user_data->texture->block_width) + x;
			*(unsigned int *)&bitstring[0] = user_data->texture->pixels[compressed_block_index * 2];
			*(unsigned int *)&bitstring[4] = user_data->texture->pixels[compressed_block_index * 2 + 1];
			goto end;
		}
		fgen_seed_random(pop, bitstring);
	end:;
		if (user_data->texture->type == TEXTURE_TYPE_ETC2_PUNCHTHROUGH)
			optimize_block_etc2_punchthrough(bitstring, user_data->alpha_pixels);
	}

	// Set the auxilliary data field for the GA population.

	static void set_user_data(BlockUserData *user_data, Image *image, Texture *texture) {
		user_data->flags = ENCODE_BIT;
		if (texture->type == TEXTURE_TYPE_ETC1)
			user_data->flags |= ETC_MODE_ALLOWED_ALL;
		else
		if (texture->type == TEXTURE_TYPE_ETC2_RGB8 || texture->type == TEXTURE_TYPE_ETC2_EAC)
			user_data->flags |= ETC2_MODE_ALLOWED_ALL;
		else
		if (texture->type == TEXTURE_TYPE_BPTC_FLOAT || texture->type == TEXTURE_TYPE_BPTC_SIGNED_FLOAT)
			user_data->flags |= BPTC_FLOAT_MODE_ALLOWED_ALL;
		user_data->image_pixels = image->pixels;
		if (image->is_half_float)
			user_data->image_rowstride = image->extended_width * 8;
		else
			user_data->image_rowstride = image->extended_width * 4;
		user_data->texture = texture;
		user_data->stop_signalled = 0;
	}

	static const char *etc2_modestr = "IDTHP";

	// Report the given GA solution in store its bitstring in the texture, printing information if required.

	static void report_solution(FgenIndividual *best, CompressFGenUserData *user_data) {
		Texture *texture = user_data->block->texture;
		int x_offset = user_data->block->x_offset;
		int y_offset = user_data->block->y_offset;
		// Calculate the block index of the block.
		int compressed_block_index = (y_offset / texture->block_height) * (texture->extended_width / texture->block_width)
			+ x_offset / texture->block_width;
		if (texture->bits_per_block == 64) {
			// Copy the 64-bit block.
			texture->pixels[compressed_block_index * 2] = *(unsigned int *)best->bitstring;
			texture->pixels[compressed_block_index * 2 + 1] = *(unsigned int *)&best->bitstring[4];
		}
		else {
			// Copy the 128-bit block.
			texture->pixels[compressed_block_index * 4] = *(unsigned int *)best->bitstring;
			texture->pixels[compressed_block_index * 4 + 1] = *(unsigned int *)&best->bitstring[4];
			texture->pixels[compressed_block_index * 4 + 2] = *(unsigned int *)&best->bitstring[8];
			texture->pixels[compressed_block_index * 4 + 3] = *(unsigned int *)&best->bitstring[12];
		}
		user_data->compOpt->compress_callback_func(user_data->block);
	}

	// Compress each block with a single GA population. Unused.

	static void compress_with_single_population(Image *image, Texture *texture, const Options *opt, CompressOptions *compOpt) {
		FgenPopulation *pop = fgen_create(
			compOpt->population_size,		// Population size.
			texture->bits_per_block,	// Number of bits.
			1,				// Data element size.
			generation_callback,
			calculate_fitness,
			fgen_seed_random,
			fgen_mutation_per_bit_fast,
			fgen_crossover_uniform_per_bit
			);
		fgen_set_parameters(
			pop,
			FGEN_ELITIST_SUS,
			FGEN_SUBTRACT_MIN_FITNESS,
			compOpt->crossover_probability,	// Crossover prob.
			compOpt->mutation_probability,	// Mutation prob. per bit
			0		// Macro-mutation prob.
			);
		fgen_set_generation_callback_interval(pop, compOpt->nu_generations);
		if (!opt->deterministic)
			fgen_random_seed_with_timer(fgen_get_rng(pop));
		//	fgen_random_seed_rng(fgen_get_rng(pop), 0);
		pop->user_data = (CompressFGenUserData *)malloc(sizeof(CompressFGenUserData));
		((CompressFGenUserData *)pop->user_data)->compOpt = compOpt;
		((CompressFGenUserData *)pop->user_data)->block = (BlockUserData *)malloc(sizeof(BlockUserData));
		set_user_data(((CompressFGenUserData *)pop->user_data)->block, image, texture);
		for (int y = 0; y < image->extended_height; y += texture->block_height)
		for (int x = 0; x < image->extended_width; x += texture->block_width) {
			CompressFGenUserData *comp_user_data = (CompressFGenUserData *)pop->user_data;
			BlockUserData *user_data = comp_user_data->block;
			user_data->x_offset = x;
			user_data->y_offset = y;
			// Threading a single population doesn't work well in this case because the of the relatively quick
			// fitness function, which causes too much overhead caused by threading.
			//			if (use_threading)
			//				fgen_run_threaded(pop, nu_generations);
			//			else
			fgen_run(pop, compOpt->nu_generations);
			report_solution(fgen_best_individual_of_population(pop), comp_user_data);
			if (user_data->stop_signalled)
				goto end;
		}
	end:
		free(((CompressFGenUserData *)pop->user_data)->block);
		free(pop->user_data);
		fgen_destroy(pop);
	}

	// Compress each block with an archipelago of algorithms running on the same block. The best one is chosen.

	static void compress_with_archipelago(Image *image, Texture *texture, const Options *opt, CompressOptions *compOpt) {
		unsigned char *alpha_pixels = (unsigned char *)alloca(texture->block_width * texture->block_height);
		if (opt->generations != -1)
			compOpt->nu_generations = opt->generations;
		if (opt->islands != -1)
			compOpt->nu_islands = opt->islands;
		FgenPopulation **pops = (FgenPopulation **)alloca(sizeof(FgenPopulation *)* compOpt->nu_islands);
		for (int i = 0; i < compOpt->nu_islands; i++) {
			pops[i] = fgen_create(
				compOpt->population_size,		// Population size.
				texture->bits_per_block,	// Number of bits.
				1,				// Data element size.
				generation_callback,
				calculate_fitness,
				seed,
				fgen_mutation_per_bit_fast,
				fgen_crossover_uniform_per_bit
				);
			fgen_set_parameters(
				pops[i],
				FGEN_ELITIST_SUS,
				FGEN_SUBTRACT_MIN_FITNESS,
				compOpt->crossover_probability,	// Crossover prob.
				compOpt->mutation_probability,	// Mutation prob. per bit
				0		// Macro-mutation prob.
				);
			fgen_set_generation_callback_interval(pops[i], compOpt->nu_generations);
			fgen_set_migration_interval(pops[i], 0);	// No migration.
			fgen_set_migration_probability(pops[i], 0.01);
			pops[i]->user_data = (CompressFGenUserData *)malloc(sizeof(CompressFGenUserData));
			((CompressFGenUserData *)pops[i]->user_data)->compOpt = compOpt;
			((CompressFGenUserData *)pops[i]->user_data)->block = (BlockUserData *)malloc(sizeof(BlockUserData));
			set_user_data(((CompressFGenUserData *)pops[i]->user_data)->block, image, texture);
			if (texture->type == TEXTURE_TYPE_ETC2_RGB8) {
				if (opt->allowed_modes_etc2 != -1)
					((CompressFGenUserData *)pops[i]->user_data)->block->flags = opt->allowed_modes_etc2 |
					ENCODE_BIT;
				else
				if (opt->modal_etc2 && compOpt->nu_islands >= 8) {
					switch (i & 7) {
					case 0:
					case 1:
					case 2:
						((CompressFGenUserData *)pops[i]->user_data)->block->flags =
							ETC_MODE_ALLOWED_INDIVIDUAL | ENCODE_BIT;
						break;
					case 3:
					case 4:
						((CompressFGenUserData *)pops[i]->user_data)->block->flags =
							ETC_MODE_ALLOWED_DIFFERENTIAL | ENCODE_BIT;
						break;
					case 5:
						((CompressFGenUserData *)pops[i]->user_data)->block->flags =
							ETC2_MODE_ALLOWED_T | ENCODE_BIT;
						break;
					case 6:
						((CompressFGenUserData *)pops[i]->user_data)->block->flags =
							ETC2_MODE_ALLOWED_H | ENCODE_BIT;
						break;
					case 7:
						((CompressFGenUserData *)pops[i]->user_data)->block->flags =
							ETC2_MODE_ALLOWED_PLANAR | ENCODE_BIT;
						break;
					}
				}
			}
			if (texture->type == TEXTURE_TYPE_BPTC_FLOAT || texture->type == TEXTURE_TYPE_BPTC_SIGNED_FLOAT) {
				if (/* opt->modal_etc2 && */ compOpt->nu_islands >= 8) {
					switch (i & 7) {
					case 0:
					case 1:	// Mode 0 (very common).
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = 0x1 | ENCODE_BIT;
						break;
					case 2:	// Modes 5 (common) and 9.
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (1 << 5) | (1 << 9) | ENCODE_BIT;
						break;
					case 3:	// Modes 2 and 6 (common).
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (1 << 2) | (1 << 6) | ENCODE_BIT;
						break;
					case 4: 	// Modes 3 and 7 (common).
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (1 << 3) | (1 << 7) | ENCODE_BIT;
						break;
					case 5:	// Modes 1, 4, 8 (common).
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (1 << 1) | (1 << 4) | (1 << 8)
							| ENCODE_BIT;
						break;
					case 6:	// Mode 11.
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (1 << 11) | ENCODE_BIT;
						break;
					case 7:	// Modes 10, 11, 12, 13.
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (0xF << 10) | ENCODE_BIT;
						break;
					}
				}
				else
				if (/* opt->modal_etc2 && */ compOpt->nu_islands >= 4) {
					switch (i & 3) {
					case 0:	// Mode 0.
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = 0x1 | ENCODE_BIT;
						break;
					case 1:	// Modes 2, 4, 6 (common), 8 (common), 9.
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (1 << 2) | (1 << 4) |
							(1 << 6) | (1 << 8) | (1 << 9) | ENCODE_BIT;
						break;
					case 2:	// Modes 1, 3, 5 (common), 7 (common).
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (1 << 1) | (1 << 3) |
							(1 << 5) | (1 << 7) | ENCODE_BIT;
						break;
					case 3:	// Modes 10, 11, 12, 13.
						((CompressFGenUserData *)pops[i]->user_data)->block->flags = (0xF << 10) | ENCODE_BIT;
						break;
					}
				}
			}
		}
		if (!opt->deterministic)
			fgen_random_seed_with_timer(fgen_get_rng(pops[0]));
		//	fgen_random_seed_rng(fgen_get_rng(pops[0]), 0);

		for (int y = 0; y < image->extended_height; y += texture->block_height)
		for (int x = 0; x < image->extended_width; x += texture->block_width) {
			// For 1-bit alpha texture, prepare the alpha values of the image block for use in the
			// seeding function.
			if (texture->type == TEXTURE_TYPE_DXT3 || texture->type == TEXTURE_TYPE_ETC2_PUNCHTHROUGH)
				set_alpha_pixels(image, x, y, texture->block_width, texture->block_height, alpha_pixels);
			// Set up the auxilliary information for each population.
			for (int i = 0; i < compOpt->nu_islands; i++) {
				BlockUserData *user_data = ((CompressFGenUserData *)pops[i]->user_data)->block;
				user_data->x_offset = x;
				user_data->y_offset = y;
				user_data->alpha_pixels = alpha_pixels;
			}
			// Run the genetic algorithm.
			if (opt->max_threads != -1 && opt->max_threads < compOpt->nu_islands)
				fgen_run_archipelago(compOpt->nu_islands, pops, -1);
			else
				fgen_run_archipelago_threaded(compOpt->nu_islands, pops, -1);
			// Report the best solution.
			FgenIndividual *best = fgen_best_individual_of_archipelago(compOpt->nu_islands, pops);
			report_solution(best, (CompressFGenUserData *)pops[0]->user_data);
			if (((CompressFGenUserData *)pops[0]->user_data)->block->stop_signalled)
				goto end;
		}
	end:
		for (int i = 0; i < compOpt->nu_islands; i++) {
			free(((CompressFGenUserData *)pops[i]->user_data)->block);
			free(pops[i]->user_data);
			fgen_destroy(pops[i]);
		}
	}

	// Compress multiple blocks concurrently. Used by --ultra setting. Note that larger population size used in this case.

	static void compress_multiple_blocks_concurrently(Image *image, Texture *texture, const Options *opt, CompressOptions *compOpt) {
		unsigned char *alpha_pixels = (unsigned char *)alloca(texture->block_width * texture->block_height * compOpt->nu_islands);
		if (opt->generations != -1)
			compOpt->nu_generations = opt->generations;
		if (opt->islands != -1)
			compOpt->nu_islands = opt->islands;
		FgenPopulation **pops = (FgenPopulation **)alloca(sizeof(FgenPopulation *)* compOpt->nu_islands);
		for (int i = 0; i < compOpt->nu_islands; i++) {
			pops[i] = fgen_create(
				compOpt->population_size,		// Population size.
				texture->bits_per_block,	// Number of bits.
				1,				// Data element size.
				generation_callback,
				calculate_fitness,
				seed2,
				fgen_mutation_per_bit_fast,
				fgen_crossover_uniform_per_bit
				);
			fgen_set_parameters(
				pops[i],
				FGEN_ELITIST_SUS,
				FGEN_SUBTRACT_MIN_FITNESS,
				compOpt->crossover_probability,	// Crossover prob.
				compOpt->mutation_probability,	// Mutation prob. per bit
				0		// Macro-mutation prob.
				);
			fgen_set_generation_callback_interval(pops[i], compOpt->nu_generations);
			fgen_set_migration_interval(pops[i], 0);	// No migration.
			fgen_set_migration_probability(pops[i], 0.01);
			pops[i]->user_data = (CompressFGenUserData *)malloc(sizeof(CompressFGenUserData));
			((CompressFGenUserData *)pops[i]->user_data)->compOpt = compOpt;
			((CompressFGenUserData *)pops[i]->user_data)->block = (BlockUserData *)malloc(sizeof(BlockUserData));
			set_user_data(((CompressFGenUserData *)pops[i]->user_data)->block, image, texture);
			if (texture->type == TEXTURE_TYPE_ETC2_RGB8 || texture->type == TEXTURE_TYPE_ETC2_EAC)
			if (opt->allowed_modes_etc2 != -1)
				((CompressFGenUserData *)pops[i]->user_data)->block->flags =
				opt->allowed_modes_etc2 | ENCODE_BIT;
		}
		if (!opt->deterministic)
			fgen_random_seed_with_timer(fgen_get_rng(pops[0]));
		// Calculate the number of full archipelagos of nu_islands that fit on each each row.
		int nu_full_archipelagos_per_row = image->extended_width / (compOpt->nu_islands * texture->block_width);
		int x_marker = nu_full_archipelagos_per_row * compOpt->nu_islands * texture->block_width;
		for (int y = 0; y < image->extended_height; y += texture->block_height) {
			// Handle nu_islands horizontal blocks at a time.
			for (int x = 0; x < x_marker; x += compOpt->nu_islands * texture->block_width) {
				for (int i = 0; i < compOpt->nu_islands; i++) {
					BlockUserData *user_data = ((CompressFGenUserData *)pops[i]->user_data)->block;
					user_data->x_offset = x + i * texture->block_width;
					user_data->y_offset = y;
					// For 1-bit alpha texture, prepare the alpha values of the image block for use in the
					// seeding function.
					if (texture->type == TEXTURE_TYPE_DXT3 || texture->type == TEXTURE_TYPE_ETC2_PUNCHTHROUGH) {
						set_alpha_pixels(image, x + i * texture->block_width, y, texture->block_width,
							texture->block_height, &alpha_pixels[i * 16]);
						user_data->alpha_pixels = &alpha_pixels[i * 16];
					}
				}
				// Run with a seperate population on each island for different blocks.
				fgen_run_archipelago_threaded(compOpt->nu_islands, pops, compOpt->nu_generations);
				for (int i = 0; i < compOpt->nu_islands; i++) {
					FgenIndividual *best = fgen_best_individual_of_population(pops[i]);
					report_solution(best, (CompressFGenUserData *)pops[i]->user_data);
					if (((CompressFGenUserData *)pops[i]->user_data)->block->stop_signalled)
						goto end;
				}
			}
			// Handle the remaining blocks on the row.
			int nu_blocks_left = (image->extended_width - x_marker + texture->block_width - 1) / texture->block_width;
			int x = x_marker;
			for (int i = 0; i < nu_blocks_left; i++) {
				BlockUserData *user_data = ((CompressFGenUserData *)pops[i]->user_data)->block;
				user_data->x_offset = x + i * texture->block_width;
				user_data->y_offset = y;
				// For 1-bit alpha texture, prepare the alpha values of the image block for use in the
				// seeding function.
				if (texture->type == TEXTURE_TYPE_DXT3 || texture->type == TEXTURE_TYPE_ETC2_PUNCHTHROUGH) {
					set_alpha_pixels(image, x + i * texture->block_width, y, texture->block_width,
						texture->block_height, &alpha_pixels[i * texture->block_width *
						texture->block_height]);
					user_data->alpha_pixels = &alpha_pixels[i * texture->block_width * texture->block_height];
				}
			}
			if (nu_blocks_left > 0) {
				if (nu_blocks_left > 1)
					fgen_run_archipelago_threaded(nu_blocks_left, pops, -1);
				else
					fgen_run(pops[0], -1);
			}
			for (int i = 0; i < nu_blocks_left; i++) {
				FgenIndividual *best = fgen_best_individual_of_population(pops[i]);
				report_solution(best, (CompressFGenUserData *)pops[i]->user_data);
				if (((CompressFGenUserData *)pops[i]->user_data)->block->stop_signalled)
					goto end;
			}
		}
	end:
		for (int i = 0; i < compOpt->nu_islands; i++) {
			free(((CompressFGenUserData *)pops[i]->user_data)->block);
			free(pops[i]->user_data);
			fgen_destroy(pops[i]);
		}
	}

	// Copy the alpha pixel values of a block into an array.

	static void set_alpha_pixels(Image *image, int x, int y, int w, int h, unsigned char *alpha_pixels) {
		for (int by = 0; by < h; by++)
		for (int bx = 0; bx < w; bx++)
		if (y + by < image->height && x + bx < image->width)
			alpha_pixels[by * w + bx] = pixel_get_a(
			image->pixels[(y + by) * image->extended_width + x + bx]);
		else
			// If the pixel falls on the border, put 0xFF.
			alpha_pixels[by * w + bx] = 0xFF;
	}

	// Optimize the alpha component of 1-bit alpha textures for the whole image.

	static void optimize_alpha(Image *image, Texture *texture) {
		if (texture->type != TEXTURE_TYPE_ETC2_PUNCHTHROUGH && texture->type != TEXTURE_TYPE_DXT3)
			return;
		unsigned char alpha_pixels[16];
		for (int y = 0; y < image->extended_height; y += texture->block_height)
		for (int x = 0; x < image->extended_width; x += texture->block_width) {
			set_alpha_pixels(image, x, y, texture->block_width, texture->block_height, alpha_pixels);
			int compressed_block_index = (y / texture->block_height) *
				(texture->extended_width / texture->block_width) + x / texture->block_width;
			unsigned char *bitstring;
			switch (texture->type) {
			case TEXTURE_TYPE_ETC2_PUNCHTHROUGH:
				bitstring = (unsigned char *)&texture->pixels[compressed_block_index * 2];
				optimize_block_etc2_punchthrough(bitstring, alpha_pixels);
				break;
			case TEXTURE_TYPE_DXT3:
				bitstring = (unsigned char *)&texture->pixels[compressed_block_index * 4];
				optimize_block_dxt3(bitstring, alpha_pixels);
				break;
			}
		}
	}

	// Calculate the RMSE threshold for adaptive block optimization.

	static double get_rmse_threshold(Texture *texture, int speed, Image *source_image, const Options *opt) {
		double threshold;
		if (!(texture->type & TEXTURE_TYPE_128BIT_BIT)) {
			// 64-bit texture formats.
			if (texture->type & TEXTURE_TYPE_ETC_BIT)
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 13.0;
					break;
				case SPEED_FAST:
					threshold = 11.5;
					break;
				case SPEED_MEDIUM:
					threshold = 11.0;
					break;
				case SPEED_SLOW:
					threshold = 10.5;
					break;
			}
			else
			if (texture->type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT)
				// R11_EAC and SIGNED_RGTC1
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 1100;
					break;
				case SPEED_FAST:
					threshold = 900;
					break;
				case SPEED_MEDIUM:
					threshold = 800;
					break;
				case SPEED_SLOW:
					threshold = 700;
					break;
			}
			else
			if (texture->info->nu_components == 1 && !(texture->type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT))
				// RGTC1, one 8-bit component.
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 6.0;
					break;
				case SPEED_FAST:
					threshold = 5.0;
					break;
				case SPEED_MEDIUM:
					threshold = 4.0;
					break;
				case SPEED_SLOW:
					threshold = 3.5;
					break;
			}
			else	// DXT1/DXT1A
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 11.5;
					break;
				case SPEED_FAST:
					threshold = 9.0;
					break;
				case SPEED_MEDIUM:
					threshold = 8.5;
					break;
				case SPEED_SLOW:
					threshold = 8.0;
					break;
			}
		}
		else {
			// 128-bit texture formats.
			if (texture->type == TEXTURE_TYPE_BPTC)
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 10.5;
					break;
				case SPEED_FAST:
					threshold = 7.0;
					break;
				case SPEED_MEDIUM:
					threshold = 6.0;
					break;
				case SPEED_SLOW:
					threshold = 5.5;
					break;
			}
			else
			if (texture->type & TEXTURE_TYPE_HALF_FLOAT_BIT)
			if (opt->hdr)
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 0.35;
					break;
				case SPEED_FAST:
					threshold = 0.20;
					break;
				case SPEED_MEDIUM:
					threshold = 0.15;
					break;
				case SPEED_SLOW:
					threshold = 0.10;
					break;
			}
			else
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 0.060;
					break;
				case SPEED_FAST:
					threshold = 0.050;
					break;
				case SPEED_MEDIUM:
					threshold = 0.040;
					break;
				case SPEED_SLOW:
					threshold = 0.035;
					break;
			}
			else
			if (texture->type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT)
				// RG11_EAC, SIGNED_RGTC2
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 2000;
					break;
				case SPEED_FAST:
					threshold = 1400;
					break;
				case SPEED_MEDIUM:
					threshold = 1200;
					break;
				case SPEED_SLOW:
					threshold = 950;
					break;
			}
			else
			if (texture->info->nu_components == 2 && !(texture->type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT))
				// RGTC2, two 16-bit components.
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 13.0;
					break;
				case SPEED_FAST:
					threshold = 8.0;
					break;
				case SPEED_MEDIUM:
					threshold = 7.0;
					break;
				case SPEED_SLOW:
					threshold = 6.5;
					break;
			}
			else	// 128-bit alpha formats ETC2_EAC, DXT3, DXT5
				switch (speed) {
				case SPEED_ULTRA:
					threshold = 11.5;
					break;
				case SPEED_FAST:
					threshold = 9.0;
					break;
				case SPEED_MEDIUM:
					threshold = 8.5;
					break;
				case SPEED_SLOW:
					threshold = 8.0;
					break;
			}
		}
		if (source_image->is_half_float && !(texture->type & TEXTURE_TYPE_HALF_FLOAT_BIT))
			// If the source image is in half-float format, but the texture type is regular RGB(A)8,
			// correct the threshold to half-float comparison values.
			threshold *= 1.0 / 255.0;
		else
		if (source_image->bits_per_component == 16 && !(texture->type & TEXTURE_TYPE_16_BIT_COMPONENTS_BIT))
			threshold *= 256.0;
		return threshold;
	}

}
