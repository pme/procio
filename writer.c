/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  writer.c
 *
 *    Description:  Simple writer
 *
 *        Version:  1.0
 *        Created:  2011-06-22 10:36
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

#define BUFSIZE 1024
char buf[BUFSIZE];

int main(int argc, char *argv[])
{
  int fd;
  int n = 1000;

  memset(buf, 'X', BUFSIZE);

  printf("pid: %d\n", getpid());

  if (argc != 2) 
    errx(EXIT_FAILURE, "Usage: %s filename", basename(argv[0]));

  if ((fd = open(argv[1], O_CREAT|O_RDWR, 0644)) == -1)
    err(EXIT_FAILURE, "open(%s)", argv[1]);

  while(n--) {
    printf("%4d\n", n);
    if (write(fd, buf, BUFSIZE) == -1)
      err(EXIT_FAILURE, "write(%s)", argv[1]);
  }

  close(fd);

  return 0;
}
