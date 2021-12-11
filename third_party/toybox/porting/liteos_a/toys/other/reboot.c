/* reboot.c - Restart, halt or powerdown the system.
 *
 * Copyright 2013 Elie De Brauwer <eliedebrauwer@gmail.com>

USE_REBOOT(NEWTOY(reboot, "", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))

config REBOOT
  bool "reboot"
  default y
  help
    usage: reboot

    Restart the system.

*/
#define FOR_reboot
#include "toys.h"
#include <sys/reboot.h>

void reboot_main(void)
{
  int types[] = {RB_AUTOBOOT, RB_HALT_SYSTEM, RB_POWER_OFF},
      sigs[] = {SIGTERM, SIGUSR1, SIGUSR2}, idx;

  idx = stridx("hp", *toys.which->name)+1;
  toys.exitval = reboot(types[idx]);
}
