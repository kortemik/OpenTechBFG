/*
    calibrate.c -- part of texgenpack, a texture compressor using fgen.

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
#include <math.h>
#include <malloc.h>
#include "../fgen/fgen.h"
#include "libtexgenpack.h"

namespace texgen {

	// Calibrate genetic algorithm parameters.

	static const int fixed_crossover_probability = 1;
	static const int real_valued_ga = 0;
	static const int deterministic = 0;

	typedef struct {
		Image *image;
		int texture_format;
	} FitUserData;

	typedef struct {
		double cumulative_rmse;
		int count;
	} MutationBin;

	typedef struct {
		Options *opt;
		FitUserData user_data;
		MutationBin mutation_bin[100];
	} CalibFitUserData;

	static void mutation_bin_initialize(CalibFitUserData *user_data) {
		for (int i = 0; i < 100; i++) {
			user_data->mutation_bin[i].cumulative_rmse = 0;
			user_data->mutation_bin[i].count = 0;
		}
	}

	static void mutation_bin_add_measurement(double mutation_prob, double rmse, CalibFitUserData *user_data) {
		int bin = floor(mutation_prob * 1000.0);
		if (bin >= 100)
			bin = 99;
		user_data->mutation_bin[bin].cumulative_rmse += rmse;
		user_data->mutation_bin[bin].count++;
	}

	static void mutation_bin_report(FILE* stat_file, CalibFitUserData *user_data) {
		if (!stat_file)
			return;
		fprintf(stat_file, "Binned results:\n");
		for (int i = 0; i < 100; i++) {
			if (user_data->mutation_bin[i].count > 0)
				fprintf(stat_file, "Range %.4lf - %.4lf: n = %3d RMSE %.4lf\n",
				(double)i / 1000.0, ((double)i + 1) / 1000.0, user_data->mutation_bin[i].count,
				user_data->mutation_bin[i].cumulative_rmse / user_data->mutation_bin[i].count);
		}
	}

	static void compress_callback(BlockUserData *user_data) {
	}

	static double calibrate_calculate_error(const Ffit *fit, const double *param) {
		FitUserData *user_data = &((CalibFitUserData *)fit->user_data)->user_data;
		Texture texture;
		float crossover_prob = param[1];
		if (fixed_crossover_probability)
			crossover_prob = 0.7;
		if (!compress_image(((CalibFitUserData *)fit->user_data)->opt, user_data->image, user_data->texture_format, compress_callback, &texture, 1, param[0], crossover_prob))
			return 0.0;
		Image image2;
		if (!convert_texture_to_image(((CalibFitUserData *)fit->user_data)->opt, &texture, &image2))
			return 0.0;
		double rmse = compare_images(((CalibFitUserData *)fit->user_data)->opt, user_data->image, &image2);
		destroy_image(&image2);
		destroy_texture(&texture);
		//printf("Mut = %.4lf, Cross = %.3lf, RMSE = %.3lf\n", param[0], crossover_prob, rmse);
		//fflush(stdout);
		mutation_bin_add_measurement(param[0], rmse, (CalibFitUserData *)fit->user_data);
		return rmse * rmse;
	}

	static void calibrate_generation_callback(Ffit *fit, int generation, const double *best_param, double best_error) {
		float crossover_prob = best_param[1];
		if (fixed_crossover_probability)
			crossover_prob = 0.7;
		//printf("Generation %d: Mut = %.4lf, Cross = %.3lf, RMSE = %.3lf\n", generation,
		//	best_param[0], crossover_prob, sqrt(best_error));
		if (generation == 5)
			ffit_signal_stop(fit);
		if (!deterministic)
			ffit_signal_model_change(fit);
	}

	void calibrate_genetic_parameters(Options *opt, Image *image, int texture_type, FILE* stat_file) {
		opt->deterministic = deterministic;	// Whether to initialize random function seed with timer.
		Ffit *fit = ffit_create(2, calibrate_generation_callback, calibrate_calculate_error);
		ffit_set_parameter_range_and_mapping(fit, 0, 0, 0.1, FFIT_MAPPING_LINEAR);	// Mutation rate.
		ffit_set_parameter_range_and_mapping(fit, 1, 0, 0.9, FFIT_MAPPING_LINEAR);	// Crossover rate.
		fit->user_data = (CalibFitUserData *)malloc(sizeof(CalibFitUserData));
		((CalibFitUserData *)fit->user_data)->user_data.image = image;
		((CalibFitUserData *)fit->user_data)->user_data.texture_format = texture_type;
		mutation_bin_initialize((CalibFitUserData *)fit->user_data);
#if 0
		if (real_valued_ga)
			ffit_run_fgen_real_valued(fit, 16, FGEN_ELITIST_SUS, 0.6, 0.4, 0.1);
		else
			ffit_run_fgen(fit, 16, 16, FGEN_ELITIST_SUS, fgen_crossover_uniform_per_element, 0.6, 0.05, 0.1);
		//		ffit_run_fgen(fit, 16, 32, FGEN_ELITIST_SUS, fgen_crossover_uniform_per_element, 0.6, 0.025, 0.1);
#else
		double mut = 0.005;
		for (; mut < 0.050; mut += 0.001) {
			int n = 2;
			if (mut > 0.008 && mut <= 0.035)
				n = 5;
			for (int i = 0; i < n; i++) {
				Texture texture;
				if (!compress_image(opt, image, texture_type, compress_callback, &texture, 1, mut, 0.7))
					goto error_goto;
				Image image2;
				if (!convert_texture_to_image(opt, &texture, &image2)) {
					destroy_texture(&texture);
					goto error_goto;
				}
				double rmse = compare_images(opt, image, &image2);
				destroy_image(&image2);
				destroy_texture(&texture);
				if (stat_file) {
					fprintf(stat_file, "Mut = %.4lf, Cross = %.3lf, RMSE = %.3lf\n", mut, 0.7, rmse);
					fflush(stat_file);
				}
				mutation_bin_add_measurement(mut, rmse, (CalibFitUserData *)fit->user_data);
			}
		}
#endif
		mutation_bin_report(stat_file, (CalibFitUserData *)fit->user_data);
	error_goto:
		free(fit->user_data);
	}

}
