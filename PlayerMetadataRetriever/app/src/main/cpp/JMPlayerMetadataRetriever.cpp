//
// Created by jiaming.huang on 2024/5/28.
//

#include "JMPlayerMetadataRetriever.h"
#include "ALOG.h"


JMPlayerMetadataRetriever::JMPlayerMetadataRetriever(){
    ALOGD("JMPlayerMetadataRetriever::JMPlayerMetadataRetriever \n");
    Client =  new JMThumbnail();
    if (!Client) {
        ALOGE("JMPlayerMetadataRetriever::JMPlayerMetadataRetriever Create Client fail \n");
    }
}

JMPlayerMetadataRetriever::~JMPlayerMetadataRetriever(){
    ALOGD("JMPlayerMetadataRetriever::~JMPlayerMetadataRetriever \n");
    Client->Close();
    if (Client)
        delete Client;
}

 int JMPlayerMetadataRetriever::setDataSource(const char *Url)
{
    ALOGD("JMPlayerMetadataRetriever::setDataSource \n");
    int ret = -1;

    ret = Client->Open(Url);

    return ret;
}

std::map<std::string, std::string> JMPlayerMetadataRetriever::getMetadata()
{
    return Client->getMetadata();
}

int JMPlayerMetadataRetriever:: getKeyMeteData(const char  *name,char *value)
{
    return Client->getKeyMetaData(name, &value);
}

const char * JMPlayerMetadataRetriever:: extractAlbumArt(void **data, int *data_size)
{
    ALOGD("JMPlayerMetadataRetriever:: extractAlbumArt \n");
    Client->getAlbumCover(data, data_size);
    return NULL;
}

int JMPlayerMetadataRetriever::getFrameAtTime(int64_t timeUs, int option, int colorFormat,bool metaOnly,void **data, int *data_size)
{
    int ret = -1;
    ret = Client->findKeyFrame(timeUs, data, data_size);

    return ret;
}