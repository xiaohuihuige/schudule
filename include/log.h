#ifndef __LOG_H__
#define __LOG_H__
#include <stdio.h>

#define RED_COLOR "\033[0;31m"
#define RED_COLOR1 "\033[0;32m"
#define RED_COLOR2 "\033[0;33m"
#define RED_COLOR3 "\033[0;37m"

#define ERR(fmt, args...) \
        printf("%s%-20s%-30s[%d]: "fmt"\n\033[0;39m",RED_COLOR,  strrchr(__FILE__, '/') + 1, __FUNCTION__,__LINE__,##args);

#define WAR(fmt, args...) \
        printf("%s%-20s%-30s[%d]: "fmt"\n\033[0;39m",RED_COLOR1, strrchr(__FILE__, '/') + 1,__FUNCTION__,__LINE__,##args);

#define LOG(fmt, args...) \
        printf("%s%-20s%-30s[%d]: "fmt"\n\033[0;39m",RED_COLOR2, strrchr(__FILE__, '/') + 1,__FUNCTION__,__LINE__,##args);

#define DBG(fmt, args...) \
        printf("%s%-20s%-30s[%d]: "fmt"\n\033[0;39m",RED_COLOR3, strrchr(__FILE__, '/') + 1,__FUNCTION__,__LINE__,##args);

#endif // !__LOG_H__
