#include <ctime>
#include <iostream>

#define DBG_LEVEL 0
#define INF_LEVEL 1
#define ERR_LEVEL 2
#define DEFAULT_LEVEL DBG_LEVEL

#define LOG(lev_str, level, format, ...)                                                             \
  {                                                                                                  \
    if (level >= DEFAULT_LEVEL)                                                                      \
    {                                                                                                \
      time_t tmp = time(nullptr);                                                                    \
      struct tm *time = localtime(&tmp);                                                             \
      char str_time[32];                                                                             \
      size_t ret = strftime(str_time, 31, "%D %H:%M:%S", time);                                      \
      printf("[%s][%s][%s:%d]\t" format "\n", lev_str, str_time, __FILE__, __LINE__, ##__VA_ARGS__); \
    }                                                                                                \
  }

#define DLOG(format, ...) LOG("DBG", DBG_LEVEL, format, ##__VA_ARGS__)
#define ILOG(format, ...) LOG("INF", INF_LEVEL, format, ##__VA_ARGS__)
#define ELOG(format, ...) LOG("ERR", ERR_LEVEL, format, ##__VA_ARGS__)

int main()
{
  DLOG("Hello World %s ", "abcd");
  ILOG("Hello World %s");
  ELOG("Hello World %s ");
  return 0;
}