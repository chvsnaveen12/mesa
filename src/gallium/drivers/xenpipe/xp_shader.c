#include <stdio.h>
#include "xp_shader.h"
#include "xp_context.h"
#include "xp_draw.h"
#include "xp_raster.h"
#include "xp_texture.h"

static xp_reg alu_src(struct xp_interp_state *st, nir_alu_instr *instr,
                      unsigned src, unsigned comp){
    nir_alu_src *s = &instr->src[src];
    return st->ssa[s->src.ssa->index][s->swizzle[comp]];
}

static void xp_exec_load_const(struct xp_interp_state *st, nir_load_const_instr *instr){
    if(instr->def.bit_size != 32){
        fprintf(stderr, "xp_exec_load_const: Unknown load size %d\n", instr->def.bit_size);
        exit(-1);
    } 

    for(unsigned c = 0; c < instr->def.num_components; c++)
        st->ssa[instr->def.index][c].u = instr->value[c].u32;
}

static void xp_exec_intrin_store_output(struct xp_interp_state *st, nir_intrinsic_instr *instr){
    nir_io_semantics sem = nir_intrinsic_io_semantics(instr);
    unsigned comp = nir_intrinsic_component(instr);
    unsigned wrmask = nir_intrinsic_write_mask(instr);
    nir_src *val = &instr->src[0];

    float *dst;

    if(st->stage == MESA_SHADER_FRAGMENT){
        if(sem.location == FRAG_RESULT_COLOR)
            dst = st->color_out;
        else{
            fprintf(stderr, "xp_exec_intrin_store_output: Unknown gl_frag_result store loc %u\n", sem.location);
            exit(-1);
        }
    }
    else{
        if(sem.location == VARYING_SLOT_POS)
            dst = st->out->pos;
        else if(sem.location >= VARYING_SLOT_VAR0 &&
                sem.location < VARYING_SLOT_VAR8)
            dst = st->out->var[sem.location - VARYING_SLOT_VAR0];
        else{
            fprintf(stderr, "xp_exec_intrin_store_output: Unknown gl_varying_slot store loc %u\n", sem.location);
            exit(-1);
        }
    }
    u_foreach_bit(c, wrmask)
        dst[comp + c] = st->ssa[val->ssa->index][c].f;
}

static void xp_exec_intrin_load_input(struct xp_interp_state *st, nir_intrinsic_instr *instr){
    // fprintf(stderr, "asdfasdf\n");
    // exit(-1);

    unsigned base = nir_intrinsic_base(instr);
    unsigned comp = nir_intrinsic_component(instr);

    nir_src *off = &instr->src[0];
    unsigned slot = base + st->ssa[off->ssa->index][0].u;

    for(unsigned c = 0; c < instr->def.num_components; c++)
        st->ssa[instr->def.index][c].f = st->in[slot][comp + c];
}

