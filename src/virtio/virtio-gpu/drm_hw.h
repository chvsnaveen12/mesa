/* Stub — stripped virtio support */
#ifndef VIRTIO_GPU_DRM_HW_H
#define VIRTIO_GPU_DRM_HW_H

#include <stdint.h>

#ifndef VIRTGPU_DRM_CAPSET_DRM
#define VIRTGPU_DRM_CAPSET_DRM 3
#endif

struct virgl_renderer_capset_drm {
    uint32_t dummy[64];
};

#endif
