#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "fluidlite.h"

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
    
/**
$ gdb --args fluidlite-test /mnt/c/tools/fluidsynth/bin/GeneralUser-GS.sf2 out.pcm
(gdb) break new_fluid_synth
(gdb) break main.c:27  #注意要gcc编译时带-g才能在gdb里指定main.c的行断点
(gdb) print *settings
$1 = {size = 7, nnodes = 2, nodes = 0x5555555852c0, del = 0x555555555a7e <fluid_settings_hash_delete>}
**/

    fluid_synth_t* synth = new_fluid_synth(settings);
/**
$4 = {settings = 0x5555555852a0, polyphony = 256, with_reverb = 1 '\001', with_chorus = 1 '\001',
  verbose = 0 '\000', dump = 0 '\000', sample_rate = 44100, midi_channels = 16, audio_channels = 1,
  audio_groups = 1, effects_channels = 2, state = 1, ticks = 0, loaders = 0x555555586710, sfont = 0x0,
  sfont_id = 0, bank_offsets = 0x0, gain = 0.20000000298023224, channel = 0x555555586730, num_channels = 0,
  nvoice = 256, voice = 0x5555555896c0, noteid = 0, storeid = 0, nbuf = 1, left_buf = 0x555555686ed0,
  right_buf = 0x555555686ef0, fx_left_buf = 0x555555687130, fx_right_buf = 0x555555687150, reverb = 0x5555556875b0,
  chorus = 0x5555556a0640, cur = 64, outbuf = '\000' <repeats 255 times>, tuning = 0x0, cur_tuning = 0x0,
  min_note_length_ticks = 441}
 **/


    fluid_synth_sfload(synth, argv[1], 1);
/**
(gdb) p *loader
$8 = {data = 0x0, free = 0x555555566dda <delete_fluid_defsfloader>, load = 0x555555566e04 <fluid_defsfloader_load>,
  fileapi = 0x55555557bc80 <default_fileapi>}
(gdb) p *sfont
$10 = {data = 0x5555556a33c0, id = 0, free = 0x555555566f0e <fluid_defsfont_sfont_delete>,
  get_name = 0x555555566f4b <fluid_defsfont_sfont_get_name>,
  get_preset = 0x555555566f6c <fluid_defsfont_sfont_get_preset>,
  iteration_start = 0x555555567057 <fluid_defsfont_sfont_iteration_start>,
  iteration_next = 0x555555567079 <fluid_defsfont_sfont_iteration_next>}

*/


    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    FILE* file = argc > 2 ? fopen(argv[2], "wb") : stdout;

    fluid_synth_noteon(synth, 0, 60, 127);
    fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);


    fluid_synth_noteoff(synth, 0, 60);
    fluid_synth_write_s16(synth, NUM_FRAMES/10, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES/10, file);

    fclose(file);

    free(buffer);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}
