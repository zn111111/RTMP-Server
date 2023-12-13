#include "log.h"

std::string curr_time()
{
    time_t t_ms = time(NULL);
    struct tm *t_tm = localtime(&t_ms);
    char buffer[30] = {0};
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", 1900 + t_tm->tm_year, 1 + t_tm->tm_mon, t_tm->tm_mday, t_tm->tm_hour,
    t_tm->tm_min, t_tm->tm_sec);
    std::string temp(buffer);
    return temp;
}