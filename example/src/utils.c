#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "fluid_synth.h"
#include "fluid_chan.h"
#include "fluid_defsfont.h"
#include "fluid_sfont.h"

const char* get_gen_name(int gen_id);
void get_flag_names(char flags, char* output, size_t output_size);
const char* get_mod_src_name(int src);

#define print_gen(iz, desc) for(int i=0; i<GEN_LAST; i++){ \
      fluid_gen_t gen = iz->gen[i];\
      if(!gen.flags) continue;\
      printf("%s %d_%s,\t n_v:%.2f, \tmod:%.2f, \tnrpn:%.2f\n", desc, i, get_gen_name(i), gen.val, gen.mod, gen.nrpn);\
    }\

static char flags1_names[256];
static char flags2_names[256];

#define print_mod(iz, desc)    fluid_mod_t * mod = iz->mod;\
    while(mod != NULL){\
        get_flag_names(mod->flags1, flags1_names, 256),\
        get_flag_names(mod->flags2, flags2_names, 256),\
        printf("\t%s:%s,\t%s, %i-%s,\t %s, %i-%s, \tamount:%.2f\n",\
            desc, get_gen_name(mod->dest), get_mod_src_name(mod->src1), mod->flags1, flags1_names, \
            get_mod_src_name(mod->src2), mod->flags2, flags2_names, mod->amount);\
        mod = mod->next;\
    }\

void print_fluid_preset_zone_global(fluid_preset_zone_t *pz){
    if(pz == NULL) return;
    printf("global_preset_zone: %s,\t keylo:%d, hi:%d,\t velLo:%d, hi:%d\n",
      pz->name, pz->keylo, pz->keyhi, pz->vello, pz->velhi);
    
    print_gen(pz, "global_preset_gen");
    print_mod(pz, "global_preset_mod");

    assert(pz->inst==NULL);
    assert(pz->next==NULL);
}

void print_fluid_preset_zone(fluid_preset_zone_t *pz){
    printf("preset_zone: %s,\t keylo:%d, hi:%d,\t velLo:%d, hi:%d\n",
      pz->name, pz->keylo, pz->keyhi, pz->vello, pz->velhi);

    print_gen(pz, "preset_gen");
    print_mod(pz, "preset_mod");
}


void print_fluid_inst_zone_global(fluid_inst_zone_t *iz){
    if(iz == NULL) return;
    printf("global_inst_zone: %s,\t keylo:%d, hi:%d,\t velLo:%d, hi:%d\n",
      iz->name, iz->keylo, iz->keyhi, iz->vello, iz->velhi);

    print_gen(iz, "global_inst_gen");
    print_mod(iz, "global_inst_mod");

    assert(iz->sample==NULL);
    assert(iz->next==NULL);
}

void print_fluid_inst_zone(fluid_inst_zone_t *iz){
    printf("\tinst_zone:\t%s,\t keylo:%d, hi:%d,\t velLo:%d, hi:%d\n",
      iz->name, iz->keylo, iz->keyhi, iz->vello, iz->velhi);
    
    print_gen(iz, "\tinst_gen");
    print_mod(iz, "inst_mod");

    fluid_sample_t* sample = iz->sample;
    printf("\tsample: %s, \t%d, %d, origpitch:%d_%d\t s_end:%d_%d\t loop:%d_%d\n", 
          sample->name, sample->samplerate,
          sample->sampletype, sample->origpitch, sample->pitchadj,
          sample->start, sample->end, sample->loopstart, sample->loopend
          );
}

void print_preset_info(fluid_preset_t *preset){
    int bank = fluid_preset_get_banknum(preset);
    int prog = fluid_preset_get_num(preset);
    const char* name = fluid_preset_get_name(preset);
	printf("bank: %d prog: %d name: %s\n", bank, prog, name);

    fluid_defpreset_t * defpreset = (fluid_defpreset_t *)preset->data;
    print_fluid_preset_zone_global(defpreset->global_zone);    
    fluid_preset_zone_t *preset_zone = defpreset->zone;
    print_fluid_preset_zone(preset_zone);

    fluid_inst_t *inst = preset_zone->inst;
    // printf("fluid_inst_t->name: %s\n", inst->name);  

    print_fluid_inst_zone_global(inst->global_zone);

    fluid_inst_zone_t *cur_zone = inst->zone;
    while(cur_zone != NULL){
      print_fluid_inst_zone(cur_zone);
      cur_zone = cur_zone->next;
    }
}


