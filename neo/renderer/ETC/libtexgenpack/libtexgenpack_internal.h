/*
    libtexgenpack.h -- part of texgenpack, a texture compressor using fgen.

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

namespace texgen {

	const float *get_half_float_table();
	const float *get_gamma_corrected_half_float_table();
	const float *get_normalized_float_table();

	void calculate_normalized_float_table();
	void calculate_half_float_table();
	void calculate_gamma_corrected_half_float_table();

}
