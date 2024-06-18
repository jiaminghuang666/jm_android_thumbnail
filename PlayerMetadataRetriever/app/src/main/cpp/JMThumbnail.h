//
// Created by jiaming.huang on 2024/5/28.
//

#ifndef PLAYERMETADATARETRIEVER_JMTHUMBNAIL_H
#define PLAYERMETADATARETRIEVER_JMTHUMBNAIL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>

};

#include <stdio.h>
#include <string>
#include <map>
using namespace std;

struct AVFormatContext ;
struct AVCodecContext ;
struct AVCodecParameters;
struct AVFrame ;
struct AVPacket;
struct AVCodec;
struct SwsContext;

struct AVDictionaryEntry;

struct AVInputFormat;
struct AVProbeData;

class JMThumbnail {

public:
    JMThumbnail();
    virtual ~JMThumbnail();
    virtual int Open(const char *url);
    virtual void Close();
    virtual std::map<std::string, std::string> getMetadata();
    virtual int getKeyMetaData(const char* key,  char **value);

    virtual int getAlbumCover(void **data, int *data_size);
    virtual int findKeyFrame(int timeUs,void **data, int *data_size);


private:
    virtual int isMusic(const char * url);
    virtual int unsupportThumbnail();
    virtual int getAudioInfo();

    AVFormatContext *fmt_ctx = 0;
    AVCodecContext *VideoCodecCtx = 0;
    AVCodecParameters *videoPara = 0;
    AVFrame *frame = 0;
    SwsContext *videoSwsCtx;
    AVFrame *Scale_frame = 0;
    AVPacket *packet = 0;
    AVCodec *codec = 0;
    bool has_audio = false;
    bool has_video = false;
    int only_audio = 0;

    int video_stream_index = -1;
    int audio_stream_index = -1;

    int mWidth = 0;
    int mHeight = 0;

    AVFrame *dst_frame = 0;

    int outWidth = 320;
    int outHeight = 240;

    AVProbeData probe_data = {0};
    const AVInputFormat *avInputFormat = 0;

    int mChannels = -1;
    int mBitRate = -1;
};

#define  MAX_PROBE_SIZE 4096
enum FileFormat {
    DTERROR = -1,
    DTAUDIO = 1,
    DTVIDEO = 2,
};

static const char *musicFormat[] = {
        "mp3",
        "flac",
        "aac",
        "alac",
        "wav",
        "aiff",
        "ogg",
        "m4a",
        "wma",
        "dsd",
        "ape",
        "mpc",
        "ac3",
        "dts",
        "vorbis",
};


#endif //PLAYERMETADATARETRIEVER_JMTHUMBNAIL_H
