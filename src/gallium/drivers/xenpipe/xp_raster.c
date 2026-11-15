#include <stdio.h>
#include "xp_draw.h"
#include "xp_context.h"
#include "xp_raster.h"
#include "xp_texture.h"
#include "xp_shader.h"
#include "pipe/p_state.h"
#include "util/blend.h"

static float xp_edge_func(float v0x, float v0y,
                          float v1x, float v1y,
                          float v2x, float v2y){
    return (v1x - v2x) * (v2y - v0y) - (v2x - v0x) * (v1y - v2y);
}

static void xp_read_pixel(struct xenpipe_context *xenpipe,
                          unsigned x, unsigned y, float *out){
    uint8_t *map = xenpipe_resource(xenpipe->framebuffer.cbufs[0].texture)->data;
    unsigned offset = y * xenpipe_resource(xenpipe->framebuffer.cbufs->texture)->stride[0] + (x * 4);


    switch(xenpipe->framebuffer.cbufs[0].format){
        case PIPE_FORMAT_R8G8B8A8_UNORM:
            out[0] = (float)(map[offset+0]) / 255.0f;
            out[1] = (float)(map[offset+1]) / 255.0f;
            out[2] = (float)(map[offset+2]) / 255.0f;
            out[3] = (float)(map[offset+3]) / 255.0f;
            break;
        case PIPE_FORMAT_R8G8B8X8_UNORM:
            out[0] = (float)(map[offset+0]) / 255.0f;
            out[1] = (float)(map[offset+1]) / 255.0f;
            out[2] = (float)(map[offset+2]) / 255.0f;
            out[3] = 1.0f;
            break;
        case PIPE_FORMAT_B8G8R8A8_UNORM:
            out[0] = (float)(map[offset+2]) / 255.0f;
            out[1] = (float)(map[offset+1]) / 255.0f;
            out[2] = (float)(map[offset+0]) / 255.0f;
            out[3] = (float)(map[offset+3]) / 255.0f;
            break;
        case PIPE_FORMAT_B8G8R8X8_UNORM:
            out[0] = (float)(map[offset+2]) / 255.0f;
            out[1] = (float)(map[offset+1]) / 255.0f;
            out[2] = (float)(map[offset+0]) / 255.0f;
            out[3] = 1.0f;
            break;
        default:
            fprintf(stderr, "xp_read_pixel: Unsupported format\n");
            exit(-1);
    }
}

static void xp_write_pixel(struct xenpipe_context *xenpipe,
                          unsigned x, unsigned y, float *src){
    unsigned colormask = xenpipe->blend->rt[0].colormask;
    uint8_t *map = xenpipe_resource(xenpipe->framebuffer.cbufs[0].texture)->data;
    unsigned offset = y * xenpipe_resource(xenpipe->framebuffer.cbufs->texture)->stride[0] + (x * 4);

    switch(xenpipe->framebuffer.cbufs[0].format){
        case PIPE_FORMAT_R8G8B8A8_UNORM:
            if(colormask & PIPE_MASK_R) map[offset+0] = (uint8_t)CLAMP(src[0] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_G) map[offset+1] = (uint8_t)CLAMP(src[1] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_B) map[offset+2] = (uint8_t)CLAMP(src[2] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_A) map[offset+3] = (uint8_t)CLAMP(src[3] * 255.0f, 0.0f, 255.0f);
            break;
        case PIPE_FORMAT_R8G8B8X8_UNORM:
            if(colormask & PIPE_MASK_R) map[offset+0] = (uint8_t)CLAMP(src[0] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_G) map[offset+1] = (uint8_t)CLAMP(src[1] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_B) map[offset+2] = (uint8_t)CLAMP(src[2] * 255.0f, 0.0f, 255.0f);
            break;
        case PIPE_FORMAT_B8G8R8A8_UNORM:
        // if(colormask & PIPE_MASK_R) fprintf(stderr, "b=%u g=%u r=%u a=%u\n", (uint8_t)CLAMP(src[2] * 255.0f, 0.0f, 255.0f),
        // (uint8_t)CLAMP(src[1] * 255.0f, 0.0f, 255.0f), (uint8_t)CLAMP(src[0] * 255.0f, 0.0f, 255.0f), (uint8_t)CLAMP(src[3] * 255.0f, 0.0f, 255.0f));
            if(colormask & PIPE_MASK_B) map[offset+0] = (uint8_t)CLAMP(src[2] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_G) map[offset+1] = (uint8_t)CLAMP(src[1] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_R) map[offset+2] = (uint8_t)CLAMP(src[0] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_A) map[offset+3] = (uint8_t)CLAMP(src[3] * 255.0f, 0.0f, 255.0f);
            break;
        case PIPE_FORMAT_B8G8R8X8_UNORM:
            if(colormask & PIPE_MASK_B) map[offset+0] = (uint8_t)CLAMP(src[2] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_G) map[offset+1] = (uint8_t)CLAMP(src[1] * 255.0f, 0.0f, 255.0f);
            if(colormask & PIPE_MASK_R) map[offset+2] = (uint8_t)CLAMP(src[0] * 255.0f, 0.0f, 255.0f);
            break;
        default:
            fprintf(stderr, "xp_write_pixel: Unsupported format\n");
            exit(-1);
    }
}

