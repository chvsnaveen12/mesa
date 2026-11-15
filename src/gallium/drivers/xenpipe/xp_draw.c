#include <stdio.h>
#include "xp_draw.h"
#include "xp_context.h"
#include "xp_shader.h"
#include "xp_raster.h"
#include "xp_texture.h"
#include "pipe/p_state.h"

static unsigned xp_get_idx(const uint8_t *idx_buf, unsigned idx_sz, unsigned idx){
    if(!idx_sz)
        return idx;
    
    switch(idx_sz){
        case 1: return (( uint8_t *)idx_buf)[idx];
        case 2: return ((uint16_t *)idx_buf)[idx];
        case 4: return ((uint32_t *)idx_buf)[idx];
        default:
            fprintf(stderr, "get_idx: Unsup idx_sz");
            exit(-1);
    }
}

static void xp_fetch_attribs(struct xenpipe_context *xenpipe,
                             float inps[][4], unsigned idx){
    for(unsigned i = 0; i < xenpipe->velems->num_elements; i++){
        struct pipe_vertex_element *velem = &xenpipe->velems->elements[i];
        struct pipe_vertex_buffer *vbuf = &xenpipe->vertex_buffers[velem->vertex_buffer_index];

        const uint8_t *map = NULL;
        if(vbuf->is_user_buffer)
            map = vbuf->buffer.user;
        else
            map = xenpipe_resource(vbuf->buffer.resource)->data;
        
        unsigned off = velem->src_offset + vbuf->buffer_offset + idx * velem->src_stride; 

        util_format_unpack_rgba(velem->src_format, inps[i], map+off, 1);
    }
}

// Think about this
static float xp_clip_dot(struct xp_clip_vert *vert, unsigned plane){
    unsigned axis = plane >> 1;
    unsigned sign = plane & 1;
    float val = vert->pos[axis];
    float w = vert->pos[3];

    if(sign)    return w - val;
    else        return w + val;
}

static void xp_clip_lerp(struct xp_clip_vert *src_a,
                         struct xp_clip_vert *src_b,
                         struct xp_clip_vert *dst,
                         float t){
    for(unsigned i = 0; i < 4; i++)
        dst->pos[i] = ((1-t) * src_a->pos[i]) + ((t) * src_b->pos[i]);

    for(unsigned v = 0; v < 8; v++)
        for(unsigned i = 0; i < 4; i++)
            dst->var[v][i] = ((1-t) * src_a->var[v][i]) + ((t) * src_b->var[v][i]); 

}

static unsigned xp_clip_plane(struct xp_clip_vert *src,
                              struct xp_clip_vert *dst,
                              unsigned n_in, unsigned plane){
    unsigned n_out = 0;

    for(unsigned i = 0; i < n_in; i++){
        unsigned j = (i+1) % n_in;

        float di = xp_clip_dot(&src[i], plane);
        float dj = xp_clip_dot(&src[j], plane);

        if(di >= 0){
            dst[n_out++] = src[i];

            if(dj < 0){
                float t = di / (di - dj);
                xp_clip_lerp(&src[i], &src[j], &dst[n_out++], t);
            }
        }
        else{
            if(dj >= 0){
                float t = di / (di - dj);
                xp_clip_lerp(&src[i], &src[j], &dst[n_out++], t);
            }
        }
    }
    return n_out;
}

static unsigned xp_clip(struct xp_clip_vert *verts){
    struct xp_clip_vert buf[12];

    unsigned n = 3;

    for(unsigned i = 0; i < 6; i++){
        struct xp_clip_vert *src = i & 1 ? buf : verts; 
        struct xp_clip_vert *dst = i & 1 ? verts : buf;
        
        n = xp_clip_plane(src, dst, n, i);
        if(n < 3)
            return 0;
    }
    return n;
}

static void xp_clip_to_window(struct xenpipe_context *xenpipe,
                              struct xp_clip_vert *vert){
    float inv_w = 1.0f /vert->pos[3];
    struct pipe_viewport_state *vp = &xenpipe->viewport;

    vert->pos[0] = vp->translate[0] + vp->scale[0] * (vert->pos[0] * inv_w);
    vert->pos[1] = vp->translate[1] + vp->scale[1] * (vert->pos[1] * inv_w);
    vert->pos[2] = vp->translate[2] + vp->scale[2] * (vert->pos[2] * inv_w);
    vert->pos[3] = inv_w;
}

static void xp_emit_triangle(struct xenpipe_context *xenpipe,
                             unsigned i0, unsigned i1, unsigned i2){
    float inps[8][4];
    struct xp_clip_vert verts[12];

    unsigned indices[] = {i0, i1, i2};
    for(unsigned i = 0; i < 3; i++){
        xp_fetch_attribs(xenpipe, inps, indices[i]);
        xp_run_vs(xenpipe, inps, &verts[i]);
    }

    unsigned n = xp_clip(verts);

    for(unsigned i = 0; i < n; i++)
        xp_clip_to_window(xenpipe, &verts[i]);
    
    for(unsigned i = 0; i+2 < n; i++)
        xp_rasterize_triangle(xenpipe, &verts[0], &verts[i+1], &verts[i+2]);
}

void xenpipe_do_draw_vbo(struct xenpipe_context *xenpipe,
                         const struct pipe_draw_info *info,
                         const struct pipe_draw_start_count_bias *draws,
                         unsigned num_draws){
    fprintf(stderr, "STUB: xenpipe_do_draw_vbo\n");

    unsigned idx_sz = info->index_size;
    const uint8_t *idx_buf = NULL;
    if(idx_sz){
        if(info->has_user_indices)
            idx_buf = info->index.user;
        else
            idx_buf = xenpipe_resource(info->index.resource)->data;
    }
        
    for(unsigned d = 0; d < num_draws; d++){
        unsigned srt = draws[d].start;
        unsigned cnt = draws[d].count;

        switch(info->mode){
            case MESA_PRIM_TRIANGLES:
                for(unsigned i = srt; i+2 < srt+cnt; i+=3){
                    xp_emit_triangle(xenpipe, xp_get_idx(idx_buf, idx_sz, i),
                                     xp_get_idx(idx_buf, idx_sz, i+1),
                                     xp_get_idx(idx_buf, idx_sz, i+2));
                }
                break;
            default:
                fprintf(stderr, "xenpipe_do_draw_vbo: Unsup mode %d\n", info->mode);
                exit(-1);
        }
    }
}
