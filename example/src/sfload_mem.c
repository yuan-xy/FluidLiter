#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#include "fluidlite.h"
#include "fluid_synth.h"


// #define SF_SIZE  290914808 //Boomwhacker.sf2
#define SF_SIZE  302328 //Boomwhacker.sf2

static unsigned char example_sf2[SF_SIZE];

void read_example_sf2(){
    FILE *file;
    size_t fileSize, bytesRead;

    // file = fopen("/mnt/d/SF2/X Piano SoundFont v1.0.1.sf2", "rb");
    file = fopen("./example/sf_/Boomwhacker.sf2", "rb");
    if (file == NULL) {
        printf("Error opening file example/sf_/Boomwhacker.sf2\n");
    }
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    bytesRead = fread(example_sf2, 1, fileSize, file);
    if (bytesRead != fileSize || fileSize != SF_SIZE) {
        printf("Error reading file.\n");
    }
    fclose(file);
}

struct FileDescriptor {
    char name[256];
    void *orig;
    int size;
    void *ptr;
};
struct FileDescriptor fd;

void *my_open(fluid_fileapi_t* fileapi, const char * filename)
{
    void *p;

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

    fluid_synth_t *synth = NEW_FLUID_SYNTH(.log_level=FLUID_DBG);
    assert(synth->verbose == 1);
    assert(synth->with_reverb == 1);

    fluid_set_default_fileapi(&my_fileapi);
    fluid_sfloader_t *my_sfloader = new_fluid_defsfloader();
//   loader->fileapi = fluid_default_fileapi;
//   loader->free = delete_fluid_defsfloader;
//   loader->load = fluid_defsfloader_load;

    fluid_synth_add_sfloader(synth, my_sfloader);


    char abused_filename[64];
    read_example_sf2();
    const void *pointer_to_sf2_in_mem = &example_sf2;
    sprintf(abused_filename, "&%p", pointer_to_sf2_in_mem);

    int id = fluid_synth_sfload(synth, abused_filename, 1); //这里必须用1, 否则没有prog设置，也没有pcm输出    
    /* now my_open() will be called with abused_filename and should have opened the memory region */

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
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    FILE* file = argc > 1 ? fopen(argv[1], "wb") : stdout;

    fluid_synth_noteon(synth, 0, 60, 127);

    for(int i=0; i<900; i++){
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
    sleep(1);
    free(buffer);


cleanup:
    /* deleting the synth also deletes my_sfloader */
    delete_fluid_synth(synth);
    return err;
}
