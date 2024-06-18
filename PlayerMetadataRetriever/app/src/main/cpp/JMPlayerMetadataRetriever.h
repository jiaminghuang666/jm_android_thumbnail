//
// Created by jiaming.huang on 2024/5/28.
//

#ifndef PLAYERMETADATARETRIEVER_JMPLAYERMETADATARETRIEVER_H
#define PLAYERMETADATARETRIEVER_JMPLAYERMETADATARETRIEVER_H

#include "JMThumbnail.h"
#include <map>

class JMPlayerMetadataRetriever {
public:
    static JMPlayerMetadataRetriever* Get()
    {
        static JMPlayerMetadataRetriever px;
        return &px;
    }


    JMPlayerMetadataRetriever();
    virtual ~JMPlayerMetadataRetriever();

    virtual int setDataSource(const char *Url);
    virtual std::map<std::string, std::string> getMetadata();
    virtual int getKeyMeteData(const char  *name,char *value);

    virtual const char *extractAlbumArt(void **data, int *data_size);
    virtual int getFrameAtTime(int64_t timeUs, int option, int colorFormat,bool metaOnly,void **data, int *data_size);



private:
    JMThumbnail *Client;

};




#endif //PLAYERMETADATARETRIEVER_JMPLAYERMETADATARETRIEVER_H
