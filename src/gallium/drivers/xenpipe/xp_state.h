#ifndef XP_STATE
#define XP_STATE

struct pipe_context;

void xenpipe_init_state_blend_funcs(struct pipe_context *pipe);
void xenpipe_init_state_clip_funcs(struct pipe_context *pipe);
void xenpipe_init_state_rasterizer_funcs(struct pipe_context *pipe);
void xenpipe_init_state_sampler_funcs(struct pipe_context *pipe);
void xenpipe_init_state_shader_funcs(struct pipe_context *pipe);
void xenpipe_init_state_texture_funcs(struct pipe_context *pipe);
void xenpipe_init_state_vertex_funcs(struct pipe_context *pipe);

#endif