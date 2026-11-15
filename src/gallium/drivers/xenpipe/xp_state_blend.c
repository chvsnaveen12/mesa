#include <stdio.h>
#include "pipe/p_state.h"
#include "xp_context.h"
#include "xp_state.h"

static void *xenpipe_create_blend_state(struct pipe_context *pipe,
                                        const struct pipe_blend_state *templ){
    fprintf(stderr, "STUB: xenpipe_create_dsa_state\n");
    struct pipe_blend_state *state = CALLOC_STRUCT(pipe_blend_state);
    if(!state)
        return NULL;
    *state = *templ;
    return state;
}

static void xenpipe_bind_blend_state(struct pipe_context *pipe, void *blend){
    fprintf(stderr, "STUB: xenpipe_bind_blend_state\n");
    xenpipe_context(pipe)->blend = blend;
}

static void xenpipe_delete_blend_state(struct pipe_context *pipe, void *blend){
    fprintf(stderr, "STUB: xenpipe_delete_blend_state\n");
    FREE(blend);
}

static void xenpipe_set_blend_color(struct pipe_context *pipe,
                                    const struct pipe_blend_color *blend_color){
    fprintf(stderr, "STUB: xenpipe_set_blend_color\n");
    xenpipe_context(pipe)->blend_color = *blend_color;
}

static void *xenpipe_create_dsa_state(struct pipe_context *pipe,
                                      const struct pipe_depth_stencil_alpha_state *templ){
    fprintf(stderr, "STUB: xenpipe_create_dsa_state\n");
    struct pipe_depth_stencil_alpha_state *state = CALLOC_STRUCT(pipe_depth_stencil_alpha_state);
    if(!state)
        return NULL;
    *state = *templ;
    return state;
}

static void xenpipe_bind_dsa_state(struct pipe_context *pipe, void *dsa){
    fprintf(stderr, "STUB: xenpipe_bind_dsa_state\n");
    xenpipe_context(pipe)->depth_stencil = dsa;
}

static void xenpipe_delete_dsa_state(struct pipe_context *pipe, void *dsa){
    fprintf(stderr, "STUB: xenpipe_delete_dsa_state\n");
    FREE(dsa);
}

static void xenpipe_set_stencil_ref(struct pipe_context *pipe,
                                    const struct pipe_stencil_ref stencil_ref){
    fprintf(stderr, "STUB: xenpipe_set_stencil_ref\n");
    xenpipe_context(pipe)->stencil_ref = stencil_ref;
}

static void xenpipe_set_sample_mask(struct pipe_context *pipe,
                                    unsigned sample_mask){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_set_sample_mask\n");
    // exit(-1);
}

void xenpipe_init_state_blend_funcs(struct pipe_context *pipe){
    fprintf(stderr, "STUB: xenpipe_init_state_blend_funcs\n");

    pipe->create_blend_state = xenpipe_create_blend_state;
    pipe->bind_blend_state = xenpipe_bind_blend_state;
    pipe->delete_blend_state = xenpipe_delete_blend_state;
    pipe->set_blend_color = xenpipe_set_blend_color;
    pipe->create_depth_stencil_alpha_state = xenpipe_create_dsa_state;
    pipe->bind_depth_stencil_alpha_state = xenpipe_bind_dsa_state;
    pipe->delete_depth_stencil_alpha_state = xenpipe_delete_dsa_state;
    pipe->set_stencil_ref = xenpipe_set_stencil_ref;
    pipe->set_sample_mask = xenpipe_set_sample_mask;
}