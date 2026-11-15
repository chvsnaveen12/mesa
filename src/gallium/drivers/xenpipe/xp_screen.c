#include <stdio.h>
#include "xp_screen.h"
#include "xp_public.h"
#include "xp_texture.h"
#include "xp_context.h"
#include "util/u_memory.h"
#include "nir.h"
#include "util/u_screen.h"

static void xenpipe_destroy_screen(struct pipe_screen *screen){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_destroy_screen\n");
    exit(-1);
}

static const char *xenpipe_get_name(struct pipe_screen *screen){
    fprintf(stderr, "STUB: xenpipe_get_name\n");
    return "balls";
}

static int xenpipe_get_screen_fd(struct pipe_screen *screen){
    fprintf(stderr, "STUB: xenpipe_get_screen_fd\n");
    return -1;
}

static bool xenpipe_is_format_supported(struct pipe_screen *screen,
                                        enum pipe_format format,
                                        enum pipe_texture_target target,
                                        unsigned sample_count,
                                        unsigned storage_sample_count,
                                        unsigned bindings){
    if(sample_count > 1 || storage_sample_count > 1)
        return false;
    return true;
}

static void xenpipe_flush_frontbuffer(struct pipe_screen *_screen,
                                      struct pipe_context *pipe,
                                      struct pipe_resource *resource,
                                      unsigned level, unsigned layer,
                                      void *context_private,
                                      unsigned nboxes,
                                      struct pipe_box *box){
    fprintf(stderr, "STUB: xenpipe_flush_frontbuffer\n");
    static int frame_count = 0;
    frame_count++;
    struct xenpipe_screen *screen = xenpipe_screen(_screen);
    struct sw_winsys *winsys = screen->winsys;
    struct xenpipe_resource *texture = xenpipe_resource(resource);
    assert(texture->dt);

    if(texture->dt && texture->data){
        uint8_t *dt_map = winsys->displaytarget_map(winsys, texture->dt, PIPE_MAP_WRITE);
        if(dt_map){
            unsigned stride = texture->stride[0];
            unsigned h = resource->height0;
            memcpy(dt_map, texture->data, stride * h);
            winsys->displaytarget_unmap(winsys, texture->dt);
        }
    }
    if(texture->dt)
        winsys->displaytarget_display(winsys, texture->dt, context_private, nboxes, box);
}

static const nir_shader_compiler_options xp_nir_options = {
};

static void xenpipe_fence_reference(struct pipe_screen *screen,
                                    struct pipe_fence_handle **ptr,
                                    struct pipe_fence_handle *fence){
    fprintf(stderr, "STUB: xenpipe_fence_reference\n");
    *ptr = fence;
}

static bool xenpipe_fence_finish(struct pipe_screen *screen,
                                 struct pipe_context *ctx,
                                 struct pipe_fence_handle *fence,
                                 uint64_t timeout){
    fprintf(stderr, "STUB: xenpipe_fence_reference\n");
    return true;
}

static void xenpipe_init_shader_caps(struct xenpipe_screen *screen){
    fprintf(stderr, "STUB: xenpipe_init_shader_caps\n");

    for(unsigned i = 0; i < MESA_ALL_SHADER_STAGES; i++){
        if(i != MESA_SHADER_VERTEX || i != MESA_SHADER_FRAGMENT)
            continue;
        
        struct pipe_shader_caps *caps = (struct pipe_shader_caps *)&screen->base.shader_caps[i];
        caps->max_instructions =
        caps->max_alu_instructions =
        caps->max_tex_indirections =
        caps->max_tex_instructions = 16384;

        caps->max_control_flow_depth = 8;
        caps->max_inputs = 8;
        caps->max_outputs = i == MESA_SHADER_FRAGMENT ? 1 : 8;
        caps->max_temps = 256;
        caps->max_const_buffer0_size = 16 * 1024 * sizeof(float);
        caps->max_const_buffers = 1;
        caps->indirect_const_addr = true;
        caps->integers = true;
        caps->max_texture_samplers = i == MESA_SHADER_FRAGMENT ? 8 : 0;
        caps->supported_irs = BITFIELD_BIT(PIPE_SHADER_IR_NIR);
    }
}

static void xenpipe_init_screen_caps(struct xenpipe_screen *screen){
    fprintf(stderr, "STUB: xenpipe_init_screen_caps\n");

    struct pipe_caps *caps = (struct pipe_caps *)&screen->base.caps;
    u_init_pipe_screen_caps(&screen->base, true);

    caps->vertex_color_unclamped = true;
    caps->fragment_color_clamped = true;
    caps->npot_textures = true;
    caps->blend_equation_separate = true;
    caps->texture_multisample = false;
    caps->texture_barrier = true;
    caps->mixed_framebuffer_sizes = true;
    caps->mixed_color_depth_bits = true;
    caps->max_texture_2d_size = 2048;
    caps->max_texture_cube_levels = XP_MAX_MIP_LEVELS;
    caps->max_texture_3d_levels = 0;
    caps->max_varyings = 8;
    caps->video_memory = 256 * 1024 * 1024;
    caps->uma = true;
    caps->alpha_test = false;
    caps->vertex_color_clamped = false;
    caps->two_sided_color = false;
    caps->texrect = false;
    caps->image_store_formatted = false;
    caps->clip_planes = 0;

    caps->supported_prim_modes = BITFIELD_BIT(MESA_PRIM_POINTS)         |
                                 BITFIELD_BIT(MESA_PRIM_LINES)          |
                                 BITFIELD_BIT(MESA_PRIM_LINE_LOOP)      |
                                 BITFIELD_BIT(MESA_PRIM_LINE_STRIP)     |
                                 BITFIELD_BIT(MESA_PRIM_TRIANGLES)      |
                                 BITFIELD_BIT(MESA_PRIM_TRIANGLE_STRIP) |
                                 BITFIELD_BIT(MESA_PRIM_TRIANGLE_FAN);
    
    caps->min_line_width =
    caps->min_line_width_aa =
    caps->min_point_size =
    caps->min_point_size_aa = 1;
    caps->point_size_granularity =
    caps->line_width_granularity = 0.1;
    caps->max_line_width =
    caps->max_line_width_aa = 32;
    caps->max_point_size =
    caps->max_point_size_aa = 512.0f;

}

struct pipe_screen *xenpipe_create_screen(struct sw_winsys *winsys){
    fprintf(stderr, "STUB: xenpipe_create_screen\n");
    fprintf(stderr, "xenpipe_create_screen: Ign fmt sup prints\n");
    struct xenpipe_screen *screen = CALLOC_STRUCT(xenpipe_screen);
    if(!screen)
        return NULL;

    screen->winsys = winsys;
    screen->base.destroy = xenpipe_destroy_screen;
    screen->base.get_name =
    screen->base.get_vendor =
    screen->base.get_device_vendor = xenpipe_get_name;
    screen->base.get_screen_fd = xenpipe_get_screen_fd;
    screen->base.is_format_supported = xenpipe_is_format_supported;
    screen->base.context_create = xenpipe_context_create;
    screen->base.flush_frontbuffer = xenpipe_flush_frontbuffer;
    
    screen->base.nir_options[MESA_SHADER_VERTEX] = &xp_nir_options;
    screen->base.nir_options[MESA_SHADER_FRAGMENT] = &xp_nir_options;

    xenpipe_init_texture_funcs(&screen->base);
    screen->base.fence_reference = xenpipe_fence_reference;
    screen->base.fence_finish = xenpipe_fence_finish;

    xenpipe_init_shader_caps(screen);
    xenpipe_init_screen_caps(screen);
    
    return &screen->base;
}
