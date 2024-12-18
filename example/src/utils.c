#include <assert.h>

#include "fluid_synth.h"
#include "fluid_chan.h"
#include "fluid_defsfont.h"
#include "fluid_sfont.h"

#define print_gen(iz, desc) for(int i=0; i<GEN_LAST; i++){ \
      fluid_gen_t gen = iz->gen[i];\
      if(!gen.flags) continue;\
      printf("%s %d,\t nominal value:%.2f, \tmod:%.2f, \tnrpn:%.2f\n", desc, i, gen.val, gen.mod, gen.nrpn);\
    }\


#define print_mod(iz, desc)    fluid_mod_t * mod = iz->mod;\
    while(mod != NULL){\
        printf("\t%s:\t%i,%i,%i,%i,%i,\tamount:%.2f\n",\
            desc, mod->src1, mod->src2, mod->flags1, mod->flags2, mod->dest, mod->amount);\
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
    printf("\tsample: %s, \t%d, %d, origpitch:%d\t s_end:%d_%d\t loop:%d_%d\n", 
          sample->name, sample->samplerate,
          sample->sampletype, sample->origpitch,
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
