#ifndef SANDBOX_LOG_H_
#define SANDBOX_LOG_H_

#include "config.h"

#ifdef SANDBOX_LOGGING_LOGCAT
#include <android/log.h>
#define  _LOG_TAG    "APPBOX"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, _LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, _LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...) fprintf(stderr, __VA_ARGS__)
#define LOGE(...) fprintf(stderr, __VA_ARGS__)
#endif

#endif /* SANDBOX_LOG_H_ */

