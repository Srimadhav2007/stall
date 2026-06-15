#include "./Simulator/Core.hpp"
#include <jni.h>
#include "com_srimadhav_Simulator.h"
Core core;


JNIEXPORT void JNICALL Java_com_srimadhav_Simulator_run(JNIEnv *env,jclass cls, jstring program,jboolean isTrace){
    try{
        const char* temp = env->GetStringUTFChars(program,nullptr);
        string prog(temp);
        env->ReleaseStringUTFChars(program,temp);
        if(isTrace)core.runFromTrace(prog);
        else core.execute(prog);
    }
    catch(const exception& e){
        jclass ex=env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(ex,e.what());
    }
}