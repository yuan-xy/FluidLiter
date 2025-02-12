#include <stdio.h>
#include <fluidlite.h>
#include "fluid_sfont.h"
#include "utils.c"


int main(int argc, char** argv)
{
	fluid_synth_t* synth = NULL;
	fluid_sfont_t* sfont = NULL;
	int err = 0, sfid = -1;

	if (argc != 2) {
		fprintf(stderr, "Usage: instruments [soundfont]\n");
		return 1;
	}

	synth = NEW_FLUID_SYNTH();
	sfid = fluid_synth_sfload(synth, argv[1], 1);
	if (sfid == -1) {
		fprintf(stderr, "Failed to load the SoundFont\n");
		err = 4;
		goto cleanup;
	}

    /* Enumeration of banks and programs */
    sfont = fluid_synth_get_sfont_by_id(synth, sfid);
    if (sfont != NULL) {
		char *fontname = fluid_sfont_get_name(sfont);
		printf("%s\n", fontname);
        fluid_preset_t struct_preset;
		fluid_preset_t *preset = &struct_preset;
        fluid_sfont_iteration_start(sfont);
        while (fluid_sfont_iteration_next(sfont, preset) != 0) {
			print_preset_info(preset);
		}
	}

 cleanup:
	if (synth) {
		delete_fluid_synth(synth);
	}
	return err;
}
