#include <jni.h>
#include <filesystem>
#include <iostream>
#include "main.cpp"

//#include "include/Rtklib/rtklib_fun.h"
#include "include/IO_rtcm.h"
#include <android/log.h>
#define LOG_TAG "YourAppTag"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
using namespace KPL_IO;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fortest_JniHelper_KPL_1initialize(JNIEnv *env, jclass clazz, jstring mode,
                                                   jobject station_parameters, jdouble cut,
                                                   jdouble intv) {
//  TODO: implement KPL_initialize();
    std::cout << "asd" << std::endl;
//    main(1, {});

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fortest_JniHelper_SDK_1init(JNIEnv *env, jclass clazz) {
    // TODO: implement SDK_init()
    double pos[3] = {-2258208.214700, 5020578.919700, 3210256.397500}, enu[3] = {0};
    const char *a = "asd";
    std::__fs::filesystem::path currentPath = std::__fs::filesystem::current_path();
    LOGD("Hello from JNI!%s", currentPath.c_str());

//    SDK_init("kinematic", "", pos, enu, 7, 1.0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fortest_JniHelper_sendLoadStorage(JNIEnv *env, jclass clazz, jstring target_path) {
    const char *path = env->GetStringUTFChars(target_path, nullptr);
    LOGD("asdasd%s", path);
    const char *c_path = "test:test@119.96.165.202:8600/TEST";
    unsigned char buff[1024] = {0};
    double pos[3] = {-2258208.214700, 5020578.919700, 3210256.397500}, enu[3] = {0};
    SDK_init("kinematic", "", pos, enu, 7, 1.0 ,path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fortest_JniHelper_sendEphData(JNIEnv *env, jclass clazz, jbyteArray jByteArray) {
    // 获取字节数组的长度
    jsize len = (env)->GetArrayLength(jByteArray);

    // 获取字节数组的元素
    jbyte *bytes = (env)->GetByteArrayElements( jByteArray, 0);

    // 创建一个普通的字节数组并拷贝数据
    // 这里使用了 C 语言中的动态内存分配，你可以根据需要使用其他方式
    unsigned char *nativeByteArray = (unsigned char *)malloc(len);
    memcpy(nativeByteArray, bytes, len);

    // 释放字节数组的元素
//    (env)->ReleaseByteArrayElements(jByteArray, bytes, 0);

    // 在这里你可以使用 nativeByteArray，然后记得在使用后释放内存
    // ...

    for (int i = 0; i < len; ++i)
    {
        IO_inputEphData(nativeByteArray[i]);
    }


    // 释放 nativeByteArray 的内存
    free(nativeByteArray);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_fortest_JniHelper_sendSsrData(JNIEnv *env, jclass clazz, jbyteArray jByteArray) {
    // TODO: implement sendSsrData()
    // 获取字节数组的长度
    jsize len = (env)->GetArrayLength(jByteArray);

    // 获取字节数组的元素
    jbyte *bytes = (env)->GetByteArrayElements( jByteArray, 0);

    // 创建一个普通的字节数组并拷贝数据
    // 这里使用了 C 语言中的动态内存分配，你可以根据需要使用其他方式
    unsigned char *nativeByteArray = (unsigned char *)malloc(len);
    memcpy(nativeByteArray, bytes, len);

    // 释放字节数组的元素
//    (env)->ReleaseByteArrayElements(jByteArray, bytes, 0);

    // 在这里你可以使用 nativeByteArray，然后记得在使用后释放内存
    // ...

    for (int i = 0; i < len; ++i)
    {
        IO_inputSsrData(nativeByteArray[i]);
    }
    // 释放 nativeByteArray 的内存
    free(nativeByteArray);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_fortest_SDK_sendObsData(JNIEnv *env, jclass clazz, jbyteArray jByteArray) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_fortest_JniHelper_sendObsData(JNIEnv *env, jclass clazz, jbyteArray jByteArray) {
    // TODO: implement sendObsData()
    // 获取字节数组的长度
    jsize len = (env)->GetArrayLength(jByteArray);

    // 获取字节数组的元素
    jbyte *bytes = (env)->GetByteArrayElements(jByteArray, 0);

    unsigned char *nativeByteArray = (unsigned char *) malloc(len);
    memcpy(nativeByteArray, bytes, len);

    // 释放字节数组的元素
    (env)->ReleaseByteArrayElements(jByteArray, bytes, 0);

    // 在这里你可以使用 nativeByteArray，然后记得在使用后释放内存

    char buff_r[1024] = {0};
    for (int i = 0; i < len; ++i) {
        if (1 == IO_inputObsData(nativeByteArray[i])) {
//            SDK_retrieve("NMEA_GGA", buff_r, 104);
//            LOGD("%s", );
            char buff[1024];
            SDK_retrieve("obsinfo", buff,1);
            LOGD("%s\n", buff_r);
            LOGD("----%s", buff);
        }
    }
    // 释放 nativeByteArray 的内存
    free(nativeByteArray);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_fortest_JniHelper_SDKRetrieve(JNIEnv *env, jclass clazz, jstring jtype, jint len) {
    // TODO: implement SDKRetrieve()
    const char * type = (env)->GetStringUTFChars(jtype, nullptr);
    char buff[1024000];
    SDK_retrieve(type, buff, len);
    env->ReleaseStringUTFChars(jtype,type);
    return (*env).NewStringUTF( buff);
}