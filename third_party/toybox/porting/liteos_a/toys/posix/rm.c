/* rm.c - remove files
 *
 * Copyright 2012 Rob Landley <rob@landley.net>
 *
 * See http://pubs.opengroup.org/onlinepubs/9699919799/utilities/rm.html

USE_RM(NEWTOY(rm, "fiRrv[-fi]", TOYFLAG_BIN))

config RM
  bool "rm"
  default y
  help
    usage: rm [-fv] FILE or rm [-rv] [PATH]...

    Remove each argument from the filesystem.

    -r	Remove empty or non empty directory
    -f	Force: remove without confirmation, no error if it doesn't exist
    -v	Verbose
*/

#define FOR_rm
#include "toys.h"

static int toybox_cmd_do_rmdir(const char *pathname)
{
  struct dirent *dirent = NULL;
  struct stat stat_info;
  DIR *dir = NULL;
  char *fullpath = NULL;
  int ret;

  (void)memset(&stat_info,  0, sizeof(struct stat));
  if (stat(pathname, &stat_info) != 0) return -1;

  if (S_ISREG(stat_info.st_mode) || S_ISLNK(stat_info.st_mode)) {
    return remove(pathname);
  }

  dir = opendir(pathname);
  if (dir == NULL) return -1;

  while (1) {
    dirent = readdir(dir);
    if (dirent == NULL) break;

    size_t fullpath_buf_size = strlen(pathname) + strlen(dirent->d_name) + 2;
    if (fullpath_buf_size <= 0) {
        (void)closedir(dir);
        return -1;
      }
    fullpath = (char *)malloc(fullpath_buf_size);
    if (fullpath == NULL) {
        (void)closedir(dir);
        return -1;
      }
    ret = snprintf(fullpath, fullpath_buf_size, "%s/%s", pathname, dirent->d_name);
    if (ret < 0) {
        xprintf("name is too long!\n");
        free(fullpath);
        (void)closedir(dir);
        return -1;
      }
    (void)toybox_cmd_do_rmdir(fullpath);
    free(fullpath);
    }
  (void)closedir(dir);
  return rmdir(pathname);
}

static int do_rm(struct dirtree *try)
{
  int ret;
  int len;
  int fd=dirtree_parentfd(try), dir=S_ISDIR(try->st.st_mode), or=0;

  // Skip . and .. (yes, even explicitly on the command line: posix says to)
  if (isdotdot(try->name)) {
    xprintf("refusing to remove '.' or '..'!\n");
    return 0;
  }

  // Intentionally fail non-recursive attempts to remove even an empty dir
  // (via wrong flags to unlinkat) because POSIX says to.
  if (dir && (toys.optflags & (FLAG_r|FLAG_R))) goto skip;

  // This is either the posix section 2(b) prompt or the section 3 prompt.
  if (!FLAG(f)
    && (!S_ISLNK(try->st.st_mode) && faccessat(fd, try->name, W_OK, 0))) or++;
  if (!(dir && try->again) && ((or && isatty(0)) || FLAG(i))) {
    char *s = dirtree_path(try, 0);

    fprintf(stderr, "rm %s%s%s\n", or ? "ro " : "", dir ? "dir " : "", s);
    free(s);
  }
  // handle directory recursion
  if (dir) {
    // Handle chmod 000 directories when -f
    if (faccessat(fd, try->name, R_OK, 0)) {
      if (FLAG(f)) wfchmodat(fd, try->name, 0700);
      else goto skip;
    }
    if (!try->again) return DIRTREE_COMEAGAIN;
    if (try->symlink) goto skip;
    if (FLAG(i)) {
      char *s = dirtree_path(try, 0);

      // This is the section 2(d) prompt. (Yes, posix says to prompt twice.)
      fprintf(stderr, "rmdir %s", s);
      free(s);
    }
  }

skip:

  // Delete the last '/' of pathname.
  len = strlen(try->name);
  while (len > 0 && try->name[len-1] == '/') len--;
  try->name[len] = '\0';

  // Remove dir or files
  if (FLAG(r)) ret = toybox_cmd_do_rmdir(try->name);
  else ret = unlink(try->name);
  if (!ret) {
    if (FLAG(v)) {
      char *s = dirtree_path(try, 0);
      xprintf("%s%s '%s'\n", toys.which->name, dir ? "dir" : "", s);
      free(s);
    }
  } else {
    if (!dir || try->symlink != (char *)2) perror_msg_raw(try->name);
  }
  return 0;
}

void rm_main(void)
{
  char **s;

  // Can't use <1 in optstring because zero arguments with -f isn't an error
  if (!toys.optc && !FLAG(f)) help_exit("Needs 1 argument");

  for (s = toys.optargs; *s; s++) {
    if (!strcmp(*s, "/")) {
      error_msg("rm /. if you mean it");
      continue;
    }

    // Files that already don't exist aren't errors for -f, so try a quick
    // unlink now to see if it succeeds or reports that it didn't exist.
    if (FLAG(f) && (!unlink(*s) || errno == ENOENT)) continue;

    // There's a race here where a file removed between the above check and
    // dirtree's stat would report the nonexistence as an error, but that's
    // not a normal "it didn't exist" so I'm ok with it.

    dirtree_read(*s, do_rm);
  }
}
