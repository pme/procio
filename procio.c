/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  procio.c
 *
 *    Description:  pme's small 'iotop' called procio
 *
 *        Version:  1.0
 *        Created:  2011-06-24 14:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Meszaros (pme), hauptadler@gmail.com
 *      Copyright:  Copyright (c) 2011
 *
 * =====================================================================================
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * =====================================================================================
 */

/*
 * =====================================================================================
 * Sample content of /proc/.../io
 * rchar: 1267
 * wchar: 0
 * syscr: 6
 * syscw: 0
 * read_bytes: 0
 * write_bytes: 0
 * cancelled_write_bytes: 0
 * =====================================================================================
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>

enum sortby {
  SortbyNone,
  SortbyWchar,
  SortbyRchar,
  SortbySyscw,
  SortbySyscr,
  SortbyWriteb,
  SortbyReadb,
  SortbyCwriteb,
};

struct procioitem {
  pid_t pid;
  uint64_t rchar;
  uint64_t wchar;
  uint64_t syscr;
  uint64_t syscw;
  uint64_t read_bytes;
  uint64_t write_bytes;
  uint64_t cancelled_write_bytes;
};

struct procio {
  struct timeval tv;
  int npio;
  enum sortby sortby;
  struct procioitem *pio;
};

struct procio pion, pioo;
char version[] = "1.0";

int filter(const struct dirent *de)
{
  const char *s = de->d_name;

  while (*s) {
    if (!isdigit(*s)) return 0; /*  pid numbers only */
    s++;
  }
  return 1;

}

int getiodata(struct procio *pio)
{
  struct dirent **de;
  int i;

  gettimeofday(&pio->tv, NULL);

  if ((pio->npio = scandir("/proc", &de, filter, versionsort)) == -1)
    err(EXIT_FAILURE, "scandir()");

  if ((pio->pio = calloc(pio->npio, sizeof(struct procioitem))) == NULL)
    errx(EXIT_FAILURE, "calloc()");

  for(i=0; i<pio->npio; i++) {
    FILE *f;
    char buf[PATH_MAX+1];

    snprintf(buf, PATH_MAX, "/proc/%s/io", de[i]->d_name);

    if ((f = fopen(buf, "r")) == NULL) { /* Should it really exists later? */
      warn("open(%s)", buf);
      continue;
    }

    pio->pio[i].pid = atoll(de[i]->d_name);
    while (fgets(buf, PATH_MAX, f)) {
      char *nam, *val;
      long long v;

      nam = strtok(buf, ": ");
      val = strtok(NULL, ": ");
      v = atoll(val);

      if (!strcmp(nam, "rchar")) {
        pio->pio[i].rchar = v;
      } else if (!strcmp(nam, "wchar")) {
        pio->pio[i].wchar = v;
      } else if (!strcmp(nam, "syscr")) {
        pio->pio[i].syscr = v;
      } else if (!strcmp(nam, "syscw")) {
        pio->pio[i].syscw = v;
      } else if (!strcmp(nam, "read_bytes")) {
        pio->pio[i].read_bytes = v;
      } else if (!strcmp(nam, "write_bytes")) {
        pio->pio[i].write_bytes = v;
      } else if (!strcmp(nam, "cancelled_write_bytes")) {
        pio->pio[i].cancelled_write_bytes = v;
      } else {
      }
    }
    fclose(f);
  }

  free(de);

  return pio->npio;
}

char * getsortbyname(enum sortby sortby)
{
	switch (sortby) {
		case SortbyNone:
			return "None";
			break;
		case SortbyWchar:
			return "Wchar";
			break;
		case SortbyRchar:
			return "Rchar";
			break;
		case SortbySyscw:
			return "Syscw";
			break;
		case SortbySyscr:
			return "Syscr";
			break;
		case SortbyWriteb:
			return "Writeb";
			break;
		case SortbyReadb:
			return "Readb";
			break;
		case SortbyCwriteb:
			return "Cwriteb";
			break;
		default:
			return "Unknown";
	}
}

void printpio(struct procio *pio, char *tag, FILE *f, int pglen)
{
  int i;
  struct tm *tm;

  tm = gmtime(&pio->tv.tv_sec);

	fprintf(f, "at %ld.%06ld '%04d-%02d-%02d %02d:%02d:%02d.%06ld UTC' numproc: %d tag: %s sortby: %s\n",
      pio->tv.tv_sec, pio->tv.tv_usec, 
			tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec, pio->tv.tv_usec, 
			pio->npio,
      tag,
      getsortbyname(pio->sortby));

  fprintf(f, "  nr    pid     rchar     wchar     syscr     syscw     rbyte     wbyte    cbytes\n");

  for(i=0; i<pglen && i<pio->npio; i++) {
    fprintf(f, "%4d %6d %9llu %9llu %9llu %9llu %9llu %9llu %9llu\n",
        i,
        pio->pio[i].pid,
        pio->pio[i].rchar,
        pio->pio[i].wchar,
        pio->pio[i].syscr,
        pio->pio[i].syscw,
        pio->pio[i].read_bytes,
        pio->pio[i].write_bytes,
        pio->pio[i].cancelled_write_bytes);
  }
}

