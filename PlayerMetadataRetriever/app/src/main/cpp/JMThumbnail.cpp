//
// Created by jiaming.huang on 2024/5/28.
//

#include "JMThumbnail.h"
#include "ALOG.h"

const unsigned char jpeg[3] = {0xFF, 0xD8, 0xFF};
const unsigned char png[4] = {0x89,0x50,0x4E,0x47};
const unsigned char gif[3] = {0x47, 0x49,0x46};
const unsigned char bmp[2] = {0x42, 0x4D};

static int debugEnable = true;

static int dumpYuvData(AVFrame *dst_frame) {
    FILE *file = fopen("/data/output.yuv","wb");
    if (!file) {
        ALOGE(" JMThumbnail::dumpYuvData debug yuv metadta create fail!!");
        return -1;
    } else {
        fwrite(dst_frame->data[0], 1, dst_frame->linesize[0]*dst_frame->height,file); // y 数据
        fwrite(dst_frame->data[1], 1, dst_frame->linesize[1]*dst_frame->height / 2,file); //u 数据
        fwrite(dst_frame->data[2], 1, dst_frame->linesize[2]*dst_frame->height / 2,file); // v数据
        fclose(file);
    }
    return 0;
}

static int dumpCoverPic( AVPacket pic_pkt)
{
   FILE *file = fopen("/data/coverpic", "wb");
   if(!file) {
       ALOGE(" JMThumbnail::dumpCoverPic  create fail!!");
       return -1;
   } else {
       fwrite(pic_pkt.data, 1, pic_pkt.size, file);
       fclose(file);
   }

    return 0;
}

static double r2d(AVRational r)
{
    return r.num == 0 || r.den ==0 ? 0:(double)r.num/(double)r.den;
}

JMThumbnail::JMThumbnail()
{
    ALOGD("JMThumbnail::JMThumbnail");
    av_register_all();
    avformat_network_init();
    avcodec_register_all();
}

JMThumbnail::~JMThumbnail()
{
    ALOGD("JMThumbnail::~JMThumbnail");
}

int JMThumbnail::getKeyMetaData(const char* key,  char **value)
{
    ALOGD("JMThumbnail::getKeyMetaData key = %s \n",key);
    AVDictionaryEntry *tag = NULL;
    tag = av_dict_get(fmt_ctx->metadata, key, tag, AV_DICT_IGNORE_SUFFIX);
    if (tag) {
        *value = tag->value;
        ALOGD("JMThumbnail::getKeyMetaData key: %s value: %s \n",tag->key ,tag->value);
        return 0;
    }

    ALOGD("JMThumbnail::getKeyMetaData none key tag \n");
    return -1;
}

std::map<std::string, std::string> JMThumbnail::getMetadata()
{
    std::map<std::string, std::string> metadata;
    AVDictionaryEntry *tag = NULL;
    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        ALOGD("%s : %s",tag->key, tag->value);
        metadata[tag->key] = tag->value;
    }

    auto it = metadata.end();
    metadata.insert(it, std::make_pair("only_audio",std::to_string(only_audio)));
    metadata.insert(it, std::make_pair("outWidth",std::to_string(outWidth)));
    metadata.insert(it, std::make_pair("outHeight",std::to_string(outHeight)));
    metadata.insert(it, std::make_pair("mChannels",std::to_string(mChannels)));
    metadata.insert(it, std::make_pair("mBitRate",std::to_string(mBitRate)));


    return metadata;
}


int JMThumbnail::getAudioInfo()
{
  if (audio_stream_index != -1) {
      mChannels = fmt_ctx->streams[audio_stream_index]->codec->channels;
      mBitRate = fmt_ctx->streams[audio_stream_index]->codec->bit_rate;
      ALOGD("JMThumbnail::getAudioInfo Additional audio stream information can be added ...... !! \n");
  }else {
      ALOGD("JMThumbnail::getAudioInfo it is not audio stream !! \n");
  }

    return 0;
}


