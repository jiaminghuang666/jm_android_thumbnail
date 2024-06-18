#include <jni.h>

// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("playermetadataretriever");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("playermetadataretriever")
//      }
//    }

#include "JMPlayerMetadataRetriever.h"
#include "ALOG.h"

JMPlayerMetadataRetriever *jmMediaRetriver  = JMPlayerMetadataRetriever::Get();

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_playermetadataretriever_PlaybackActivity_setDataSource(JNIEnv *env, jobject thiz,
                                                                        jstring url,
                                                                        jobject handle) {
    // TODO: implement setDataSource()
    //JMPlayerMetadataRetriever *jmMediaRetriver  = new JMPlayerMetadataRetriever();
    const char *myUil = env->GetStringUTFChars(url, 0);
    int ret  = -1;
    ret = jmMediaRetriver->setDataSource(myUil);
    if (ret) {
        ALOGE("Java_com_example_playermetadataretriever_VideoDetailsFragment_setDataSource setDataSource fail %s !!",myUil);
    }
    //delete jmMediaRetriver;

    env->ReleaseStringUTFChars(url,myUil);
    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_playermetadataretriever_PlaybackActivity_releaseSource(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseSource()
    if (jmMediaRetriver != nullptr)
        delete jmMediaRetriver;
    return;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_playermetadataretriever_PlaybackActivity_getMetadata(JNIEnv *env, jobject thiz) {
    // TODO: implement getMetadata()
    std::map<std::string, std::string> metadata = jmMediaRetriver->getMetadata();

    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jmethodID hashMapCtor = env->GetMethodID(hashMapClass, "<init>", "()V");
    jobject hashMap = env->NewObject(hashMapClass, hashMapCtor);

    jmethodID putMethod = env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    for (const auto &pair : metadata) {
        jstring key = env->NewStringUTF(pair.first.c_str());
        jstring value = env->NewStringUTF(pair.second.c_str());
        env->CallObjectMethod(hashMap, putMethod, key, value);
        env->DeleteLocalRef(key);
        env->DeleteLocalRef(value);
    }

    return hashMap;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_playermetadataretriever_PlaybackActivity_getKeyAlbumInfo(JNIEnv *env, jobject thiz,
                                                                          jstring name) {
    // TODO: implement getKeyAlbumInfo()
    ALOGD("Java_com_example_playermetadataretriever_PlaybackActivity_getKeyAlbumInfo");
    const char *jname = env->GetStringUTFChars(name, 0);
    char *value;
    int ret =  jmMediaRetriver->getKeyMeteData(jname,value);
    if (ret) {
        ALOGD("Java_com_example_playermetadataretriever_PlaybackActivity_getKeyAlbumInfo cant get key value");
        return nullptr;
    }

    return env->NewStringUTF(value);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_playermetadataretriever_PlaybackActivity_extractAlbumArt(JNIEnv *env,
                                                                          jobject thiz) {
    // TODO: implement extractAlbumArt()
    int data_size = 0;
    void *data ;
    ALOGD("Java_com_example_playermetadataretriever_PlaybackActivity_extractAlbumArt");
    jmMediaRetriver->extractAlbumArt(&data, &data_size);

    jbyteArray  byteArray = (*env).NewByteArray(data_size);
    (*env).SetByteArrayRegion(byteArray, 0, data_size,(jbyte*)(data));
    jclass cls = (*env).GetObjectClass(thiz);

    jmethodID mid = (*env).GetMethodID(cls, "onAlbumArtExtracted", "([B)V");
    (*env).CallVoidMethod(thiz, mid, byteArray);
    (*env).DeleteLocalRef(byteArray);

    return;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_playermetadataretriever_PlaybackActivity_getFrameAtTime(JNIEnv *env, jobject thiz,
                                                                         jlong time_us,
                                                                         jint option) {
    // TODO: implement getFrameAtTime()
    int ret = -1;
    int data_size = 0;
    void *data;

    ret = jmMediaRetriver->getFrameAtTime(time_us,option,0,0,&data, &data_size);

    jbyteArray  byteArray = (*env).NewByteArray(data_size);
    (*env).SetByteArrayRegion(byteArray, 0, data_size,(jbyte*)(data));
    jclass cls = (*env).GetObjectClass(thiz);

    jmethodID mid = (*env).GetMethodID(cls, "onAlbumArtExtracted", "([B)V");
    (*env).CallVoidMethod(thiz, mid, byteArray);
    (*env).DeleteLocalRef(byteArray);

    return ret;
}
