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

/**
gdb --args ./sfont_load_test example/sf_/GMGSx_2.sf2

(gdb) b fixup_sample
(gdb) p *(SFSample *)sf->sample->data
$14 = {name = "000289", '\000' <repeats 14 times>, start = 0, end = 13703, loopstart = 11989, loopend = 13671,
  samplerate = 22050, origpitch = 36 '$', pitchadj = -4 '\374', sampletype = 1}
(gdb) p *(SFSample *)sf->sample->next->data
$15 = {name = "000290", '\000' <repeats 14 times>, start = 13735, end = 26869, loopstart = 24565, loopend = 26837,
  samplerate = 22050, origpitch = 41 ')', pitchadj = -1 '\377', sampletype = 1}
(gdb) p *(SFSample *)sf->sample->next->next->data
$16 = {name = "000291", '\000' <repeats 14 times>, start = 26901, end = 38100, loopstart = 36214, loopend = 38068,
  samplerate = 22050, origpitch = 48 '0', pitchadj = 0 '\000', sampletype = 1}

fixup_sample将采样的end/loopstart/loopend修改为相对于start的位置。而start则代表采样在smpl中的起始偏移地址。

(gdb) b sfont_close
(gdb) p *(SFSample *)sf->sample->data
$17 = {name = "000289", '\000' <repeats 14 times>, start = 0, end = 13702, loopstart = 11989, loopend = 13671,
  samplerate = 22050, origpitch = 36 '$', pitchadj = -4 '\374', sampletype = 1}
(gdb) p *(SFSample *)sf->sample->next->data
$18 = {name = "000290", '\000' <repeats 14 times>, start = 13735, end = 13133, loopstart = 10830, loopend = 13102,
  samplerate = 22050, origpitch = 41 ')', pitchadj = -1 '\377', sampletype = 1}
(gdb) p *(SFSample *)sf->sample->next->next->data
$19 = {name = "000291", '\000' <repeats 14 times>, start = 26901, end = 11198, loopstart = 9313, loopend = 11167,
  samplerate = 22050, origpitch = 48 '0', pitchadj = 0 '\000', sampletype = 1}

拷贝到fluid_sample_t结构体，short *data则指向采样数据。

(gdb) b fluid_sfont_get_name
(gdb) p *(fluid_sample_t *)sfont->sample->data
$8 = {name = "000289", '\000' <repeats 14 times>, start = 0, end = 13702, loopstart = 11989, loopend = 13671,
  samplerate = 22050, origpitch = 36, pitchadj = -4, sampletype = 1, valid = 1, data = 0xf7c40010,
  amplitude_that_reaches_noise_floor_is_valid = 1, amplitude_that_reaches_noise_floor = 4.7806253951271703e-05}
(gdb) p *(fluid_sample_t *)sfont->sample->next->data
$9 = {name = "000290", '\000' <repeats 14 times>, start = 13735, end = 26868, loopstart = 24565, loopend = 26837,
  samplerate = 22050, origpitch = 41, pitchadj = -1, sampletype = 1, valid = 1, data = 0xf7c40010,
  amplitude_that_reaches_noise_floor_is_valid = 1, amplitude_that_reaches_noise_floor = 4.3456964767251669e-05}
(gdb) p *(fluid_sample_t *)sfont->sample->next->next->data
$10 = {name = "000291", '\000' <repeats 14 times>, start = 26901, end = 38099, loopstart = 36214, loopend = 38068,
  samplerate = 22050, origpitch = 48, pitchadj = 0, sampletype = 1, valid = 1, data = 0xf7c40010,
  amplitude_that_reaches_noise_floor_is_valid = 1, amplitude_that_reaches_noise_floor = 6.5158083117916089e-05}



*/