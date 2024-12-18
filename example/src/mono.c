#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "fluidlite.h"
#include "utils.c"


#define SAMPLE_RATE 44100
#define SAMPLE_SIZE sizeof(int16_t) //s16
#define DURATION 2 //second
#define NUM_FRAMES SAMPLE_RATE*DURATION
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

int main(int argc, char *argv[]) {
    if (argc < 2) {
      printf("Usage: %s <soundfont> [<output>]\n", argv[0]);
      return 1;
    }

    fluid_settings_t* settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "yes"); //在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 6); 
    fluid_settings_setint(settings, "synth.midi-channels", 1); 
    char *vb = "   ";
    fluid_settings_getstr(settings, "synth.verbose", &vb);
    assert(strcmp(vb, "yes") == 0);
    int polyphony;
    fluid_settings_getint(settings, "synth.polyphony", &polyphony);
    assert(polyphony == 6);
    int midi_channels;
    fluid_settings_getint(settings, "synth.midi-channels", &midi_channels);
    assert(midi_channels == 1);

    fluid_synth_t* synth = new_fluid_synth(settings);
    assert(synth->verbose == 1);
    assert(synth->polyphony == 6);
    assert(synth->midi_channels == 1);

    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    fluid_preset_t *preset = synth->channel[0]->preset;
    print_preset_info(preset);
    
    fluid_defpreset_t * defpreset = (fluid_defpreset_t *)preset->data;
    fluid_preset_zone_t *preset_zone = defpreset->zone;
    fluid_inst_t *inst = preset_zone->inst;
    fluid_inst_zone_t *zone = inst->zone;
    bool stereo_sample = zone->next !=NULL && (zone->sample->sampletype + zone->next->sample->sampletype == 2+4);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);
    FILE* file = argc > 2 ? fopen(argv[2], "wb") : stdout;

    fluid_synth_noteon(synth, 0, 60, 50);
    assert(synth->voice[0]->id == 0);
    assert(synth->voice[0]->chan == 0);
    assert(synth->voice[0]->key == 60);
    assert(synth->voice[0]->vel == 50);
    if(stereo_sample){
      assert(synth->voice[1]->id == 0);
      assert(synth->voice[1]->chan == 0);
      assert(synth->voice[1]->key == 60);
      assert(synth->voice[1]->vel == 50);
      assert(synth->voice[0]->sample->start != synth->voice[1]->sample->start);
    }

    fluid_synth_noteon(synth, 0, 67, 80);
    if(stereo_sample){
      assert(synth->voice[2]->id == 1);
      assert(synth->voice[2]->chan == 0);
      assert(synth->voice[2]->key == 67);
      assert(synth->voice[2]->vel == 80);
      assert(synth->voice[3]->id == 1);
      assert(synth->voice[3]->chan == 0);
      assert(synth->voice[3]->key == 67);
      assert(synth->voice[3]->vel == 80);
    }else{
      assert(synth->voice[1]->id == 1);
      assert(synth->voice[1]->chan == 0);
      assert(synth->voice[1]->key == 67);
      assert(synth->voice[1]->vel == 80);
    }


    fluid_synth_noteon(synth, 0, 76, 100);
    if(stereo_sample){
      assert(synth->voice[4]->id == 2);
      assert(synth->voice[4]->chan == 0);
      assert(synth->voice[4]->key == 76);
      assert(synth->voice[4]->vel == 100);
      assert(synth->voice[5]->id == 2);
      assert(synth->voice[5]->chan == 0);
      assert(synth->voice[5]->key == 76);
      assert(synth->voice[5]->vel == 100);
    }else{
      assert(synth->voice[2]->id == 2);
      assert(synth->voice[2]->chan == 0);
      assert(synth->voice[2]->key == 76);
      assert(synth->voice[2]->vel == 100);
    }


    fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);

    fluid_synth_noteoff(synth, 0, 60);
    fluid_synth_noteoff(synth, 0, 67);
    fluid_synth_noteoff(synth, 0, 76);
    fluid_synth_write_s16_mono(synth, NUM_FRAMES/10, buffer);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES/10, file);

    fclose(file);
    free(buffer);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}
