#ifndef XP_DRAW
#define XP_DRAW

struct xenpipe_context;
struct pipe_draw_info;
struct pipe_draw_start_count_bias;

struct xp_clip_vert{
    float pos[4];
    float var[8][4];
    float psize;
};

void xenpipe_do_draw_vbo(struct xenpipe_context *xenpipe,
                         const struct pipe_draw_info *info,
                         const struct pipe_draw_start_count_bias *draws,
                         unsigned num_draws);
#endif