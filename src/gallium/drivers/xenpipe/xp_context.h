#ifndef XP_CONTEXT
#define XP_CONTEXT

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "nir/nir.h"

#define XP_MAX_ATTRIBS 8
#define XP_MAX_SAMPLERS 8

struct pipe_screen;

struct xp_vertex_element_state{
    unsigned num_elements;
    struct pipe_vertex_element elements[XP_MAX_ATTRIBS];
};

struct xenpipe_context{
    struct pipe_context base;
    struct pipe_framebuffer_state framebuffer;

    struct pipe_viewport_state viewport;
    struct pipe_vertex_buffer vertex_buffers[XP_MAX_ATTRIBS];
    struct xp_vertex_element_state *velems;

    nir_shader *vs;
    nir_shader *fs;

    struct pipe_blend_state *blend;
    struct pipe_scissor_state scissor;
    struct pipe_rasterizer_state *rasterizer;
    struct pipe_blend_color blend_color;

    struct pipe_constant_buffer vs_cbuf;
    struct pipe_constant_buffer fs_cbuf;

    struct pipe_depth_stencil_alpha_state *depth_stencil;
    struct pipe_stencil_ref stencil_ref;

    struct pipe_sampler_view *sampler_views[XP_MAX_SAMPLERS];
    unsigned num_sampler_views;
    struct pipe_sampler_state *samplers[XP_MAX_SAMPLERS];
    unsigned num_samplers;
};

struct pipe_context *xenpipe_context_create(struct pipe_screen *screen,
                                            void *priv, unsigned flags);

static inline struct xenpipe_context *xenpipe_context(struct pipe_context *pipe){
    return (struct xenpipe_context *)pipe;
}

#endif