static void xp_scissor_test(struct xenpipe_context *xenpipe,
                            struct xp_quad *quad){
    if(!xenpipe->rasterizer->scissor)
        return;

    struct pipe_scissor_state *sc = &xenpipe->scissor;
    unsigned dx[] = {0, 1, 0, 1}; 
    unsigned dy[] = {0, 0, 1, 1};

 
    for(unsigned c = 0; c < 4; c++){
        if(!(quad->mask & (1 << c)))
            continue;
        unsigned px = quad->qx + dx[c];
        unsigned py = quad->qy + dy[c];
        if(px >= sc->maxx || px < sc->minx || py >= sc->maxy || py < sc->miny)
            quad->mask &= ~(1 << c);
    }
}

static float xp_read_depth(struct xenpipe_context *xenpipe,
                           unsigned x, unsigned y){
    struct xenpipe_resource *zsbuf = xenpipe_resource(xenpipe->framebuffer.zsbuf.texture);
    uint8_t *map = zsbuf->data;

    switch(xenpipe->framebuffer.zsbuf.format){
        case PIPE_FORMAT_Z24X8_UNORM:{
            unsigned offset = y * zsbuf->stride[0] + (x * 4);
            return (float)((*(uint32_t *)&(map[offset])) & 0xffffff) / (float)0xffffff;}
        default:
            fprintf(stderr, "xp_read_depth: Unhandled format %s\n", util_format_name(xenpipe->framebuffer.zsbuf.format));
            exit(-1);
    }
}

static void xp_write_depth(struct xenpipe_context *xenpipe,
                          unsigned x, unsigned y, float z){
    struct xenpipe_resource *zsbuf = xenpipe_resource(xenpipe->framebuffer.zsbuf.texture);
    uint8_t *map = zsbuf->data;

    switch(xenpipe->framebuffer.zsbuf.format){
        case PIPE_FORMAT_Z24X8_UNORM:{
            unsigned offset = y * zsbuf->stride[0] + (x * 4);
            unsigned z_s = (uint32_t)(z * (float)0xffffff) & 0x00ffffff;
            *(uint32_t *)&(map[offset]) = (*(uint32_t *)&(map[offset]) & 0xff000000) | z_s;
            break;}
        default:
            fprintf(stderr, "xp_write_depth: Unhandled format %s\n", util_format_name(xenpipe->framebuffer.zsbuf.format));
            exit(-1);
    }

}

static bool xp_compare_depth(struct xenpipe_context *xenpipe, float frag_z, float buf_z){
    switch(xenpipe->depth_stencil->depth_func){
        case PIPE_FUNC_LESS:        return frag_z < buf_z;
        default:
            fprintf(stderr, "xp_read_depth: Unhandled depth_func %u\n", xenpipe->depth_stencil->depth_func);
            exit(-1);
    }
}

