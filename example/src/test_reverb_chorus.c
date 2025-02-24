#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#include "fluidliter.h"
#include "fluid_synth.h"
#include "utils.c"
#include "fluid_sfont.h"
#include "misc.h"
#include "fluid_chorus.h"
#include "fluid_voice.h"

#define SAMPLE_RATE 44100


void gen_song(const char *name, fluid_synth_t *synth, const char *fontname){
    char pcmname[100] = "";
    strcat(pcmname, name);
    strcat(pcmname, ".pcm");

    FILE* file = fopen(pcmname, "wb");
    
    int sfid = fluid_synth_sfload(synth, fontname, 1);
    fluid_synth_program_select(synth, 0, sfid, 0, 0);
    print_gen_values(synth, 0, TRUE);


    int notes[] = {NN3, NN6, NN1+12, NN7, NN6, NN1+12, NN6, NN7, NN6, NN4, NN5, NN3};
                //    ,NN3, NN6, NN1+12, NN7, NN6, NN1+12, NN6, NN7, NN6, NN3, NN3-1, NN2};
    float dura[] =  {0.5, 0.5, 0.5,   0.5,  0.5, 0.5,    0.5, 0.5, 0.5, 0.5, 0.5, 1.5};
                    // ,0.5, 0.5, 0.5,   0.5,  0.5, 0.5,    0.5, 0.5, 0.5, 0.5, 0.5, 1.5};

    for(int i=0; i<sizeof(notes)/sizeof(int); i++){
        int frame = dura[i]*SAMPLE_RATE;
        int samples = frame * 2;
        // printf("%d-%f, \t%d\n",notes[i],dura[i], frame);
        fluid_synth_noteon(synth, 0, notes[i], 127);
        int16_t buffer[samples];
        fluid_synth_write_s16(synth, frame, buffer, 0, 2, buffer, 1, 2);
        fwrite(buffer, sizeof(int16_t), samples, file);
        fluid_synth_noteoff(synth, 0, notes[i]);
    }
    print_gen_values(synth, 0, TRUE);

    fclose(file);

    char cmd[256] = "";
    strcat(cmd, "ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 2 -i ");
    strcat(cmd, name);
    strcat(cmd, ".pcm ");
    strcat(cmd, name);
    strcat(cmd, ".wav");
    system(cmd);
}

void assert_gen_GMGSx_2(fluid_synth_t *synth){
    assert(synth->voice[0]->mod_count == 9);
    assert(synth->voice[0]->amp_reverb > 1e-6);
    assert(synth->voice[0]->gen[GEN_REVERBSEND].val == 800);
    assert(float_eq(synth->voice[0]->reverb_send, 0.5+0.3)); //ins:50 + global_preset:30
    assert(synth->voice[0]->amp_chorus > 1e-6);
    assert(synth->voice[0]->gen[GEN_CHORUSSEND].val == 700);
    assert(float_eq(synth->voice[0]->chorus_send, 0.4+0.3)); //global_ins:40 + global_preset:30
}

void assert_gen_zero(fluid_synth_t *synth){
#ifdef USING_CALLOC
    assert(synth->polyphony == synth->nvoice);
    assert(synth->polyphony == 10);
    assert(synth->voice[0]->chan == NO_CHANNEL);
    assert(synth->voice[0]->amp_reverb == 0);
    assert(float_eq(synth->voice[0]->reverb_send, 0));
    assert(synth->voice[0]->amp_chorus == 0);
    assert(float_eq(synth->voice[0]->chorus_send, 0));
#endif
}



extern fluid_mod_t default_reverb_mod, default_chorus_mod;