const char * detectImageFormat(const unsigned char *data)
{
    if (data[0] == jpeg[0] && data[1] == jpeg[1] &&
        data[2] == jpeg[2] ) {
        return "jpeg";
    } else if (data[0] == png[0] && data[1] == png[1] &&
               data[2] == png[2] && data[3] == png[3]) {
        return "png";
    } else if (data[0] == gif[0] && data[1] == gif[1] &&
               data[2] == gif[2] ) {
        return "gif";
    } else if (data[0] == bmp[0] && data[1] == bmp[1] ) {
        return "bmp";
    } else {
        return "NULL";
    }
}

int JMThumbnail::getAlbumCover(void **data, int *data_size)
{
    ALOGD("JMThumbnail::getAlbumCover AV_DISPOSITION_ATTACHED_PIC 0 \n");
    for (int i = 0 ;i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            ALOGD("JMThumbnail::getAlbumCover AV_DISPOSITION_ATTACHED_PIC \n");
           if (fmt_ctx->streams[i]->attached_pic.size > 0) {
               ALOGD("JMThumbnail::getAlbumCover size :%d \n",fmt_ctx->streams[i]->attached_pic.size);
               AVPacket pic_pkt;
               av_init_packet(&pic_pkt);
               pic_pkt.data = fmt_ctx->streams[i]->attached_pic.data;
               pic_pkt.size = fmt_ctx->streams[i]->attached_pic.size;
               *data = (void *)fmt_ctx->streams[i]->attached_pic.data;
               *data_size = fmt_ctx->streams[i]->attached_pic.size;
               if (debugEnable) {
                   const char *temp = detectImageFormat((const unsigned char *)pic_pkt.data);
                   ALOGD("JMThumbnail::getAlbumCover format :%s \n", temp);
                   dumpCoverPic(pic_pkt);
               }
               break;
           }
        }
    }
    ALOGD("JMThumbnail::getAlbumCover AV_DISPOSITION_ATTACHED_PIC 1 \n");
    return NULL;
}


int JMThumbnail::unsupportThumbnail()
{
    int ret = 0;

    if (mWidth * mHeight > 4096 * 2304) {
        ALOGE("JMThumbnail::Open  unsupport 4K Thumbnail \n");
        return -1;
    }

    if (mWidth == 0 || mHeight ==0) {
        ALOGE("JMThumbnail::Open mWidth == 0 || mHeight ==0 \n");
        return -1;
    }

    return ret;
}