static void xp_depth_test(struct xenpipe_context *xenpipe,
                          struct xp_quad *quad){
    if(!xenpipe->depth_stencil->depth_enabled)
        return;
    
    unsigned dx[] = {0, 1, 0, 1}; 
    unsigned dy[] = {0, 0, 1, 1};

    for(unsigned p = 0; p < 4; p++){
        if(!(quad->mask & (1 << p)))
            continue;

        float frag_z = quad->z[p];
        float buf_z = xp_read_depth(xenpipe, quad->qx + dx[p], quad->qy + dy[p]);

        if(xp_compare_depth(xenpipe, frag_z, buf_z)){
            xp_write_depth(xenpipe, quad->qx + dx[p], quad->qy + dy[p], frag_z);
            continue;
        }
        quad->mask &= ~(1 << p);
    }
}

static void xp_blend_factor(float *out, float *src, float *dst, unsigned c, unsigned factor){
    switch(factor){
        case PIPE_BLENDFACTOR_SRC_ALPHA:
            out[0] = src[3];
            break;
        case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
            out[0] = 1.0f - src[3];
            break;
        default:
            fprintf(stderr, "xp_blend_factor: Unhandled factor %u\n", factor);
            exit(-1);
    }
}

static void xp_blend_eqn(float *out, float *src, float *dst, unsigned func){
    switch(func){
        case PIPE_BLEND_ADD:
            out[0] = src[0] + dst[0];
            break;
        default:
            fprintf(stderr, "xp_blend_eqn: Unhandled func %u\n", func);
            exit(-1);
    }
}

static void xp_blend(struct xenpipe_context *xenpipe,
                     struct xp_quad *quad){
    struct pipe_rt_blend_state *blend = &xenpipe->blend->rt[0];
    if(!blend->blend_enable)
        return;
    
    float src_factored[4];
    float dst_factored[4];
    float dst[4];
    float final[4];
    unsigned dx[] = {0, 1, 0, 1}; 
    unsigned dy[] = {0, 0, 1, 1};


    for(unsigned p = 0; p < 4; p++){
        xp_read_pixel(xenpipe, quad->qx + dx[p], quad->qy + dy[p], dst);
        for(unsigned c = 0; c < 4; c++){
            unsigned src_factor = (c < 3) ? blend->rgb_src_factor : blend->alpha_src_factor;
            unsigned dst_factor = (c < 3) ? blend->rgb_dst_factor : blend->alpha_dst_factor;
            unsigned blend_func = (c < 3) ? blend->rgb_func : blend->alpha_func;
            
            xp_blend_factor(&src_factored[c], quad->color_out[p], dst, c, src_factor);
            xp_blend_factor(&dst_factored[c], quad->color_out[p], dst, c, dst_factor);
            src_factored[c] = quad->color_out[p][c] * src_factored[c];
            dst_factored[c] = dst[c] * dst_factored[c];
            xp_blend_eqn(&final[c], &src_factored[c], &dst_factored[c], blend_func);
        }
        for(unsigned c = 0; c < 4; c++)
            quad->color_out[p][c] = final[c];
    }

        
}

