#include <stdio.h>
#include <fluidliter.h>
#include "fluid_sfont.h"
#include "utils.c"


int main(int argc, char** argv)
{
	fluid_synth_t* synth = NULL;
	fluid_sfont_t* sfont = NULL;
	int err = 0, sfid = -1;

	char *file = "example/sf_/Boomwhacker.sf2";
	if (argc >= 2) {
		file = argv[1];
	}

	synth = NEW_FLUID_SYNTH();
	sfid = fluid_synth_sfload(synth, file, 1);
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
		fluid_preset_t *preset;
        fluid_sfont_iteration_start(sfont);
        while ((preset=fluid_sfont_iteration_next(sfont)) != NULL) {
			print_preset_info(preset);
		}
	}

 cleanup:
	if (synth) {
		delete_fluid_synth(synth);
	}
	return err;
}
