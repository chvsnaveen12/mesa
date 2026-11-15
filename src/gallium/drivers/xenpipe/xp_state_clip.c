#include <stdio.h>
#include "pipe/p_context.h"
#include "xp_state.h"
#include "xp_context.h"

static void xenpipe_set_viewport_states(struct pipe_context *pipe,
                                        unsigned start_slot, unsigned num_ports,
                                        const struct pipe_viewport_state *viewports){
    fprintf(stderr, "STUB: xenpipe_set_viewport_states\n");
    xenpipe_context(pipe)->viewport = viewports[0];
}


static void xenpipe_set_scissor_states(struct pipe_context *pipe,
                                       unsigned start_slot, unsigned num_scissors,
                                       const struct pipe_scissor_state *scissor){
    fprintf(stderr, "STUB: xenpipe_set_scissor_states\n");
    xenpipe_context(pipe)->scissor = scissor[0];
}

static void xenpipe_set_polygon_stipple(struct pipe_context *pipe,
                                        const struct pipe_poly_stipple *stipple){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_set_polygon_stipple\n");
}

void xenpipe_init_state_clip_funcs(struct pipe_context *pipe){
    fprintf(stderr, "STUB: xenpipe_init_state_clip_funcs\n");

    pipe->set_viewport_states = xenpipe_set_viewport_states;
    pipe->set_scissor_states = xenpipe_set_scissor_states;
    pipe->set_polygon_stipple = xenpipe_set_polygon_stipple;
}