int JMThumbnail::findKeyFrame(int timeUs,void **data, int *data_size)
{
    //读取第一帧
    int ret = -1;
    char *rgb = new char[1920*1080*4];
    int64_t timestamp = 0;
    int duration = fmt_ctx->duration / AV_TIME_BASE;
    //int dstDataSize =0;

    packet = av_packet_alloc();
    frame = av_frame_alloc();  // 分配帧

    if (!frame) {
        ALOGE(" JMThumbnail::findKeyFrame av_frame_alloc fail ret = %d ", ret);
        goto err0;
    }

    dst_frame = av_frame_alloc(); //最后存放的数据
    if (!dst_frame) {
        ALOGE(" JMThumbnail::findKeyFrame dst_frame fail !!");
        goto err0;
    }

    if (timeUs >= 0) {
        timestamp = timeUs;
        if (fmt_ctx->start_time != (int64_t)AV_NOPTS_VALUE) {
            timestamp += fmt_ctx->start_time;
        }
    } else {
        timestamp = duration / 3;
        timestamp = timestamp * AV_TIME_BASE + fmt_ctx->start_time;
    }

    ALOGD("JMThumbnail::findKeyFrame  timestamp= %lld  time= %d fmt_ctx->start_time=%lld duration=%d ", timestamp, timeUs,fmt_ctx->start_time, duration);

    timestamp = av_rescale(timestamp, fmt_ctx->streams[video_stream_index]->time_base.den, AV_TIME_BASE * (int64_t)fmt_ctx->streams[video_stream_index]->time_base.num);
    ALOGD("JMThumbnail::findKeyFrame  timestamp= %lld   ", timestamp);

    //往后跳转到关键帧
    if (av_seek_frame(fmt_ctx, video_stream_index, timestamp,AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_FRAME) < 0) {
        ALOGE(" JMThumbnail::findKeyFrame seek fail " );
    }

    for (;;) {
        ret = av_read_frame(fmt_ctx, packet);
        ALOGI("JMThumbnail::findKeyFrame av_read_frame");
        if (ret != 0) {
            ALOGI("JMThumbnail::findKeyFrame seek to random packet");
            int pos = 20 * r2d(fmt_ctx->streams[video_stream_index]->time_base);
            av_seek_frame(fmt_ctx, video_stream_index, pos,AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_FRAME);
            continue;
        }

        if (packet->stream_index == video_stream_index) {
            ret = avcodec_send_packet(VideoCodecCtx, packet);
            av_packet_unref(packet);
            ALOGI("JMThumbnail::findKeyFrame avcodec_send_packet ");
            for (;;) {
               ret = avcodec_receive_frame(VideoCodecCtx, frame);
               if (ret != 0) {
                   ALOGE(" JMThumbnail::findKeyFrame avcodec_receive_frame fail \n");
                   break;
               }
               ALOGI("JMThumbnail::findKeyFrame avcodec_receive_frame frame format :%d !!!! ",frame->format);
                videoSwsCtx = sws_getContext(frame->width,
                                                  frame->height,
                                                  (AVPixelFormat)frame->format,
                                                  outWidth,
                                                  outHeight,
                                                  AV_PIX_FMT_YUV420P,
                                                  SWS_BILINEAR,
                                                  NULL,NULL,NULL);
                if (!videoSwsCtx) {
                    ALOGE(" JMThumbnail::findKeyFrame videoSwsCtx fail !!");
                } else {
                   ALOGD(" JMThumbnail::findKeyFrame sws_scale frame width=%d  height=%d  \n",frame->width,frame->height);
                   dst_frame->format = AV_PIX_FMT_YUV420P;
                   dst_frame->width = outWidth;
                   dst_frame->height = outHeight;

                   ret = av_frame_get_buffer(dst_frame, 32);
                   if (ret < 0) {
                       ALOGE(" JMThumbnail::findKeyFrame av_frame_get_buffer fail !!");
                       goto err0;
                   }
                   int h = sws_scale(videoSwsCtx,
                           (const uint8_t **)frame->data,
                           frame->linesize,
                           0,
                           frame->height,
                           dst_frame->data,
                           dst_frame->linesize
                           );
                    //dstDataSize = av_image_get_buffer_size(dst_frame->format, dst_frame->width, dst_frame->height, 1);
                    *data_size = (dst_frame->linesize[0]* dst_frame->height) + 2 * (dst_frame->linesize[1]* dst_frame->height / 2);
                    *data = (void *)dst_frame->data;
                   ALOGD(" JMThumbnail::findKeyFrame sws_scale h =%d  data_size=%d dst_frame->linesize=%d\n", h,*data_size,dst_frame->linesize[0]);
                   if (debugEnable) {
                       dumpYuvData(dst_frame);
                   }

                   goto ok;
               }
            }

        }
    }

ok:
    ALOGI("JMThumbnail::findKeyFrame ok !!!! ");
    sws_freeContext(videoSwsCtx);
    av_packet_free(&packet);
    av_frame_free(&frame);
    av_frame_free(&dst_frame);

    return 0;

err0:
    av_packet_free(&packet);
    av_frame_free(&dst_frame);
    av_frame_free(&frame);

    return -1;
}



