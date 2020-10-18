#include "mpeg.hpp"

#define STREAM_FRAME_RATE 50 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

#define SCALE_FLAGS SWS_BICUBIC

#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER
#define CODEC_CAP_VARIABLE_FRAME_SIZE AV_CODEC_CAP_VARIABLE_FRAME_SIZE
#define AVFMT_RAWPICTURE 0x0020

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)
#define RGB2Y(R, G, B) CLIP(( (  66 * (R) + 129 * (G) +  25 * (B) + 128) >> 8) +  16)
#define RGB2U(R, G, B) CLIP(( ( -38 * (R) -  74 * (G) + 112 * (B) + 128) >> 8) + 128)
#define RGB2V(R, G, B) CLIP(( ( 112 * (R) -  94 * (G) -  18 * (B) + 128) >> 8) + 128)

int Mpeg::IOWriteFunc(void *opaque, uint8_t *buf, int buf_size) {
    Mpeg* mpeg = (Mpeg*)opaque;
    memcpy(mpeg->mpegBuf, buf, buf_size);
    mpeg->mpegLen = buf_size;
    return buf_size;
}

int64_t Mpeg::IOSeekFunc (void *opaque, int64_t offset, int whence) {
    switch(whence){
        case SEEK_SET:
            return 1;
            break;
        case SEEK_CUR:
            return 1;
            break;
        case SEEK_END:
            return 1;
            break;
        case AVSEEK_SIZE:
            return 4096;
            break;
        default:
        return -1;
    }
    return 1;
}

void Mpeg::add_video_stream(OutputStream *ost, AVFormatContext *oc, enum AVCodecID codec_id)
{
    AVCodecContext *c;
    AVCodec *codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    ost->st = avformat_new_stream(oc, codec);
    if (!ost->st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c = ost->st->codec;

    /* Put sample parameters. */
    //c->bit_rate = 400000;
    c->bit_rate = 1000000;
    //c->bit_rate = 4000;
    /* Resolution must be a multiple of two. */
    //c->width    = 352;
    //c->height   = 288;
    c->width    = _WIDTH;
    c->height   = _HEIGHT;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
    * of which frame timestamps are represented. For fixed-fps content,
    * timebase should be 1/framerate and timestamp increments should be
    * identical to 1. */
    ost->st->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
    c->time_base       = ost->st->time_base;

    c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt       = STREAM_PIX_FMT;
    if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /* Needed to avoid using macroblocks in which some coeffs overflow.
        * This does not happen with normal video, it just happens here as
        * the motion of the chroma plane does not match the luma plane. */
        c->mb_decision = 2;
    }
    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

AVFrame *Mpeg::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

void Mpeg::open_video(AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext *c;

    c = ost->st->codec;

    /* open the codec */
    if (avcodec_open2(c, NULL, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    /* Allocate the encoded raw picture. */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        fprintf(stderr, "Could not allocate picture\n");
        exit(1);
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
    * picture is needed too. It is then converted to the required
    * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }
}

void Mpeg::fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)
{
    int x, y, i, ret;

    /* when we pass a frame to the encoder, it may keep a reference to it
    * internally;
    * make sure we do not overwrite it here
    */
    ret = av_frame_make_writable(pict);
    if (ret < 0)
        exit(1);

    i = frame_index;

    /* Y */
    for (y = 0; y < _HEIGHT; y++) {
        for (x = 0; x < _WIDTH; x++) {
            //int R = rand() % 255;
            //int G = rand() % 255;
            //int B = rand() % 255;
            //int offset = (x * _HEIGHT + y);
            int offset = (x + y * _WIDTH);
            int R = mpegRgb[offset * 3 + 0];
            int G = mpegRgb[offset * 3 + 1];
            int B = mpegRgb[offset * 3 + 2];
            pict->data[0][y * pict->linesize[0] + x] = RGB2Y(R, G, B);
            if (x % 2 == 0 && y % 2 == 0) {
                pict->data[1][(y / 2) * pict->linesize[1] + (x / 2)] = RGB2U(R, G, B);
                pict->data[2][(y / 2) * pict->linesize[2] + (x / 2)] = RGB2V(R, G, B);
            }
            //pict->data[0][y * pict->linesize[0] + x] = x;
        }
    }
}

AVFrame* Mpeg::get_video_frame(OutputStream *ost)
{
    AVCodecContext *c = ost->st->codec;

    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        /* as we only generate a YUV420P picture, we must convert it
        * to the codec pixel format if needed */
        if (!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(c->width, c->height,
                                        AV_PIX_FMT_YUV420P,
                                        c->width, c->height,
                                        c->pix_fmt,
                                        SCALE_FLAGS, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                fprintf(stderr,
                        "Cannot initialize the conversion context\n");
                exit(1);
            }
        }
        fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);
        sws_scale(ost->sws_ctx, ost->tmp_frame->data, ost->tmp_frame->linesize,
                0, c->height, ost->frame->data, ost->frame->linesize);
    } else {
        fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);
    }

    ost->frame->pts = ost->next_pts++;

    return ost->frame;
}

