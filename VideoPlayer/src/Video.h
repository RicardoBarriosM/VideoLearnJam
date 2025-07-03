#pragma once
#include "Debug.h"

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <inttypes.h>
	#include <libavutil/error.h>
	#include <libswscale/swscale.h>
	#include <libavutil/imgutils.h>
}

class Video
{
public:
	bool Load(const char* path);
	bool GetNextFrame(uint8_t** outData);
	double GetFrameDelay() const;
	int width, height;
	~Video();

private:
	AVFormatContext* formatContext = nullptr;
	AVCodecContext* codecContext = nullptr;
	SwsContext* swsContext = nullptr;
	AVFrame* decodedFrame = nullptr;
	AVFrame* rgbaFrame = nullptr;
	AVPacket* packet = nullptr;
	int videoStreamIndex = -1;
	uint8_t* rgbaBuffer = nullptr;
};