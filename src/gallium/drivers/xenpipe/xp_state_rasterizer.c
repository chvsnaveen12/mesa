#include <stdio.h>
#include "pipe/p_context.h"
#include "xp_state.h"
#include "xp_context.h"

static void *xenpipe_create_rasterizer_state(struct pipe_context *pipe,
                                             const struct pipe_rasterizer_state *templ){
    fprintf(stderr, "STUB: xenpipe_create_rasterizer_state\n");
    struct pipe_rasterizer_state *state = CALLOC_STRUCT(pipe_rasterizer_state);
    if(!state)
        return NULL;

    *state = *templ;
    return state;
}

static void xenpipe_bind_rasterizer_state(struct pipe_context *pipe, void *rast){
    fprintf(stderr, "STUB: xenpipe_bind_rasterizer_state\n");
    xenpipe_context(pipe)->rasterizer = rast;
}

static void xenpipe_delete_rasterizer_state(struct pipe_context *pipe,
                                            void *rast){
    fprintf(stderr, "STUB: xenpipe_delete_rasterizer_state\n");
    FREE(rast);
}

void xenpipe_init_state_rasterizer_funcs(struct pipe_context *pipe){
    fprintf(stderr, "STUB: xenpipe_init_state_rasterizer_funcs\n");
    pipe->create_rasterizer_state = xenpipe_create_rasterizer_state;
    pipe->bind_rasterizer_state = xenpipe_bind_rasterizer_state;
    pipe->delete_rasterizer_state = xenpipe_delete_rasterizer_state;
}