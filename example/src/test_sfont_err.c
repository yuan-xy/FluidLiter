#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "fluidliter.h"

int main() {
	char *filename = "example/sf_/README.md";
    fluid_synth_t* synth = NEW_FLUID_SYNTH();
    int sfont = fluid_synth_sfload(synth, filename, 1);
    assert(sfont == FLUID_FAILED);
    delete_fluid_synth(synth);
}
