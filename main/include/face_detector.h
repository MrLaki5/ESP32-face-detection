#ifndef FACE_DETECTOR_H_
#define FACE_DETECTOR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void inference_face_detection(uint16_t *image_data, int width, int height, int channels);

#ifdef __cplusplus
}
#endif

#endif /* FACE_DETECTOR_H_ */
