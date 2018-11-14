#include "rtc.h"

#include "api/string.h"
#include "api/print.h"

static char ts_hour[4] = {0};
static char ts_min[4] = {0};
static char ts_sec[4] = {0};

static char ts_complete[10] = { 0 };

uint32_t get_hours(uint32_t secs)
{
    return (secs / 3600);
}

uint32_t get_minutes(uint32_t secs)
{
    return ((secs / 60) % 60);
}

uint32_t get_seconds(uint32_t secs)
{
    return (secs % 60);
}

char *get_hours_string(uint32_t hours)
{
   if (hours < 10) {
       ts_hour[0] = '0';
       sprintf(&(ts_hour[1]), 3, "%d", hours);
       return ts_hour;
   }
   sprintf(ts_hour, 4, "%d", hours);
   return ts_hour;
}

char *get_minutes_string(uint32_t mins)
{
    if (mins < 10) {
        ts_min[0] = '0';
        sprintf(&(ts_min[1]), 3, "%d", mins);
        return ts_min;
    }
    sprintf(ts_min, 4, "%d", mins);
    return ts_min;
}

char *get_seconds_string(uint32_t secs)
{
    if (secs < 10) {
        ts_sec[0] = '0';
        sprintf(&(ts_sec[1]), 3, "%d", secs);
        return ts_sec;
    }
    sprintf(ts_sec, 4, "%d", secs);
    return ts_sec;
}

char *get_timestamp(uint32_t secs)
{
    sprintf(ts_complete, 9, "%s:%s:%s",
            get_hours_string(get_hours(secs)),
            get_minutes_string(get_minutes(secs)),
            get_seconds_string(get_seconds(secs)));
    return ts_complete;
}


