#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define _WIDTH 512
#define _HEIGHT 512

extern "C" {
    #include <libavutil/channel_layout.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/opt.h>
    #include <libavformat/avformat.h>
    #include <libavresample/avresample.h>
    #include <libswscale/swscale.h>
}

// a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream *st;

    /* pts of the next frame that will be generated */
    int64_t next_pts;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    AVAudioResampleContext *avr;
} OutputStream;

class Mpeg {
    OutputStream video_st = { 0 }, audio_st = { 0 };
    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    int have_video = 0;

    unsigned char* _outbuffer=NULL;

    char mpegBuf[65535];
    size_t mpegLen = 0;
    unsigned char* mpegRgb;

    static int IOWriteFunc(void *opaque, uint8_t *buf, int buf_size);
    static int64_t IOSeekFunc (void *opaque, int64_t offset, int whence);
    void add_video_stream(OutputStream *ost, AVFormatContext *oc, enum AVCodecID codec_id);
    AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
    void open_video(AVFormatContext *oc, OutputStream *ost);
    void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height);
    AVFrame *get_video_frame(OutputStream *ost);
    int write_video_frame(AVFormatContext *oc, OutputStream *ost);
    void close_stream(AVFormatContext *oc, OutputStream *ost);
public:
    static void init();
    void addFrame(unsigned char* rgb, char** out, size_t* len);
    Mpeg();
    int create();
    void close();
};