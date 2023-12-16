#ifndef __LOG_H__
#define __LOG_H__

#include <ctime>
#include <cstdio>
#include <string>

extern std::string curr_time();

#define LOG_INFO(msg, ...) printf("[%s][%s][%s][line:%d][info]", curr_time().c_str(), __FILE__, __FUNCTION__, __LINE__);\
                            printf(msg, ##__VA_ARGS__); printf("\n")
#define LOG_ERROR(msg, ...) printf("[%s][%s][%s][line:%d][error]", curr_time().c_str(), __FILE__, __FUNCTION__, __LINE__);\
                            printf(msg, ##__VA_ARGS__); printf("\n")

#endif