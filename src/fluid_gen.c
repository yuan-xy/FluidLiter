
#include "fluid_gen.h"
#include "fluid_chan.h"

/**
 * Set an array of generators to their default values.
 * @param gen Array of generators (should be #GEN_LAST in size).
 * @return Always returns 0
 */
int fluid_gen_set_default_values(fluid_gen_t *gen) {
    int i;

    for (i = 0; i < GEN_LAST; i++) {
        gen[i].flags = GEN_UNUSED;
        gen[i].mod = 0.0;
        gen[i].nrpn = 0.0;
        gen[i].val = fluid_gen_info[i].def;
    }

    return FLUID_OK;
}

/* fluid_gen_init
 *
 * Set an array of generators to their initial value
 */
int fluid_gen_init(fluid_gen_t *gen, fluid_channel_t *channel) {
    int i;

    fluid_gen_set_default_values(gen);

    for (i = 0; i < GEN_LAST; i++) {
        gen[i].nrpn = fluid_channel_get_gen(channel, i);

        /* This is an extension to the SoundFont standard. More
         * documentation is available at the fluid_synth_set_gen2()
         * function. */
        if (fluid_channel_get_gen_abs(channel, i)) {
            gen[i].flags = GEN_ABS_NRPN;
        }
    }

    return FLUID_OK;
}

fluid_real_t fluid_gen_scale(int gen, float value) {
    return (fluid_gen_info[gen].min +
            value * (fluid_gen_info[gen].max - fluid_gen_info[gen].min));
}

fluid_real_t fluid_gen_scale_nrpn(int gen, int data) {
    fluid_real_t value = (float)data - 8192.0f;
    fluid_clip(value, -8192, 8192);
    return value * (float)fluid_gen_info[gen].nrpn_scale;
}



fluid_sf_gen_t * fluid_sf_gen_create(uint8_t num) {
    fluid_sf_gen_t *gen = FLUID_NEW(fluid_sf_gen_t);
    gen->num = num;
    gen->val = fluid_gen_info[num].def;
    return gen;
}

fluid_sf_gen_t *fluid_sf_gen_get(fluid_list_t *sf_gen_list, uint8_t num) {
    fluid_sf_gen_t *gen;
    fluid_list_t *p = sf_gen_list;
    while (p != NULL) {
        gen = (fluid_sf_gen_t *) p->data;
        if (gen->num == num)
            return gen;

        p = fluid_list_next(p);
    }
    return NULL;
}
