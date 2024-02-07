#include "face_detector.h"

#include <model_zoo/human_face_detect_msr01.hpp>
#include <image/dl_image.hpp>
#include <esp_log.h>
#include <cstdio>

extern "C" {
    static const float score_threshold = 0.1;
    static const float nms_threshold = 0.3;
    static const int top_k = 10;
    static const float resize_scale = 0.3;
    static HumanFaceDetectMSR01 detector(score_threshold, nms_threshold, top_k, resize_scale);

    void draw_hollow_rectangle(uint16_t *image, const uint32_t image_height, const uint32_t image_width,
                           uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
                           const uint16_t color) {
        int line_width = 5; // Line width in pixels
        uint32_t x, y, w;

        // Ensure the rectangle boundaries are within the image
        x1 = (x1 < image_width) ? x1 : image_width - 1;
        y1 = (y1 < image_height) ? y1 : image_height - 1;
        x2 = (x2 < image_width) ? x2 : image_width - 1;
        y2 = (y2 < image_height) ? y2 : image_height - 1;

        // Draw horizontal lines with line width
        for (w = 0; w < line_width; ++w) {
            for (x = x1; x <= x2; ++x) {
                if (y1 + w < image_height) { // Check for bottom boundary
                    image[(y1 + w) * image_width + x] = color; // Top edge
                }
                if (y2 - w >= 0 && y2 - w < image_height) { // Check for top boundary
                    image[(y2 - w) * image_width + x] = color; // Bottom edge
                }
            }
        }

        // Draw vertical lines with line width
        for (w = 0; w < line_width; ++w) {
            for (y = y1; y <= y2; ++y) {
                if (x1 + w < image_width) { // Check for right boundary
                    image[y * image_width + x1 + w] = color; // Left edge
                }
                if (x2 - w >= 0 && x2 - w < image_width) { // Check for left boundary
                    image[y * image_width + x2 - w] = color; // Right edge
                }
            }
        }
    }

    void inference_face_detection(uint16_t *image_data, int width, int height, int channels) {
        std::list<dl::detect::result_t> results = detector.infer<uint16_t>(image_data, {height, width, channels});

        int i = 0;
        for (std::list<dl::detect::result_t>::iterator prediction = results.begin(); prediction != results.end(); prediction++, i++) {
            printf("[%d] score: %f, box: [%d, %d, %d, %d]\n", i, prediction->score, prediction->box[0], prediction->box[1], prediction->box[2], prediction->box[3]);

            draw_hollow_rectangle(image_data, height, width,
                                  DL_MAX(prediction->box[0], 0),
                                  DL_MAX(prediction->box[1], 0),
                                  DL_MAX(prediction->box[2], 0),
                                  DL_MAX(prediction->box[3], 0),
                                  0b1110000000000111);
            break;
        }
    }
}