const char* getMIDIControlName(int cc) {
    switch (cc) {
        case 0: return "Bank Select";
        case 1: return "Modulation Wheel";
        case 2: return "Breath Controller";
        case 3: return "Undefined";
        case 4: return "Foot Controller";
        case 5: return "Portamento Time";
        case 6: return "Data Entry MSB";
        case 7: return "Channel Volume";
        case 8: return "Balance";
        case 9: return "Undefined";
        case 10: return "Pan";
        case 11: return "Expression Controller";
        case 12: return "Effect Control 1";
        case 13: return "Effect Control 2";
        case 14: return "Undefined";
        case 15: return "Undefined";
        case 16: return "General Purpose Controller 1";
        case 17: return "General Purpose Controller 2";
        case 18: return "General Purpose Controller 3";
        case 19: return "General Purpose Controller 4";
        case 20: return "Undefined";
        case 21: return "Undefined";
        case 22: return "Undefined";
        case 23: return "Undefined";
        case 24: return "Undefined";
        case 25: return "Undefined";
        case 26: return "Undefined";
        case 27: return "Undefined";
        case 28: return "Undefined";
        case 29: return "Undefined";
        case 30: return "Undefined";
        case 31: return "Undefined";
        case 32: return "LSB for Control 0 (Bank Select)";
        case 33: return "LSB for Control 1 (Modulation Wheel)";
        case 34: return "LSB for Control 2 (Breath Controller)";
        case 35: return "LSB for Control 3 (Undefined)";
        case 36: return "LSB for Control 4 (Foot Controller)";
        case 37: return "LSB for Control 5 (Portamento Time)";
        case 38: return "LSB for Control 6 (Data Entry)";
        case 39: return "LSB for Control 7 (Channel Volume)";
        case 40: return "LSB for Control 8 (Balance)";
        case 41: return "LSB for Control 9 (Undefined)";
        case 42: return "LSB for Control 10 (Pan)";
        case 43: return "LSB for Control 11 (Expression Controller)";
        case 44: return "LSB for Control 12 (Effect Control 1)";
        case 45: return "LSB for Control 13 (Effect Control 2)";
        case 46: return "LSB for Control 14 (Undefined)";
        case 47: return "LSB for Control 15 (Undefined)";
        case 48: return "LSB for Control 16 (General Purpose Controller 1)";
        case 49: return "LSB for Control 17 (General Purpose Controller 2)";
        case 50: return "LSB for Control 18 (General Purpose Controller 3)";
        case 51: return "LSB for Control 19 (General Purpose Controller 4)";
        case 52: return "LSB for Control 20 (Undefined)";
        case 53: return "LSB for Control 21 (Undefined)";
        case 54: return "LSB for Control 22 (Undefined)";
        case 55: return "LSB for Control 23 (Undefined)";
        case 56: return "LSB for Control 24 (Undefined)";
        case 57: return "LSB for Control 25 (Undefined)";
        case 58: return "LSB for Control 26 (Undefined)";
        case 59: return "LSB for Control 27 (Undefined)";
        case 60: return "LSB for Control 28 (Undefined)";
        case 61: return "LSB for Control 29 (Undefined)";
        case 62: return "LSB for Control 30 (Undefined)";
        case 63: return "LSB for Control 31 (Undefined)";
        case 64: return "Damper Pedal (Sustain)";
        case 65: return "Portamento On/Off";
        case 66: return "Sostenuto";
        case 67: return "Soft Pedal";
        case 68: return "Legato Footswitch";
        case 69: return "Hold 2";
        case 70: return "Sound Controller 1 (Timbre)";
        case 71: return "Sound Controller 2 (Brightness)";
        case 72: return "Sound Controller 3";
        case 73: return "Sound Controller 4";
        case 74: return "Sound Controller 5";
        case 75: return "Sound Controller 6";
        case 76: return "Sound Controller 7";
        case 77: return "Sound Controller 8";
        case 78: return "Sound Controller 9";
        case 79: return "Sound Controller 10";
        case 80: return "General Purpose Controller 5";
        case 81: return "General Purpose Controller 6";
        case 82: return "General Purpose Controller 7";
        case 83: return "General Purpose Controller 8";
        case 84: return "Portamento Control";
        case 85: return "Undefined";
        case 86: return "Undefined";
        case 87: return "Undefined";
        case 88: return "Undefined";
        case 89: return "Undefined";
        case 90: return "Undefined";
        case 91: return "Effects 1 Depth";
        case 92: return "Effects 2 Depth";
        case 93: return "Effects 3 Depth";
        case 94: return "Effects 4 Depth";
        case 95: return "Effects 5 Depth";
        case 96: return "Data Increment";
        case 97: return "Data Decrement";
        case 98: return "Non-Registered Parameter Number LSB";
        case 99: return "Non-Registered Parameter Number MSB";
        case 100: return "Registered Parameter Number LSB";
        case 101: return "Registered Parameter Number MSB";
        case 102: return "Undefined";
        case 103: return "Undefined";
        case 104: return "Undefined";
        case 105: return "Undefined";
        case 106: return "Undefined";
        case 107: return "Undefined";
        case 108: return "Undefined";
        case 109: return "Undefined";
        case 110: return "Undefined";
        case 111: return "Undefined";
        case 112: return "Undefined";
        case 113: return "Undefined";
        case 114: return "Undefined";
        case 115: return "Undefined";
        case 116: return "Undefined";
        case 117: return "Undefined";
        case 118: return "Undefined";
        case 119: return "Undefined";
        case 120: return "All Sound Off";
        case 121: return "Reset All Controllers";
        case 122: return "Local Control";
        case 123: return "All Notes Off";
        case 124: return "Omni Mode Off";
        case 125: return "Omni Mode On";
        case 126: return "Mono Mode On";
        case 127: return "Poly Mode On";
        default: return "Invalid CC value (must be 0-127)";
    }
}


