#if 0
//
// Created by caiiiycuk on 03.04.2020.
//
#include <string>
#include <memory>
#include <algorithm>
#include <cassert>
#include <cstring>

constexpr int frameWidth = 340;
constexpr int frameHeight = 255;

struct ImageFrame {
   uint8_t *rgb = nullptr;
};

struct ImageData {
    std::string file;

    ImageFrame *previewFrame = new ImageFrame;
    ImageFrame *frames;

    int frameCount;
    int activeFrame = 0;
    double activeFrameRenderedAt = 0;
};

struct ImageDataRequest {
    ImageData *data = nullptr;
};

std::unordered_map<std::string, ImageData *> cachedImageData;
std::vector<ImageDataRequest> requests;

ImageData *selectedImageData = nullptr;
ImageData *activeImageData = nullptr;
int pendingLoads = 0;


// TODO: replace placeholders

void loadTick() {
    if (pendingLoads < 3 && !requests.empty()) {
        pendingLoads++;
        ImageDataRequest request = requests.back();
        requests.pop_back();

        ImageData *imageData = request.data;

        abc()->async()->load(imageData->file, [imageData](gamepix::Async::FileStatus) {
            pendingLoads--;

            abc()->image()->loadWebp(imageData->file, [imageData](const gamepix::Image::Raw& raw) {
                assert(raw.bpp == 3);
                assert(imageData->frameCount == 0 && imageData->frames == nullptr);
                assert(imageData->previewFrame->rgb != nullptr);

                imageData->frameCount = raw.height / frameHeight + 1;
                imageData->frames = new ImageFrame[imageData->frameCount];

                for (int i = 0; i < imageData->frameCount - 1; ++i) {
                    auto &activeImageFrame = imageData->frames[i + 1];
                    activeImageFrame.rgb = new uint8_t [raw.width * frameHeight * raw.bpp];
                    memcpy(activeImageFrame.rgb, raw.pixels + raw.width * frameHeight * raw.bpp * i, raw.width * frameHeight * raw.bpp);
                }

                imageData->frames[0].rgb = imageData->previewFrame->rgb;

            });
        }, gamepix::Async::REGULAR);
    }
    abc()->async()->postTask(loadTick, gamepix::Async::REGULAR, gamepix::Async::NORMAL);
}

bool initAvi() {
    abc()->async()->postTask(loadTick, gamepix::Async::REGULAR, gamepix::Async::NORMAL);
    return true;
}

int AVIopen(char *filename, int flags, int channel, void **avi) {
    static bool inited = initAvi();
    std::string imagePath = std::string(filename) + ".decoded";

    auto imageDataPtr = cachedImageData.find(imagePath);

    ImageData *imageData;
    if (imageDataPtr == cachedImageData.end()) {
        imageData = new ImageData();

        std::string previewFile;
        if (imagePath.find("empty.avi") != std::string::npos) {
            imageData->frames = nullptr;
            imageData->file = "";
            previewFile = "resource/video/empty.avi.decoded/encoded.001.webp";
        } else if (imagePath.find("/text/") != std::string::npos) {
            imageData->frames = nullptr;
            imageData->file = "";
            previewFile = imagePath + "/encoded.001.webp";
        } else {
            imageData->frames = nullptr;
            imageData->file = imagePath + "/encoded.webp";
            previewFile = imagePath + "/encoded.001.webp";
        }

        // load preview
        if (!abc()->fs()->isFileExists(previewFile, gamepix::Fs::DEVICE)) {
            imageData->frames = nullptr;
            imageData->file = "";
            previewFile = "resource/video/empty.avi.decoded/encoded.001.webp";
        }

        abc()->image()->loadWebp(previewFile, [imageData, previewFile](const gamepix::Image::Raw& raw) {
            assert(raw.bpp == 3);
            assert(frameWidth == raw.width && frameHeight == raw.height);
            assert(imageData->frameCount == 0);
            assert(imageData->frames == nullptr);
            assert(imageData->previewFrame->rgb == nullptr);
            imageData->previewFrame->rgb = new uint8_t[raw.width * raw.height * raw.bpp];
            memcpy(imageData->previewFrame->rgb, raw.pixels, raw.width * raw.height * raw.bpp);
        });

        cachedImageData.insert(std::make_pair<>(imagePath, imageData));
    } else {
        imageData = imageDataPtr->second;
    }

    *avi = new std::string(imagePath);
    return 1;
}

