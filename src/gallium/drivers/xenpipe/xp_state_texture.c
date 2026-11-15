#include <stdio.h>
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "xp_state.h"
#include "xp_texture.h"
#include "xp_screen.h"
#include "frontend/sw_winsys.h"
#include "util/u_inlines.h"
#include "util/u_surface.h"

static unsigned xenpipe_get_tex_image_offset(const struct xenpipe_resource *xpr,
                                             unsigned level, unsigned layer){
    fprintf(stderr, "STUB: xenpipe_get_tex_image_offset\n");
    unsigned offset = xpr->level_offset[level];
    offset += layer * xpr->img_stride[level];
    return offset;
}

static void *xenpipe_buffer_map(struct pipe_context *pipe,
                                struct pipe_resource *pr,
                                unsigned level, unsigned usage,
                                const struct pipe_box *box,
                                struct pipe_transfer **transfer){
    fprintf(stderr, "STUB: xenpipe_buffer_map\n");
    struct xenpipe_resource *xpr = xenpipe_resource(pr);
    
    struct xenpipe_transfer *xpt = CALLOC_STRUCT(xenpipe_transfer);
    if(!xpt)
        return NULL;

    pipe_resource_reference(&xpt->base.resource, pr);
    xpt->base.level = level;
    xpt->base.usage = usage;
    xpt->base.box = *box;
    xpt->base.stride = xpr->stride[level];
    xpt->base.layer_stride = xpr->img_stride[level];

    xpt->offset = xenpipe_get_tex_image_offset(xpr, level, box->z);

    if(pr->target == PIPE_BUFFER)
        xpt->offset += box->x;
    
    uint8_t *map = xpr->data;
    *transfer = &xpt->base;
    return map + xpt->offset;
}

static void *xenpipe_texture_map(struct pipe_context *pipe,
                                 struct pipe_resource *pr,
                                 unsigned level, unsigned usage,
                                 const struct pipe_box *box,
                                 struct pipe_transfer **transfer){
    fprintf(stderr, "STUB: xenpipe_texture_map\n");
    struct xenpipe_resource *xpr = xenpipe_resource(pr);
    
    struct xenpipe_transfer *xpt = CALLOC_STRUCT(xenpipe_transfer);
    if(!xpt)
        return NULL;

    pipe_resource_reference(&xpt->base.resource, pr);
    xpt->base.level = level;
    xpt->base.usage = usage;
    xpt->base.box = *box;
    xpt->base.stride = xpr->stride[level];
    xpt->base.layer_stride = xpr->img_stride[level];

    unsigned offset = xenpipe_get_tex_image_offset(xpr, level, box->z);
    unsigned bpp = util_format_get_blocksize(pr->format);
    offset += box->y * xpt->base.stride + box->x * bpp;

    xpt->offset = offset;
    
    uint8_t *map = xpr->data;
    *transfer = &xpt->base;
    return map + xpt->offset;
}


static void xenpipe_buffer_unmap(struct pipe_context *pipe,
                                 struct pipe_transfer *transfer){
    fprintf(stderr, "STUB: xenpipe_buffer_unmap\n");
    assert(transfer->resource);
    pipe_resource_reference(&transfer->resource, NULL);
    FREE(transfer);
}

static void xenpipe_texture_unmap(struct pipe_context *pipe,
                                  struct pipe_transfer *transfer){
    fprintf(stderr, "STUB: xenpipe_texture_unmap\n");
    assert(transfer->resource);
    pipe_resource_reference(&transfer->resource, NULL);
    FREE(transfer);
}

static void xenpipe_transfer_flush_region(struct pipe_context *pipe,
                                          struct pipe_transfer *transfer,
                                          const struct pipe_box *box){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_transfer_flush_region\n");
    exit(-1);
}

