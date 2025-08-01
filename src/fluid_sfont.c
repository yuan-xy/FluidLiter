#include "fluid_sfont.h"
#include "fluid_gen.h"

#ifdef FLUID_NO_LOG
#define gerr(...)  (FAIL)
#else
int gerr(int ev, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    FLUID_LOG(FLUID_ERR, fmt, args);
    va_end(args);
    return (FAIL);
}
#endif

// allocator for SFZone/SFGen/SFSample etc. which use FLUID_NEW_SF
// Not apply to fluid_inst_zone_t/fluid_sf_gen_t/fluid_sample_t etc. which use FLUID_NEW
#if defined(__arm__)
    #define FLUID_MALLOC_SF(_n) malloc(_n)
    #define FLUID_NEW_SF(_t) (_t *)malloc(sizeof(_t))
    #define FLUID_FREE_SF(_p) free(_p)
#else
    #define FLUID_MALLOC_SF(_n) FLUID_MALLOC(_n)
    #define FLUID_NEW_SF(_n) FLUID_NEW(_n)
    #define FLUID_FREE_SF(_p) FLUID_FREE(_p)
#endif



/***************************************************************
 *
 *                           SFONT LOADER
 */

static void *default_fopen(fluid_fileapi_t *fileapi, const char *path) {
    return FLUID_FOPEN(path, "rb");
}

static int default_fclose(void *handle) {
    return FLUID_FCLOSE((FILE *)handle);
}

static long default_ftell(void *handle) {
    return FLUID_FTELL((FILE *)handle);
}

static int safe_fread(void *buf, int count, void *handle) {
    if (FLUID_FREAD(buf, count, 1, (FILE *)handle) != 1) {
        if (feof((FILE *)handle)){
            FLUID_LOG(FLUID_ERR, _("EOF while attemping to read %d bytes"), count);
        }else{
            FLUID_LOG(FLUID_ERR, _("File read failed"));
        }
        return FLUID_FAILED;
    }
    return FLUID_OK;
}

static int safe_fseek(void *handle, long ofs, int whence) {
    if (FLUID_FSEEK((FILE *)handle, ofs, whence) != 0) {
        FLUID_LOG(FLUID_ERR, _("File seek failed with offset = %ld and whence = %d"), ofs, whence);
        return FLUID_FAILED;
    }
    return FLUID_OK;
}

const fluid_fileapi_t default_fileapi = {NULL,       default_fopen,  safe_fread,   NULL,
                                                safe_fseek, default_fclose, default_ftell};

static fluid_fileapi_t *fluid_default_fileapi = (fluid_fileapi_t *)&default_fileapi;

void fluid_set_default_fileapi(fluid_fileapi_t *fileapi) {
    if (fileapi == NULL) return;
    fluid_fileapi_delete(fluid_default_fileapi);
    fluid_default_fileapi = fileapi;
}

fluid_fileapi_t *fluid_get_default_fileapi(void) {
    return fluid_default_fileapi;
}

