#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <android/log.h>

#ifndef kprintf
#define  LOG_TAG    "BitmapReader===="
#define kprintf(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#ifndef READ_START_INDEX
#define READ_START_INDEX 1
#endif

#ifndef RGBA_AB
#define RGBA_AB true
#define RGBA_B(p) (((p) & 0x00FF0000) >> 16)
#define VALUE(p) (((((p) & 0x000000FF))) + ((((p)&0x0000ff00)) + (((p) & 0x00FF0000))))
#endif


jstring readValueByKeyIndex(JNIEnv *env, void *pixels, jint keyindex, jint width, jint height);

jstring charToJstring(JNIEnv *env, const char *pat);

int *fetchValuePosInfoBuyKeyIndex(int32_t *tags, int width, int keyIndex);


int *fetchReadIndexTagPosInfo(int32_t *pixelsChar, int width, int height);

int *fetchValueReadTagPosInfo(int32_t *pixelsChar, int width, int height, int *resultInfo);

int *fetchValueTagsPosInfo(int32_t *pixelsChar, int *startLineIndex, int width, int height);

jstring fetchJString(JNIEnv *env, int32_t *pixelsChar, int startIndex, int endIndex, int width, int height);

int lastPost = -1;
int lastEndPos = -1;
bool isRead = false;

extern "C" JNIEXPORT jstring
JNICALL
Java_com_yjj_wsg_core_impl_BitmapReaderImpl_getValueByIndex(JNIEnv *env,
                                                                      jclass clss, jobject bitmap, jint keyIndex) {
    jstring resultValue = nullptr;
    AndroidBitmapInfo bitmapInfo;
    void *pixels = 0;
    kprintf("BitmapReader start \n");
    if (AndroidBitmap_getInfo(env, bitmap, &bitmapInfo) < 0) {
        kprintf("BitmapReader getInfo error\n");
        return resultValue;
    }
    if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        kprintf("BitmapReader getInfo error format must rgba 8888\n");
        return resultValue;
    }
    if (bitmapInfo.height < 0 || bitmapInfo.width < 0) {
        kprintf("bitmapinfo width or height is 0 \n");
        return resultValue;
    }
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
        kprintf("BitmapReader locakPixels error \n");
        return resultValue;
    }
    if (pixels == NULL) {
        kprintf("BitmapReader get pixels is null \n");
        return resultValue;
    }
    resultValue = readValueByKeyIndex(env, pixels, keyIndex, bitmapInfo.width, bitmapInfo.height);
    isRead = true;
    AndroidBitmap_unlockPixels(env, bitmap);
    return resultValue;

}

jstring readValueByKeyIndex(JNIEnv *env, void *pixels, jint keyIndex, jint width, jint height) {
    int32_t *pixelsChar = (int32_t *) pixels;
    if (lastPost >= 0 && lastEndPos >= 0) {
        int *valuePosInfo = fetchValuePosInfoBuyKeyIndex(pixelsChar, width, keyIndex);
        if (valuePosInfo == nullptr) {
            return nullptr;
        }
        return fetchJString(env, pixelsChar, valuePosInfo[0] * width + valuePosInfo[1],
                            valuePosInfo[2] * width + valuePosInfo[3], width, height);
    }

    if (isRead && lastPost == -1 && lastEndPos == -1) {
        return nullptr;
    }
    int *readPosResult = fetchReadIndexTagPosInfo(pixelsChar, width, height);
    if (readPosResult == nullptr) {
        return nullptr;
    }
    int *valueReadPosInfo = fetchValueReadTagPosInfo(pixelsChar, width, height, readPosResult);
    if (valueReadPosInfo == nullptr) {
        return nullptr;
    }
    int *valueTagsPosInfo = fetchValueTagsPosInfo(pixelsChar, valueReadPosInfo, width, height);
    if (valueTagsPosInfo == nullptr) {
        return nullptr;
    }
    if ((valueTagsPosInfo[2] * width + valueTagsPosInfo[3]) - (valueTagsPosInfo[0] * width + valueTagsPosInfo[1]) <=
        0) {
        return nullptr;
    }
    lastPost = width * valueTagsPosInfo[0] + valueTagsPosInfo[1];
    lastEndPos = valueTagsPosInfo[2] * width + valueTagsPosInfo[3];
    int *valuePosInfo = fetchValuePosInfoBuyKeyIndex(pixelsChar, width, keyIndex);
    if (valuePosInfo == nullptr) {
        return nullptr;
    }
    return fetchJString(env, pixelsChar, valuePosInfo[0] * width + valuePosInfo[1],
                        valuePosInfo[2] * width + valuePosInfo[3], width, height);

}

jstring charToJstring(JNIEnv *env, const char *pat) {
    jclass strClass = (env)->FindClass("java/lang/String");
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte *) pat);
    jstring encoding = (env)->NewStringUTF("GBK");
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

