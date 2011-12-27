/* Copyright 2008 Bernhard R. Fischer, Daniel Haslinger.
 *
 * This file is part of OnionCat.
 *
 * OnionCat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * OnionCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OnionCat. If not, see <http://www.gnu.org/licenses/>.
 */

/*! @file
 *  File contains logging functions.
 *  @author Bernhard R. Fischer
 *  @version 2008/10/1
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


#define SIZE_1K 1024
#define TIMESTRLEN 64
#define CBUFLEN SIZE_1K

#ifndef LOG_PRI
#define LOG_PRI(p) ((p) & LOG_PRIMASK)
#endif

static const char *flty_[8] = {"emerg", "alert", "crit", "err", "warning", "notice", "info", "debug"};
//! FILE pointer to log
static FILE *log_ = NULL;
static int level_ = LOG_INFO;


FILE *init_log(const char *s, int level)
{
   level_ = level;

   if (!strcmp(s, "stderr"))
      log_ = stderr;
   else if ((log_ = fopen(s, "a")) == NULL)
      fprintf(stderr, "*** could not open logfile %s: %s. Logging to syslog.\n", s, strerror(errno));

   return log_;
}


/*! Log a message to a file.
 *  @param out Open FILE pointer
 *  @param lf Logging priority (equal to syslog)
 *  @param fmt Format string
 *  @param ap Variable parameter list
 */
void vlog_msgf(FILE *out, int lf, const char *fmt, va_list ap)
{
   static struct timeval tv_stat = {0, 0};
   struct timeval tv, tr;
   struct tm *tm;
   time_t t;
   char timestr[TIMESTRLEN] = "", timez[TIMESTRLEN] = "";
   int level = LOG_PRI(lf);
   char buf[SIZE_1K];

   if (level_ < level) return;

   //t = time(NULL);
   if (gettimeofday(&tv, NULL) == -1)
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, strerror(errno)), exit(EXIT_FAILURE);

   if (!tv_stat.tv_sec) tv_stat = tv;

   tr.tv_sec = tv.tv_sec - tv_stat.tv_sec;
   tr.tv_usec = tv.tv_usec - tv_stat.tv_usec;
   if (tr.tv_usec < 0)
   {
      tr.tv_usec += 1000000;
      tr.tv_sec--;
   }

   t = tv.tv_sec;
   if ((tm = localtime(&t)))
   {
      //(void) strftime(timestr, TIMESTRLEN, "%a, %d %b %Y %H:%M:%S", tm);
      (void) strftime(timestr, TIMESTRLEN, "%H:%M:%S", tm);
      //(void) strftime(timez, TIMESTRLEN, "%z", tm);
   }

   if (out)
   {
      fprintf(out, "%s.%03d %s (+%2d.%03d) [%7s] ", timestr, (int) (tv.tv_usec / 1000), timez, (int) tr.tv_sec, (int) (tr.tv_usec / 1000), flty_[level]);
      vfprintf(out, fmt, ap);
      fprintf(out, "\n");
   }
   else
   {
      // log to syslog if no output stream is available
      //vsyslog(level | LOG_DAEMON, fmt, ap);
      vsnprintf(buf, SIZE_1K, fmt, ap);
      syslog(level | LOG_DAEMON, "%s", buf);

   }
   tv_stat = tv;
}


/*! Log a message. This function automatically determines
 *  to which streams the message is logged.
 *  @param lf Log priority.
 *  @param fmt Format string.
 *  @param ... arguments
 */
void log_msg(int lf, const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   vlog_msgf(log_, lf, fmt, ap);
   va_end(ap);
/*   if (lf)
   {
      va_start(ap, fmt);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      fprintf(stderr, "\n");
   }*/
}

