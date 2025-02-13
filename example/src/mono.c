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

#define POLYPHONY 8

int main(int argc, char *argv[]) {
    if (argc < 2) {
      printf("Usage: %s <soundfont> [<output>]\n", argv[0]);
      return 1;
    }

    fluid_synth_t* synth = NEW_FLUID_SYNTH(.polyphony=POLYPHONY);
    set_log_level(FLUID_DBG);
    assert(synth->with_reverb == 1);
    assert(synth->polyphony == POLYPHONY);
    assert(synth->midi_channels == 1);
    assert(synth->ticks == 0);
    assert(synth->noteid == 0);
    assert(synth->storeid == 0);
    assert(synth->cur == FLUID_BUFSIZE);

    fluid_synth_set_reverb_preset(synth, 4);

    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    fluid_preset_t *preset = synth->channel[0]->preset;
    print_preset_info(preset);
    
    fluid_preset_zone_t *preset_zone = preset->zone;
    fluid_inst_t *inst = preset_zone->inst;
    fluid_inst_zone_t *zone = inst->zone;
    bool stereo_sample = zone->next !=NULL && (zone->sample->sampletype + zone->next->sample->sampletype == 2+4);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);
    FILE* file = argc > 2 ? fopen(argv[2], "wb") : stdout;

    fluid_synth_noteon(synth, 0, 60, 50);
    assert(synth->ticks == 0);
    assert(synth->noteid == 1);
    assert(synth->storeid == 0);
    assert(synth->cur == FLUID_BUFSIZE);
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

    for(int i=0; i<synth->polyphony; i++){
      fluid_voice_t* voice = synth->voice[i];
      if(i<2){
        if(i==0 || (i==1 && stereo_sample)) assert(voice->status == 1);  //一次noteon激活两个voice，左右声道
      }else{
        assert(voice->status == 0);
      }
      printf("voice %d：%d\n", i, voice->status);
      print_voice_modulator(voice);
    }


    fluid_synth_noteon(synth, 0, 67, 80);
    assert(synth->ticks == 0);
    assert(synth->noteid == 2);
    assert(synth->storeid == 1);
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
    assert(synth->ticks == 0);
    assert(synth->noteid == 3);
    assert(synth->storeid == 2);
    assert(synth->cur == FLUID_BUFSIZE);
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


    fluid_voice_t copy0 = *synth->voice[0];
    assert(copy0.chan == 0);
    fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);
    assert(synth->ticks == 88256);
    assert(synth->noteid == 3);
    assert(synth->storeid == 2);
    assert(synth->cur == 8);
    assert(synth->voice[0]->id == 0);
    assert(synth->voice[0]->key == 60);
    assert(synth->voice[0]->vel == 50);

    assert(copy0.chan == 0);
    bool already_off = FALSE;
    if(synth->voice[0]->chan == NO_CHANNEL){
      already_off = TRUE;
      printf("noteoff in fluid_voice_write\n");
      assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVFINISHED);
    }


    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);


    if(!already_off){
      assert(synth->voice[0]->volenv_section < FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[0]->volenv_count > 0);
      assert(synth->voice[0]->modenv_section < FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[0]->modenv_count > 0);
      assert(synth->voice[0]->status == FLUID_VOICE_ON);
    }
    fluid_synth_noteoff(synth, 0, 60);
    if(!already_off){
      assert(synth->voice[0]->volenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[0]->modenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[0]->status == FLUID_VOICE_ON);
    }else{
      assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVFINISHED);
      assert(synth->voice[0]->modenv_section  == FLUID_VOICE_ENVFINISHED);
      assert(synth->voice[0]->status == FLUID_VOICE_OFF);
    }
    assert(synth->voice[0]->volenv_count  == 0);
    assert(synth->voice[0]->modenv_count  == 0);
    

    if(stereo_sample && !already_off){
      assert(synth->voice[2]->volenv_section < FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[2]->volenv_count > 0);
      assert(synth->voice[2]->modenv_section < FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[2]->modenv_count > 0);
      assert(synth->voice[2]->status == FLUID_VOICE_ON);
      assert(synth->voice[3]->volenv_section < FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[3]->volenv_count > 0);
      assert(synth->voice[3]->modenv_section < FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[3]->modenv_count > 0);
      assert(synth->voice[2]->status == FLUID_VOICE_ON);
    }else if(!already_off){
      assert(synth->voice[1]->volenv_section < FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[1]->volenv_count > 0);
      assert(synth->voice[1]->modenv_section < FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[1]->modenv_count > 0);
      assert(synth->voice[1]->status == FLUID_VOICE_ON);
    }
    fluid_synth_noteoff(synth, 0, 67);
    if(stereo_sample && !already_off){
      assert(synth->voice[2]->volenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[2]->volenv_count  == 0);
      assert(synth->voice[2]->modenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[2]->modenv_count  == 0);
      assert(synth->voice[2]->status == FLUID_VOICE_ON);
      assert(synth->voice[3]->volenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[3]->volenv_count  == 0);
      assert(synth->voice[3]->modenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[3]->modenv_count  == 0);
      assert(synth->voice[3]->status == FLUID_VOICE_ON);
    }else if(!already_off){
      assert(synth->voice[1]->volenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[1]->volenv_count  == 0);
      assert(synth->voice[1]->modenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[1]->modenv_count  == 0);
      assert(synth->voice[1]->status == FLUID_VOICE_ON);
    }

    fluid_synth_noteoff(synth, 0, 76);
    if(stereo_sample && !already_off){
      assert(synth->voice[4]->volenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[4]->volenv_count  == 0);
      assert(synth->voice[4]->modenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[4]->modenv_count  == 0);
      assert(synth->voice[4]->status == FLUID_VOICE_ON);
      assert(synth->voice[5]->volenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[5]->volenv_count  == 0);
      assert(synth->voice[5]->modenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[5]->modenv_count  == 0);
      assert(synth->voice[5]->status == FLUID_VOICE_ON);
    }else if(!already_off){
      assert(synth->voice[2]->volenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[2]->volenv_count  == 0);
      assert(synth->voice[2]->modenv_section  == FLUID_VOICE_ENVRELEASE);
      assert(synth->voice[2]->modenv_count  == 0);
      assert(synth->voice[2]->status == FLUID_VOICE_ON);
    }

    fluid_synth_write_s16_mono(synth, NUM_FRAMES/10, buffer);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES/10, file);

    fclose(file);
    free(buffer);

    delete_fluid_synth(synth);
}
