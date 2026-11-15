#include <stdio.h>
#include "xp_state.h"
#include "xp_context.h"
#include "util/u_inlines.h"
#include "pipe/p_context.h"

static int xp_type_size(const struct glsl_type *type, bool bindless){
    return glsl_count_attribute_slots(type, false) * 16;
}

static nir_shader *xp_lower_and_compile(nir_shader *nir){
    NIR_PASS(_, nir, nir_lower_vars_to_ssa);
    NIR_PASS(_, nir, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
             xp_type_size, 0);
    NIR_PASS(_, nir, nir_lower_uniforms_to_ubo, false, false);

    // NIR_PASS(_, nir, nir_lower_io_to_scalar,
    //          nir_var_shader_in | nir_var_shader_out | nir_var_mem_ubo, NULL, NULL);
    // NIR_PASS(_, nir, nir_lower_load_const_to_scalar);
    // NIR_PASS(_, nir, nir_lower_alu_to_scalar, NULL, NULL);
    // NIR_PASS(_, nir, nir_lower_phis_to_scalar, NULL, NULL);

    /* booleans as 0 / ~0 so they fit an xp_reg and work as lane masks */
    NIR_PASS(_, nir, nir_lower_bool_to_int32);

    /* the scalarizers leave a lot of redundant moves behind */
    NIR_PASS(_, nir, nir_opt_copy_prop);
    NIR_PASS(_, nir, nir_opt_constant_folding);
    NIR_PASS(_, nir, nir_opt_dce);
    NIR_PASS(_, nir, nir_opt_cse);

    NIR_PASS(_, nir, nir_lower_flrp, 16 | 32 | 64, false /* always_precise */);

    nir_index_ssa_defs(nir_shader_get_entrypoint(nir));
    nir_print_function_body(nir_shader_get_entrypoint(nir), stderr);
    return nir;
}

static void *xenpipe_create_vs_state(struct pipe_context *pipe,
                                     const struct pipe_shader_state *shader){
    fprintf(stderr, "STUB: xenpipe_create_vs_state\n");
    if(shader->type != PIPE_SHADER_IR_NIR || !shader->ir.nir){
        fprintf(stderr, "xenpipe_create_vs_state: Unsup or no IR\n");
        exit(-1);
    }
    xp_lower_and_compile(shader->ir.nir);
    return shader->ir.nir;
}

static void xenpipe_bind_vs_state(struct pipe_context *pipe, void *vs){
    fprintf(stderr, "STUB: xenpipe_bind_vs_state\n");
    xenpipe_context(pipe)->vs = vs;
}

static void xenpipe_delete_vs_state(struct pipe_context *pipe, void *vs){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_delete_vs_state\n");
    exit(-1);
}

static void *xenpipe_create_fs_state(struct pipe_context *pipe,
                                     const struct pipe_shader_state *shader){
    fprintf(stderr, "STUB: xenpipe_create_fs_state\n");
    if(shader->type != PIPE_SHADER_IR_NIR || !shader->ir.nir){
        fprintf(stderr, "xenpipe_create_fs_state: Unsup or no IR\n");
        exit(-1);
    }
    xp_lower_and_compile(shader->ir.nir);
    return shader->ir.nir;
}

static void xenpipe_bind_fs_state(struct pipe_context *pipe, void *fs){
    fprintf(stderr, "STUB: xenpipe_bind_fs_state\n");
    xenpipe_context(pipe)->fs = fs;
}

static void xenpipe_delete_fs_state(struct pipe_context *pipe, void *fs){
    fprintf(stderr, "STUB & INCOMPLETE: xenpipe_delete_fs_state\n");
    exit(-1);
}

static void xenpipe_set_constant_buffer(struct pipe_context *pipe,
                                        mesa_shader_stage stage, unsigned index,
                                        const struct pipe_constant_buffer *buf){
    fprintf(stderr, "STUB: xenpipe_set_constant_buffer\n");
    
    struct pipe_constant_buffer *dst = NULL;
    if(stage == MESA_SHADER_VERTEX)
        dst = &xenpipe_context(pipe)->vs_cbuf;
    else
        dst = &xenpipe_context(pipe)->fs_cbuf;

    // This is just weird
    if(buf){
        pipe_resource_reference(&dst->buffer, buf->buffer);
        dst->buffer_offset = buf->buffer_offset;
        dst->buffer_size = buf->buffer_size;
        dst->user_buffer = buf->user_buffer;
    }
}

void xenpipe_init_state_shader_funcs(struct pipe_context *pipe){
    fprintf(stderr, "STUB: xenpipe_init_state_shader_funcs\n");
    pipe->create_vs_state = xenpipe_create_vs_state;
    pipe->bind_vs_state = xenpipe_bind_vs_state;
    pipe->delete_vs_state = xenpipe_delete_vs_state;
    pipe->create_fs_state = xenpipe_create_fs_state;
    pipe->bind_fs_state = xenpipe_bind_fs_state;
    pipe->delete_fs_state = xenpipe_delete_fs_state;
    pipe->set_constant_buffer = xenpipe_set_constant_buffer;
}