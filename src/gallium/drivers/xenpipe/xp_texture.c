#include <stdio.h>
#include "xp_texture.h"
#include "xp_screen.h"
#include "pipe/p_screen.h"
#include "frontend/sw_winsys.h"

static const char *bind_string(unsigned bind){
    static char buf[256];
    buf[0] = '\0';
    if(bind & PIPE_BIND_DEPTH_STENCIL){
        bind &= ~PIPE_BIND_DEPTH_STENCIL;
        strcat(buf, "DEPTH ");
    }   
    if(bind & PIPE_BIND_RENDER_TARGET){
        bind &= ~PIPE_BIND_RENDER_TARGET;
        strcat(buf, "RT ");
    }
    if(bind & PIPE_BIND_SAMPLER_VIEW){
        bind &= ~PIPE_BIND_SAMPLER_VIEW;
        strcat(buf, "SAMP ");
    }
    if(bind & PIPE_BIND_VERTEX_BUFFER){
        bind &= ~PIPE_BIND_VERTEX_BUFFER;
        strcat(buf, "VB ");
    }
    if(bind & PIPE_BIND_INDEX_BUFFER){
        bind &= ~PIPE_BIND_INDEX_BUFFER;
        strcat(buf, "IDX ");
    }
    if(bind & PIPE_BIND_CONSTANT_BUFFER){
        bind &= ~PIPE_BIND_CONSTANT_BUFFER;
        strcat(buf, "CONST ");
    }
    if(bind & PIPE_BIND_DISPLAY_TARGET){
        bind &= ~PIPE_BIND_DISPLAY_TARGET;
        strcat(buf, "DT ");
    }
    if(bind & PIPE_BIND_SCANOUT){
        bind &= ~PIPE_BIND_SCANOUT;
        strcat(buf, "SCAN ");
    }
    if(bind & PIPE_BIND_SHARED){
        bind &= ~PIPE_BIND_SHARED;
        strcat(buf, "SHARED ");
    }
    if(bind){
        fprintf(stderr, "Unhandled bind flag %d\n", bind);
        while(1);
    }

    if(buf[0] == '\0') strcat(buf, "NONE");
    return buf;
}

static bool xenpipe_resource_layout(struct pipe_screen *screen, struct xenpipe_resource *xpr){
    struct pipe_resource *pr = &xpr->base;
    unsigned level;
    unsigned width = pr->width0;
    unsigned height = pr->height0;

    unsigned buffer_size = 0;

    if(pr->target == PIPE_BUFFER){
        xpr->stride[0] = pr->width0;
        xpr->data = align_malloc(width, 64);
        return !!(xpr->data);
    }

    unsigned int slices = (pr->target == PIPE_TEXTURE_CUBE) ? 6 : 1;

    for(level = 0; level <= pr->last_level; level++){
        xpr->stride[level] = util_format_get_stride(pr->format, width);
        unsigned nblocksy = util_format_get_nblocksy(pr->format, height);

        xpr->img_stride[level] = xpr->stride[level] * nblocksy;
        xpr->level_offset[level] = buffer_size;

        buffer_size += xpr->img_stride[level] * slices;

        width = MAX2(1, width >> 1);
        height = MAX2(1, height >> 1);
    }
    xpr->data = align_malloc(buffer_size, 64);
    if(xpr->data){
        memset(xpr->data, 0, buffer_size);
        return true;
    }
    return false;
}

static struct pipe_resource *xenpipe_resource_create_front(struct pipe_screen *screen,
                                                           const struct pipe_resource *pr,
                                                           const void *map_front_private){
    fprintf(stderr, "STUB: xenpipe_resource_create_front: target=%d format=%s(%d) %dx%dx%d last_level=%d bind=[%s]\n",
            pr->target, util_format_name(pr->format), pr->format, pr->width0, pr->height0, pr->depth0,
            pr->last_level, bind_string(pr->bind));

    struct xenpipe_resource *xpr = CALLOC_STRUCT(xenpipe_resource);
    if(!xpr)
        return NULL;
    
    assert(pr->format != PIPE_FORMAT_NONE);

    xpr->base = *pr;
    xpr->base.reference.count = 1;
    xpr->base.screen = screen;
    xpr->pot = (util_is_power_of_two_or_zero(pr->width0) &&
                util_is_power_of_two_or_zero(pr->height0) &&
                util_is_power_of_two_or_zero(pr->depth0));

    // display path
    if(xpr->base.bind & (PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_SCANOUT | PIPE_BIND_SHARED)){
        struct sw_winsys *winsys = xenpipe_screen(screen)->winsys;

        xpr->dt = winsys->displaytarget_create(winsys, pr->usage, pr->format,
                                               pr->width0, pr->height0, 64, map_front_private,
                                               &xpr->stride[0]);
        if(!xpr->dt){
            FREE(xpr);
            return NULL;
        }
        unsigned sz = xpr->stride[0] * pr->height0;
        xpr->data = align_malloc(sz, 64);
        // We should have a bail for when this fails
        if(xpr->data)
            memset(xpr->data, 0, sz);
    }   // The buffer path
    else{
        if(!xenpipe_resource_layout(screen, xpr)){
            FREE(xpr);
            return NULL;
        }
    }

    return &xpr->base;
}


static struct pipe_resource *xenpipe_resource_create(struct pipe_screen *screen,
                                                     const struct pipe_resource *pr){
    fprintf(stderr, "STUB: xenpipe_resource_create\n");
    return xenpipe_resource_create_front(screen, pr, NULL);
}

static void xenpipe_resource_destroy(struct pipe_screen *screen,
                                     struct pipe_resource *pr){
    fprintf(stderr, "STUB: xenpipe_resource_destroy\n");
    
    struct xenpipe_resource *xpr = xenpipe_resource(pr);

    if(xpr->dt){
        struct sw_winsys *winsys = xenpipe_screen(screen)->winsys;
        winsys->displaytarget_destroy(winsys, xpr->dt);
    }
    align_free(xpr->data);
    FREE(xpr);
}

static struct pipe_resource *xenpipe_resource_from_handle(struct pipe_screen *screen,
                                                          const struct pipe_resource *templat,
                                                          struct winsys_handle *whandle,
                                                          unsigned usage){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_resource_from_handle\n");
    exit(-1);
    return NULL;
}

static bool xenpipe_resource_get_handle(struct pipe_screen *screen, struct pipe_context *pipe,
                                        struct pipe_resource *pr, struct winsys_handle *whandle,
                                        unsigned usage){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_resource_get_handle\n");
    exit(-1);
    return false;
}

static bool xenpipe_can_create_resource(struct pipe_screen *screen,
                                        const struct pipe_resource *pr){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_can_create_resource\n");
    return true;
}

void xenpipe_init_texture_funcs(struct pipe_screen *screen){
    fprintf(stderr, "STUB: xenpipe_init_texture_funcs\n");
    screen->resource_create = xenpipe_resource_create;
    screen->resource_create_front = xenpipe_resource_create_front;

    screen->resource_destroy = xenpipe_resource_destroy;
    screen->resource_from_handle = xenpipe_resource_from_handle;
    screen->resource_get_handle = xenpipe_resource_get_handle;

    screen->can_create_resource = xenpipe_can_create_resource;
}