static void xp_exec_alu(struct xp_interp_state *st, nir_alu_instr *instr){
    // fprintf(stderr, "adfadf\n");
    // exit(-1);

    if(instr->def.bit_size != 32){
        fprintf(stderr, "xp_exec_alu: Unknown load size %d\n", instr->def.bit_size);
        exit(-1);
    }
    xp_reg dst[4];

    for(unsigned c = 0; c < instr->def.num_components; c++){
        switch(instr->op){
            case nir_op_frcp:
                dst[c].f = 1.0f / alu_src(st, instr, 0, c).f;
                break;
            case nir_op_fpow:{
                float a = alu_src(st, instr, 0, c).f;
                float b = alu_src(st, instr, 1, c).f;
                dst[c].f = powf(a, b);
                break;}
            case nir_op_fsat:{
                float a = alu_src(st, instr, 0, c).f;
                dst[c].f = fminf(fmaxf(a, 0.0f), 1.0f);
                break;}
            case nir_op_fdiv:
                dst[c].f = alu_src(st, instr, 0, c).f / alu_src(st, instr, 1, c).f;
                break;
            case nir_op_fsqrt:
                dst[c].f = sqrtf(alu_src(st, instr, 0, c).f);
                break;
            case nir_op_fsin:
                dst[c].f = sinf(alu_src(st, instr, 0, c).f);
                break;
            case nir_op_mov:
                dst[c] = alu_src(st, instr, 0, c);
                break;
            case nir_op_fmod:{
                float a = alu_src(st, instr, 0, c).f;
                float b = alu_src(st, instr, 1, c).f;
                dst[c].f = a - b * floorf(a / b);
                break;}
            case nir_op_ffloor:
                dst[c].f = floorf(alu_src(st, instr, 0, c).f);
                break;
            case nir_op_b2f32:
                dst[c].f = alu_src(st, instr, 0, c).u ? 1.0f : 0.0f;
                break;
            case nir_op_fge32:
                dst[c].u = alu_src(st, instr, 0, c).f >= alu_src(st, instr, 1, c).f ? ~0u : 0u;
                break;
            case nir_op_ffract:{
                float a = alu_src(st, instr, 0, c).f;
                dst[c].f = a - floorf(a);
                break;}
            case nir_op_fmax:
                dst[c].f = fmaxf(alu_src(st, instr, 0, c).f, alu_src(st, instr, 1, c).f);
                break;
            case nir_op_fneg:
                dst[c].f = -alu_src(st, instr, 0, c).f;
                break;
            case nir_op_frsq:
                dst[c].f = 1.0f / sqrtf(alu_src(st, instr, 0, c).f);
                break;
            case nir_op_fdot2:
            case nir_op_fdot3:
            case nir_op_fdot4:{
                unsigned n = nir_op_infos[instr->op].input_sizes[0];
                float sum = 0.0f;
                for(unsigned i = 0; i < n; i++)
                    sum += alu_src(st, instr, 0, i).f * alu_src(st, instr, 1, i).f;
                dst[c].f = sum;
                break;}
            case nir_op_fadd:
                dst[c].f = alu_src(st, instr, 0, c).f + alu_src(st, instr, 1, c).f;
                break;
            case nir_op_fmul:
                dst[c].f = alu_src(st, instr, 0, c).f * alu_src(st, instr, 1, c).f;
                break;
            case nir_op_vec2:
            case nir_op_vec3:
            case nir_op_vec4:
                dst[c] = alu_src(st, instr, c, 0);
                break;
            default:
                fprintf(stderr, "xp_exec_alu: Unknown op type %s\n", nir_op_infos[instr->op].name);
                exit(-1);
        }
    }
    for(unsigned c = 0; c < instr->def.num_components; c++)
        st->ssa[instr->def.index][c] = dst[c];
}

static void xp_exec_load_ubo(struct xp_interp_state *st, nir_intrinsic_instr *instr){
    struct xenpipe_context *xenpipe = st->xenpipe;
    struct pipe_constant_buffer *cbuf = st->stage == MESA_SHADER_FRAGMENT ? &xenpipe->fs_cbuf : &xenpipe->vs_cbuf;

    const uint8_t *uniform = NULL;
    if(cbuf->user_buffer)
        uniform = (uint8_t *)cbuf->user_buffer + cbuf->buffer_offset;
    else if(cbuf->buffer)
        uniform = (uint8_t *)(xenpipe_resource(cbuf->buffer)->data) + cbuf->buffer_offset;


    if(!uniform){
        fprintf(stderr, "xp_exec_load_ubo: No const buffer\n");
        exit(-1);
    }
    if(instr->def.bit_size != 32){
        fprintf(stderr, "xp_exec_load_ubo: Unknown load size %d\n", instr->def.bit_size);
        exit(-1);
    }
    unsigned index = st->ssa[instr->src[0].ssa->index][0].u;
    unsigned byte_off = st->ssa[instr->src[1].ssa->index][0].u;
    unsigned ncomp = instr->def.num_components;

    if(index){
        fprintf(stderr, "xp_exec_load_ubo: Non 0 constant buffer\n");
        exit(-1);
    }
    if (cbuf->buffer_size && byte_off + ncomp * 4 > cbuf->buffer_size) {
        fprintf(stderr, "xp_exec_load_ubo: OOB [%u,%u) of %u\n",
                byte_off, byte_off + ncomp * 4, cbuf->buffer_size);
        exit(-1);
    }

    for(unsigned c = 0; c < ncomp; c++)
        memcpy(&st->ssa[instr->def.index][c].u, uniform + byte_off + c * 4, 4);

    // fprintf(stderr, "asdfasdfasdf\n");
    // exit(-1);
}

