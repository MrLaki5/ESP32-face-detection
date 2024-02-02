#include "face_detector.h"

#include <model_zoo/human_face_detect_msr01.hpp>
#include <dl_image.hpp>
#include <esp_log.h>
#include <cstdio>

extern "C" {
    static const float score_threshold = 0.0;
    static const float nms_threshold = 0.3;
    static const int top_k = 10;
    static const float resize_scale = 0.3;
    static HumanFaceDetectMSR01 detector(score_threshold, nms_threshold, top_k, resize_scale);

    void inference_face_detection(uint16_t *image_data, int width, int height, int channels) {
        std::list<dl::detect::result_t> results = detector.infer<uint16_t>(image_data, {height, width, channels});

        int i = 0;
        for (std::list<dl::detect::result_t>::iterator prediction = results.begin(); prediction != results.end(); prediction++, i++) {
            printf("[%d] score: %f, box: [%d, %d, %d, %d]\n", i, prediction->score, prediction->box[0], prediction->box[1], prediction->box[2], prediction->box[3]);

            dl::image::draw_hollow_rectangle(image_data, height, width,
                                         DL_MAX(prediction->box[0], 0),
                                         DL_MAX(prediction->box[1], 0),
                                         DL_MAX(prediction->box[2], 0),
                                         DL_MAX(prediction->box[3], 0),
                                         0b1110000000000111);
        }
    }
}