void AVIplay(void *avi, int xx, int yy) {
}

void AVIstop(void *avi) {
}

void AVIclose(void *avi) {
    delete avi;
}

int AVIwidth(void *avi) {
    return frameWidth;
}

int AVIheight(void *avi) {
    return frameHeight;
}

void AVIPrepareFrame(void *avi) {
    auto *imagePath = reinterpret_cast<std::string *>(avi);
    if (!imagePath) {
        return;
    }

    auto imageDataPtr = cachedImageData.find(*imagePath);
    if (imageDataPtr == cachedImageData.end()) {
        return;
    }

    ImageData *imageData = imageDataPtr->second;
    if (selectedImageData == imageData) {
        return;
    }

    // clear previous to free memory
    if (selectedImageData != nullptr) {
        for (int i = 0; i < selectedImageData->frameCount; ++i) {
            if (selectedImageData->frames[i].rgb != selectedImageData->previewFrame->rgb) {
                delete[] selectedImageData->frames[i].rgb;
                selectedImageData->frames[i].rgb = nullptr;
            }
        }

        delete[] selectedImageData->frames;
        selectedImageData->frames = nullptr;
        imageData->frameCount = 0;
    }

    selectedImageData = imageData;
    requests.clear();

    if (imageData->frameCount == 0 && !imageData->file.empty()) {
        requests.push_back({imageData});
    }
}

void AVIDrawFrame(void *avi, int offsetX, int offsetY, int lineWidth, uint32_t* rgba, float bright) {
    bright = std::min(bright, 1.0f);

    auto *imagePath = reinterpret_cast<std::string *>(avi);
    if (!imagePath) {
        return;
    }

    auto imageDataPtr = cachedImageData.find(*imagePath);
    if (imageDataPtr == cachedImageData.end()) {
        return;
    }

    auto imageData = imageDataPtr->second;

    ImageFrame *frame;
    if (imageData->frameCount == 0) {
        frame = imageData->previewFrame;
    } else {
        auto now = gamepix::currentTimeMs().toDouble();
        frame = &imageData->frames[imageData->activeFrame];
        if (now - imageData->activeFrameRenderedAt > (1100 / imageData->frameCount)) {
            int nextFrame = (imageData->activeFrame + 1) % imageData->frameCount;

            ImageFrame *next = &imageData->frames[nextFrame];
            if (next->rgb != nullptr) {
                frame = next;
                imageData->activeFrame = nextFrame;
                imageData->activeFrameRenderedAt = now;
            }
        }
    }

    if (frame->rgb == nullptr) {
        return;
    }

    int width = AVIwidth(avi);
    int height = AVIheight(avi);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t *pixel = (uint8_t *)(rgba + (y + offsetY) * lineWidth + x + offsetX);
            uint8_t *framePixel = frame->rgb + (y * width  + x) * 3;
#ifdef NG_MOD
			pixel[2] = std::max(std::min(framePixel[2] * bright, 255.0f), 0.0f);
			pixel[1] = std::max(std::min(framePixel[1] * bright, 255.0f), 0.0f);
			pixel[0] = std::max(std::min(framePixel[0] * bright, 255.0f), 0.0f);
#else
			pixel[0] = std::max(std::min(framePixel[2] * bright, 255.0f), 0.0f);
            pixel[1] = std::max(std::min(framePixel[1] * bright, 255.0f), 0.0f);
            pixel[2] = std::max(std::min(framePixel[0] * bright, 255.0f), 0.0f);
#endif
            pixel[3] = 255;
        }
    }
}
#endif
