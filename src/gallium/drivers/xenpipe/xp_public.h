#ifndef XP_PUBLIC
#define XP_PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

struct pipe_screen;
struct sw_winsys;

struct pipe_screen *xenpipe_create_screen(struct sw_winsys *winsys);

#ifdef __cplusplus
}
#endif

#endif
