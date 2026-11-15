#include <stdio.h>
#include "xp_state.h"
#include "xp_context.h"
#include "pipe/p_context.h"
#include "util/u_inlines.h"

static void *xenpipe_create_sampler_state(struct pipe_context *pipe,
                                          const struct pipe_sampler_state *sampler){
    fprintf(stderr, "STUB: xenpipe_create_sampler_state\n");
    struct pipe_sampler_state *state = CALLOC_STRUCT(pipe_sampler_state);
    if(!state)
        return NULL;
    memcpy(state, sampler, sizeof(*state));
    return state;
}

static void xenpipe_bind_sampler_states(struct pipe_context *pipe,
                                        mesa_shader_stage stage,
                                        unsigned start_slot, unsigned num_samplers,
                                        void **samplers){
    fprintf(stderr, "STUB: xenpipe_bind_rasterizer_state\n");
    struct xenpipe_context *xenpipe = xenpipe_context(pipe);

    if(stage != MESA_SHADER_FRAGMENT){
        fprintf(stderr, "xenpipe_bind_sampler_state: Samplers only for fs\n");
        exit(-1);
    }

    for(unsigned i = start_slot; i < start_slot + num_samplers; i++)
        xenpipe->samplers[i] = samplers ? samplers[i] : NULL;
    xenpipe->num_samplers = start_slot + num_samplers;
}

static void xenpipe_delete_sampler_state(struct pipe_context *pipe, void *sampler){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_delete_sampler_state\n");
    exit(-1);
}

static struct pipe_sampler_view *xenpipe_create_sampler_view(struct pipe_context *pipe,
                                                             struct pipe_resource *resource,
                                                             const struct pipe_sampler_view *templ){
    fprintf(stderr, "STUB: xenpipe_create_sampler_view\n");
    
    struct pipe_sampler_view *state = CALLOC_STRUCT(pipe_sampler_view);
    if(!state)
        return NULL;
    
    *state = *templ;
    state->texture = NULL;
    state->context = pipe;
    pipe_reference_init(&state->reference, 1);
    pipe_resource_reference(&state->texture, resource);
    return state;
}

static void xenpipe_set_sampler_views(struct pipe_context *pipe,
                                      mesa_shader_stage stage,
                                      unsigned start_slot, unsigned num_views,
                                      unsigned unbind_trail,
                                      struct pipe_sampler_view **views){
    fprintf(stderr, "STUB: xenpipe_set_sampler_views\n");
    struct xenpipe_context *xenpipe = xenpipe_context(pipe);

    for(unsigned i = start_slot; i < start_slot + num_views; i++)
        pipe_sampler_view_reference(&xenpipe->sampler_views[i], views ? views[i] : NULL);

    for(unsigned i = start_slot + num_views; i < start_slot + num_views + unbind_trail; i++)
        pipe_sampler_view_reference(&xenpipe->sampler_views[i], NULL);
    xenpipe->num_sampler_views = start_slot + num_views;
}

static void xenpipe_sampler_view_destroy(struct pipe_context *pipe,
                                         struct pipe_sampler_view *view){
    fprintf(stderr, "STUB: xenpipe_sampler_view_destroy\n");
    pipe_resource_reference(&view->texture, NULL);
    FREE(view);
}

static void xenpipe_sampler_view_release(struct pipe_context *pipe,
                                         struct pipe_sampler_view *view){
    fprintf(stderr, "STUB: xenpipe_sampler_view_release\n");
    u_default_sampler_view_release(pipe, view);
}

void xenpipe_init_state_sampler_funcs(struct pipe_context *pipe){
    fprintf(stderr, "STUB: xenpipe_init_state_sampler_funcs\n");
    pipe->create_sampler_state = xenpipe_create_sampler_state; 
    pipe->bind_sampler_states = xenpipe_bind_sampler_states; 
    pipe->delete_sampler_state = xenpipe_delete_sampler_state;
    pipe->create_sampler_view = xenpipe_create_sampler_view; 
    pipe->set_sampler_views = xenpipe_set_sampler_views;
    pipe->sampler_view_destroy = xenpipe_sampler_view_destroy;
    pipe->sampler_view_release = xenpipe_sampler_view_release;
}