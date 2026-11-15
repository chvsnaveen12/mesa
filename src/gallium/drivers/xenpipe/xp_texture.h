#ifndef XP_TEXTURE
#define XP_TEXTURE

#include "pipe/p_state.h"

struct pipe_screen;

void xenpipe_init_texture_funcs(struct pipe_screen *screen);

struct xenpipe_resource{
    struct pipe_resource base;

    unsigned stride[15];
    unsigned img_stride[15];
    unsigned level_offset[15];

    void *data;
    struct sw_displaytarget *dt;
    bool pot;
};

struct xenpipe_transfer{
    struct pipe_transfer base;
    unsigned long offset;
};

static inline struct xenpipe_transfer *xenpipe_transfer(struct xenpipe_transfer *transfer){
    return (struct xenpipe_transfer *)transfer;
}

static inline struct xenpipe_resource *xenpipe_resource(struct pipe_resource *resource){
    return (struct xenpipe_resource *)resource;
}

#endif