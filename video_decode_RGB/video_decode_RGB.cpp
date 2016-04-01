/*
*      Filename:  video_deocde_RGB.cpp
*
*   Description: Read video from a file and decode it.Convert the decoded frame to RGB 
*                 and save to picture in the PPM format
*       Version:  1.0
*       Created:  2016Äê4ÔÂ1ÈÕ
*      Revision:  none
*      Compiler:  Visual Studio 2015 update 1
*
*        Author:  Jeff Young
*       Company:  Home
*/

#include <stdio.h>  

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"avutil.lib")


void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
	FILE *pFile;
	char szFilename[32];
	int  y;

	// Open file
	sprintf_s(szFilename, "frame%d.ppm", iFrame);
	fopen_s(&pFile, szFilename, "wb");
	if (pFile == NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	// Write pixel data
	for (y = 0; y < height; y++)
		fwrite(pFrame->data[0] + y*pFrame->linesize[0], 1, width * 3, pFile);

	// Close file
	fclose(pFile);
}

int main(int argc, char *argv[]) {
	// Initalizing these to NULL prevents segfaults!
	AVFormatContext   *pFormatCtx = nullptr;
	int               i, videoStream;
	AVCodecContext    *pCodecCtxOrig = nullptr;
	AVCodecContext    *pCodecCtx = nullptr;
	AVCodec           *pCodec = nullptr;
	AVFrame           *pFrame = nullptr;
	AVFrame           *pFrameRGB = nullptr;
	AVPacket          packet;
	int               frameFinished;
	int               numBytes;
	uint8_t           *buffer = nullptr;
	struct SwsContext *sws_ctx = nullptr;

	const char *inputFile = "G:\\a.mp4";

	// Register all formats and codecs
	av_register_all();

	// Open video file
	if (avformat_open_input(&pFormatCtx, inputFile, NULL, NULL) != 0)
		return -1; // Couldn't open file

				   // Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return -1; // Couldn't find stream information

				   // Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, inputFile, 0);

	// Find the first video stream
	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	if (videoStream == -1)
		return -1; // Didn't find a video stream

				   // Get a pointer to the codec context for the video stream
	pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;
	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (pCodec == NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}
	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		return -1; // Could not open codec

				   // Allocate video frame
	pFrame = av_frame_alloc();

	// Allocate an AVFrame structure
	pFrameRGB = av_frame_alloc();
	if (pFrameRGB == NULL)
		return -1;

	// Determine required buffer size and allocate buffer£ºwidth*height*3(24bits)
	numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
		pCodecCtx->height, 1);
	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	int linesize = 0;
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24,
		pCodecCtx->width, pCodecCtx->height, 1);

	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		AV_PIX_FMT_RGB24,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);

	// Read frames and save first five frames to disk
	i = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream) {
			// Decode video frame
			int iret = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			// Did we get a video frame?
			if (frameFinished) {
				// Convert the image from its native format to RGB
				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height,
					pFrameRGB->data, pFrameRGB->linesize);

				// Save the frame to disk
				if (++i <= 5)
					SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height,
						i);
			}
		}

		// Free the packet that was allocated by av_read_frame
		av_packet_unref(&packet);
	}

	// Free the RGB image
	//av_free(buffer);
	av_frame_free(&pFrameRGB);

	// Free the YUV frame
	av_frame_free(&pFrame);

	// Close the codecs
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	return 0;
}