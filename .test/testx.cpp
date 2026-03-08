#include <jni.h>
#include "Testx.h"
JNIEXPORT jint JNICALL Java_Testx_add(JNIEnv *env, jobject obj, jint a,jint b){
    return a+b;
}