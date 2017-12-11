/*    copyapt.c
 *
 * Copyright 2017 Robert L (Bob) Parker rlp1938@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
*/
/* From a source computer (or from backup) generate oldapt:
 * apt list --installed > oldapt
 * At the newly installed computer (or computer out of date with
 * what is installed):
 * apt list --installed > newapt
 * then:
 * copyapt --cut-at '/xenial' oldapt newapt
 *
 * Program will leave package names in oldapt and newapt with the
 * extraneous data in each line cut off from '/xenial' on.
 * Next, it will remove any line in oldapt that exists in newapt.
 * Finally it will invoke apt to install whatever remains in oldapt.
 * There will be many reported errors as it trys to install numerous
 * libraries that are already installed as a consequence of installing
 * main packages. This is harmless.
 *
 * The program must be run as root or apt will fail.
 * */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdarg.h>
#include <getopt.h>
#include <ctype.h>
#include <limits.h>
#include <linux/limits.h>
#include <libgen.h>
#include <errno.h>
#include "str.h"
#include "dirs.h"
#include "files.h"
#include "gopt.h"
// typdefs/structs here.
typedef struct mds {
	mdata *old;
	mdata *new;
} mds;

static mds
*checkargs(char **argv);
static void
truncstr(char **list, const char *cutat);
static char
**packstrarray(char **list);
static void
runapt(char **aptlist);

int main(int argc, char **argv)
{
	options_t opts = process_options(argc, argv);
	char *cutat = opts.o_c;
	mds *mx = checkargs(argv);
	char **oldlist = memblocktoarray(mx->old, 1);
	char **newlist = memblocktoarray(mx->new, 1);
	if (cutat)
	{
		truncstr(oldlist, cutat);
		truncstr(newlist, cutat);
		free(cutat);	// was strdup'd
	}
	size_t i;
	for (i = 0; oldlist[i] ; i++)
		if (inlist(oldlist[i], newlist)) oldlist[i][0] = 0;
	// done with newlist
	destroystrarray(newlist, 0);
	char **worklist = packstrarray(oldlist);	// discard 0 length.
	destroystrarray(oldlist, 0);
	writestrarray(worklist);
	runapt(worklist);
	destroystrarray(worklist, 0);
	return 0;
} //main()

mds
*checkargs(char **argv)
{ /* Requires 2 arguments and they both must be existing files. */
	int i;
	int j = optind + 2;
	for (i = optind; i < j; i++)
	{
		if (!argv[i])
		{	/* Check arg exists. */
			fprintf(stderr, "No file argument provided.\n");
			exit(EXIT_FAILURE);
		} else
		{	/* Check that arg is an existing file. */
			if (!exists_file(argv[i]))
			{
				fprintf(stderr, "%s is not a file.\n", argv[i]);
				exit(EXIT_FAILURE);
			}
		}
	}
	mds *mdres = xmalloc(sizeof(mds *));
	mdres->old = readfile(argv[optind++], 0, 1);
	mdres->new = readfile(argv[optind], 0, 1);
	return mdres;
} // checkargs()

void
truncstr(char **list, const char *cutat)
{ /* Truncate every string in the list at cutat */
	size_t i = 0;
	while (list[i])
	{
		char *tocut = strstr(list[i], cutat);
		if(tocut) *tocut = 0;
		else list[i][0] = 0;	// goodbye nonsense line(s).
		i++;
	}
} // truncstr()

char
**packstrarray(char **list)
{	/* Create a new array of strings, dumping the 0 length items. */
	size_t i, j;
	j = 0;
	for (i = 0; list[i]; i++)
		if (list[i][0]) j++;
	j++;	// terminator
	char **result = xmalloc(j * sizeof(char *));
	result[j - 1] = (char *)NULL;
	j = 0;
	for (i = 0; list[i]; i++)
		if (list[i][0]) result[j++] = xstrdup(list[i]);
	return result;
} // packstrarray

void
runapt(char **aptlist)
{ /* Install the items left in aptlist. Must be NULL terminated. */
	size_t i;
	for (i = 0; aptlist[i] ; i++)
	{
		char command[PATH_MAX];
		sprintf(command, "apt -y install %s", aptlist[i]);
		int res = xsystem(command, 0);
		if(res) fprintf(stderr, "Return code: %d\n", res);
	}
} // runapt()
