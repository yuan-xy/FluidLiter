#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "fluidliter.h"

#include "GMGSx_1.h"

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE sizeof(fluid_real_t)
#define DURATION 0.1
#define NUM_FRAMES 44100*5
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

static fluid_real_t WAV_BUFFER[NUM_SAMPLES];
 
#define SF_SIZE sizeof(SF_BIN)
#define SF_ADDR &SF_BIN

struct FileDescriptor {
    void *orig;
    int size;
    void *ptr;
};
struct FileDescriptor fd;

void *my_open(fluid_fileapi_t* fileapi, const char * filename){
    fd.size = SF_SIZE;
    fd.ptr = (void *)SF_ADDR;
    fd.orig = fd.ptr;
    return &fd;
}

int my_read(void *buf, int count, void *handle)
{
    struct FileDescriptor *fdp = (struct FileDescriptor *)handle;
    memcpy(buf, fdp->ptr, count);
    fdp->ptr += count;
    return FLUID_OK;
}

void* my_read_zero_memcpy(int count, void *handle)
{
    struct FileDescriptor *fdp = (struct FileDescriptor *)handle;
    void* ret = fdp->ptr;
    fdp->ptr += count;
    return ret;
}

int my_seek(void *handle, long offset, int origin)
{
    struct FileDescriptor *fdp = (struct FileDescriptor *)handle;
    switch (origin)
    {
    case SEEK_SET:
        fdp->ptr = fdp->orig + offset;
        break;
    case SEEK_CUR:
        fdp->ptr += offset;
        break;
    case SEEK_END:
        fdp->ptr = fdp->orig + offset + SF_SIZE;
        break;
    default:
        printf("unknown seek origin.");
        exit(1);
        break;
    };
    return FLUID_OK;
}

int my_close(void *handle)
{
    return FLUID_OK;
}

long my_tell(void *handle)
{
    struct FileDescriptor *fdp = (struct FileDescriptor *)handle;
    return fdp->ptr - fdp->orig;
}

static fluid_fileapi_t my_fileapi =
{
  NULL,
  my_open,
  my_read,
  my_read_zero_memcpy,
  my_seek,
  my_close,
  my_tell
};

static fluid_synth_t *synth = NULL;
static int sfont_id = -1;

int fluid_init(bool reverb, bool chorus) {
    fluid_set_default_fileapi(&my_fileapi);
    synth = NEW_FLUID_SYNTH(.with_reverb=reverb, .with_chorus=chorus, .gain=0.4f);
    if(synth == NULL) return FLUID_FAILED;
    sfont_id = fluid_synth_sfload(synth, "", 1); //这里必须用1, 否则没有prog设置，也没有pcm输出
    if(sfont_id == FLUID_FAILED){
        printf("oops FLUID_FAILED!\n");
    }
    return sfont_id;
}

int fluid_program_select(unsigned int bank_num, unsigned int preset_num) {
    return fluid_synth_program_select(synth, 0, sfont_id, bank_num, preset_num);
}

int fluid_noteon(int key, int vel) {
    return fluid_synth_noteon(synth, 0, key, vel);
}

int fluid_noteoff(int key) {
    return fluid_synth_noteoff(synth, 0, key);
}

// int fluid_write_s16() {
//     return fluid_synth_write_s16(synth, NUM_FRAMES, WAV_BUFFER, 0, NUM_CHANNELS, WAV_BUFFER, 1, NUM_CHANNELS);
// }

int fluid_write_float() {
    return fluid_synth_write_float(synth, NUM_FRAMES, WAV_BUFFER, 0, NUM_CHANNELS, WAV_BUFFER, 1, NUM_CHANNELS);
}

int16_t* get_buffer_ptr() {
    return WAV_BUFFER;
}

int get_buffer_size() {
    return NUM_SAMPLES;
}