void xp_rasterize_triangle(struct xenpipe_context *xenpipe,
                           struct xp_clip_vert *v0,
                           struct xp_clip_vert *v1,
                           struct xp_clip_vert *v2){
        fprintf(stderr, "STUB: xp_rasterize_triangle\n");
        
        float area = xp_edge_func(v0->pos[0], v0->pos[1], v1->pos[0], v1->pos[1], v2->pos[0], v2->pos[1]);

        if(fabsf(area) < 1e-6f)
            return;


        float inv_area = 1.0f / area;

        bool front_face = (area < 0) == xenpipe->rasterizer->front_ccw;
        if((front_face && (xenpipe->rasterizer->cull_face & PIPE_FACE_FRONT)) || 
        (!front_face && (xenpipe->rasterizer->cull_face & PIPE_FACE_BACK)))
        return;

        unsigned minx = (unsigned)fmaxf(floorf(fminf(fminf(v0->pos[0], v1->pos[0]), v2->pos[0])), 0) & ~1;
        unsigned miny = (unsigned)fmaxf(floorf(fminf(fminf(v0->pos[1], v1->pos[1]), v2->pos[1])), 0) & ~1;
        unsigned maxx = (unsigned)fminf(ceilf(fmaxf(fmaxf(v0->pos[0], v1->pos[0]), v2->pos[0])), xenpipe->framebuffer.width);
        unsigned maxy = (unsigned)fminf(ceilf(fmaxf(fmaxf(v0->pos[1], v1->pos[1]), v2->pos[1])), xenpipe->framebuffer.height);


        unsigned dx[] = {0, 1, 0, 1};
        unsigned dy[] = {0, 0, 1, 1};
        float red[] = {1.0f, 0.0f, 0.0f, 1.0f};

        struct xp_quad quad;
        quad.front_face = front_face;

        // fprintf(stderr, "minx=%u miny=%u maxx=%u maxy=%u\n", minx, miny, maxx, maxy);

        for(unsigned qx = minx; qx <= maxx; qx+=2){
            for(unsigned qy = miny; qy <= maxy; qy+=2){
                quad.mask = 0;
                for(unsigned p = 0; p < 4; p++){
                    quad.qx = qx;
                    quad.qy = qy;
                float px = (float)(qx + dx[p]) + 0.5f;
                float py = (float)(qy + dy[p]) + 0.5f;
                float w0 = xp_edge_func(v1->pos[0], v1->pos[1], v2->pos[0], v2->pos[1], px, py) * inv_area;
                float w1 = xp_edge_func(v2->pos[0], v2->pos[1], v0->pos[0], v0->pos[1], px, py) * inv_area;
                float w2 = xp_edge_func(v0->pos[0], v0->pos[1], v1->pos[0], v1->pos[1], px, py) * inv_area;

                float b0 = w0 * v0->pos[3];
                float b1 = w1 * v1->pos[3];
                float b2 = w2 * v2->pos[3];

                float scale = 1.0f / (b0 + b1 + b2);
                // fprintf(stderr, "w0=%f w1=%f w2=%f\n", w0, w1, w2);
                bool inside = (w0 > 0 && w1 > 0 && w2 > 0) || (w0 < 0 && w1 < 0 && w2 < 0);
                quad.mask |= inside << p;
                if(!inside)
                    continue;

                quad.z[p] = w0 * v0->pos[2] + w1 * v1->pos[2] + w2 * v2->pos[2]; 
                
                for(unsigned v = 0; v < 8; v++)
                    for(unsigned c = 0; c < 4; c++)
                        quad.var[p][v][c] = (b0 * v0->var[v][c] + b1 * v1->var[v][c] + b2 * v2->var[v][c]) * scale;
            }

            if(!quad.mask)
                continue;
            
            xp_scissor_test(xenpipe, &quad);
            if(!quad.mask)
                continue;

            xp_run_fs(xenpipe, &quad);

            // Stencil test explicitly ignored for now.

            if(xenpipe->depth_stencil->stencil->enabled){
                fprintf(stderr, "xp_rasterize_triangle: Stencil test not implemented\n");
                exit(-1);
            }
            if(!quad.mask)
                continue;


            xp_depth_test(xenpipe, &quad);

            if(!quad.mask)
                continue;

            xp_blend(xenpipe, &quad);
            
            for(unsigned p = 0; p < 4; p++){
                // fprintf(stderr, "quad.mask=%x\n", quad.mask);
                if((quad.mask & (1 << p)))
                    xp_write_pixel(xenpipe, quad.qx + dx[p], quad.qy + dy[p], quad.color_out[p]);
            }
        }
        // while(1);
    }
}