int *fetchValuePosInfoBuyKeyIndex(int32_t *pixe, int width, int keyIndex) {
    int target[4];
    int32_t *start = pixe + lastPost;
    char tag = '&';
    int count = lastEndPos - lastPost;
    int tagCount = 0;
    for (int i = 0; i < count; i++) {
        if (RGBA_B(start[i]) == tag) {
            tagCount++;
        }
    }
    if ((tagCount + 1) % 5 != 0) {
        return nullptr;
    }
    int groupCount = (tagCount + 1) / 5;
    for (int i = 0; i < groupCount; i++) {
        int pos = i * 10;
        if (VALUE(start[pos]) == keyIndex) {
            target[0] = VALUE(start[pos + 2]);
            target[1] = VALUE(start[pos + 4]);
            target[2] = VALUE(start[pos + 6]);
            target[3] = VALUE(start[pos + 8]);
            break;
        }
    }
    if ((target[0] * width + target[1]) >= target[2] * width + target[3]) {
        return nullptr;
    }
    return target;
}

/**
 * 读取读标记位置信息，返回四元数组信息
 * 这里从第20行开始查找数据，第100列开始识
 * **/
int *fetchReadIndexTagPosInfo(int32_t *pixelsChar, int width, int height) {
    int startReadRow = -1;
    int startReadColumn = -1;
    char tag1 = '#';
    char tag2 = '%';
    int32_t *startChar;
    for (int line = height/3; line < height; line++) {
        for (int column = 0; column < width - 2; column++) {
            startChar = pixelsChar + line * width + column;
            if (tag1 == RGBA_B(startChar[0]) && tag2 == RGBA_B(startChar[1])) {
                startReadColumn = column + 2;
                startReadRow = line;
                break;
            }
        }
        if (startReadRow >= 0) {
            break;
        }
    }
    if (startReadColumn == width - 1) {
        return nullptr;
    }
    if (startReadRow == height) {
        return nullptr;
    }
    if (startReadColumn < 0) {
        return nullptr;
    }
    if (startReadRow < 0) {
        return nullptr;
    }

    int result[2];
    result[0] = startReadRow;
    result[1] = startReadColumn;
    return result;
}

/**
 * 返回读标记索引位置
 * */
int *
fetchValueReadTagPosInfo(int32_t *pixelsChar, int width, int height, int *resultInfo) {
    int32_t *desc = pixelsChar + resultInfo[0] * width + resultInfo[1];
    int result[2];
    result[0] = VALUE(desc[0]);
    result[1] = VALUE(desc[2]);
    if (result[0] * width + result[1] >= (height * width - 1)) {
        return nullptr;
    }
    return result;
}

/**
 * 获取valuetags info,返回四元数组
 * */
int *fetchValueTagsPosInfo(int32_t *pixelsChar, int *startLineIndex, int width, int height) {
    int endTagLineIndex = -1;
    int endTagColumnIndex = -1;
    char tag1 = '#';
    char tag2 = '&';
    int32_t *start = pixelsChar + startLineIndex[0] * width + startLineIndex[1];
    if (RGBA_B(start[0]) != tag1) {
        return nullptr;
    }
    if (RGBA_B(start[1]) != tag2) {
        return nullptr;
    }

    for (int line = startLineIndex[0]; line < height; line++) {
        for (int column = 0; column < width; column++) {
            start = pixelsChar + line * width + column;
            if (tag2 == RGBA_B(*start) && tag1 == RGBA_B(*(start + 1))) {
                endTagColumnIndex = column - 1;
                endTagLineIndex = line;
                break;
            }
        }
        if (endTagLineIndex >= 0) {
            break;
        }
    }
    if (endTagColumnIndex >= width) {
        return nullptr;
    }
    if (endTagColumnIndex < 0) {
        return nullptr;
    }
    if (endTagLineIndex < 0) {
        return nullptr;
    }
    if (endTagLineIndex >= height) {
        return nullptr;
    }
    int result[4];
    result[0] = startLineIndex[0];
    result[1] = startLineIndex[1] + 2;
    result[2] = endTagLineIndex;
    result[3] = endTagColumnIndex;
    return result;
}

jstring fetchJString(JNIEnv *env, int32_t *pixelsChar, int startIndex, int endIndex, int width, int height) {

    if (startIndex >= width * height) {
        return nullptr;
    }
    int count = endIndex - startIndex + 1;
    if (count <= 0) {
        return nullptr;
    }
    int32_t *start = pixelsChar + startIndex;
    char target[count + 1];
    int value;
    for (int i = 0; i < count; i++) {
        value = RGBA_B(start[i]);
        target[i] = value;
    }
    target[count] = '\0';
    jstring result = charToJstring(env, target);
    return result;
}
