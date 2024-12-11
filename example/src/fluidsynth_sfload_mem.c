#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fluidlite.h"

typedef enum {
  FLUID_OK = 0,
  FLUID_FAILED = -1
} fluid_status;

#define TEN0 0,0,0,0,0,0,0,0,0,0
const static unsigned char MinimalSoundFont[] =
{
	'R','I','F','F',220,1,0,0,'s','f','b','k',
	'L','I','S','T',88,1,0,0,'p','d','t','a',
	'p','h','d','r',76,TEN0,TEN0,TEN0,TEN0,0,0,0,0,TEN0,0,0,0,0,0,0,0,255,0,255,0,1,TEN0,0,0,0,
	'p','b','a','g',8,0,0,0,0,0,0,0,1,0,0,0,'p','m','o','d',10,TEN0,0,0,0,'p','g','e','n',8,0,0,0,41,0,0,0,0,0,0,0,
	'i','n','s','t',44,TEN0,TEN0,0,0,0,0,0,0,0,0,TEN0,0,0,0,0,0,0,0,1,0,
	'i','b','a','g',8,0,0,0,0,0,0,0,2,0,0,0,'i','m','o','d',10,TEN0,0,0,0,
	'i','g','e','n',12,0,0,0,54,0,1,0,53,0,0,0,0,0,0,0,
	's','h','d','r',92,TEN0,TEN0,0,0,0,0,0,0,0,50,0,0,0,0,0,0,0,49,0,0,0,34,86,0,0,60,0,0,0,1,TEN0,TEN0,TEN0,TEN0,0,0,0,0,0,0,0,
	'L','I','S','T',112,0,0,0,'s','d','t','a','s','m','p','l',100,0,0,0,86,0,119,3,31,7,147,10,43,14,169,17,58,21,189,24,73,28,204,31,73,35,249,38,46,42,71,46,250,48,150,53,242,55,126,60,151,63,108,66,126,72,207,
		70,86,83,100,72,74,100,163,39,241,163,59,175,59,179,9,179,134,187,6,186,2,194,5,194,15,200,6,202,96,206,159,209,35,213,213,216,45,220,221,223,76,227,221,230,91,234,242,237,105,241,8,245,118,248,32,252
};

#define SF_SIZE  302328

static unsigned char example_sf2[SF_SIZE];

void read_example_sf2(){
    FILE *file;
    size_t fileSize, bytesRead;

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
  NULL,
  my_open,
  my_read,
  my_seek,
  my_close,
  my_tell
};

int main(int argc, char *argv[]) {
    int err = 0;

    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "yes");
    fluid_synth_t *synth = new_fluid_synth(settings);

    fluid_set_default_fileapi(&my_fileapi);
    fluid_sfloader_t *my_sfloader = new_fluid_defsfloader();
//   loader->fileapi = fluid_default_fileapi;
//   loader->free = delete_fluid_defsfloader;
//   loader->load = fluid_defsfloader_load;

    fluid_synth_add_sfloader(synth, my_sfloader);


    char abused_filename[64];
    // const void *pointer_to_sf2_in_mem = &MinimalSoundFont;
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
#define DURATION 2 //second
#define NUM_FRAMES SAMPLE_RATE*DURATION
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    FILE* file = argc > 1 ? fopen(argv[1], "wb") : stdout;

    fluid_synth_noteon(synth, 0, 60, 127);
    fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);


    fluid_synth_noteoff(synth, 0, 60);
    fluid_synth_write_s16(synth, NUM_FRAMES/10, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES/10, file);

    fclose(file);

    free(buffer);


cleanup:
    /* deleting the synth also deletes my_sfloader */
    delete_fluid_synth(synth);

    delete_fluid_settings(settings);

    return err;
}