int Mpeg::write_video_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret = 0;
    AVCodecContext *c;
    AVFrame *frame;
    int got_packet = 0;

    c = ost->st->codec;

    frame = get_video_frame(ost);

    if (oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* a hack to avoid data copy with some raw video muxers */
        AVPacket pkt;
        av_init_packet(&pkt);

        if (!frame)
            return 1;

        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.stream_index  = ost->st->index;
        pkt.data          = (uint8_t *)frame;
        pkt.size          = sizeof(AVPicture);

        pkt.pts = pkt.dts = frame->pts;
        av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);

        ret = av_interleaved_write_frame(oc, &pkt);
    } else {
        AVPacket pkt = { 0 };
        av_init_packet(&pkt);

        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
        if (ret < 0) {
            fprintf(stderr, "Error encoding a video frame\n");
            exit(1);
        }

        if (got_packet) {
            av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(oc, &pkt);
        }
    }
    if (ret != 0) {
        fprintf(stderr, "Error while writing video frame\n");
        exit(1);
    }

    return (frame || got_packet) ? 0 : 1;
}

void Mpeg::close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_close(ost->st->codec);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    avresample_free(&ost->avr);
}

void Mpeg::init()
{
    av_register_all();
}

void Mpeg::addFrame(unsigned char* rgb, char** out, size_t* len)
{
    mpegRgb = rgb;
    write_video_frame(oc, &video_st);
    *out = mpegBuf;
    *len = mpegLen;
}

Mpeg::Mpeg()
{
}

int Mpeg::create()
{
    filename = "wao.ts";

    /* Autodetect the output format from the name. default is MPEG. */
    fmt = av_guess_format(NULL, filename, NULL);
    if (!fmt) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        fmt = av_guess_format("mpeg", NULL, NULL);
    }
    if (!fmt) {
        fprintf(stderr, "Could not find suitable output format\n");
        return 1;
    }

    /* Allocate the output media context. */
    oc = avformat_alloc_context();
    if (!oc) {
        fprintf(stderr, "Memory error\n");
        return 1;
    }
    oc->oformat = fmt;
    //snprintf(oc->filename, sizeof(oc->filename), "%s", filename);

    /* Add the audio and video streams using the default format codecs
    * and initialize the codecs. */
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        add_video_stream(&video_st, oc, AV_CODEC_ID_MPEG1VIDEO);//fmt->video_codec);
        have_video = 1;
    }

    if (have_video)
        open_video(oc, &video_st);
    av_dump_format(oc, 0, filename, 1);
    _outbuffer=(unsigned char*)av_malloc(32768);
    AVIOContext* ioCtx = avio_alloc_context(_outbuffer, 32768, 1, this, 0, IOWriteFunc, IOSeekFunc);
    oc->pb = ioCtx;
    oc->flags = AVFMT_FLAG_CUSTOM_IO;
    avformat_write_header(oc, NULL);
    return 0;
}

void Mpeg::close() {
    av_write_trailer(oc);
    if (have_video)
        close_stream(oc, &video_st);
    avformat_free_context(oc);
}