fluid_sfont_t *fluid_soundfont_load(fluid_fileapi_t *fileapi, const char *filename) {
    fluid_sfont_t *sfont = FLUID_NEW(fluid_sfont_t);
    if (sfont == NULL) {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    sfont->filename = NULL;
    sfont->samplepos = 0;
    sfont->samplesize = 0;
    sfont->sample = NULL;
    sfont->sampledata = NULL;
    sfont->preset = NULL;

    if (fluid_sfont_load(sfont, filename, fileapi) == FLUID_FAILED) {
        delete_fluid_sfont(sfont);
        return NULL;
    }

    return sfont;
}

/***************************************************************
 *
 *                           PUBLIC INTERFACE
 */

int delete_fluid_sfont(fluid_sfont_t *sfont) {
    fluid_list_t *list;
    fluid_preset_t *preset;

    if (sfont->filename != NULL) {
        FLUID_FREE(sfont->filename);
    }

    for (list = sfont->sample; list; list = fluid_list_next(list)) {
        delete_fluid_sample((fluid_sample_t *)fluid_list_get(list));
    }

    if (sfont->sample) {
        delete_fluid_list(sfont->sample);
    }

    if (sfont->sampledata != NULL && !sfont->is_rom) {
        FLUID_FREE(sfont->sampledata);
    }

    preset = sfont->preset;
    while (preset != NULL) {
        sfont->preset = preset->next;
        delete_fluid_preset(preset);
        preset = sfont->preset;
    }

    FLUID_FREE(sfont);
    return FLUID_OK;
}

char *fluid_sfont_get_name(fluid_sfont_t *sfont) {
    return sfont->filename;
}

void (*preset_callback)(unsigned int bank, unsigned int num, char *name) = NULL;
void fluid_synth_set_preset_callback(void *callback) {
    preset_callback = callback;
}

int fluid_sfont_load(fluid_sfont_t *sfont, const char *filename, fluid_fileapi_t *fapi) {
    SFData *sfdata;
    fluid_list_t *p;
    SFPreset *sfpreset;
    SFSample *sfsample;
    fluid_sample_t *sample;
    fluid_preset_t *preset;

    sfont->filename = FLUID_STRDUP(filename);

    /* The actual loading is done in the sfont and sffile files */
    sfdata = sfload_file(filename, fapi);
    if (sfdata == NULL) {
        FLUID_LOG(FLUID_ERR, "Couldn't load soundfont file");
        return FLUID_FAILED;
    }

    /* Keep track of the position and size of the sample data because
       it's loaded separately (and might be unoaded/reloaded in future) */
    sfont->samplepos = sfdata->samplepos;
    sfont->samplesize = sfdata->samplesize;
    sfont->is_compressed = sfdata->is_compressed;

    /* load sample data in one block */
    if (fluid_sfont_load_sampledata(sfont, fapi) != FLUID_OK) goto err_exit;

    /* Create all the sample headers */
    p = sfdata->sample;
    while (p != NULL) {
        sfsample = (SFSample *)p->data;

        sample = new_fluid_sample();
        if (sample == NULL) goto err_exit;

        if (fluid_sample_import_sfont(sample, sfsample, sfont) != FLUID_OK) goto err_exit;

        fluid_sfont_add_sample(sfont, sample);
#if DEBUG
        fluid_voice_optimize_sample(sample);
#endif
        p = fluid_list_next(p);
    }

    /* Load all the presets */
    p = sfdata->preset;
    while (p != NULL) {
        sfpreset = (SFPreset *)p->data;
        preset = new_fluid_preset(sfont);
        if (preset == NULL) goto err_exit;

        if (fluid_preset_import_sfont(preset, sfpreset, sfont) != FLUID_OK) goto err_exit;

        fluid_sfont_add_preset(sfont, preset);
        if (preset_callback) preset_callback(preset->bank, preset->num, preset->name);
        p = fluid_list_next(p);
    }
    sfont_close(sfdata, fapi);

    return FLUID_OK;

err_exit:
    sfont_close(sfdata, fapi);
    return FLUID_FAILED;
}

int fluid_sfont_add_sample(fluid_sfont_t *sfont, fluid_sample_t *sample) {
    sfont->sample = fluid_list_append(sfont->sample, sample);
    return FLUID_OK;
}

int fluid_sfont_add_preset(fluid_sfont_t *sfont, fluid_preset_t *preset) {
    fluid_preset_t *cur, *prev;
    if (sfont->preset == NULL) {
        preset->next = NULL;
        sfont->preset = preset;
    } else {
        /* sort them as we go along. very basic sorting trick. */
        cur = sfont->preset;
        prev = NULL;
        while (cur != NULL) {
            if ((preset->bank < cur->bank) ||
                ((preset->bank == cur->bank) && (preset->num < cur->num))) {
                if (prev == NULL) {
                    preset->next = cur;
                    sfont->preset = preset;
                } else {
                    preset->next = cur;
                    prev->next = preset;
                }
                return FLUID_OK;
            }
            prev = cur;
            cur = cur->next;
        }
        preset->next = NULL;
        prev->next = preset;
    }
    return FLUID_OK;
}

static decompress_callback *decompress_cb = NULL;
void fluid_sfont_set_decompress_callback(decompress_callback *cb){
    decompress_cb = cb;
}


int fluid_sfont_load_sampledata(fluid_sfont_t *sfont, fluid_fileapi_t *fapi) {
    fluid_file fd;
    fd = fapi->fopen(fapi, sfont->filename);
    if (fd == NULL) {
        FLUID_LOG(FLUID_ERR, "Can't open soundfont file");
        return FLUID_FAILED;
    }
    if (fapi->fseek(fd, sfont->samplepos, SEEK_SET) == FLUID_FAILED) {
        perror("error");
        FLUID_LOG(FLUID_ERR, "Failed to seek position in data file");
        return FLUID_FAILED;
    }

    if (fapi->fread_zero_memcpy != NULL) {
        sfont->sampledata = fapi->fread_zero_memcpy(sfont->samplesize, fd);
        sfont->is_rom = 1; 
    } else {
        sfont->is_rom = 0;
        sfont->sampledata = (short *)FLUID_MALLOC_SF(sfont->samplesize);
        FLUID_LOG(FLUID_INFO, "sfont->samplesize %d\n", sfont->samplesize);

        if (sfont->sampledata == NULL) {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return FLUID_FAILED;
        }
        if(sfont->is_compressed){
            int compressed_size = sfont->samplesize/COMPRESS_RATIO;
            char *buffer = malloc(compressed_size);
            if (fapi->fread(buffer, compressed_size, fd) == FLUID_FAILED) {
                FLUID_LOG(FLUID_ERR, "Failed to read sample data");
                return FLUID_FAILED;
            }
            if(decompress_cb == NULL){
                FLUID_LOG(FLUID_ERR, "Failed to get decompress_callback");
                return FLUID_FAILED;
            }
            bool flag = decompress_cb(buffer, compressed_size, (char *)sfont->sampledata, sfont->samplesize);
            free(buffer);
            if(!flag) return FLUID_FAILED;
        }else{
            if (fapi->fread(sfont->sampledata, sfont->samplesize, fd) == FLUID_FAILED) {
                FLUID_LOG(FLUID_ERR, "Failed to read sample data");
                return FLUID_FAILED;
            }
        }

        fapi->fclose(fd);
    }
    return FLUID_OK;
}

fluid_sample_t *fluid_sfont_get_sample(fluid_sfont_t *sfont, char *s) {
    fluid_list_t *list;
    fluid_sample_t *sample;

    for (list = sfont->sample; list; list = fluid_list_next(list)) {
        sample = (fluid_sample_t *)fluid_list_get(list);

        if (FLUID_STRCMP(sample->name, s) == 0) {
            return sample;
        }
    }

    return NULL;
}


fluid_preset_t *fluid_sfont_get_preset(fluid_sfont_t *sfont, unsigned int bank,
                                             unsigned int num) {
    fluid_preset_t *preset = sfont->preset;
    while (preset != NULL) {
        if ((preset->bank == bank) && ((preset->num == num))) {
            return preset;
        }
        preset = preset->next;
    }
    return NULL;
}

void fluid_sfont_iteration_start(fluid_sfont_t *sfont) {
    sfont->iter_cur = sfont->preset;
}

fluid_preset_t * fluid_sfont_iteration_next(fluid_sfont_t *sfont) {
    fluid_preset_t *preset = sfont->iter_cur;
    if(preset == NULL) return NULL;
    sfont->iter_cur = fluid_preset_next(sfont->iter_cur);
    return preset;
}



fluid_preset_t *new_fluid_preset(fluid_sfont_t *sfont) {
    fluid_preset_t *preset = FLUID_NEW(fluid_preset_t);
    if (preset == NULL) {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    preset->next = NULL;
    preset->sfont = sfont;
    preset->name[0] = 0;
    preset->bank = 0;
    preset->num = 0;
    preset->global_zone = NULL;
    preset->zone = NULL;
    return preset;
}

int delete_fluid_preset(fluid_preset_t *preset) {
    int err = FLUID_OK;
    fluid_preset_zone_t *zone;
    if (preset->global_zone != NULL) {
        if (delete_fluid_preset_zone(preset->global_zone) != FLUID_OK) {
            err = FLUID_FAILED;
        }
        preset->global_zone = NULL;
    }
    zone = preset->zone;
    while (zone != NULL) {
        preset->zone = zone->next;
        if (delete_fluid_preset_zone(zone) != FLUID_OK) {
            err = FLUID_FAILED;
        }
        zone = preset->zone;
    }
    FLUID_FREE(preset);
    return err;
}

int fluid_preset_get_banknum(fluid_preset_t *preset) {
    return preset->bank;
}

int fluid_preset_get_num(fluid_preset_t *preset) {
    return preset->num;
}

char *fluid_preset_get_name(fluid_preset_t *preset) {
    return preset->name;
}

fluid_preset_t *fluid_preset_next(fluid_preset_t *preset) {
    return preset->next;
}

int fluid_preset_noteon(fluid_preset_t *preset, fluid_synth_t *synth, int chan, int key,
                           int vel) {
    fluid_preset_zone_t *preset_zone, *global_preset_zone;
    fluid_inst_t *inst;
    fluid_inst_zone_t *inst_zone, *global_inst_zone;
    fluid_sample_t *sample;
    fluid_voice_t *voice;
    fluid_mod_list_t *mod;
    fluid_mod_list_t *mod_list[FLUID_NUM_MOD]; /* list for 'sorting' preset modulators */
    int mod_list_count;
    int i;

    global_preset_zone = fluid_preset_get_global_zone(preset);

    /* run thru all the zones of this preset */
    preset_zone = fluid_preset_get_zone(preset);
    while (preset_zone != NULL) {
        /* check if the note falls into the key and velocity range of this
         * preset */
        if (fluid_preset_zone_inside_range(preset_zone, key, vel)) {
            inst = fluid_preset_zone_get_inst(preset_zone);
            global_inst_zone = fluid_inst_get_global_zone(inst);

            /* run thru all the zones of this instrument */
            inst_zone = fluid_inst_get_zone(inst);
            while (inst_zone != NULL) {
                /* make sure this instrument zone has a valid sample */
                sample = fluid_inst_zone_get_sample(inst_zone);
                if (sample == NULL) {
                    inst_zone = fluid_inst_zone_next(inst_zone);
                    continue;
                }

                /* check if the note falls into the key and velocity range of
                 * this instrument */
                if (fluid_inst_zone_inside_range(inst_zone, key, vel) && (sample != NULL)) {
                    /* this is a good zone. allocate a new synthesis process and
                     * initialize it */
                    voice = fluid_synth_alloc_voice(synth, sample, chan, key, vel);
                    if (voice == NULL) {
                        return FLUID_FAILED;
                    }

                    /* Instrument level, generators */
                    if (1) {
                        uint8_t inst_excluded[GEN_LAST] = {0};
                        fluid_sf_gen_t *gen;
                        fluid_list_t *p;

                        p = inst_zone->sf_gen;
                        while (p != NULL) {
                            gen = (fluid_sf_gen_t *)p->data;
                            uint8_t i = gen->num;
                            fluid_voice_gen_set(voice, i, gen->val);
                            inst_excluded[i] = 1;
                            p = fluid_list_next(p);
                        }

                        if (global_inst_zone) {
                            p = global_inst_zone->sf_gen;
                            while (p != NULL) {
                                gen = (fluid_sf_gen_t *)p->data;
                                uint8_t i = gen->num;
                                if (!inst_excluded[i]) fluid_voice_gen_set(voice, i, gen->val);

                                p = fluid_list_next(p);
                            }
                        }
                    }
                    /* global instrument zone, modulators: Put them all into a
                     * list. */

                    mod_list_count = 0;

                    if (global_inst_zone) {
                        mod = global_inst_zone->mod;
                        while (mod) {
                            mod_list[mod_list_count++] = mod;
                            mod = mod->next;
                        }
                    }

                    /* local instrument zone, modulators.
                     * Replace modulators with the same definition in the list:
                     * SF 2.01 page 69, 'bullet' 8
                     */
                    mod = inst_zone->mod;
                    while (mod) {
                        /* 'Identical' modulators will be deleted by setting
                         * their list entry to NULL.  The list length is known,
                         * NULL entries will be ignored later.  SF2.01
                         * section 9.5.1 page 69, 'bullet' 3 defines
                         * 'identical'.  */

                        for (i = 0; i < mod_list_count; i++) {
                            if (mod_list[i] &&
                                fluid_mod_test_identity((fluid_mod_t *)mod,
                                                        (fluid_mod_t *)mod_list[i])) {
                                mod_list[i] = NULL;
                            }
                        }

                        /* Finally add the new modulator to to the list. */
                        mod_list[mod_list_count++] = mod;
                        mod = mod->next;
                    }

                    /* Add instrument modulators (global / local) to the voice.
                     */
                    for (i = 0; i < mod_list_count; i++) {
                        mod = mod_list[i];
                        if (mod != NULL) { /* disabled modulators CANNOT be skipped. */

                            /* Instrument modulators -supersede- existing
                             * (default) modulators.  SF 2.01 page 69, 'bullet'
                             * 6 */
                            fluid_voice_add_mod(voice, (fluid_mod_t *)mod, FLUID_VOICE_OVERWRITE);
                        }
                    }

                    /* Preset level, generators */

                    if (1) {
                        uint8_t preset_excluded[GEN_LAST] = {0};
                        fluid_sf_gen_t *gen;
                        fluid_list_t *p;

                        p = preset_zone->sf_gen;
                        while (p != NULL) {
                            gen = (fluid_sf_gen_t *)p->data;
                            uint8_t i = gen->num;
                            if ((i != GEN_STARTADDROFS) && (i != GEN_ENDADDROFS) &&
                                (i != GEN_STARTLOOPADDROFS) && (i != GEN_ENDLOOPADDROFS) &&
                                (i != GEN_STARTADDRCOARSEOFS) && (i != GEN_ENDADDRCOARSEOFS) &&
                                (i != GEN_STARTLOOPADDRCOARSEOFS) && (i != GEN_KEYNUM) &&
                                (i != GEN_VELOCITY) && (i != GEN_ENDLOOPADDRCOARSEOFS) &&
                                (i != GEN_SAMPLEMODE) && (i != GEN_EXCLUSIVECLASS) &&
                                (i != GEN_OVERRIDEROOTKEY)) {
                                fluid_voice_gen_incr(voice, i, gen->val);
                                preset_excluded[i] = 1;
                            }
                            p = fluid_list_next(p);
                        }

                        if (global_preset_zone) {
                            p = global_preset_zone->sf_gen;
                            while (p != NULL) {
                                gen = (fluid_sf_gen_t *)p->data;
                                uint8_t i = gen->num;
                                if ((i != GEN_STARTADDROFS) && (i != GEN_ENDADDROFS) &&
                                    (i != GEN_STARTLOOPADDROFS) && (i != GEN_ENDLOOPADDROFS) &&
                                    (i != GEN_STARTADDRCOARSEOFS) && (i != GEN_ENDADDRCOARSEOFS) &&
                                    (i != GEN_STARTLOOPADDRCOARSEOFS) && (i != GEN_KEYNUM) &&
                                    (i != GEN_VELOCITY) && (i != GEN_ENDLOOPADDRCOARSEOFS) &&
                                    (i != GEN_SAMPLEMODE) && (i != GEN_EXCLUSIVECLASS) &&
                                    (i != GEN_OVERRIDEROOTKEY)) {
                                    if (!preset_excluded[i])
                                        fluid_voice_gen_incr(voice, i, gen->val);
                                }
                                p = fluid_list_next(p);
                            } // end while
                        } // end if
                    }

                    /* Global preset zone, modulators: put them all into a
                     * list. */
                    mod_list_count = 0;
                    if (global_preset_zone) {
                        mod = global_preset_zone->mod;
                        while (mod) {
                            mod_list[mod_list_count++] = mod;
                            mod = mod->next;
                        }
                    }

                    /* Process the modulators of the local preset zone.  Kick
                     * out all identical modulators from the global preset zone
                     * (SF 2.01 page 69, second-last bullet) */

                    mod = preset_zone->mod;
                    while (mod) {
                        for (i = 0; i < mod_list_count; i++) {
                            if (mod_list[i] &&
                                fluid_mod_test_identity((fluid_mod_t *)mod,
                                                        (fluid_mod_t *)mod_list[i])) {
                                mod_list[i] = NULL;
                            }
                        }

                        /* Finally add the new modulator to the list. */
                        mod_list[mod_list_count++] = mod;
                        mod = mod->next;
                    }

                    /* Add preset modulators (global / local) to the voice. */
                    for (i = 0; i < mod_list_count; i++) {
                        mod = mod_list[i];
                        if ((mod != NULL) &&
                            (mod->amount != 0)) { // disabled modulators can be skipped.
                            /* Preset modulators -add- to existing instrument /
                             * default modulators.  SF2.01 page 70 first bullet
                             * on page */
                            fluid_voice_add_mod(voice, (fluid_mod_t *)mod, FLUID_VOICE_ADD);
                        }
                    }

                    /* add the synthesis process to the synthesis loop. */
                    fluid_synth_start_voice(synth, voice);

                    /* Store the ID of the first voice that was created by this
                     * noteon event. Exclusive class may only terminate older
                     * voices. That avoids killing voices, which have just been
                     * created. (a noteon event can create several voice
                     * processes with the same exclusive class - for example
                     * when using stereo samples)
                     */
                }

                inst_zone = fluid_inst_zone_next(inst_zone);
            }
        }
        preset_zone = fluid_preset_zone_next(preset_zone);
    }

    return FLUID_OK;
}

/*
 * fluid_preset_set_global_zone
 */
int fluid_preset_set_global_zone(fluid_preset_t *preset, fluid_preset_zone_t *zone) {
    preset->global_zone = zone;
    return FLUID_OK;
}

int fluid_preset_import_sfont(fluid_preset_t *preset, SFPreset *sfpreset,
                                 fluid_sfont_t *sfont) {
    fluid_list_t *p;
    SFZone *sfzone;
    fluid_preset_zone_t *zone;
    int count;
    if (FLUID_STRLEN(sfpreset->name) > 0) {
        strncpy(preset->name, sfpreset->name, sizeof(preset->name));
    } else {
        #if DEBUG
        snprintf(preset->name, sizeof(preset->name), "Bank%d,Preset%d", sfpreset->bank,
                 sfpreset->prenum);
        #endif
    }
    preset->bank = sfpreset->bank;
    preset->num = sfpreset->prenum;
    p = sfpreset->zone;
    count = 0;
    while (p != NULL) {
        sfzone = (SFZone *)p->data;
        zone = new_fluid_preset_zone();
        if (zone == NULL) {
            return FLUID_FAILED;
        }
        if (fluid_preset_zone_import_sfont(zone, sfzone, sfont) != FLUID_OK) {
            return FLUID_FAILED;
        }
        if ((count == 0) && (fluid_preset_zone_get_inst(zone) == NULL)) {
            fluid_preset_set_global_zone(preset, zone);
        } else if (fluid_preset_add_zone(preset, zone) != FLUID_OK) {
            return FLUID_FAILED;
        }
        p = fluid_list_next(p);
        count++;
    }
    return FLUID_OK;
}


int fluid_preset_add_zone(fluid_preset_t *preset, fluid_preset_zone_t *zone) {
    if (preset->zone == NULL) {
        zone->next = NULL;
        preset->zone = zone;
    } else {
        zone->next = preset->zone;
        preset->zone = zone;
    }
    return FLUID_OK;
}


fluid_preset_zone_t *fluid_preset_get_zone(fluid_preset_t *preset) {
    return preset->zone;
}


fluid_preset_zone_t *fluid_preset_get_global_zone(fluid_preset_t *preset) {
    return preset->global_zone;
}

fluid_preset_zone_t *fluid_preset_zone_next(fluid_preset_zone_t *preset) {
    return preset->next;
}

fluid_preset_zone_t *new_fluid_preset_zone() {
    fluid_preset_zone_t *zone = FLUID_NEW(fluid_preset_zone_t);
    if (zone == NULL) {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    zone->next = NULL;
    zone->inst = NULL;
    zone->keylo = 0;
    zone->keyhi = 128;
    zone->vello = 0;
    zone->velhi = 128;
    zone->sf_gen = NULL;
    zone->mod = NULL; /* list of modulators */
    return zone;
}

/***************************************************************
 *
 *                           PRESET_ZONE
 */

int delete_fluid_preset_zone(fluid_preset_zone_t *zone) {
    fluid_mod_list_t *mod, *tmp;

    mod = zone->mod;
    while (mod) /* delete the modulators */
    {
        tmp = mod;
        mod = mod->next;
        fluid_mod_list_delete(tmp);
    }

    if (zone->inst) delete_fluid_inst(zone->inst);
    FLUID_FREE(zone);
    return FLUID_OK;
}

int fluid_preset_zone_import_sfont(fluid_preset_zone_t *zone, SFZone *sfzone,
                                   fluid_sfont_t *sfont) {
    fluid_list_t *r;
    SFGen *sfgen;
    fluid_sf_gen_t *gen = NULL;
    int count;
    for (count = 0, r = sfzone->gen; r != NULL; count++) {
        sfgen = (SFGen *)r->data;
        switch (sfgen->id) {
        case GEN_KEYRANGE:
            zone->keylo = sfgen->amount.range.lo;
            zone->keyhi = sfgen->amount.range.hi;
            break;
        case GEN_VELRANGE:
            zone->vello = sfgen->amount.range.lo;
            zone->velhi = sfgen->amount.range.hi;
            break;
        default:
            if (fluid_gen_info[sfgen->id].def != (fluid_real_t)sfgen->amount.sword) {
                gen = fluid_sf_gen_get(zone->sf_gen, sfgen->id);
                if (!gen) {
                    gen = fluid_sf_gen_create(sfgen);
                    zone->sf_gen = fluid_list_append(zone->sf_gen, gen);
                } else {
                    FLUID_LOG(FLUID_WARN, "unexpect preset gen(%d, %f) exsits.", gen->num,
                              gen->val);
                }
            } else {
                FLUID_LOG(FLUID_DBG, "good news: don't repeat create preset gen(%d, %d).",
                          sfgen->id, sfgen->amount.sword);
            }
            break;
        }
        r = fluid_list_next(r);
    }
    if ((sfzone->instsamp != NULL) && (sfzone->instsamp->data != NULL)) {
        zone->inst = (fluid_inst_t *)new_fluid_inst();
        if (zone->inst == NULL) {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return FLUID_FAILED;
        }
        if (fluid_inst_import_sfont(zone->inst, (SFInst *)sfzone->instsamp->data, sfont) !=
            FLUID_OK) {
            return FLUID_FAILED;
        }
    }

    /* Import the modulators (only SF2.1 and higher) */
    for (count = 0, r = sfzone->mod; r != NULL; count++) {
        SFMod *mod_src = (SFMod *)r->data;
        fluid_mod_list_t *mod_dest = fluid_mod_list_new();
        int type;

        if (mod_dest == NULL) {
            return FLUID_FAILED;
        }
        mod_dest->next = NULL; /* pointer to next modulator, this is the end of
                                  the list now.*/

        /* *** Amount *** */
        mod_dest->amount = mod_src->amount;

        /* *** Source *** */
        mod_dest->src1 = mod_src->src & 127; /* index of source 1, seven-bit value, SF2.01
                                                section 8.2, page 50 */
        mod_dest->flags1 = 0;

        /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
        if (mod_src->src & (1 << 7)) {
            mod_dest->flags1 |= FLUID_MOD_CC;
        } else {
            mod_dest->flags1 |= FLUID_MOD_GC;
        }

        /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
        if (mod_src->src & (1 << 8)) {
            mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
        } else {
            mod_dest->flags1 |= FLUID_MOD_POSITIVE;
        }

        /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
        if (mod_src->src & (1 << 9)) {
            mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
        } else {
            mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;
        }

        /* modulator source types: SF2.01 section 8.2.1 page 52 */
        type = (mod_src->src) >> 10;
        type &= 63; /* type is a 6-bit value */
        if (type == 0) {
            mod_dest->flags1 |= FLUID_MOD_LINEAR;
        } else if (type == 1) {
            mod_dest->flags1 |= FLUID_MOD_CONCAVE;
        } else if (type == 2) {
            mod_dest->flags1 |= FLUID_MOD_CONVEX;
        } else if (type == 3) {
            mod_dest->flags1 |= FLUID_MOD_SWITCH;
        } else {
            /* This shouldn't happen - unknown type!
             * Deactivate the modulator by setting the amount to 0. */
            mod_dest->amount = 0;
        }

        /* *** Dest *** */
        mod_dest->dest = mod_src->dest; /* index of controlled generator */

        /* *** Amount source *** */
        mod_dest->src2 = mod_src->amtsrc & 127; /* index of source 2, seven-bit value, SF2.01
                                                   section 8.2, p.50 */
        mod_dest->flags2 = 0;

        /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
        if (mod_src->amtsrc & (1 << 7)) {
            mod_dest->flags2 |= FLUID_MOD_CC;
        } else {
            mod_dest->flags2 |= FLUID_MOD_GC;
        }

        /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
        if (mod_src->amtsrc & (1 << 8)) {
            mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
        } else {
            mod_dest->flags2 |= FLUID_MOD_POSITIVE;
        }

        /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
        if (mod_src->amtsrc & (1 << 9)) {
            mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
        } else {
            mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;
        }

        /* modulator source types: SF2.01 section 8.2.1 page 52 */
        type = (mod_src->amtsrc) >> 10;
        type &= 63; /* type is a 6-bit value */
        if (type == 0) {
            mod_dest->flags2 |= FLUID_MOD_LINEAR;
        } else if (type == 1) {
            mod_dest->flags2 |= FLUID_MOD_CONCAVE;
        } else if (type == 2) {
            mod_dest->flags2 |= FLUID_MOD_CONVEX;
        } else if (type == 3) {
            mod_dest->flags2 |= FLUID_MOD_SWITCH;
        } else {
            /* This shouldn't happen - unknown type!
             * Deactivate the modulator by setting the amount to 0. */
            mod_dest->amount = 0;
        }

        /* *** Transform *** */
        /* SF2.01 only uses the 'linear' transform (0).
         * Deactivate the modulator by setting the amount to 0 in any other
         * case.
         */
        if (mod_src->trans != 0) {
            mod_dest->amount = 0;
        }

        /* Store the new modulator in the zone The order of modulators
         * will make a difference, at least in an instrument context: The
         * second modulator overwrites the first one, if they only differ
         * in amount. */
        if (count == 0) {
            zone->mod = mod_dest;
        } else {
            fluid_mod_list_t *last_mod = zone->mod;

            /* Find the end of the list */
            while (last_mod->next != NULL) {
                last_mod = last_mod->next;
            }

            last_mod->next = mod_dest;
        }

        r = fluid_list_next(r);
    } /* foreach modulator */

    return FLUID_OK;
}

/*
 * fluid_preset_zone_get_inst
 */
fluid_inst_t *fluid_preset_zone_get_inst(fluid_preset_zone_t *zone) {
    return zone->inst;
}

/*
 * fluid_preset_zone_inside_range
 */
int fluid_preset_zone_inside_range(fluid_preset_zone_t *zone, int key, int vel) {
    return ((zone->keylo <= key) && (zone->keyhi >= key) && (zone->vello <= vel) &&
            (zone->velhi >= vel));
}

/***************************************************************
 *
 *                           INST
 */

fluid_inst_t *new_fluid_inst() {
    fluid_inst_t *inst = FLUID_NEW(fluid_inst_t);
    if (inst == NULL) {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    inst->global_zone = NULL;
    inst->zone = NULL;
    return inst;
}

/*
 * delete_fluid_inst
 */
int delete_fluid_inst(fluid_inst_t *inst) {
    fluid_inst_zone_t *zone;
    int err = FLUID_OK;
    if (inst->global_zone != NULL) {
        if (delete_fluid_inst_zone(inst->global_zone) != FLUID_OK) {
            err = FLUID_FAILED;
        }
        inst->global_zone = NULL;
    }
    zone = inst->zone;
    while (zone != NULL) {
        inst->zone = zone->next;
        if (delete_fluid_inst_zone(zone) != FLUID_OK) {
            err = FLUID_FAILED;
        }
        zone = inst->zone;
    }
    FLUID_FREE(inst);
    return err;
}

int fluid_inst_set_global_zone(fluid_inst_t *inst, fluid_inst_zone_t *zone) {
    inst->global_zone = zone;
    return FLUID_OK;
}

int fluid_inst_import_sfont(fluid_inst_t *inst, SFInst *sfinst, fluid_sfont_t *sfont) {
    fluid_list_t *p;
    SFZone *sfzone;
    fluid_inst_zone_t *zone;
    int count;

    p = sfinst->zone;
    if (FLUID_STRLEN(sfinst->name) > 0) {
        FLUID_LOG(FLUID_INFO, "ignore inst->name: %s.", sfinst->name);
    }

    count = 0;
    while (p != NULL) {
        sfzone = (SFZone *)p->data;
        zone = new_fluid_inst_zone();
        if (zone == NULL) {
            return FLUID_FAILED;
        }

        if (fluid_inst_zone_import_sfont(zone, sfzone, sfont) != FLUID_OK) {
            return FLUID_FAILED;
        }

        if ((count == 0) && (fluid_inst_zone_get_sample(zone) == NULL)) {
            fluid_inst_set_global_zone(inst, zone);
        } else if (fluid_inst_add_zone(inst, zone) != FLUID_OK) {
            return FLUID_FAILED;
        }

        p = fluid_list_next(p);
        count++;
    }
    return FLUID_OK;
}

int fluid_inst_add_zone(fluid_inst_t *inst, fluid_inst_zone_t *zone) {
    if (inst->zone == NULL) {
        zone->next = NULL;
        inst->zone = zone;
    } else {
        zone->next = inst->zone;
        inst->zone = zone;
    }
    return FLUID_OK;
}

fluid_inst_zone_t *fluid_inst_get_zone(fluid_inst_t *inst) {
    return inst->zone;
}

/*
 * fluid_inst_get_global_zone
 */
fluid_inst_zone_t *fluid_inst_get_global_zone(fluid_inst_t *inst) {
    return inst->global_zone;
}

/***************************************************************
 *
 *                           INST_ZONE
 */

fluid_inst_zone_t *new_fluid_inst_zone() {
    fluid_inst_zone_t *zone = FLUID_NEW(fluid_inst_zone_t);
    if (zone == NULL) {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    zone->next = NULL;
    zone->sample = NULL;
    zone->keylo = 0;
    zone->keyhi = 128;
    zone->vello = 0;
    zone->velhi = 128;
    zone->sf_gen = NULL;
    zone->mod = NULL; /* list of modulators */
    return zone;
}

int delete_fluid_inst_zone(fluid_inst_zone_t *zone) {
    fluid_mod_list_t *mod, *tmp;

    mod = zone->mod;
    while (mod) /* delete the modulators */
    {
        tmp = mod;
        mod = mod->next;
        fluid_mod_list_delete(tmp);
    }

    fluid_list_t *gen = zone->sf_gen;
    while (gen != NULL) {
        fluid_sf_gen_t *tmp = (fluid_sf_gen_t *)gen->data;
        FLUID_FREE(tmp);
        gen = fluid_list_next(gen);
    }
    delete_fluid_list(zone->sf_gen);

    FLUID_FREE(zone);
    return FLUID_OK;
}

fluid_inst_zone_t *fluid_inst_zone_next(fluid_inst_zone_t *zone) {
    return zone->next;
}

int fluid_inst_zone_import_sfont(fluid_inst_zone_t *zone, SFZone *sfzone, fluid_sfont_t *sfont) {
    fluid_list_t *r;
    SFGen *sfgen;
    fluid_sf_gen_t *gen = NULL;
    int count;

    for (count = 0, r = sfzone->gen; r != NULL; count++) {
        sfgen = (SFGen *)r->data;
        switch (sfgen->id) {
        case GEN_KEYRANGE:
            zone->keylo = (int)sfgen->amount.range.lo;
            zone->keyhi = (int)sfgen->amount.range.hi;
            break;
        case GEN_VELRANGE:
            zone->vello = (int)sfgen->amount.range.lo;
            zone->velhi = (int)sfgen->amount.range.hi;
            break;
        default:
            if (fluid_gen_info[sfgen->id].def != (fluid_real_t)sfgen->amount.sword) {
                gen = fluid_sf_gen_get(zone->sf_gen, sfgen->id);
                if (!gen) {
                    gen = fluid_sf_gen_create(sfgen);
                    zone->sf_gen = fluid_list_append(zone->sf_gen, gen);
                } else {
                    FLUID_LOG(FLUID_WARN, "unexpect gen(%d, %f) exsits.", gen->num, gen->val);
                }
            } else {
                FLUID_LOG(FLUID_DBG, "good news: don't repeat create gen(%d, %d).", sfgen->id,
                          sfgen->amount.sword);
            }
            break;
        }
        r = fluid_list_next(r);
    }

    if ((sfzone->instsamp != NULL) && (sfzone->instsamp->data != NULL)) {
        zone->sample = fluid_sfont_get_sample(sfont, ((SFSample *)sfzone->instsamp->data)->name);
        if (zone->sample == NULL) {
            FLUID_LOG(FLUID_ERR, "Couldn't find sample name");
            return FLUID_FAILED;
        }
    }

    /* Import the modulators (only SF2.1 and higher) */
    for (count = 0, r = sfzone->mod; r != NULL; count++) {
        SFMod *mod_src = (SFMod *)r->data;
        int type;
        fluid_mod_list_t *mod_dest;

        mod_dest = fluid_mod_list_new();
        if (mod_dest == NULL) {
            return FLUID_FAILED;
        }

        mod_dest->next = NULL; /* pointer to next modulator, this is the end of
                                  the list now.*/

        /* *** Amount *** */
        mod_dest->amount = mod_src->amount;

        /* *** Source *** */
        mod_dest->src1 = mod_src->src & 127; /* index of source 1, seven-bit value, SF2.01
                                                section 8.2, page 50 */
        mod_dest->flags1 = 0;

        /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
        if (mod_src->src & (1 << 7)) {
            mod_dest->flags1 |= FLUID_MOD_CC;
        } else {
            mod_dest->flags1 |= FLUID_MOD_GC;
        }

        /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
        if (mod_src->src & (1 << 8)) {
            mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
        } else {
            mod_dest->flags1 |= FLUID_MOD_POSITIVE;
        }

        /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
        if (mod_src->src & (1 << 9)) {
            mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
        } else {
            mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;
        }

        /* modulator source types: SF2.01 section 8.2.1 page 52 */
        type = (mod_src->src) >> 10;
        type &= 63; /* type is a 6-bit value */
        if (type == 0) {
            mod_dest->flags1 |= FLUID_MOD_LINEAR;
        } else if (type == 1) {
            mod_dest->flags1 |= FLUID_MOD_CONCAVE;
        } else if (type == 2) {
            mod_dest->flags1 |= FLUID_MOD_CONVEX;
        } else if (type == 3) {
            mod_dest->flags1 |= FLUID_MOD_SWITCH;
        } else {
            /* This shouldn't happen - unknown type!
             * Deactivate the modulator by setting the amount to 0. */
            mod_dest->amount = 0;
        }

        /* *** Dest *** */
        mod_dest->dest = mod_src->dest; /* index of controlled generator */

        /* *** Amount source *** */
        mod_dest->src2 = mod_src->amtsrc & 127; /* index of source 2, seven-bit value, SF2.01
                                                   section 8.2, page 50 */
        mod_dest->flags2 = 0;

        /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
        if (mod_src->amtsrc & (1 << 7)) {
            mod_dest->flags2 |= FLUID_MOD_CC;
        } else {
            mod_dest->flags2 |= FLUID_MOD_GC;
        }

        /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
        if (mod_src->amtsrc & (1 << 8)) {
            mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
        } else {
            mod_dest->flags2 |= FLUID_MOD_POSITIVE;
        }

        /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
        if (mod_src->amtsrc & (1 << 9)) {
            mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
        } else {
            mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;
        }

        /* modulator source types: SF2.01 section 8.2.1 page 52 */
        type = (mod_src->amtsrc) >> 10;
        type &= 63; /* type is a 6-bit value */
        if (type == 0) {
            mod_dest->flags2 |= FLUID_MOD_LINEAR;
        } else if (type == 1) {
            mod_dest->flags2 |= FLUID_MOD_CONCAVE;
        } else if (type == 2) {
            mod_dest->flags2 |= FLUID_MOD_CONVEX;
        } else if (type == 3) {
            mod_dest->flags2 |= FLUID_MOD_SWITCH;
        } else {
            /* This shouldn't happen - unknown type!
             * Deactivate the modulator by setting the amount to 0. */
            mod_dest->amount = 0;
        }

        /* *** Transform *** */
        /* SF2.01 only uses the 'linear' transform (0).
         * Deactivate the modulator by setting the amount to 0 in any other
         * case.
         */
        if (mod_src->trans != 0) {
            mod_dest->amount = 0;
        }

        /* Store the new modulator in the zone
         * The order of modulators will make a difference, at least in an
         * instrument context: The second modulator overwrites the first one, if
         * they only differ in amount. */
        if (count == 0) {
            zone->mod = mod_dest;
        } else {
            fluid_mod_list_t *last_mod = zone->mod;
            /* Find the end of the list */
            while (last_mod->next != NULL) {
                last_mod = last_mod->next;
            }
            last_mod->next = mod_dest;
        }

        r = fluid_list_next(r);
    } /* foreach modulator */
    return FLUID_OK;
}

/*
 * fluid_inst_zone_get_sample
 */
fluid_sample_t *fluid_inst_zone_get_sample(fluid_inst_zone_t *zone) {
    return zone->sample;
}

int fluid_inst_zone_inside_range(fluid_inst_zone_t *zone, int key, int vel) {
    return ((zone->keylo <= key) && (zone->keyhi >= key) && (zone->vello <= vel) &&
            (zone->velhi >= vel));
}

/***************************************************************
 *
 *                           SAMPLE
 */

fluid_sample_t *new_fluid_sample() {
    fluid_sample_t *sample = NULL;

    sample = FLUID_NEW(fluid_sample_t);
    if (sample == NULL) {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    memset(sample, 0, sizeof(fluid_sample_t));
    sample->valid = 1;

    return sample;
}


int delete_fluid_sample(fluid_sample_t *sample) {
    FLUID_FREE(sample);
    return FLUID_OK;
}


int fluid_sample_import_sfont(fluid_sample_t *sample, SFSample *sfsample, fluid_sfont_t *sfont) {
    strncpy(sample->name, sfsample->name, sizeof(sample->name));
    sample->data = sfont->sampledata;
    sample->start = sfsample->start;
    sample->end = sfsample->start + sfsample->end;
    sample->loopstart = sfsample->start + sfsample->loopstart;
    sample->loopend = sfsample->start + sfsample->loopend;
    sample->samplerate = sfsample->samplerate;
    sample->origpitch = sfsample->origpitch;
    sample->pitchadj = sfsample->pitchadj;
    sample->sampletype = sfsample->sampletype;

    if (sample->end - sample->start < 8) {
        sample->valid = 0;
        FLUID_LOG(FLUID_WARN, "Ignoring sample %s: too few sample data points", sample->name);
    } else {
        /*      if (sample->loopstart < sample->start + 8) { */
        /*        FLUID_LOG(FLUID_WARN, "Fixing sample %s: at least 8 data
         * points required before loop start", sample->name);     */
        /*        sample->loopstart = sample->start + 8; */
        /*      } */
        /*      if (sample->loopend > sample->end - 8) { */
        /*        FLUID_LOG(FLUID_WARN, "Fixing sample %s: at least 8 data
         * points required after loop end", sample->name);     */
        /*        sample->loopend = sample->end - 8; */
        /*      } */
    }
    return FLUID_OK;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

/*=================================sfload.c========================
  Borrowed from Smurf SoundFont Editor by Josh Green
  =================================================================*/

/*
   functions for loading data from sfont files, with appropriate byte swapping
   on big endian machines. Sfont IDs are not swapped because the ID read is
   equivalent to the matching ID list in memory regardless of LE/BE machine
*/

#ifdef WORDS_BIGENDIAN
#define READCHUNK(var, fd, fapi)                                                                   \
    G_STMT_START {                                                                                 \
        if (fapi->fread(var, 8, fd) == FLUID_FAILED) return (FAIL);                                \
        ((SFChunk *)(var))->size = GUINT32_FROM_BE(((SFChunk *)(var))->size);                      \
    }                                                                                              \
    G_STMT_END
#else
#define READCHUNK(var, fd, fapi)                                                                   \
    G_STMT_START {                                                                                 \
        if (fapi->fread(var, 8, fd) == FLUID_FAILED) return (FAIL);                                \
        ((SFChunk *)(var))->size = GUINT32_FROM_LE(((SFChunk *)(var))->size);                      \
    }                                                                                              \
    G_STMT_END
#endif
#define READID(var, fd, fapi)                                                                      \
    G_STMT_START {                                                                                 \
        if (fapi->fread(var, 4, fd) == FLUID_FAILED) return (FAIL);                                \
    }                                                                                              \
    G_STMT_END
#define READSTR(var, fd, fapi)                                                                     \
    G_STMT_START {                                                                                 \
        if (fapi->fread(var, 20, fd) == FLUID_FAILED) return (FAIL);                               \
        (var)[20] = '\0';                                                                          \
    }                                                                                              \
    G_STMT_END
#ifdef WORDS_BIGENDIAN
#define READD(var, fd, fapi)                                                                       \
    G_STMT_START {                                                                                 \
        unsigned int _temp;                                                                        \
        if (fapi->fread(&_temp, 4, fd) == FLUID_FAILED) return (FAIL);                             \
        var = GINT32_FROM_BE(_temp);                                                               \
    }                                                                                              \
    G_STMT_END
#else
#define READD(var, fd, fapi)                                                                       \
    G_STMT_START {                                                                                 \
        unsigned int _temp;                                                                        \
        if (fapi->fread(&_temp, 4, fd) == FLUID_FAILED) return (FAIL);                             \
        var = GINT32_FROM_LE(_temp);                                                               \
    }                                                                                              \
    G_STMT_END
#endif
#ifdef WORDS_BIGENDIAN
#define READW(var, fd, fapi)                                                                       \
    G_STMT_START {                                                                                 \
        unsigned short _temp;                                                                      \
        if (fapi->fread(&_temp, 2, fd) == FLUID_FAILED) return (FAIL);                             \
        var = GINT16_FROM_BE(_temp);                                                               \
    }                                                                                              \
    G_STMT_END
#else
#define READW(var, fd, fapi)                                                                       \
    G_STMT_START {                                                                                 \
        unsigned short _temp;                                                                      \
        if (fapi->fread(&_temp, 2, fd) == FLUID_FAILED) return (FAIL);                             \
        var = GINT16_FROM_LE(_temp);                                                               \
    }                                                                                              \
    G_STMT_END
#endif
#define READB(var, fd, fapi)                                                                       \
    G_STMT_START {                                                                                 \
        if (fapi->fread(&var, 1, fd) == FLUID_FAILED) return (FAIL);                               \
    }                                                                                              \
    G_STMT_END
#define FSKIP(size, fd, fapi)                                                                      \
    G_STMT_START {                                                                                 \
        if (fapi->fseek(fd, size, SEEK_CUR) == FLUID_FAILED) return (FAIL);                        \
    }                                                                                              \
    G_STMT_END
#define FSKIPW(fd, fapi)                                                                           \
    G_STMT_START {                                                                                 \
        if (fapi->fseek(fd, 2, SEEK_CUR) == FLUID_FAILED) return (FAIL);                           \
    }                                                                                              \
    G_STMT_END

/* removes and advances a fluid_list_t pointer */
#define SLADVREM(list, item)                                                                       \
    G_STMT_START {                                                                                 \
        fluid_list_t *_temp = item;                                                                \
        item = fluid_list_next(item);                                                              \
        list = fluid_list_remove_link(list, _temp);                                                \
        delete1_fluid_list(_temp);                                                                 \
    }                                                                                              \
    G_STMT_END

static int chunkid(unsigned int id);
static int load_body(unsigned int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int read_listchunk(SFChunk *chunk, void *fd, fluid_fileapi_t *fapi);
static int process_info(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int process_sdta(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int pdtahelper(unsigned int expid, unsigned int reclen, SFChunk *chunk, int *size, void *fd,
                      fluid_fileapi_t *fapi);
static int process_pdta(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_phdr(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_pbag(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_pmod(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_pgen(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_ihdr(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_ibag(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_imod(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_igen(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int load_shdr(unsigned int size, SFData *sf, void *fd, fluid_fileapi_t *fapi);
static int fixup_pgen(SFData *sf);
static int fixup_igen(SFData *sf);
static int fixup_sample(SFData *sf);

const static char idlist[] = {"RIFFLISTsfbkINFOsdtapdtaifilisngINAMiromiverICRDIENGIPRD"
                              "ICOPICMTISFTsnamsmplphdrpbagpmodpgeninstibagimodigenshdr"};

/* sound font file load functions */
static int chunkid(unsigned int id) {
    unsigned int i;
    unsigned int *p;

    p = (unsigned int *)idlist;
    for (i = 0; i < sizeof(idlist) / sizeof(int); i++, p += 1)
        if (*p == id) return (i + 1);

    return (UNKN_ID);
}

SFData *sfload_file(const char *fname, fluid_fileapi_t *fapi) {
    SFData *sf = NULL;
    void *fd;
    int fsize = 0;
    int err = FALSE;

    if ((fd = fapi->fopen(fapi, fname)) == NULL) {
        FLUID_LOG(FLUID_ERR, _("Unable to open file \"%s\""), fname);
        return (NULL);
    }

    if (!(sf = FLUID_NEW_SF(SFData))) {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        err = TRUE;
    }

    if (!err) {
        memset(sf, 0, sizeof(SFData));   /* zero sfdata */
        sf->fname = FLUID_STRDUP(fname); /* copy file name */
        sf->sffd = fd;
    }

#if defined(__arm__) || defined(__riscv)
    (void)fsize;
    if (!err && !load_body(0, sf, fd, fapi)) err = TRUE; /* load_body skip size check */
#else
    /* get size of file */
    if (!err && fapi->fseek(fd, 0L, SEEK_END) == FLUID_FAILED) { /* seek to end of file */
        err = TRUE;
        FLUID_LOG(FLUID_ERR, _("Seek to end of file failed"));
    }
    if (!err && (fsize = fapi->ftell(fd)) == FLUID_FAILED) { /* position = size */
        err = TRUE;
        FLUID_LOG(FLUID_ERR, _("Get end of file position failed"));
    }
    if (!err) fapi->fseek(fd, 0, SEEK_SET);

    if (!err && !load_body(fsize, sf, fd, fapi)) err = TRUE; /* load the sfont */
#endif

    if (err) {
        if (sf) sfont_close(sf, fapi);
        return (NULL);
    }

    return (sf);
}

static int load_body(unsigned int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    SFChunk chunk;

    READCHUNK(&chunk, fd, fapi);        /* load RIFF chunk */
    if (chunkid(chunk.id) != RIFF_ID && chunk.id != COMPRESS_HEADER_INT) { /* error if not RIFF */
        FLUID_LOG(FLUID_ERR, _("Not a RIFF file"));
        return (FAIL);
    }
    if(chunk.id == COMPRESS_HEADER_INT){
        // 下面两行是为了和read_listchunk行为一致
        READID(&chunk.id, fd, fapi); /* read sdta */
        chunk.size -= 4;

        sf->is_compressed = true;
        goto skip_header;
    }

    if (size != 0 && chunk.size != size - 8) {
        FLUID_LOG(FLUID_ERR, _("Sound font file size mismatch"));
        return (FAIL);
    }

    READID(&chunk.id, fd, fapi);        /* load file ID */
    if (chunkid(chunk.id) != SFBK_ID) { /* error if not SFBK_ID */
        FLUID_LOG(FLUID_ERR, _("Not a sound font file"));
        return (FAIL);
    }

    /* Process INFO block */
    if (!read_listchunk(&chunk, fd, fapi)) return (FAIL);
    if (chunkid(chunk.id) != INFO_ID)
        return (gerr(ErrCorr, _("Invalid ID found when expecting INFO chunk")));
    if (!process_info(chunk.size, sf, fd, fapi)) return (FAIL);

    /* Process sample chunk */
    if (!read_listchunk(&chunk, fd, fapi)) return (FAIL);
    skip_header:
    if (chunkid(chunk.id) != SDTA_ID)
        return (gerr(ErrCorr, _("Invalid ID found when expecting SAMPLE chunk")));
    if (!process_sdta(chunk.size, sf, fd, fapi)) return (FAIL);

    /* process HYDRA chunk */
    if (!read_listchunk(&chunk, fd, fapi)) return (FAIL);
    if (chunkid(chunk.id) != PDTA_ID)
        return (gerr(ErrCorr, _("Invalid ID found when expecting HYDRA chunk")));
    if (!process_pdta(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!fixup_pgen(sf)) return (FAIL);
    if (!fixup_igen(sf)) return (FAIL);
    if (!fixup_sample(sf)) return (FAIL);

    /* sort preset list by bank, preset # */
    sf->preset = fluid_list_sort(sf->preset, (fluid_compare_func_t)sfont_preset_compare_func);

    return (OK);
}

static int read_listchunk(SFChunk *chunk, void *fd, fluid_fileapi_t *fapi) {
    READCHUNK(chunk, fd, fapi);        /* read list chunk */
    if (chunkid(chunk->id) != LIST_ID) /* error if ! list chunk */
        return (gerr(ErrCorr, "expect LIST: %s\n", (char *)chunk->id));
    READID(&chunk->id, fd, fapi); /* read id string */
    chunk->size -= 4;
    return (OK);
}

static int process_info(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    SFChunk chunk;
    unsigned char id;
    char *item;
    unsigned short ver;

    while (size > 0) {
        READCHUNK(&chunk, fd, fapi);
        size -= 8;

        id = chunkid(chunk.id);

        if (id == IFIL_ID) { /* sound font version chunk? */
            if (chunk.size != 4)
                return (gerr(ErrCorr, _("Sound font version info chunk has invalid size")));

            READW(ver, fd, fapi);
            sf->version.major = ver;
            READW(ver, fd, fapi);
            sf->version.minor = ver;

            if (sf->version.major < 2) {
                FLUID_LOG(FLUID_ERR,
                          _("Sound font version is %d.%d which is not"
                            " supported, convert to version 2.0x"),
                          sf->version.major, sf->version.minor);
                return (FAIL);
            }

            if (sf->version.major > 2) {
                FLUID_LOG(FLUID_WARN,
                          _("Sound font version is %d.%d which is newer than"
                            " what this version of FLUID Synth was designed "
                            "for (v2.0x)"),
                          sf->version.major, sf->version.minor);
                return (FAIL);
            }
        } else if (id != UNKN_ID) {
            if ((id != ICMT_ID && chunk.size > 256) || (chunk.size > 65536) || (chunk.size % 2))
                return (gerr(ErrCorr,
                             _("INFO sub chunk %.4s has invalid chunk size"
                               " of %d bytes"),
                             &chunk.id, chunk.size));

            /* alloc for chunk id and da chunk */
            if (!(item = FLUID_MALLOC_SF(chunk.size + 1))) {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                return (FAIL);
            }

            /* attach to INFO list, sfont_close will cleanup if FAIL occurs */
            sf->info = fluid_list_append(sf->info, item);

            *(unsigned char *)item = id;
            if (fapi->fread(&item[1], chunk.size, fd) == FLUID_FAILED) return (FAIL);

            /* force terminate info item (don't forget uint8 info ID) */
            *(item + chunk.size) = '\0';
        } else
            return (gerr(ErrCorr, _("Invalid chunk id in INFO chunk")));
        size -= chunk.size;
    }

    if (size < 0) return (gerr(ErrCorr, _("INFO chunk size mismatch")));

    return (OK);
}

static int process_sdta(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    SFChunk chunk;

    READCHUNK(&chunk, fd, fapi);
    if (chunkid(chunk.id) != SMPL_ID)
        return (gerr(ErrCorr, _("Expected SMPL chunk found invalid id instead")));

    if ((size - chunk.size) != 8) return (gerr(ErrCorr, "SDTA chunk size mismatch:%d,%d\n", size, chunk.size));

    /* sample data follows */
    sf->samplepos = fapi->ftell(fd);
    if(sf->is_compressed){
        sf->samplesize = chunk.size * COMPRESS_RATIO;
    }else{
        sf->samplesize = chunk.size;
    }

    FSKIP(chunk.size, fd, fapi);

    return (OK);
}

static int pdtahelper(unsigned int expid, unsigned int reclen, SFChunk *chunk, int *size, void *fd,
                      fluid_fileapi_t *fapi) {
    unsigned int id;
    const char *expstr;

    expstr = CHNKIDSTR(expid); /* in case we need it */
    (void)expstr; // -Wunused-but-set-variable

    READCHUNK(chunk, fd, fapi);
    *size -= 8;

    if ((id = chunkid(chunk->id)) != expid)
        return (gerr(ErrCorr,
                     _("Expected"
                       " PDTA sub-chunk \"%.4s\" found invalid id instead"),
                     expstr));

    if (chunk->size % reclen) /* valid chunk size? */
        return (
            gerr(ErrCorr, _("\"%.4s\" chunk size is not a multiple of %d bytes"), expstr, reclen));
    if ((*size -= chunk->size) < 0)
        return (gerr(ErrCorr, _("\"%.4s\" chunk size exceeds remaining PDTA chunk size"), expstr));
    return (OK);
}

static int process_pdta(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    SFChunk chunk;

    if (!pdtahelper(PHDR_ID, SFPHDRSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_phdr(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!pdtahelper(PBAG_ID, SFBAGSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_pbag(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!pdtahelper(PMOD_ID, SFMODSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_pmod(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!pdtahelper(PGEN_ID, SFGENSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_pgen(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!pdtahelper(IHDR_ID, SFIHDRSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_ihdr(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!pdtahelper(IBAG_ID, SFBAGSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_ibag(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!pdtahelper(IMOD_ID, SFMODSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_imod(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!pdtahelper(IGEN_ID, SFGENSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_igen(chunk.size, sf, fd, fapi)) return (FAIL);

    if (!pdtahelper(SHDR_ID, SFSHDRSIZE, &chunk, &size, fd, fapi)) return (FAIL);
    if (!load_shdr(chunk.size, sf, fd, fapi)) return (FAIL);

    return (OK);
}

/* preset header loader */
static int load_phdr(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    int i, i2;
    SFPreset *p, *pr = NULL; /* ptr to current & previous preset */
    unsigned short zndx, pzndx = 0;

    if (size % SFPHDRSIZE || size == 0)
        return (gerr(ErrCorr, _("Preset header chunk size is invalid")));

    i = size / SFPHDRSIZE - 1;
    if (i == 0) { /* at least one preset + term record */
        FLUID_LOG(FLUID_WARN, _("File contains no presets"));
        FSKIP(SFPHDRSIZE, fd, fapi);
        return (OK);
    }

    for (; i > 0; i--) { /* load all preset headers */
        p = FLUID_NEW_SF(SFPreset);
        sf->preset = fluid_list_append(sf->preset, p);
        p->zone = NULL;             /* In case of failure, sfont_close can cleanup */
        READSTR(p->name, fd, fapi); /* possible read failure ^ */
        READW(p->prenum, fd, fapi);
        READW(p->bank, fd, fapi);
        READW(zndx, fd, fapi);
        READD(p->libr, fd, fapi);
        READD(p->genre, fd, fapi);
        READD(p->morph, fd, fapi);

        if (pr) { /* not first preset? */
            if (zndx < pzndx) return (gerr(ErrCorr, _("Preset header indices not monotonic")));
            i2 = zndx - pzndx;
            while (i2--) {
                pr->zone = fluid_list_prepend(pr->zone, NULL);
            }
        } else if (zndx > 0) /* 1st preset, warn if ofs >0 */
            FLUID_LOG(FLUID_WARN, _("%d preset zones not referenced, discarding"), zndx);
        pr = p; /* update preset ptr */
        pzndx = zndx;
    }

    FSKIP(24, fd, fapi);
    READW(zndx, fd, fapi); /* Read terminal generator index */
    FSKIP(12, fd, fapi);

    if (zndx < pzndx) return (gerr(ErrCorr, _("Preset header indices not monotonic")));
    i2 = zndx - pzndx;
    while (i2--) {
        pr->zone = fluid_list_prepend(pr->zone, NULL);
    }

    return (OK);
}

/* preset bag loader */
static int load_pbag(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    fluid_list_t *p, *p2;
    SFZone *z, *pz = NULL;
    unsigned short genndx, modndx;
    unsigned short pgenndx = 0, pmodndx = 0;
    unsigned short i;

    if (size % SFBAGSIZE || size == 0) /* size is multiple of SFBAGSIZE? */
        return (gerr(ErrCorr, _("Preset bag chunk size is invalid")));

    p = sf->preset;
    while (p) { /* traverse through presets */
        p2 = ((SFPreset *)(p->data))->zone;
        while (p2) { /* traverse preset's zones */
            if ((size -= SFBAGSIZE) < 0)
                return (gerr(ErrCorr, _("Preset bag chunk size mismatch")));
            z = FLUID_NEW_SF(SFZone);
            p2->data = z;
            z->gen = NULL;           /* Init gen and mod before possible failure, */
            z->mod = NULL;           /* to ensure proper cleanup (sfont_close) */
            READW(genndx, fd, fapi); /* possible read failure ^ */
            READW(modndx, fd, fapi);
            z->instsamp = NULL;

            if (pz) { /* if not first zone */
                if (genndx < pgenndx)
                    return (gerr(ErrCorr, _("Preset bag generator indices not monotonic")));
                if (modndx < pmodndx)
                    return (gerr(ErrCorr, _("Preset bag modulator indices not monotonic")));
                i = genndx - pgenndx;
                while (i--) pz->gen = fluid_list_prepend(pz->gen, NULL);
                i = modndx - pmodndx;
                while (i--) pz->mod = fluid_list_prepend(pz->mod, NULL);
            }
            pz = z;           /* update previous zone ptr */
            pgenndx = genndx; /* update previous zone gen index */
            pmodndx = modndx; /* update previous zone mod index */
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    size -= SFBAGSIZE;
    if (size != 0) return (gerr(ErrCorr, _("Preset bag chunk size mismatch")));

    READW(genndx, fd, fapi);
    READW(modndx, fd, fapi);

    if (!pz) {
        if (genndx > 0) FLUID_LOG(FLUID_WARN, _("No preset generators and terminal index not 0"));
        if (modndx > 0) FLUID_LOG(FLUID_WARN, _("No preset modulators and terminal index not 0"));
        return (OK);
    }

    if (genndx < pgenndx) return (gerr(ErrCorr, _("Preset bag generator indices not monotonic")));
    if (modndx < pmodndx) return (gerr(ErrCorr, _("Preset bag modulator indices not monotonic")));
    i = genndx - pgenndx;
    while (i--) pz->gen = fluid_list_prepend(pz->gen, NULL);
    i = modndx - pmodndx;
    while (i--) pz->mod = fluid_list_prepend(pz->mod, NULL);

    return (OK);
}

/* preset modulator loader */
static int load_pmod(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    fluid_list_t *p, *p2, *p3;
    SFMod *m;

    p = sf->preset;
    while (p) { /* traverse through all presets */
        p2 = ((SFPreset *)(p->data))->zone;
        while (p2) { /* traverse this preset's zones */
            p3 = ((SFZone *)(p2->data))->mod;
            while (p3) { /* load zone's modulators */
                if ((size -= SFMODSIZE) < 0)
                    return (gerr(ErrCorr, _("Preset modulator chunk size mismatch")));
                m = FLUID_NEW_SF(SFMod);
                p3->data = m;
                READW(m->src, fd, fapi);
                READW(m->dest, fd, fapi);
                READW(m->amount, fd, fapi);
                READW(m->amtsrc, fd, fapi);
                READW(m->trans, fd, fapi);
                p3 = fluid_list_next(p3);
            }
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    /*
       If there isn't even a terminal record
       Hmmm, the specs say there should be one, but..
     */
    if (size == 0) return (OK);

    size -= SFMODSIZE;
    if (size != 0) return (gerr(ErrCorr, _("Preset modulator chunk size mismatch")));
    FSKIP(SFMODSIZE, fd, fapi); /* terminal mod */

    return (OK);
}

/* -------------------------------------------------------------------
 * preset generator loader
 * generator (per preset) loading rules:
 * Zones with no generators or modulators shall be annihilated
 * Global zone must be 1st zone, discard additional ones (instrumentless zones)
 *
 * generator (per zone) loading rules (in order of decreasing precedence):
 * KeyRange is 1st in list (if exists), else discard
 * if a VelRange exists only preceded by a KeyRange, else discard
 * if a generator follows an instrument discard it
 * if a duplicate generator exists replace previous one
 * ------------------------------------------------------------------- */
static int load_pgen(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    fluid_list_t *p, *p2, *p3, *dup, **hz = NULL;
    SFZone *z;
    SFGen *g;
    SFGenAmount genval;
    unsigned short genid;
    int level, skip, drop, gzone, discarded;

    p = sf->preset;
    while (p) { /* traverse through all presets */
        gzone = FALSE;
        discarded = FALSE;
        p2 = ((SFPreset *)(p->data))->zone;
        if (p2) hz = &p2;
        while (p2) { /* traverse preset's zones */
            level = 0;
            z = (SFZone *)(p2->data);
            p3 = z->gen;
            while (p3) { /* load zone's generators */
                dup = NULL;
                skip = FALSE;
                drop = FALSE;
                if ((size -= SFGENSIZE) < 0)
                    return (gerr(ErrCorr, _("Preset generator chunk size mismatch")));

                READW(genid, fd, fapi);

                if (genid == Gen_KeyRange) { /* nothing precedes */
                    if (level == 0) {
                        level = 1;
                        READB(genval.range.lo, fd, fapi);
                        READB(genval.range.hi, fd, fapi);
                    } else
                        skip = TRUE;
                } else if (genid == Gen_VelRange) { /* only KeyRange precedes */
                    if (level <= 1) {
                        level = 2;
                        READB(genval.range.lo, fd, fapi);
                        READB(genval.range.hi, fd, fapi);
                    } else
                        skip = TRUE;
                } else if (genid == Gen_Instrument) { /* inst is last gen */
                    level = 3;
                    READW(genval.uword, fd, fapi);
                    ((SFZone *)(p2->data))->instsamp = GINT_TO_POINTER(genval.uword + 1);
                    break; /* break out of generator loop */
                } else {
                    level = 2;
                    if (gen_validp(genid)) { /* generator valid? */
                        READW(genval.sword, fd, fapi);
                        dup = gen_inlist(genid, z->gen);
                    } else
                        skip = TRUE;
                }

                if (!skip) {
                    if (!dup) { /* if gen ! dup alloc new */
                        g = FLUID_NEW_SF(SFGen);
                        p3->data = g;
                        g->id = genid;
                    } else {
                        g = (SFGen *)(dup->data); /* ptr to orig gen */
                        drop = TRUE;
                    }
                    g->amount = genval;
                } else { /* Skip this generator */
                    discarded = TRUE;
                    drop = TRUE;
                    FSKIPW(fd, fapi);
                }

                if (!drop)
                    p3 = fluid_list_next(p3); /* next gen */
                else
                    SLADVREM(z->gen, p3); /* drop place holder */

            } /* generator loop */

            if (level == 3)
                SLADVREM(z->gen, p3); /* zone has inst? */
            else {                    /* congratulations its a global zone */
                if (!gzone) {         /* Prior global zones? */
                    gzone = TRUE;

                    /* if global zone is not 1st zone, relocate */
                    if (*hz != p2) {
                        void *save = p2->data;
                        FLUID_LOG(FLUID_WARN, _("Preset \"%s\": Global zone is not first zone"),
                                  ((SFPreset *)(p->data))->name);
                        SLADVREM(*hz, p2);
                        *hz = fluid_list_prepend(*hz, save);
                        continue;
                    }
                } else { /* previous global zone exists, discard */
                    FLUID_LOG(FLUID_WARN, _("Preset \"%s\": Discarding invalid global zone"),
                              ((SFPreset *)(p->data))->name);
                    sfont_zone_delete(sf, hz, (SFZone *)(p2->data));
                }
            }

            while (p3) { /* Kill any zones following an instrument */
                discarded = TRUE;
                if ((size -= SFGENSIZE) < 0)
                    return (gerr(ErrCorr, _("Preset generator chunk size mismatch")));
                FSKIP(SFGENSIZE, fd, fapi);
                SLADVREM(z->gen, p3);
            }

            p2 = fluid_list_next(p2); /* next zone */
        }
        if (discarded)
            FLUID_LOG(FLUID_WARN, _("Preset \"%s\": Some invalid generators were discarded"),
                      ((SFPreset *)(p->data))->name);
        p = fluid_list_next(p);
    }

    /* in case there isn't a terminal record */
    if (size == 0) return (OK);

    size -= SFGENSIZE;
    if (size != 0) return (gerr(ErrCorr, _("Preset generator chunk size mismatch")));
    FSKIP(SFGENSIZE, fd, fapi); /* terminal gen */

    return (OK);
}

/* instrument header loader */
static int load_ihdr(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    int i, i2;
    SFInst *p, *pr = NULL; /* ptr to current & previous instrument */
    unsigned short zndx, pzndx = 0;

    if (size % SFIHDRSIZE || size == 0) /* chunk size is valid? */
        return (gerr(ErrCorr, _("Instrument header has invalid size")));

    size = size / SFIHDRSIZE - 1;
    if (size == 0) { /* at least one preset + term record */
        FLUID_LOG(FLUID_WARN, _("File contains no instruments"));
        FSKIP(SFIHDRSIZE, fd, fapi);
        return (OK);
    }

    for (i = 0; i < size; i++) { /* load all instrument headers */
        p = FLUID_NEW_SF(SFInst);
        sf->inst = fluid_list_append(sf->inst, p);
        p->zone = NULL;             /* For proper cleanup if fail (sfont_close) */
        READSTR(p->name, fd, fapi); /* Possible read failure ^ */
        READW(zndx, fd, fapi);

        if (pr) { /* not first instrument? */
            if (zndx < pzndx) return (gerr(ErrCorr, _("Instrument header indices not monotonic")));
            i2 = zndx - pzndx;
            while (i2--) pr->zone = fluid_list_prepend(pr->zone, NULL);
        } else if (zndx > 0) /* 1st inst, warn if ofs >0 */
            FLUID_LOG(FLUID_WARN, _("%d instrument zones not referenced, discarding"), zndx);
        pzndx = zndx;
        pr = p; /* update instrument ptr */
    }

    FSKIP(20, fd, fapi);
    READW(zndx, fd, fapi);

    if (zndx < pzndx) return (gerr(ErrCorr, _("Instrument header indices not monotonic")));
    i2 = zndx - pzndx;
    while (i2--) pr->zone = fluid_list_prepend(pr->zone, NULL);

    return (OK);
}

/* instrument bag loader */
static int load_ibag(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    fluid_list_t *p, *p2;
    SFZone *z, *pz = NULL;
    unsigned short genndx, modndx, pgenndx = 0, pmodndx = 0;
    int i;

    if (size % SFBAGSIZE || size == 0) /* size is multiple of SFBAGSIZE? */
        return (gerr(ErrCorr, _("Instrument bag chunk size is invalid")));

    p = sf->inst;
    while (p) { /* traverse through inst */
        p2 = ((SFInst *)(p->data))->zone;
        while (p2) { /* load this inst's zones */
            if ((size -= SFBAGSIZE) < 0)
                return (gerr(ErrCorr, _("Instrument bag chunk size mismatch")));
            z = FLUID_NEW_SF(SFZone);
            p2->data = z;
            z->gen = NULL;           /* In case of failure, */
            z->mod = NULL;           /* sfont_close can clean up */
            READW(genndx, fd, fapi); /* READW = possible read failure */
            READW(modndx, fd, fapi);
            z->instsamp = NULL;

            if (pz) { /* if not first zone */
                if (genndx < pgenndx)
                    return (gerr(ErrCorr, _("Instrument generator indices not monotonic")));
                if (modndx < pmodndx)
                    return (gerr(ErrCorr, _("Instrument modulator indices not monotonic")));
                i = genndx - pgenndx;
                while (i--) pz->gen = fluid_list_prepend(pz->gen, NULL);
                i = modndx - pmodndx;
                while (i--) pz->mod = fluid_list_prepend(pz->mod, NULL);
            }
            pz = z; /* update previous zone ptr */
            pgenndx = genndx;
            pmodndx = modndx;
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    size -= SFBAGSIZE;
    if (size != 0) return (gerr(ErrCorr, _("Instrument chunk size mismatch")));

    READW(genndx, fd, fapi);
    READW(modndx, fd, fapi);

    if (!pz) { /* in case that all are no zoners */
        if (genndx > 0)
            FLUID_LOG(FLUID_WARN, _("No instrument generators and terminal index not 0"));
        if (modndx > 0)
            FLUID_LOG(FLUID_WARN, _("No instrument modulators and terminal index not 0"));
        return (OK);
    }

    if (genndx < pgenndx) return (gerr(ErrCorr, _("Instrument generator indices not monotonic")));
    if (modndx < pmodndx) return (gerr(ErrCorr, _("Instrument modulator indices not monotonic")));
    i = genndx - pgenndx;
    while (i--) pz->gen = fluid_list_prepend(pz->gen, NULL);
    i = modndx - pmodndx;
    while (i--) pz->mod = fluid_list_prepend(pz->mod, NULL);

    return (OK);
}

/* instrument modulator loader */
static int load_imod(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    fluid_list_t *p, *p2, *p3;
    SFMod *m;

    p = sf->inst;
    while (p) { /* traverse through all inst */
        p2 = ((SFInst *)(p->data))->zone;
        while (p2) { /* traverse this inst's zones */
            p3 = ((SFZone *)(p2->data))->mod;
            while (p3) { /* load zone's modulators */
                if ((size -= SFMODSIZE) < 0)
                    return (gerr(ErrCorr, _("Instrument modulator chunk size mismatch")));
                m = FLUID_NEW_SF(SFMod);
                p3->data = m;
                READW(m->src, fd, fapi);
                READW(m->dest, fd, fapi);
                READW(m->amount, fd, fapi);
                READW(m->amtsrc, fd, fapi);
                READW(m->trans, fd, fapi);
                p3 = fluid_list_next(p3);
            }
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    /*
       If there isn't even a terminal record
       Hmmm, the specs say there should be one, but..
     */
    if (size == 0) return (OK);

    size -= SFMODSIZE;
    if (size != 0) return (gerr(ErrCorr, _("Instrument modulator chunk size mismatch")));
    FSKIP(SFMODSIZE, fd, fapi); /* terminal mod */

    return (OK);
}

/* load instrument generators (see load_pgen for loading rules) */
static int load_igen(int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    fluid_list_t *p, *p2, *p3, *dup, **hz = NULL;
    SFZone *z;
    SFGen *g;
    SFGenAmount genval;
    unsigned short genid;
    int level, skip, drop, gzone, discarded;

    p = sf->inst;
    while (p) { /* traverse through all instruments */
        gzone = FALSE;
        discarded = FALSE;
        p2 = ((SFInst *)(p->data))->zone;
        if (p2) hz = &p2;
        while (p2) { /* traverse this instrument's zones */
            level = 0;
            z = (SFZone *)(p2->data);
            p3 = z->gen;
            while (p3) { /* load zone's generators */
                dup = NULL;
                skip = FALSE;
                drop = FALSE;
                if ((size -= SFGENSIZE) < 0) return (gerr(ErrCorr, _("IGEN chunk size mismatch")));

                READW(genid, fd, fapi);

                if (genid == Gen_KeyRange) { /* nothing precedes */
                    if (level == 0) {
                        level = 1;
                        READB(genval.range.lo, fd, fapi);
                        READB(genval.range.hi, fd, fapi);
                    } else
                        skip = TRUE;
                } else if (genid == Gen_VelRange) { /* only KeyRange precedes */
                    if (level <= 1) {
                        level = 2;
                        READB(genval.range.lo, fd, fapi);
                        READB(genval.range.hi, fd, fapi);
                    } else
                        skip = TRUE;
                } else if (genid == Gen_SampleId) { /* sample is last gen */
                    level = 3;
                    READW(genval.uword, fd, fapi);
                    ((SFZone *)(p2->data))->instsamp = GINT_TO_POINTER(genval.uword + 1);
                    break; /* break out of generator loop */
                } else {
                    level = 2;
                    if (gen_valid(genid)) { /* gen valid? */
                        READW(genval.sword, fd, fapi);
                        dup = gen_inlist(genid, z->gen);
                    } else
                        skip = TRUE;
                }

                if (!skip) {
                    if (!dup) { /* if gen ! dup alloc new */
                        g = FLUID_NEW_SF(SFGen);
                        p3->data = g;
                        g->id = genid;
                    } else {
                        g = (SFGen *)(dup->data);
                        drop = TRUE;
                    }
                    g->amount = genval;
                } else { /* skip this generator */
                    discarded = TRUE;
                    drop = TRUE;
                    FSKIPW(fd, fapi);
                }

                if (!drop)
                    p3 = fluid_list_next(p3); /* next gen */
                else
                    SLADVREM(z->gen, p3);

            } /* generator loop */

            if (level == 3)
                SLADVREM(z->gen, p3); /* zone has sample? */
            else {                    /* its a global zone */
                if (!gzone) {
                    gzone = TRUE;

                    /* if global zone is not 1st zone, relocate */
                    if (*hz != p2) {
                        void *save = p2->data;
                        FLUID_LOG(FLUID_WARN,
                                  _("Instrument \"%s\": Global zone is not "
                                    "first zone"),
                                  ((SFPreset *)(p->data))->name);
                        SLADVREM(*hz, p2);
                        *hz = fluid_list_prepend(*hz, save);
                        continue;
                    }
                } else { /* previous global zone exists, discard */
                    FLUID_LOG(FLUID_WARN, _("Instrument \"%s\": Discarding invalid global zone"),
                              ((SFInst *)(p->data))->name);
                    sfont_zone_delete(sf, hz, (SFZone *)(p2->data));
                }
            }

            while (p3) { /* Kill any zones following a sample */
                discarded = TRUE;
                if ((size -= SFGENSIZE) < 0)
                    return (gerr(ErrCorr, _("Instrument generator chunk size mismatch")));
                FSKIP(SFGENSIZE, fd, fapi);
                SLADVREM(z->gen, p3);
            }

            p2 = fluid_list_next(p2); /* next zone */
        }
        if (discarded)
            FLUID_LOG(FLUID_WARN, _("Instrument \"%s\": Some invalid generators were discarded"),
                      ((SFInst *)(p->data))->name);
        p = fluid_list_next(p);
    }

    /* for those non-terminal record cases, grr! */
    if (size == 0) return (OK);

    size -= SFGENSIZE;
    if (size != 0) return (gerr(ErrCorr, _("IGEN chunk size mismatch")));
    FSKIP(SFGENSIZE, fd, fapi); /* terminal gen */

    return (OK);
}

/* sample header loader */
static int load_shdr(unsigned int size, SFData *sf, void *fd, fluid_fileapi_t *fapi) {
    unsigned int i;
    SFSample *p;

    if (size % SFSHDRSIZE || size == 0) /* size is multiple of SHDR size? */
        return (gerr(ErrCorr, _("Sample header has invalid size")));

    size = size / SFSHDRSIZE - 1;
    if (size == 0) { /* at least one sample + term record? */
        FLUID_LOG(FLUID_WARN, _("File contains no samples"));
        FSKIP(SFSHDRSIZE, fd, fapi);
        return (OK);
    }

    /* load all sample headers */
    for (i = 0; i < size; i++) {
        p = FLUID_NEW_SF(SFSample);
        sf->sample = fluid_list_append(sf->sample, p);
        READSTR(p->name, fd, fapi);
        READD(p->start, fd, fapi);
        READD(p->end, fd, fapi);       /* - end, loopstart and loopend */
        READD(p->loopstart, fd, fapi); /* - will be checked and turned into */
        READD(p->loopend, fd, fapi);   /* - offsets in fixup_sample() */
        READD(p->samplerate, fd, fapi);
        READB(p->origpitch, fd, fapi);
        READB(p->pitchadj, fd, fapi);
        FSKIPW(fd, fapi); /* skip sample link */
        READW(p->sampletype, fd, fapi);
    }

    FSKIP(SFSHDRSIZE, fd, fapi); /* skip terminal shdr */

    return (OK);
}

/* "fixup" (inst # -> inst ptr) instrument references in preset list */
static int fixup_pgen(SFData *sf) {
    fluid_list_t *p, *p2, *p3;
    SFZone *z;
    uintptr i;

    p = sf->preset;
    while (p) {
        p2 = ((SFPreset *)(p->data))->zone;
        while (p2) { /* traverse this preset's zones */
            z = (SFZone *)(p2->data);
            if ((i = GPOINTER_TO_INT(z->instsamp))) { /* load instrument # */
                p3 = fluid_list_nth(sf->inst, i - 1);
                if (!p3)
                    return (gerr(ErrCorr, _("Preset %03d %03d: Invalid instrument reference"),
                                 ((SFPreset *)(p->data))->bank, ((SFPreset *)(p->data))->prenum));
                z->instsamp = p3;
            } else
                z->instsamp = NULL;
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    return (OK);
}

/* "fixup" (sample # -> sample ptr) sample references in instrument list */
static int fixup_igen(SFData *sf) {
    fluid_list_t *p, *p2, *p3;
    SFZone *z;
    uintptr i;

    p = sf->inst;
    while (p) {
        p2 = ((SFInst *)(p->data))->zone;
        while (p2) { /* traverse instrument's zones */
            z = (SFZone *)(p2->data);
            if ((i = GPOINTER_TO_INT(z->instsamp))) { /* load sample # */
                p3 = fluid_list_nth(sf->sample, i - 1);
                if (!p3)
                    return (gerr(ErrCorr, _("Instrument \"%s\": Invalid sample reference"),
                                 ((SFInst *)(p->data))->name));
                z->instsamp = p3;
            }
            p2 = fluid_list_next(p2);
        }
        p = fluid_list_next(p);
    }

    return (OK);
}

/* convert sample end, loopstart and loopend to offsets and check if valid */
static int fixup_sample(SFData *sf) {
    fluid_list_t *p;
    SFSample *sam;

    p = sf->sample;
    while (p) {
        sam = (SFSample *)(p->data);

        /* if sample end is over the sample data chunk
           or sam start is greater than 4 less than the end (at least 4 samples)
         */
        if (sam->end > sf->samplesize || sam->start > (sam->end - 4)) {
            FLUID_LOG(FLUID_WARN,
                      _("Sample '%s' start/end file positions are invalid,"
                        " disabling and will not be saved"),
                      sam->name);

            /* disable sample by setting all sample markers to 0 */
            sam->start = sam->end = sam->loopstart = sam->loopend = 0;

            return (OK);
        }
        else if (sam->loopend > sam->end || sam->loopstart >= sam->loopend ||
                   sam->loopstart <= sam->start) { /* loop is fowled?? (cluck cluck :) */
            /* can pad loop by 8 samples and ensure at least 4 for loop (2*8+4)
             */
            if ((sam->end - sam->start) >= 20) {
                sam->loopstart = sam->start + 8;
                sam->loopend = sam->end - 8;
            } else { /* loop is fowled, sample is tiny (can't pad 8 samples) */
                sam->loopstart = sam->start + 1;
                sam->loopend = sam->end - 1;
            }
        }

        /* convert sample end, loopstart, loopend to offsets from sam->start */
        sam->end -= sam->start + 1; /* marks last sample, contrary to SF spec. */
        sam->loopstart -= sam->start;
        sam->loopend -= sam->start;

        p = fluid_list_next(p);
    }

    return (OK);
}

/*=================================sfont.c========================
  Smurf SoundFont Editor
  ================================================================*/

/* optimum chunk area sizes (could be more optimum) */
#define PRESET_CHUNK_OPTIMUM_AREA 256
#define INST_CHUNK_OPTIMUM_AREA 256
#define SAMPLE_CHUNK_OPTIMUM_AREA 256
#define ZONE_CHUNK_OPTIMUM_AREA 256
#define MOD_CHUNK_OPTIMUM_AREA 256
#define GEN_CHUNK_OPTIMUM_AREA 256

/* list of bad generators */
const unsigned short badgen[] = {Gen_Unused1,   Gen_Unused2,   Gen_Unused3,   Gen_Unused4,
                                 Gen_Reserved1, Gen_Reserved2, Gen_Reserved3, 0};

/* list of bad preset generators */
const unsigned short badpgen[] = {Gen_StartAddrOfs,
                                  Gen_EndAddrOfs,
                                  Gen_StartLoopAddrOfs,
                                  Gen_EndLoopAddrOfs,
                                  Gen_StartAddrCoarseOfs,
                                  Gen_EndAddrCoarseOfs,
                                  Gen_StartLoopAddrCoarseOfs,
                                  Gen_Keynum,
                                  Gen_Velocity,
                                  Gen_EndLoopAddrCoarseOfs,
                                  Gen_SampleModes,
                                  Gen_ExclusiveClass,
                                  Gen_OverrideRootKey,
                                  0};

/* close SoundFont file and delete a SoundFont structure */
void sfont_close(SFData *sf, fluid_fileapi_t *fapi) {
    fluid_list_t *p, *p2;

    if (sf->sffd) fapi->fclose(sf->sffd);

    if (sf->fname) free(sf->fname);

    p = sf->info;
    while (p) {
        FLUID_FREE_SF(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(sf->info);
    sf->info = NULL;

    p = sf->preset;
    while (p) { /* loop over presets */
        p2 = ((SFPreset *)(p->data))->zone;
        while (p2) { /* loop over preset's zones */
            sfont_free_zone(p2->data);
            p2 = fluid_list_next(p2);
        } /* free preset's zone list */
        delete_fluid_list(((SFPreset *)(p->data))->zone);
        FLUID_FREE_SF(p->data); /* free preset chunk */
        p = fluid_list_next(p);
    }
    delete_fluid_list(sf->preset);
    sf->preset = NULL;

    p = sf->inst;
    while (p) { /* loop over instruments */
        p2 = ((SFInst *)(p->data))->zone;
        while (p2) { /* loop over inst's zones */
            sfont_free_zone(p2->data);
            p2 = fluid_list_next(p2);
        } /* free inst's zone list */
        delete_fluid_list(((SFInst *)(p->data))->zone);
        FLUID_FREE_SF(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(sf->inst);
    sf->inst = NULL;

    p = sf->sample;
    while (p) {
        FLUID_FREE_SF(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(sf->sample);
    sf->sample = NULL;

    FLUID_FREE_SF(sf);
}

/* free all elements of a zone (Preset or Instrument) */
void sfont_free_zone(SFZone *zone) {
    fluid_list_t *p;

    if (!zone) return;

    p = zone->gen;
    while (p) { /* Free gen chunks for this zone */
        if (p->data) FLUID_FREE_SF(p->data);
        // fluid_sf_gen_delete((fluid_sf_gen_t *)p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(zone->gen); /* free genlist */

    p = zone->mod;
    while (p) { /* Free mod chunks for this zone */
        if (p->data) FLUID_FREE_SF(p->data);
        p = fluid_list_next(p);
    }
    delete_fluid_list(zone->mod); /* free modlist */

    FLUID_FREE_SF(zone); /* free zone chunk */
}

/* preset sort function, first by bank, then by preset # */
int sfont_preset_compare_func(void *a, void *b) {
    int aval, bval;

    aval = (int)(((SFPreset *)a)->bank) << 16 | ((SFPreset *)a)->prenum;
    bval = (int)(((SFPreset *)b)->bank) << 16 | ((SFPreset *)b)->prenum;

    return (aval - bval);
}

/* delete zone from zone list */
void sfont_zone_delete(SFData *sf, fluid_list_t **zlist, SFZone *zone) {
    *zlist = fluid_list_remove(*zlist, (void *)zone);
    sfont_free_zone(zone);
}

/* Find generator in gen list */
fluid_list_t *gen_inlist(int gen, fluid_list_t *genlist) { /* is generator in gen list? */
    fluid_list_t *p;

    p = genlist;
    while (p) {
        if (p->data == NULL) return (NULL);
        if (gen == ((SFGen *)p->data)->id) break;
        p = fluid_list_next(p);
    }
    return (p);
}

/* check validity of instrument generator */
int gen_valid(int gen) { /* is generator id valid? */
    int i = 0;

    if (gen > Gen_MaxValid) return (FALSE);
    while (badgen[i] && badgen[i] != gen) i++;
    return (badgen[i] == 0);
}

/* check validity of preset generator */
int gen_validp(int gen) { /* is preset generator valid? */
    int i = 0;

    if (!gen_valid(gen)) return (FALSE);
    while (badpgen[i] && badpgen[i] != (unsigned short)gen) i++;
    return (badpgen[i] == 0);
}


#define SKIP_sdtasmpl 12  // 12：4字节"sdta"+4字节"smpl"+4字节size

bool compress_sf2(const char *fname, const char *out_file, compress_callback ccb) {
    SFChunk chunk;
    FILE *fd;
    fluid_fileapi_t *fapi = fluid_get_default_fileapi();

    if ((fd = fapi->fopen(fapi, fname)) == NULL) {
        FLUID_LOG(FLUID_ERR, _("Unable to open file \"%s\""), fname);
        return (FAIL);
    }
    FILE* out = fopen(out_file, "wb");
    if (!out){
        FLUID_LOG(FLUID_ERR, _("Unable to open file \"%s\""), out_file);
        return (FAIL);
    }

    READCHUNK(&chunk, fd, fapi);        /* load RIFF chunk */
    if (chunkid(chunk.id) != RIFF_ID) { /* error if not RIFF */
        FLUID_LOG(FLUID_ERR, _("Not a RIFF file"));
        return (FAIL);
    }

    READID(&chunk.id, fd, fapi);        /* load file ID */
    if (chunkid(chunk.id) != SFBK_ID) { /* error if not SFBK_ID */
        FLUID_LOG(FLUID_ERR, _("Not a sound font file"));
        return (FAIL);
    }

    /* Process INFO block */
    if (!read_listchunk(&chunk, fd, fapi)) return (FAIL);
    if (chunkid(chunk.id) != INFO_ID)
        return (gerr(ErrCorr, _("Invalid ID found when expecting INFO chunk")));

    if (fapi->fseek(fd, chunk.size, SEEK_CUR) == FLUID_FAILED) {
        return (gerr(ErrCorr, _("Failed to seek position in data file")));
    }
    //以上的都不写入out

    /* Process sample chunk */
    READCHUNK(&chunk, fd, fapi);        /* read list chunk */
    if (chunkid(chunk.id) != LIST_ID) /* error if ! list chunk */
        return (gerr(ErrCorr, _("Invalid ID found when expecting LIST chunk")));
    
    SFChunk zip_header;
    zip_header.id = COMPRESS_HEADER_INT;
    zip_header.size = (chunk.size - SKIP_sdtasmpl)/COMPRESS_RATIO + SKIP_sdtasmpl;
    fwrite(&zip_header, sizeof(chunk), 1, out);

    READID(&chunk.id, fd, fapi);
    if (chunkid(chunk.id) != SDTA_ID)
        return (gerr(ErrCorr, _("Invalid ID found when expecting SAMPLE chunk")));
    fwrite(&chunk.id, sizeof(chunk.id), 1, out);

    SFChunk chunk_smpl;
    READCHUNK(&chunk_smpl, fd, fapi);
    if (chunkid(chunk_smpl.id) != SMPL_ID)
        return (gerr(ErrCorr, _("Expected SMPL chunk found invalid id instead")));

    if ((chunk.size - chunk_smpl.size) != SKIP_sdtasmpl){
        return (gerr(ErrCorr, "SDTA chunk size mismatch:%d,%d\n", chunk.size, chunk_smpl.size));
    }
        
    {
        int orig_size = chunk_smpl.size;
        if(orig_size % COMPRESS_RATIO != 0) return (gerr(ErrCorr, "orig_size %d %% 4 != 0 \n", orig_size));
        int compressed_size = chunk_smpl.size/COMPRESS_RATIO;
        chunk_smpl.size = compressed_size;
        fwrite(&chunk_smpl, sizeof(chunk_smpl), 1, out);

        char *buffer = malloc(compressed_size);
        char *orig_buf = malloc(orig_size);
        fapi->fread(orig_buf, orig_size, fd);

        bool flag = ccb(buffer, compressed_size, orig_buf, orig_size);
        if(!flag) return (gerr(ErrCorr, "compress error"));

        fwrite(buffer, compressed_size, 1, out);
        free(buffer);
        free(orig_buf);
    }

    /* process HYDRA chunk */
    READCHUNK(&chunk, fd, fapi);        /* read list chunk */
    if (chunkid(chunk.id) != LIST_ID) /* error if ! list chunk */
        return (gerr(ErrCorr, _("Invalid ID found when expecting LIST chunk")));

    fwrite(&chunk, sizeof(chunk), 1, out);
    {
        int read_size = chunk.size;
        char *buffer = malloc(read_size);
        fapi->fread(buffer, read_size, fd);
        fwrite(buffer, read_size, 1, out);
        free(buffer);
    }
    fclose(out);
    fclose(fd);
    return (OK);
}
