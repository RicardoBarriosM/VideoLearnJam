#include "Video.h"

bool Video::Load(const char* path)
{
	//Open the file using libavformat
	formatContext = avformat_alloc_context();

	if (avformat_open_input(&formatContext, path, NULL, NULL) != 0)
	{
		Debug::LogError("Couldn't open video file");
		return false;
	}

	//find the first valid video stream in file
	videoStreamIndex = -1;
	AVCodecParameters* avCodecParams = nullptr;
	const AVCodec* codec = nullptr;

	for (int i = 0; i < formatContext->nb_streams; i++)
	{
		avCodecParams = formatContext->streams[i]->codecpar;
		codec = avcodec_find_decoder(avCodecParams->codec_id);
		if (!codec)
			continue;

		if (avCodecParams->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStreamIndex = i;
			break;
		}
	}

	if (videoStreamIndex == -1)
	{
		Debug::LogError("Couldn't find valid video stream in file");
		return false;
	}

	codecContext = avcodec_alloc_context3(codec);

	if (avcodec_parameters_to_context(codecContext, avCodecParams) < 0)
	{
		Debug::LogError("Couldn't initialize avcodec parameters to context");
		return false;
	}

	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		Debug::LogError("Couldn't open codec");
		return false;
	}

	decodedFrame = av_frame_alloc();

	rgbaFrame = av_frame_alloc();
	rgbaFrame->format = AV_PIX_FMT_RGBA;
	rgbaFrame->width = width;
	rgbaFrame->height = height;

	packet = av_packet_alloc();

	width = codecContext->width;
	height = codecContext->height;

	rgbaBuffer = new uint8_t[width * height * 4];

	return true;
}

bool Video::GetNextFrame(uint8_t** outData)
{
	//av_read_frame always returns the next packet, it maintains an internal file position pointer
	while (av_read_frame(formatContext, packet) >= 0) {
		// we are only interested in the video stream we found in Load()
		if (packet->stream_index != videoStreamIndex) {
			av_packet_unref(packet);
			continue;
		}

		// sends the packet to the decoder 
		if (avcodec_send_packet(codecContext, packet) < 0) {
			av_packet_unref(packet);
			return false;
		}

		av_packet_unref(packet);

		// Retrieves one decoded frame from the decoder
		if (avcodec_receive_frame(codecContext, decodedFrame) == 0) {

			// Frames in videos are commonly stored as YUV (Y = Brightness, U = blue minus luma, V = red minus luma)
			// However, OpenGL expects RGBA frames, so we need to convert them
			av_image_fill_arrays(rgbaFrame->data, rgbaFrame->linesize, rgbaBuffer, AV_PIX_FMT_RGBA, width, height, 1);

			if (!swsContext)
			{
				swsContext = sws_getContext(
					width, height, codecContext->pix_fmt, // source format
					width, height, AV_PIX_FMT_RGBA,		// target format
					SWS_BILINEAR, nullptr, nullptr, nullptr
				);
			}

			// Actual conversion
			sws_scale(swsContext, decodedFrame->data, decodedFrame->linesize,
				0, codecContext->height, rgbaFrame->data, rgbaFrame->linesize);

			*outData = rgbaBuffer;
			return true;
		}
	}
	return false;
}

double Video::GetFrameDelay() const
{
	AVRational fps = formatContext->streams[videoStreamIndex]->avg_frame_rate;
	if (fps.num != 0 && fps.den != 0)
	{
		double frameRate = av_q2d(fps);      // e.g., 24.0
		return 1.0 / frameRate;              // e.g., ~0.041666 seconds per frame
	}
	return 1.0 / 30.0;
}


Video::~Video()
{
	if (rgbaBuffer) delete[] rgbaBuffer;
	if (packet) av_packet_free(&packet);
	if (decodedFrame) av_frame_free(&decodedFrame);
	if (rgbaFrame) av_frame_free(&rgbaFrame);
	if (swsContext) sws_freeContext(swsContext);
	if (codecContext) avcodec_free_context(&codecContext);
	if (formatContext) avformat_close_input(&formatContext);
}