void print_cc_values(fluid_synth_t* synth, int ch) {
    fluid_channel_t *chan = synth->channel[ch];
    printf("CC Values for Channel %d:\n", chan->channum);
    for (int i = 0; i < 128; i++) {
        if(chan->cc[i]!=0) printf("CC %3d %s: \t %3d\n", i, getMIDIControlName(i), chan->cc[i]);
    }
}


// 生成器名称数组
const char* gen_names[] = {
    "GEN_STARTADDROFS",          // 0
    "GEN_ENDADDROFS",            // 1
    "GEN_STARTLOOPADDROFS",      // 2
    "GEN_ENDLOOPADDROFS",        // 3
    "GEN_STARTADDRCOARSEOFS",    // 4
    "GEN_MODLFOTOPITCH",         // 5
    "GEN_VIBLFOTOPITCH",         // 6
    "GEN_MODENVTOPITCH",         // 7
    "GEN_FILTERFC",              // 8
    "GEN_FILTERQ",               // 9
    "GEN_MODLFOTOFILTERFC",      // 10
    "GEN_MODENVTOFILTERFC",      // 11
    "GEN_ENDADDRCOARSEOFS",      // 12
    "GEN_MODLFOTOVOL",           // 13
    "GEN_UNUSED1",               // 14
    "GEN_CHORUSSEND",            // 15
    "GEN_REVERBSEND",            // 16
    "GEN_PAN",                   // 17
    "GEN_UNUSED2",               // 18
    "GEN_UNUSED3",               // 19
    "GEN_UNUSED4",               // 20
    "GEN_MODLFODELAY",           // 21
    "GEN_MODLFOFREQ",            // 22
    "GEN_VIBLFODELAY",           // 23
    "GEN_VIBLFOFREQ",            // 24
    "GEN_MODENVDELAY",           // 25
    "GEN_MODENVATTACK",          // 26
    "GEN_MODENVHOLD",            // 27
    "GEN_MODENVDECAY",           // 28
    "GEN_MODENVSUSTAIN",         // 29
    "GEN_MODENVRELEASE",         // 30
    "GEN_KEYTOMODENVHOLD",       // 31
    "GEN_KEYTOMODENVDECAY",      // 32
    "GEN_VOLENVDELAY",           // 33
    "GEN_VOLENVATTACK",          // 34
    "GEN_VOLENVHOLD",            // 35
    "GEN_VOLENVDECAY",           // 36
    "GEN_VOLENVSUSTAIN",         // 37
    "GEN_VOLENVRELEASE",         // 38
    "GEN_KEYTOVOLENVHOLD",       // 39
    "GEN_KEYTOVOLENVDECAY",      // 40
    "GEN_INSTRUMENT",            // 41
    "GEN_RESERVED1",             // 42
    "GEN_KEYRANGE",              // 43
    "GEN_VELRANGE",              // 44
    "GEN_STARTLOOPADDRCOARSEOFS",// 45
    "GEN_KEYNUM",                // 46
    "GEN_VELOCITY",              // 47
    "GEN_ATTENUATION",           // 48
    "GEN_RESERVED2",             // 49
    "GEN_ENDLOOPADDRCOARSEOFS",  // 50
    "GEN_COARSETUNE",            // 51
    "GEN_FINETUNE",              // 52
    "GEN_SAMPLEID",              // 53
    "GEN_SAMPLEMODE",            // 54
    "GEN_RESERVED3",             // 55
    "GEN_SCALETUNE",             // 56
    "GEN_EXCLUSIVECLASS",        // 57
    "GEN_OVERRIDEROOTKEY",       // 58
    "GEN_PITCH",                 // 59
    "GEN_LAST"                   // 60
};

