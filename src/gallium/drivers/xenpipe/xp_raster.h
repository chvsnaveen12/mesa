#ifndef XP_RASTER
#define XP_RASTER

struct xp_quad{
    unsigned qx, qy;
    float z[4];
    float var[4][8][4];
    char mask;
    bool front_face;
    float color_out[4][4];
};

void xp_rasterize_triangle(struct xenpipe_context *xenpipe, 
                           struct xp_clip_vert *v0,
                           struct xp_clip_vert *v1,
                           struct xp_clip_vert *v2);

#endif