#include <stdio.h>
#include "xp_context.h"
#include "xp_state.h"
#include "xp_draw.h"
#include "util/u_upload_mgr.h"
#include "util/u_inlines.h"
#include "util/u_surface.h"

static void xenpipe_destroy(struct pipe_context *pipe){
    fprintf(stderr, "STUB: xenpipe_destroy\n");
    struct xenpipe_context *xenpipe = xenpipe_context(pipe);

    if(xenpipe->base.stream_uploader)
        u_upload_destroy(pipe->stream_uploader);
    
    pipe_resource_reference(&xenpipe->vs_cbuf.buffer, NULL);
    pipe_resource_reference(&xenpipe->fs_cbuf.buffer, NULL);

    FREE(xenpipe);
}

static void xenpipe_draw_vbo(struct pipe_context *pipe,
                             const struct pipe_draw_info *info,
                             unsigned drawid_offset,
                             const struct pipe_draw_indirect_info *indirect,
                             const struct pipe_draw_start_count_bias *draws,
                             unsigned num_draws){
    fprintf(stderr, "STUB: xenpipe_draw_vbo\n");
    xenpipe_do_draw_vbo(xenpipe_context(pipe), info, draws, num_draws);
}

static void xenpipe_set_framebuffer_state(struct pipe_context *pipe,
                                          const struct pipe_framebuffer_state *fb){
    fprintf(stderr, "STUB: xenpipe_set_framebuffer_state\n");
    xenpipe_context(pipe)->framebuffer = *fb;
}

static void xenpipe_flush(struct pipe_context *pipe,
                          struct pipe_fence_handle **fence,
                          unsigned flags){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_flush\n");
    // no-op for now
}

static void xenpipe_clear(struct pipe_context *pipe,
                          unsigned buffers,
                          unsigned color_clear_mask,
                          unsigned char stencil_clear_mask,
                          const struct pipe_scissor_state *scissor_state,
                          const union pipe_color_union *color,
                          double depth,
                          unsigned stencil){
    fprintf(stderr, "STUB: xenpipe_clear\n");

    struct xenpipe_context *xenpipe = xenpipe_context(pipe);
    unsigned w = xenpipe->framebuffer.width;
    unsigned h = xenpipe->framebuffer.height;

    if(xenpipe->framebuffer.nr_cbufs && (buffers & PIPE_CLEAR_COLOR0))
        util_clear_render_target(pipe, &xenpipe->framebuffer.cbufs[0], color,
                                 0, 0, w, h);

    if(xenpipe->framebuffer.zsbuf.texture && (buffers & (PIPE_CLEAR_DEPTHSTENCIL)))
        util_clear_depth_stencil(pipe, &xenpipe->framebuffer.zsbuf,
                                 buffers & (PIPE_CLEAR_DEPTHSTENCIL),
                                 depth, stencil, 0, 0, w, h);
}

struct pipe_context *xenpipe_context_create(struct pipe_screen *screen,
                                            void *priv, unsigned flags){
    fprintf(stderr, "STUB: xenpipe_context_create\n");
    struct xenpipe_context *pipe = CALLOC_STRUCT(xenpipe_context);

    util_init_math();

    pipe->base.screen = screen;
    pipe->base.destroy = xenpipe_destroy;
    pipe->base.priv = priv;

    pipe->base.draw_vbo = xenpipe_draw_vbo;
    pipe->base.clear = xenpipe_clear;
    pipe->base.flush = xenpipe_flush;
    pipe->base.set_framebuffer_state = xenpipe_set_framebuffer_state;

    xenpipe_init_state_blend_funcs(&pipe->base);
    xenpipe_init_state_clip_funcs(&pipe->base);
    xenpipe_init_state_rasterizer_funcs(&pipe->base);
    xenpipe_init_state_sampler_funcs(&pipe->base);
    xenpipe_init_state_shader_funcs(&pipe->base);
    xenpipe_init_state_texture_funcs(&pipe->base);
    xenpipe_init_state_vertex_funcs(&pipe->base);

    pipe->base.const_uploader = 
    pipe->base.stream_uploader = u_upload_create_default(&pipe->base);
    return &pipe->base;
}