static int xp_wrap(float coord, unsigned wrap_mode, unsigned size){
    int i = (int)floorf(coord * (float)size);
    switch(wrap_mode){
        case PIPE_TEX_WRAP_REPEAT:
            i %= (int)size;
            return i < 0 ? i + (int)size : i;
        case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
            return CLAMP(i, 0, (int)size - 1);
        default:
            fprintf(stderr, "xp_wrap: Unsupported wrap mode %d\n", wrap_mode);
            exit(-1);
    }
}

static void xp_exec_tex(struct xp_interp_state *st, nir_tex_instr *instr){
    if(instr->op != nir_texop_tex){
        fprintf(stderr, "xp_exec_tex: Unsupported texop %u\n", instr->op);   
        exit(-1);
    }
    if(instr->sampler_dim != GLSL_SAMPLER_DIM_2D ||
       instr->is_array || instr->is_shadow || instr->is_sparse ||
       instr->coord_components != 2){
        fprintf(stderr, "xp_exec_tex: Only plain 2d sampling\n");   
        exit(-1);
    }
    if(instr->def.bit_size != 32){
        fprintf(stderr, "xp_exec_tex: Unknown load size %d\n", instr->def.bit_size);
        exit(-1);
    }

    nir_src *coord = NULL;
    for(unsigned i = 0; i < instr->num_srcs; i++){
        if(instr->src[i].src_type == nir_tex_src_coord){
            coord = &instr->src[i].src;
        }
        else{
            fprintf(stderr, "xp_exec_tex: Unsup tex src %d\n", instr->src[i].src_type);
            exit(-1);
        }
    }
    if(!coord){
        fprintf(stderr, "xp_exec_tex: no coord source\n");
        exit(-1);
    }

    struct xenpipe_context *xenpipe = st->xenpipe;
    unsigned idx = instr->texture_index;
    if(idx >= XP_MAX_SAMPLERS){
        fprintf(stderr, "xp_exec_tex: tex idx %u out of range\n", idx);
        exit(-1);
    }

    struct pipe_sampler_view *view = xenpipe->sampler_views[idx];
    struct pipe_sampler_state *samp = xenpipe->samplers[idx];
    if(!view || !samp || !view->texture){
        fprintf(stderr, "xp_exec_tex: nothing bound at unit %u\n", idx);
        exit(-1);
    }

    struct xenpipe_resource *res = xenpipe_resource(view->texture);
    if (!res->data) {
        fprintf(stderr, "xp_exec_tex: resource has no backing store\n");
        exit(-1);
    }

    // Actual tex logic
    float s = st->ssa[coord->ssa->index][0].f;
    float t = st->ssa[coord->ssa->index][1].f;

    unsigned w = view->texture->width0;
    unsigned h = view->texture->height0;
    int x = xp_wrap(s, samp->wrap_s, w);
    int y = xp_wrap(t, samp->wrap_t, h);

    unsigned blocksize = util_format_get_blocksize(view->format);
    const uint8_t *texel = (const uint8_t *)res->data +
                           res->level_offset[0] +
                           (unsigned)y * res->stride[0] +
                           (unsigned)x * blocksize;

    float rgba[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    util_format_unpack_rgba(view->format, rgba, texel, 1);

    for(unsigned c = 0; c < instr->def.num_components; c++)
        st->ssa[instr->def.index][c].f = rgba[c];
}

static void xp_exec_block(struct xp_interp_state *st, struct nir_block  *block){
    nir_foreach_instr(instr, block){
        switch(instr->type){
            case nir_instr_type_tex:
                xp_exec_tex(st, nir_instr_as_tex(instr));
                break;
            case nir_instr_type_alu:
                xp_exec_alu(st, nir_instr_as_alu(instr));
                break;
            case nir_instr_type_load_const:
                xp_exec_load_const(st, nir_instr_as_load_const(instr));
                break;
            case nir_instr_type_intrinsic:
                nir_intrinsic_instr *intrin_instr = nir_instr_as_intrinsic(instr);
                switch(intrin_instr->intrinsic){
                    case nir_intrinsic_store_output:
                        xp_exec_intrin_store_output(st, intrin_instr);
                        break;
                    case nir_intrinsic_load_input:
                        xp_exec_intrin_load_input(st, intrin_instr);
                        break;
                    case nir_intrinsic_load_ubo:
                        xp_exec_load_ubo(st, intrin_instr);
                        break;
                    default:
                        fprintf(stderr, "xp_exec_block: Unknown intrin type %u\n", intrin_instr->intrinsic);
                        exit(-1);
                }
                break;
            default:
                fprintf(stderr, "xp_exec_block: Unknown instr type %u\n", instr->type);
                exit(-1);
        }
    }
}

static void xp_exec_cf_list(struct xp_interp_state *st, struct exec_list *list){
    foreach_list_typed(nir_cf_node, node, node, list){
        switch(node->type){
            case nir_cf_node_block:
                xp_exec_block(st, nir_cf_node_as_block(node));
                break;
            default:
                fprintf(stderr, "xp_exec_cf_list: Unknown cf type %d\n", node->type);
                exit(-1);
        }
    }
}

void xp_run_vs(struct xenpipe_context *xenpipe,
               float inps[][4], struct xp_clip_vert *out){
    fprintf(stderr, "STUB & VERY INCOMPLETE: xp_run_vs\n");        

    if(!xenpipe->vs){
        out->pos[0] = inps[0][0];
        out->pos[1] = inps[0][1];
        out->pos[2] = inps[0][2];
        out->pos[3] = 1.0f;
    }
    // if(out->pos[1] == 0.8f){
    //     out->pos[1] = 3.0f;
    // }

    // fprintf(stderr, "inps[0][0]=%f\n", inps[0][0]);
    // fprintf(stderr, "inps[0][1]=%f\n", inps[0][1]);
    // if(inps[0][0] == -0.5f && inps[0][1] == -0.5f){
    //     out->var[0][0] = 1.0f;
    //     out->var[0][1] = 0.0f;
    //     out->var[0][2] = 0.0f;
    //     out->var[0][3] = 1.0f;
    //     fprintf(stderr, "1111111111111\n");
    // }
    // if(inps[0][0] == 0.5f && inps[0][1] == -0.5f){
    //     out->var[0][0] = 0.0f;
    //     out->var[0][1] = 1.0f;
    //     out->var[0][2] = 0.0f;
    //     out->var[0][3] = 1.0f;
    //     fprintf(stderr, "22222222222222222\n");
    // }
    // if(inps[0][0] == 0.0f && inps[0][1] == 0.5f){
    //     out->var[0][0] = 0.0f;
    //     out->var[0][1] = 0.0f;
    //     out->var[0][2] = 1.0f;
    //     out->var[0][3] = 1.0f;
    //     fprintf(stderr, "33333333333333333\n");
    // }

    struct xp_interp_state st;
    st.xenpipe = xenpipe;
    st.nir = xenpipe->vs;
    st.stage = MESA_SHADER_VERTEX;
    st.in = inps;
    st.out = out;

    nir_function_impl *entry = nir_shader_get_entrypoint(xenpipe->vs);
    xp_exec_cf_list(&st, &entry->body);
}

void xp_run_fs(struct xenpipe_context *xenpipe,
               struct xp_quad *quad){
    if(!xenpipe->fs){
        for(unsigned p = 0; p < 4; p++){
            quad->color_out[p][0] = 1.0f;
            quad->color_out[p][1] = 0.0f;
            quad->color_out[p][2] = 0.0f;
            quad->color_out[p][3] = 0.0f;
        }
        return;
    }

    struct xp_interp_state st;
    // float dvdx[8][4];
    // float dvdy[8][4];
    // for(unsigned v = 0; v < 8; v++){
    //     for(unsigned c = 0; c < 4; c++){
    //         dvdx[v][c] = quad->var[3][v][c] - quad->var[2][v][c];
    //         dvdx[v][c] = quad->var[2][v][c] - quad->var[0][v][c];
    //     }
    // }

    nir_function_impl *entry = nir_shader_get_entrypoint(xenpipe->fs);

    st.xenpipe = xenpipe;
    st.nir = xenpipe->fs;
    st.stage = MESA_SHADER_FRAGMENT;

    for(unsigned p = 0; p < 4; p++){
        st.in = quad->var[p];
        xp_exec_cf_list(&st, &entry->body);

        for(unsigned c = 0; c < 4; c++)
            quad->color_out[p][c] = st.color_out[c];
    }
}