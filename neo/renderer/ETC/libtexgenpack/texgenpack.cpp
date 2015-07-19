/*
    texgenpack.c -- part of texgenpack, a texture compressor using fgen.

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

#include "libtexgenpack.h"

namespace texgen {

	void init_options(Options *opt) {
		if (opt) {
			opt->max_threads = -1;
			opt->orientation = 0;
			opt->speed = SPEED_FAST;
			opt->modal_etc2 = 0;
			opt->allowed_modes_etc2 = -1;
			opt->generations = -1;
			opt->islands = -1;
			opt->texture_format = -1;
			opt->mipmaps = 0;
			opt->flip_vertical = 0;
			opt->block_width = 4;
			opt->block_height = 4;
			opt->half_float = 0;
			opt->deterministic = 0;
			opt->hdr = 0;
			opt->progress_callback = NULL;
		}
	}

}
