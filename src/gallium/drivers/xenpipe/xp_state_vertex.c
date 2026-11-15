#include <stdio.h>
#include "pipe/p_context.h"
#include "xp_state.h"
#include "xp_context.h"
#include "util/u_memory.h"

static void *xenpipe_create_vertex_elements_state(struct pipe_context *pipe,
                                                  unsigned num_elements,
                                                  const struct pipe_vertex_element *attribs){
    fprintf(stderr, "STUB: xenpipe_create_vertex_elements_state\n");
    for(unsigned i = 0; i < num_elements; i++)
        fprintf(stderr, "  elem[%u]: src_offset=%u src_stride=%u src_format=%d vbuf_idx=%u\n",
                i, attribs[i].src_offset, attribs[i].src_stride, attribs[i].src_format,
                attribs[i].vertex_buffer_index);


    struct xp_vertex_element_state *velems = CALLOC_STRUCT(xp_vertex_element_state);
    if(!velems)
        return NULL;

    velems->num_elements = num_elements;
    memcpy(velems->elements, attribs, sizeof(struct pipe_vertex_element) * num_elements);
    return velems;
}

static void xenpipe_bind_vertex_elements_state(struct pipe_context *pipe, void *velems){
    fprintf(stderr, "STUB: xenpipe_delete_vertex_elements_state\n");
    xenpipe_context(pipe)->velems = velems;
}

static void xenpipe_delete_vertex_elements_state(struct pipe_context *pipe, void *velems){
    fprintf(stderr, "STUB: xenpipe_delete_vertex_elements_state\n");
    FREE(velems);
}

static void xenpipe_set_vertex_buffers(struct pipe_context *pipe, unsigned count,
                                       const struct pipe_vertex_buffer *buffers){
    fprintf(stderr, "STUB: xenpipe_set_vertex_buffers\n");
    for(unsigned i = 0; i < count; i++)
        fprintf(stderr, "  vb[%u]: offset=%u buffer=%p is_user=%u\n",
                i, buffers[i].buffer_offset, buffers[i].buffer.resource, buffers[i].is_user_buffer);

    struct xenpipe_context *xenpipe = xenpipe_context(pipe);
    memcpy(xenpipe->vertex_buffers, buffers, sizeof(struct pipe_vertex_buffer) * count);
}

void xenpipe_init_state_vertex_funcs(struct pipe_context *pipe){
    fprintf(stderr, "STUB: xenpipe_init_state_vertex\n");

    pipe->create_vertex_elements_state = xenpipe_create_vertex_elements_state; 
    pipe->bind_vertex_elements_state = xenpipe_bind_vertex_elements_state;
    pipe->delete_vertex_elements_state = xenpipe_delete_vertex_elements_state;
    pipe->set_vertex_buffers = xenpipe_set_vertex_buffers;
}