#include "face_detector.h"

#include <model_zoo/human_face_detect_msr01.hpp>
#include <esp_log.h>

extern "C" {
    static const float score_threshold = 0.3;
    static const float nms_threshold = 0.5;
    static const int top_k = 10;
    static const float resize_scale = 0.2;
    static HumanFaceDetectMSR01 detector(score_threshold, nms_threshold, top_k, resize_scale);

    void inference_face_detection(uint8_t *image_data, int width, int height, int channels) {
        ESP_LOGI("face_detector", "Inference face detection starting");
        std::list<dl::detect::result_t> results = detector.infer(image_data, {height, width, channels});
        ESP_LOGI("face_detector", "Found %d faces", results.size());
    }
}