void deltapio(struct procio *pioo, struct procio *pion)
{
  int io=0, in=0;

  while (io<pioo->npio && in<pion->npio) {
    if (pioo->pio[io].pid == pion->pio[in].pid) {
        pioo->pio[io].rchar       	        = pion->pio[in].rchar                 - pioo->pio[io].rchar;
        pioo->pio[io].wchar       	        = pion->pio[in].wchar                 - pioo->pio[io].wchar;
        pioo->pio[io].syscr       	        = pion->pio[in].syscr                 - pioo->pio[io].syscr;
        pioo->pio[io].syscw       	        = pion->pio[in].syscw                 - pioo->pio[io].syscw;
        pioo->pio[io].read_bytes       	    = pion->pio[in].read_bytes            - pioo->pio[io].read_bytes;
        pioo->pio[io].write_bytes           = pion->pio[in].write_bytes           - pioo->pio[io].write_bytes;
        pioo->pio[io].cancelled_write_bytes = pioo->pio[in].cancelled_write_bytes - pioo->pio[io].cancelled_write_bytes;
        io++;
        in++;
    }
    else if (pioo->pio[io].pid < pion->pio[in].pid) io++;
    else if (pioo->pio[io].pid > pion->pio[in].pid) in++;
  }
}

int sortbyrchar(const void *p1, const void *p2)
{
  const struct procioitem *pi1 = p1, *pi2 = p2;

  if (pi1->rchar < pi2->rchar) return +1;
  else if (pi1->rchar > pi2->rchar) return -1;

  return 0;
}

int sortbywchar(const void *p1, const void *p2)
{
  const struct procioitem *pi1 = p1, *pi2 = p2;

  if (pi1->wchar < pi2->wchar) return +1;
  else if (pi1->wchar > pi2->wchar) return -1;

  return 0;
}

int sortbysyscr(const void *p1, const void *p2)
{
  const struct procioitem *pi1 = p1, *pi2 = p2;

  if (pi1->syscr < pi2->syscr) return +1;
  else if (pi1->syscr > pi2->syscr) return -1;

  return 0;
}

int sortbysyscw(const void *p1, const void *p2)
{
  const struct procioitem *pi1 = p1, *pi2 = p2;

  if (pi1->syscw < pi2->syscw) return +1;
  else if (pi1->syscw > pi2->syscw) return -1;

  return 0;
}

int sortbyreadb(const void *p1, const void *p2)
{
  const struct procioitem *pi1 = p1, *pi2 = p2;

  if (pi1->read_bytes < pi2->read_bytes) return +1;
  else if (pi1->read_bytes > pi2->read_bytes) return -1;

  return 0;
}

int sortbywriteb(const void *p1, const void *p2)
{
  const struct procioitem *pi1 = p1, *pi2 = p2;

  if (pi1->write_bytes < pi2->write_bytes) return +1;
  else if (pi1->write_bytes > pi2->write_bytes) return -1;

  return 0;
}

int sortbycwriteb(const void *p1, const void *p2)
{
  const struct procioitem *pi1 = p1, *pi2 = p2;

  if (pi1->cancelled_write_bytes < pi2->cancelled_write_bytes) return +1;
  else if (pi1->cancelled_write_bytes > pi2->cancelled_write_bytes) return -1;

  return 0;
}

void sortpio(struct procio *pio, enum sortby sortby)
{
  pio->sortby = sortby;
	switch (sortby) {
		case SortbyWchar:
			qsort(pio->pio, pio->npio,  sizeof(struct procioitem), sortbywchar);
			break;
		case SortbyRchar:
			qsort(pio->pio, pio->npio,  sizeof(struct procioitem), sortbyrchar);
			break;
		case SortbySyscw:
			qsort(pio->pio, pio->npio,  sizeof(struct procioitem), sortbysyscw);
			break;
		case SortbySyscr:
			qsort(pio->pio, pio->npio,  sizeof(struct procioitem), sortbysyscr);
			break;
		case SortbyWriteb:
			qsort(pio->pio, pio->npio,  sizeof(struct procioitem), sortbywriteb);
			break;
		case SortbyReadb:
			qsort(pio->pio, pio->npio,  sizeof(struct procioitem), sortbyreadb);
			break;
		case SortbyCwriteb:
			qsort(pio->pio, pio->npio,  sizeof(struct procioitem), sortbycwriteb);
			break;
		case SortbyNone:
      break;
	}
}

int main(int argc, char *argv[])
{
  int opt;
  int delay = 1;
  int verbose = 0;
  int pglen = 10;
	enum sortby sortby = SortbyWchar;

	while ((opt = getopt(argc, argv, "hd:vp:s:")) != -1) {
		switch (opt) {
			case 'd':
				delay = atoi(optarg);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'p':
				pglen = atoi(optarg);
				break;
			case 's':
				sortby = atoi(optarg);
				break;
			case 'h':
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-h] [-d secs] [-v] [-p pagelen] [-s sortby] # version: %s\n", basename(argv[0]), version);
        fprintf(stderr, "  -h        - Prints this message\n");
        fprintf(stderr, "  -d #      - delay in secs (def: 1s)\n");
        fprintf(stderr, "  -v        - verbose\n");
        fprintf(stderr, "  -p #      - page length (def: 10)\n");
        fprintf(stderr, "  -s [1-7]  - order by descending\n");
        fprintf(stderr, "    sortby: 1 - wchar, 2 - rchar, 3 - syscw, 4 - syscr, 5 - writeb, 6 - readb or 7 - cwriteb\n");
				exit(opt == 'h' ? EXIT_SUCCESS : EXIT_FAILURE);
		}
	}

	while (1) {
    getiodata(&pion);

    if (verbose) printpio(&pion, "NEW", stdout, INT_MAX);

    if (pioo.pio) {
      deltapio(&pioo, &pion);
      sortpio(&pioo, sortby);
      printpio(&pioo, "DELTA", stdout, pglen);
      free(pioo.pio);
    }

    pioo = pion;

    fprintf(stdout, "=================================================================================\n");
    sleep(delay);
  }

  return 0;
}