int JMThumbnail::isMusic(const char * url)
{
    int ret  = 0;
    int score_max = 0;
    uint8_t buf[MAX_PROBE_SIZE];
    FILE *file = fopen(url,"rb");
    if (!file) {
        ALOGE("FFThumbnail::Open fopen %s fail ", url);
        return DTERROR;
    }

    fseek(file, 0, SEEK_SET);
    size_t file_size = fread(buf, 1, sizeof (buf), file);

    probe_data.filename = url;
    probe_data.buf = buf;
    probe_data.buf_size = (unsigned int)file_size;
    probe_data.mime_type = NULL;

    avInputFormat = av_probe_input_format2(&probe_data, 1, &score_max);
    if (ret < 0){
        ALOGE("FFThumbnail::Open av_probe_input_format2 fail ");
        fclose(file);
        return DTERROR;
    }
    ALOGD("FFThumbnail::Open av_probe_input_format2 %s ",avInputFormat->name);
    if (!avInputFormat) {
        ALOGE("FFThumbnail::Open avInputFormat fail ");
        fclose(file);
        return DTERROR;
    }

    fclose(file);

    static const size_t numMusicFormat = sizeof(musicFormat) / sizeof(musicFormat[0]);
    for (int i = 0; i < numMusicFormat; i++) {
      if ( 0 == strcmp (avInputFormat->name, musicFormat[i])) {
          ALOGD("JMThumbnail::isMusic input file is: %s ", musicFormat[i]);
          ALOGD("JMThumbnail::isMusic detect input file avInputFormat name: %s ", avInputFormat->name);
          return DTAUDIO;
      }
    }

    return DTVIDEO;
}

int JMThumbnail::Open(const char *url)
{
    int ret = 0 ;

    ret = avformat_open_input(&fmt_ctx, url, 0, 0);  // 打开输入文件
    if (ret != 0) {
        ALOGE("FFThumbnail::Open avformat_open_input fail ret = %d url =%s \n", ret, url);
        return -1;
        //goto err0;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);  // 获取流信息
    if (ret != 0) {
        ALOGE("FFThumbnail::Open avformat_find_stream_info fail ret = %d ", ret);
        goto err0;
        //return -1 ;
    }

    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            has_video = true;
            video_stream_index = i;
        }else if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            has_audio = true;
            audio_stream_index = i;
        }else { //subtile or nothing
            has_audio = false;
            has_video = false;
        }
    }

    only_audio = isMusic(url);  //Special MP3 file have audio and video track flags
    if (only_audio == DTAUDIO) {
        ALOGD("JMThumbnail::isMusic ffmpeg Detect packaging format %d ", only_audio);
    }

    ALOGD("FFThumbnail::Open has_video %d has_audio %d only_audio=%d \n", has_video,has_audio,only_audio);
    if ((only_audio == DTVIDEO ) && has_video) {  // has video stream
        ALOGD("FFThumbnail::Open file maybe is video format !! ");
        video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL,0);
        if (video_stream_index) {
            ALOGE("FFThumbnail::Open av_find_best_stream fail ret = %d ", video_stream_index);
            goto err0;
        }

        videoPara = fmt_ctx->streams[video_stream_index]->codecpar;

        codec = avcodec_find_decoder(videoPara->codec_id);  // 找到解码器并打开
        if (!codec) {
            ALOGE("FFThumbnail::Open avcodec_find_decoder NULL codec ");
            goto err0;
        }

        //初始化解码器
        VideoCodecCtx  = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(VideoCodecCtx, videoPara);

        ret = avcodec_open2(VideoCodecCtx, 0, 0);
        if (ret) {
            ALOGE("FFThumbnail::Open avcodec_open2 fail ret = %d ", ret);
            goto err0;
        }

        mWidth = VideoCodecCtx->width;
        mHeight = VideoCodecCtx->height;
        ALOGI("JMThumbnail::Open  width:%d height:%d \n",mWidth, mHeight);
        if (unsupportThumbnail()) {
            goto err0;
        }

        return 0;

    } else if ((only_audio == DTAUDIO ) && has_audio) { //only audio stream
        ALOGD("FFThumbnail::Open file maybe is audio format !! ");
        getAudioInfo();
        return 0;
    }else {
        ALOGD("FFThumbnail::Open file maybe is subtitle !! ");
    }

err1:
    avcodec_close(VideoCodecCtx);
err0:
    avformat_close_input(&fmt_ctx);

    return -1;
}

void JMThumbnail::Close()
{
//    av_frame_free(Scale_frame);
    ALOGD("JMThumbnail::Close !! ");
    av_frame_free(&frame);
    avformat_close_input(&fmt_ctx);
}



