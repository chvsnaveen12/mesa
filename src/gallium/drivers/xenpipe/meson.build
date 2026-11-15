files_xenpipe = files(
    'xp_screen.c',
    'xp_screen.h',
    'xp_public.h',
    'xp_context.c',
    'xp_context.h',
    'xp_texture.c',
    'xp_texture.h',
    'xp_state.h',
    'xp_state_vertex.c',
    'xp_state_texture.c',
    'xp_state_shader.c',
    'xp_state_sampler.c',
    'xp_state_rasterizer.c',
    'xp_state_clip.c',
    'xp_state_blend.c',
    'xp_draw.c',
    'xp_draw.h',
    'xp_shader.c',
    'xp_shader.h',
    'xp_raster.c',
    'xp_raster.h',
)

libxenpipe = static_library(
    'xenpipe',
    files_xenpipe,
    include_directories : [inc_gallium_aux, inc_gallium, inc_include, inc_src],
    c_args : [c_msvc_compat_args],
    gnu_symbol_visibility : 'hidden',
    dependencies : [idep_nir, idep_mesautil],
)

driver_xenpipe = declare_dependency(
    compile_args : '-DGALLIUM_XENPIPE',
    link_with : libxenpipe,
)
