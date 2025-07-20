#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "fluidliter.h"
#include "fluid_synth.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: sfont_load_test sfont_file.sf2\n");
	}
    char *filename = argv[1];
    fluid_synth_t* synth = NEW_FLUID_SYNTH();
    int sfont_id = fluid_synth_sfload(synth, filename, 1);
    if(sfont_id == FLUID_FAILED){
        printf("load failed.\n");
    }else{
        fluid_sfont_t *sfont = synth->sfont->data;
        printf("load sfont: %s\n", fluid_sfont_get_name(sfont));
    }
    delete_fluid_synth(synth);
}
