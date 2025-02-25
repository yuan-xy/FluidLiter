#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <stdbool.h>
#include "GMGSx_1.h"
#include "fluidliter.h"
#include "fluid_synth.h"


#define SF_SIZE sizeof(SF_BIN)


struct FileDescriptor {
    char name[256];
    void *orig;
    int size;
    void *ptr;
};
struct FileDescriptor fd;

void *my_open(fluid_fileapi_t* fileapi, const char * filename)
{
    if(filename[0] != '&')
    {
        return NULL;
    }

    sscanf(filename, "&%s", fd.name);
    fd.size = SF_SIZE;
    sscanf(filename, "&%p", &(fd.ptr));
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

int main(int argc, char *argv[]) {
    assert(GEN_LAST==60);
    int err = 0;
    fluid_set_default_fileapi(&my_fileapi);
    fluid_synth_t* synth = NEW_FLUID_SYNTH();
    set_log_level(FLUID_DBG);
    assert(synth->with_reverb == 1);

    char abused_filename[64];
    const void *pointer_to_sf2_in_mem = &SF_BIN;
    sprintf(abused_filename, "&%p", pointer_to_sf2_in_mem);

    int id = fluid_synth_sfload(synth, abused_filename, 1); //这里必须用1, 否则没有prog设置，也没有pcm输出    

    if(id == FLUID_FAILED)
    {
        puts("oops");
        err = -1;
        goto cleanup;
    }

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE sizeof(int16_t) //s16
#define DURATION 0.01 //second
#define NUM_FRAMES SAMPLE_RATE*DURATION
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    FILE* file = fopen("test2.pcm", "wb");

    fluid_synth_noteon(synth, 0, 60, 127);

    for(int i=0; i<500; i++){
        fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
        fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);
    }


    fluid_synth_noteoff(synth, 0, 60);
    fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);


    for(int j=1; j<10; j++){
        fluid_synth_noteon(synth, 0, 60+j, 127);
        for(int i=0; i<100; i++){
            fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
            fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);
        }
        fluid_synth_noteoff(synth, 0, 60+j);
        fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
        fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);        
    }

    fclose(file);
    // sleep(1);
    free(buffer);
    system("ffmpeg -y -f s16le -ar 44100 -ac 2 -i test2.pcm test2.wav");


cleanup:
    /* deleting the synth also deletes my_sfloader */
    delete_fluid_synth(synth);
    return err;
}
