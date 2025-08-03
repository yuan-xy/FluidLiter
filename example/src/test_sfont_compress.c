#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// READ_SAMPLE这里定义无效，因为FluidLiter是做为库被引用的。
// #define READ_SAMPLE(base, pos) (printf("sample:%d[%d]\n", base, pos), base[pos])

#include "fluidliter.h"
#include "fluid_sfont.h"
#include "fluid_synth.h"

bool compress_cb(char *buffer, int compressed_size, char *orig_buf, int orig_size){
    memcpy(buffer, orig_buf, compressed_size); //直接截取前compressed_size，实际未执行压缩
    return true;
}

bool decompress_cb(char *buffer, int compressed_size, char *orig_buf, int orig_size){
    memcpy(orig_buf, buffer, compressed_size);
    memcpy(orig_buf+compressed_size, buffer, compressed_size);
    memcpy(orig_buf+compressed_size*2, buffer, compressed_size);
    memcpy(orig_buf+compressed_size*3, buffer, compressed_size);
    return true;
}

int main(int argc, char *argv[]) {
    char *filename = "tmp.sf3";
    assert(compress_sf2("example/sf_/GMGSx_2.sf2", filename, compress_cb));

    fluid_sfont_set_decompress_callback(decompress_cb);
    fluid_synth_t* synth = NEW_FLUID_SYNTH();
    int sfont_id = fluid_synth_sfload(synth, filename, 1);
    assert(sfont_id != FLUID_FAILED);
    fluid_sfont_t *sfont = synth->sfont->data;
    assert(sfont->is_rom == 0);
    assert(sfont->is_compressed == 1);
    fluid_synth_program_select(synth, 0, sfont_id, 0, 0);

    FILE* file = fopen("compressed.pcm", "wb");
    for(int i=38; i<100; i+=5){
        int frame = 44100;
        uint16_t buffer[frame];
        fluid_synth_noteon(synth, 0, i, 100);
        fluid_synth_write_s16(synth, frame, buffer, 0, 1, NULL, 0, 0);
        assert(buffer[100] != 0); //取中间一段，assert其不是静音
        assert(buffer[101] != 0);
        assert(buffer[102] != 0);
        assert(buffer[103] != 0);
        fwrite(buffer, sizeof(uint16_t), frame, file);
        fluid_synth_noteoff(synth, 0, i);
    }
    fclose(file);
    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 2 -i compressed.pcm compressed.wav >nul 2>&1");

    delete_fluid_synth(synth);
}