// 打印生成器名称的函数
const char* get_gen_name(int gen_id) {
    if (gen_id >= 0 && gen_id <= GEN_LAST) {
        return gen_names[gen_id];
    } else {
        return "Invalid generator ID";
    }
}

// 将 fluid_mod_flags 转换为名称字符串
// (1) 极性（Polarity）
// FLUID_MOD_POSITIVE (0)：
// 调制信号为正方向。
// 例如，调制轮的值增加时，目标参数的值也增加。
// FLUID_MOD_NEGATIVE (1)：
// 调制信号为负方向。
// 例如，调制轮的值增加时，目标参数的值减少。

// (2) 映射方式（Mapping Type）
// FLUID_MOD_UNIPOLAR (0)：
// 单极性映射。
// 调制信号的范围为 0 到最大值。
// FLUID_MOD_BIPOLAR (2)：
// 双极性映射。
// 调制信号的范围为负最大值到正最大值。
//  (3)
// FLUID_MOD_LINEAR (0)：
// 线性映射。
// 调制信号与目标参数的关系是线性的。
// FLUID_MOD_CONCAVE (4)：
// 凹曲线映射。
// 调制信号的变化在低值时较快，在高值时较慢。
// FLUID_MOD_CONVEX (8)：
// 凸曲线映射。
// 调制信号的变化在低值时较慢，在高值时较快。
// FLUID_MOD_SWITCH (12)：
// 开关映射。
// 调制信号只有两个状态（开或关）。

// (4) 调制源类型（Source Type）
// FLUID_MOD_GC (0)：
// 调制源是通用控制器（General Controller, GC）。
// 通用控制器是 SoundFont 2 规范中定义的特殊控制器。
// FLUID_MOD_CC (16)：
// 调制源是 MIDI 控制改变消息（Control Change, CC）。
// 例如，调制轮（CC1）、音量（CC7）等。

void get_flag_names(char flags, char* output, size_t output_size) {
    char buffer[256] = {0}; // 临时缓冲区

    // 极性组
    const char* polarity = (flags & 1) ? "NEGATIVE" : "POSITIVE";

    // 映射方式组
    const char* mapping = (flags & 2) ? "BIPOLAR" : "UNIPOLAR";

    // 曲线类型组
    const char* curve = "LINEAR"; // 默认值
    if (flags & 4) {
        curve = "CONCAVE";
    } else if (flags & 8) {
        curve = "CONVEX";
    } else if (flags & 12) {
        curve = "SWITCH";
    }
    const char* source = (flags & 16) ? "CC" : "GC";

    snprintf(buffer, sizeof(buffer), "%s|%s|%s|%s", polarity, mapping, curve, source);
    strncpy(output, buffer, output_size - 1);
    output[output_size - 1] = '\0';
}

const char* get_mod_src_name(int src) {
    switch (src) {
        case FLUID_MOD_NONE: return "SRC_NONE";
        case FLUID_MOD_VELOCITY: return "SRC_VELOCITY";
        case FLUID_MOD_KEY: return "SRC_KEY";
        case FLUID_MOD_KEYPRESSURE: return "SRC_KEYPRESSURE";
        case FLUID_MOD_CHANNELPRESSURE: return "SRC_CHANNELPRESSURE";
        case FLUID_MOD_PITCHWHEEL: return "SRC_PITCHWHEEL";
        case FLUID_MOD_PITCHWHEELSENS: return "SRC_PITCHWHEELSENS";
        default: return "Unknown fluid_mod_src";
    }
}

void print_gen_values(fluid_synth_t* synth, int ch, bool all) {
    fluid_channel_t *chan = synth->channel[ch];
    printf("Generator for Channel %d:\n", chan->channum);
    for (int i = 0; i < GEN_LAST; i++) {
        if(all || chan->gen[i]!=0)
        printf("GEN_%d: %s, value=%f, \t gen_abs=%d\n", i, get_gen_name(i), chan->gen[i], chan->gen_abs[i]);
    }
}
