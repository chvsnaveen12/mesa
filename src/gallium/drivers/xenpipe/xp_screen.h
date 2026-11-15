#ifndef XP_SCREEN
#define XP_SCREEN

#include "pipe/p_screen.h"
#include "frontend/sw_winsys.h"

#define XP_MAX_MIP_LEVELS 12

struct xenpipe_screen {
    struct pipe_screen base;
    struct sw_winsys *winsys;
};

static inline struct xenpipe_screen *xenpipe_screen(struct pipe_screen *screen){
    return (struct xenpipe_screen *)screen;
}

#endif