int main(){
    set_log_level(FLUID_DBG);
    assert(GEN_REVERBSEND==16);
    assert(GEN_CHORUSSEND==15);

    fluid_synth_t *synth;

    synth = NEW_FLUID_SYNTH(.with_reverb=false);
    assert_gen_zero(synth);
    assert(default_reverb_mod.src1 == 91);
    assert(default_chorus_mod.src1 == 93);
    assert(default_reverb_mod.amount == 200);
    assert(default_chorus_mod.amount == 200); //TODO：这里的amount有什么用，何时用到？

    assert(!synth->with_reverb);
    assert(!synth->with_chorus);
    assert(synth->reverb == NULL);
    assert(synth->fx_left_buf == NULL);
    assert(synth->fx_left_buf2 == NULL);
    assert(synth->chorus == NULL);
    gen_song("song_no_reverb_no_chorus", synth, "example/sf_/GMGSx_2.sf2");
    assert_gen_GMGSx_2(synth);
    delete_fluid_synth(synth);

    synth = NEW_FLUID_SYNTH(.with_reverb=true);
    assert_gen_zero(synth);
    assert(synth->reverb != NULL);
    if(synth->reverb != EMPTY_REVERB_STUB){
        assert(float_eq(synth->reverb->damp, FLUID_REVERB_DEFAULT_DAMP));
        assert(float_eq(synth->reverb->level, FLUID_REVERB_DEFAULT_LEVEL));
        assert(float_eq(synth->reverb->roomsize, FLUID_REVERB_DEFAULT_ROOMSIZE));
        assert(float_eq(synth->reverb->width, FLUID_REVERB_DEFAULT_WIDTH));
    }
    assert(synth->chorus == NULL);
    assert(synth->fx_left_buf != NULL);
    assert(synth->fx_left_buf2 == NULL);
    // fluid_revmodel_setlevel(synth->reverb, 0.9);
    // fluid_revmodel_setdamp(synth->reverb, 0.01);
    gen_song("song_reverb_no_chorus", synth, "example/sf_/GMGSx_2.sf2");
    assert_gen_GMGSx_2(synth);
    delete_fluid_synth(synth);

    synth = NEW_FLUID_SYNTH(.with_reverb=false);
    assert_gen_zero(synth);
    delete_fluid_synth(synth);
    synth = NEW_FLUID_SYNTH(.with_reverb=false, .with_chorus=true);
    assert_gen_zero(synth);
    assert(synth->reverb == NULL);
    assert(synth->with_chorus);
    assert(synth->fx_left_buf == NULL);
    assert(synth->fx_left_buf2 != NULL);
    assert(synth->chorus != NULL);
    if(synth->chorus != EMPTY_CHORUS_STUB){
        assert(synth->chorus->number_blocks == FLUID_CHORUS_DEFAULT_N);
        assert(synth->chorus->level == FLUID_CHORUS_DEFAULT_LEVEL);
        assert(synth->chorus->speed_Hz == FLUID_CHORUS_DEFAULT_SPEED);
        assert(synth->chorus->depth_ms == FLUID_CHORUS_DEFAULT_DEPTH);
        assert(synth->chorus->type == FLUID_CHORUS_DEFAULT_TYPE);
    }
    fluid_chorus_set(synth->chorus, FLUID_CHORUS_SET_NR, 50, 0, 0, 0, 0);
    gen_song("song_no_reverb_chorus", synth, "example/sf_/GMGSx_2.sf2");
    assert_gen_GMGSx_2(synth);
    delete_fluid_synth(synth);

    synth = NEW_FLUID_SYNTH(.with_reverb=true, .with_chorus=true);
    assert_gen_zero(synth);
    assert(synth->with_reverb);
    assert(synth->with_chorus);
    assert(synth->reverb != NULL);
    assert(synth->chorus != NULL);
    assert(synth->fx_left_buf != NULL);
    assert(synth->fx_left_buf2 != NULL);
    gen_song("song_reverb_chorus", synth, "example/sf_/GMGSx_2.sf2");
    assert_gen_GMGSx_2(synth);
    delete_fluid_synth(synth);

    synth = NEW_FLUID_SYNTH(.with_reverb=true, .with_chorus=true);
    assert_gen_zero(synth);
    //soundfont里的reverb_send/chorus_send, 和代码里的with_reverb/with_chorus是独立的。
    //soundfont里没有设置reverb_send/chorus_send，下面的声音就没效果
    gen_song("song_reverb_chorus_but_no_effect", synth, "example/sf_/GMGSx_1.sf2");
    assert_gen_zero(synth);
    assert(synth->voice[0]->mod_count == 9);
    delete_fluid_synth(synth);

    synth = NEW_FLUID_SYNTH(.with_reverb=true, .with_chorus=true);
    assert_gen_zero(synth);
    fluid_synth_set_gen(synth, 0, GEN_REVERBSEND, 0.8); 
    fluid_synth_set_gen(synth, 0, GEN_CHORUSSEND, 0.7); 
    assert(synth->channel[0]->gen[GEN_REVERBSEND] == 800);
    assert(synth->channel[0]->gen[GEN_CHORUSSEND] == 700);
    assert_gen_zero(synth);
#ifdef USING_CALLOC
    assert(synth->voice[0]->gen[GEN_REVERBSEND].val == 0);
    assert(synth->voice[0]->gen[GEN_CHORUSSEND].val == 0);
#endif
    // 这段代码未执行：if (voice->chan == chan) fluid_voice_set_param(voice, param, v, absolute);
    // voice->chan  = 255 NO_CHANNEL， chan=0， 所以voice里的gen未设置。

    gen_song("song_reverb_chorus_Boomwhacker", synth, "example/sf_/GMGSx_1.sf2");
    assert(synth->voice[0]->amp_reverb > 1e-6);
    assert(synth->voice[0]->gen[GEN_REVERBSEND].nrpn == 800);
    assert(float_eq(synth->voice[0]->reverb_send, 0.8));
    assert(synth->voice[0]->amp_chorus > 1e-6);
    assert(synth->voice[0]->gen[GEN_CHORUSSEND].nrpn == 700);
    assert(float_eq(synth->voice[0]->chorus_send, 0.7));
    delete_fluid_synth(synth);

    synth = NEW_FLUID_SYNTH(.with_reverb=true, .with_chorus=true);
    assert_gen_zero(synth);
    fluid_synth_cc(synth, 0, GEN_REVERBSEND, 0.2); //没有效果！因为voice->chan还未初始化。要等到note_on以后，才会调用fluid_voice_init。
    fluid_synth_cc(synth, 0, GEN_CHORUSSEND, 0.2); 
    assert_gen_zero(synth);
    gen_song("song_reverb_chorus_using_cc", synth, "example/sf_/GMGSx_1.sf2");
    assert_gen_zero(synth);

    return 0;
}