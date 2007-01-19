/*
Copyright (C) 2004 Ronny T. Lampert

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation; either version 2.1 of the License,
or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
*/

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include "msg.h"

#ifdef __cplusplus
extern "C" {
#endif

static int msg_level=MSG_DEFAULT;
static char *MSG_TAB[]={ "FATAL  ", "VERMONT", "ERROR  ", "DEBUG  ", "INFO   ", 0};

/*
 we need to serialize for msg_stat()
 just use a global lock, this isn't a contended lock
 */
static pthread_mutex_t stat_lock = PTHREAD_MUTEX_INITIALIZER;
static FILE *stat_file;

/* the log functions which the message logger thread calls and stuff needed for them */
static LOGFUNCTION log_functions[MAX_LOG_FUNCTIONS];
static void *log_function_params[MAX_LOG_FUNCTIONS];
static int num_log_functions;

/* each log_timeout, the logger thread will call all registered logging functions */
static struct timespec log_timeout = {
        .tv_sec=300,
        .tv_nsec=0
};
static pthread_t log_thread;

/*
 the main logging routine

 we don't have to lock, because glibc's printf() system is doing it already

 if you, however, change this function to a custom backend, you WILL have to lock
 because msg() can and will be called by concurrent threads!
 */
void msg(int level, const char *fmt, ...)
{
        /* nummerically higher value means lower priority */
        if (level > msg_level) {
                return;
        } else {
                va_list args;
                printf("%s: ", MSG_TAB[level]);
                va_start(args, fmt);
                vprintf(fmt, args);
                va_end(args);
                printf("\n");
        }
}


void msg_setlevel(int level)
{
        msg_level=level;
}


/*
 output statistics; usually to file

 call msg_stat_setup() before
 we need a lock (can be called concurrently) to not clutter it all up
 keep critical section as small as possible
 */
int msg_stat(int level, const char *fmt, ...)
{
        if (level > msg_level) {
                return 0;
        }
                                        
        /* have to check if subsys is on. Else just ignore */
        if(stat_file) {
                va_list args;
                va_start(args, fmt);

                pthread_mutex_lock(&stat_lock);
                vfprintf(stat_file, fmt, args);
                fputs("\n", stat_file);
                pthread_mutex_unlock(&stat_lock);

                va_end(args);
        }

        return 0;
}


/* this is future compatible to interact with the system in an ioctl() style */
int msg_stat_setup(int mode, FILE *f)
{
        if(f) {
                pthread_mutex_lock(&stat_lock);
                stat_file=f;
                pthread_mutex_unlock(&stat_lock);

                return 0;
        }

        return 1;

}


int msg_thread_add_log_function(LOGFUNCTION f, void *param)
{
        int ret;

        pthread_mutex_lock(&stat_lock);
        if(num_log_functions < MAX_LOG_FUNCTIONS) {
                log_functions[num_log_functions] = f;
                log_function_params[num_log_functions] = param;
                num_log_functions++;
                ret=0;
        } else {
                ret=1;
        }
        pthread_mutex_unlock(&stat_lock);

        return(ret);
}


void msg_thread_set_timeout(int ms)
{
        assert(ms > 0);
        log_timeout.tv_sec = ms / 1000;
        log_timeout.tv_nsec = ((long)ms % 1000L) * 1000000L;
}


/* start the logger thread with the configured log functions */
int msg_thread_start(void)
{
        return(pthread_create(&log_thread, NULL, msg_thread, NULL));
}


/* this stops the logger thread. hard. */
int msg_thread_stop(void)
{
        return(pthread_cancel(log_thread));
}


/* this is the main message logging thread */
void * msg_thread(void *arg)
{
        int i;

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        while (1) {
                /*
                 we use nanosleep here because nanosleep
                 unlike sleep and usleep, is a thread cancellation point
                 */
                nanosleep(&log_timeout, NULL);

                /* now walk through all log functions and call them */
                pthread_mutex_lock(&stat_lock);
                for(i=0; i < num_log_functions; i++) {
                        if(log_functions[i]) {
                                (log_functions[i])(log_function_params[i]);
                        }
                }

                pthread_mutex_unlock(&stat_lock);
        }

        return NULL;
}

#ifdef __cplusplus
}
#endif
