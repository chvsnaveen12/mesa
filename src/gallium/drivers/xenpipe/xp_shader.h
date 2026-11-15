#ifndef XP_SHADER
#define XP_SHADER

#include "nir/nir.h"

struct xenpipe_context;
struct xp_clip_vert;
struct xp_quad;

typedef union xp_reg{
    float f;
    uint32_t u;
    int32_t i;
} xp_reg;

struct xp_interp_state{
    struct xenpipe_context *xenpipe;
    float (*in)[4];
    struct xp_clip_vert *out;
    
    float color_out[4];
    
    xp_reg ssa[1024][4];
    nir_shader *nir;
    mesa_shader_stage stage;
};


void xp_run_vs(struct xenpipe_context *xenpipe,
               float inps[][4], struct xp_clip_vert *out);

void xp_run_fs(struct xenpipe_context *xenpipe,
               struct xp_quad *quad);

#endif