static void xenpipe_buffer_subdata(struct pipe_context *pipe,
                                   struct pipe_resource *pr,
                                   unsigned usage, unsigned offset,
                                   unsigned size, const void *data){
    fprintf(stderr, "STUB: xenpipe_buffer_subdata\n");
    struct pipe_transfer *transfer = NULL;
    struct pipe_box box;

    usage |= PIPE_MAP_WRITE;
    
    if(!(usage & PIPE_MAP_DIRECTLY)){
        if(offset == 0 && size == pr->width0)
            usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
        else
            usage |= PIPE_MAP_DISCARD_RANGE;
    }

    u_box_1d(offset, size, &box);

    uint8_t *map = xenpipe_buffer_map(pipe, pr, 0, usage,
                                      &box, &transfer);
    if(!map)
        return;
    memcpy(map, data, size);
    pipe_buffer_unmap(pipe, transfer);

}

static void xenpipe_texture_subdata(struct pipe_context *pipe,
                                    struct pipe_resource *pr,
                                    unsigned level, unsigned usage,
                                    const struct pipe_box *box,
                                    const void *data,
                                    unsigned stride,
                                    uintptr_t layer_stride){
    fprintf(stderr, "STUB: xenpipe_texture_subdata\n");
    struct pipe_transfer *transfer = NULL;
    usage |= PIPE_MAP_WRITE;

    uint8_t *map = xenpipe_texture_map(pipe, pr, level, usage, box, &transfer);
    if(!map)
        return;
    
    util_copy_box(map, pr->format,
                  transfer->stride, transfer->layer_stride,
                  0, 0, 0, box->width, box->height, box->depth,
                  data, stride, layer_stride, 0, 0, 0);
    xenpipe_texture_unmap(pipe, transfer);
}

static struct pipe_surface *xenpipe_create_surface(struct pipe_context *pipe,
                                                   struct pipe_resource *pr,
                                                   const struct pipe_surface *surf_templ){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_create_surface\n");
    exit(-1);
}

static void xenpipe_surface_destroy(struct pipe_context *pipe,
                                    struct pipe_surface *surface){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_surface_destroy\n");
    exit(-1);
}

static void xenpipe_clear_texture(struct pipe_context *pipe,
                                  struct pipe_resource *resource,
                                  unsigned level,
                                  const struct pipe_box *box,
                                  const void *data){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_clear_texture\n");
    exit(-1);
}

static void xenpipe_resource_copy_region(struct pipe_context *pipe,
                                         struct pipe_resource *pr,
                                         unsigned dst_level,
                                         unsigned dst_x, unsigned dst_y,
                                         unsigned dst_z,
                                         struct pipe_resource *src,
                                         unsigned src_level,
                                         const struct pipe_box *src_box_in){
    fprintf(stderr, "STUB: xenpipe_resource_copy_region\n");
    util_resource_copy_region(pipe, pr, dst_level, dst_x, dst_y, dst_z,
                              src, src_level, src_box_in);
}

static void xenpipe_blit(struct pipe_context *pipe,
                         const struct pipe_blit_info *info){
    fprintf(stderr, "STUB: xenpipe_blit\n");
    util_try_blit_via_copy_region(pipe, info, false);
}


static void xenpipe_resource_release(struct pipe_context *pipe,
                                     struct pipe_resource *pr){
    fprintf(stderr, "STUB: xenpipe_resource_release\n");
    u_default_resource_release(pipe, pr);
}

void xenpipe_init_state_texture_funcs(struct pipe_context *pipe){
    fprintf(stderr, "STUB: xenpipe_init_state_texture_funcs\n");

    pipe->buffer_map = xenpipe_buffer_map;
    pipe->buffer_unmap = xenpipe_buffer_unmap;
    pipe->texture_map = xenpipe_texture_map;
    pipe->texture_unmap = xenpipe_texture_unmap;
    pipe->transfer_flush_region = xenpipe_transfer_flush_region;
    pipe->buffer_subdata = xenpipe_buffer_subdata;
    pipe->texture_subdata = xenpipe_texture_subdata;
    pipe->create_surface = xenpipe_create_surface;
    pipe->surface_destroy = xenpipe_surface_destroy;
    pipe->clear_texture = xenpipe_clear_texture;
    pipe->resource_copy_region = xenpipe_resource_copy_region;
    pipe->blit = xenpipe_blit;
    pipe->resource_release = xenpipe_resource_release;
}