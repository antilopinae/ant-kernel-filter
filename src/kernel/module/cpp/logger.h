/*
* Linux kernel logger for C and C++
*/

#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif
void kern_log(const char *fmt, ...);
#ifdef __cplusplus
}
#endif

#endif
