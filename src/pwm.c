
/* pwm.c
   Controls the PWM channels using the linux kernel interface sysfs.
   
   Copyright 2018 Antonio Serrano Hernandez

   This file is part of rfsgpio.

   rfsgpio is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   rfsgpio is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with rfsgpio; see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include "rfsgpio.h"
#include "sysfs.h"

#include <errno.h>  // errno
#include <stdio.h>  // snprintf
#include <stdlib.h> // atoi
#include <string.h> // strcmp

#define RFS_PWM_BASE_DIR        "/sys/class/pwm/pwmchip%hhu/"
#define RFS_PWM_EXPORT_FILE     RFS_PWM_BASE_DIR "export"
#define RFS_PWM_UNEXPORT_FILE   RFS_PWM_BASE_DIR "unexport"
#define RFS_PWM_CHANNEL_DIR     RFS_PWM_BASE_DIR "pwm%hhu/"
#define RFS_PWM_PERIOD_FILE     RFS_PWM_CHANNEL_DIR "period"
#define RFS_PWM_DUTY_CYCLE_FILE RFS_PWM_CHANNEL_DIR "duty_cycle"
#define RFS_PWM_ENABLE_FILE     RFS_PWM_CHANNEL_DIR "enable"

/* Open a PWM channel using the linux sysfs interface.

   The pin is first exported and then the period of the PWM signal is set. If
   the flag RFS_GPIO_DONT_EXPORT is set, the pin is not exported by the library
   and must be externally exported to use it.

   Parameters:
     * pwm: information to open the PWM channel.
         pwm.channel must contain the number of the PWM channel to open.
         pwm.flags contains modifying flags.
         pwm.period contains the period of the PWM signal.
         If flag RFS_GPIO_DONT_EXPORT is set, the pin is not exported by this
         library.

   Return 0 if the pin was successfully opened, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_pwm_open(struct pwm_t *pwm)
{
    char channelstr[RFS_MAX_SYSFS_STR];

    // Initialize the file names for this chip/channel
    snprintf(pwm->export_file, RFS_PWM_FILENAME_MAX_LEN, RFS_PWM_EXPORT_FILE,
        pwm->chip);
    snprintf(pwm->unexport_file, RFS_PWM_FILENAME_MAX_LEN,
        RFS_PWM_UNEXPORT_FILE, pwm->chip);
    snprintf(pwm->period_file, RFS_PWM_FILENAME_MAX_LEN, RFS_PWM_PERIOD_FILE,
        pwm->chip, pwm->channel);
    snprintf(pwm->duty_cycle_file, RFS_PWM_FILENAME_MAX_LEN,
        RFS_PWM_DUTY_CYCLE_FILE, pwm->chip, pwm->channel);
    snprintf(pwm->enable_file, RFS_PWM_FILENAME_MAX_LEN,
        RFS_PWM_ENABLE_FILE, pwm->chip, pwm->channel);

    // Export the pin, if necessary
    if (!(pwm->flags & RFS_DONT_EXPORT)) {
        snprintf(channelstr, RFS_MAX_SYSFS_STR, "%hhu", pwm->channel);
        if (write_sysfs_file(pwm->export_file, channelstr)) {
            return -1;
        }
    }

    if (rfs_pwm_set_period(pwm, pwm->period) || rfs_pwm_set_duty_cycle(pwm, 0)
        || rfs_pwm_set_enabled(pwm, RFS_PWM_ENABLED))
    {
        // In case of error, unexport the channel (if it was actually exported)
        if (!(pwm->flags & RFS_DONT_EXPORT)) {
            write_sysfs_file(pwm->unexport_file, channelstr);
        }
        return -1;
    }
    return 0;
}

/* Get the PWM signal's duty cycle

   Parameters:
     * pwm: the PWM channel descriptor.

   Return the duty cycle in case of succes, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_pwm_get_duty_cycle(struct pwm_t *pwm)
{
    char dcstr[RFS_MAX_SYSFS_STR];

    if (read_sysfs_file(pwm->duty_cycle_file, dcstr, RFS_MAX_SYSFS_STR)) {
        return -1;
    }
    return atoi(dcstr);
}

/* Get the enabled state of the PWM channel

   Parameters:
     * pwm: the PWM channel descriptor.

   Return the enabled status in case of succes, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_pwm_get_enabled(struct pwm_t *pwm)
{
    char enabledstr[RFS_MAX_SYSFS_STR];

    if (read_sysfs_file(pwm->enable_file, enabledstr, RFS_MAX_SYSFS_STR)) {
        return -1;
    }
    if (strcmp(enabledstr, "0\n") == 0) {
        return RFS_PWM_DISABLED;
    }
    return RFS_PWM_ENABLED;
}

/* Get the PWM signal's period

   Parameters:
     * pwm: the PWM channel descriptor.

   Return the period in case of succes, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_pwm_get_period(struct pwm_t *pwm)
{
    char periodstr[RFS_MAX_SYSFS_STR];

    if (read_sysfs_file(pwm->period_file, periodstr, RFS_MAX_SYSFS_STR)) {
        return -1;
    }
    return atoi(periodstr);
}

/* Set the duty cycle of the PWM signal.

   Parameters:
     * pwm: the PWM channel descriptor.
     * duty_cycle: the new duty cycle.

   Return 0 if the duty cycle was correctly set, -1 otherwise (and errno is
   set accordingly).
*/
int
rfs_pwm_set_duty_cycle(struct pwm_t *pwm, pwm_time_t duty_cycle)
{
    char dcstr[RFS_MAX_SYSFS_STR];

    // Check the duty_cycle value
    if (duty_cycle < 0) {
        errno = EINVAL;
        return -1;
    }
    // Write the duty cycle value
    snprintf(dcstr, RFS_MAX_SYSFS_STR, "%u", duty_cycle);
    if (write_sysfs_file(pwm->duty_cycle_file, dcstr))
    {
        return -1;
    }
    return 0;
}

/* Set the enabled state of the PWM channel.

   Parameters:
     * pwm: the PWM channel descriptor.
     * enable: RFS_PWM_DISABLED to disable, RFS_PWM_ENABLED to enable.

   Return 0 if the pwm channel is correctly enabled/disabled, -1 otherwise (and
   errno is set accordingly).
*/
int
rfs_pwm_set_enabled(struct pwm_t *pwm, enum pwm_enable_t enabled)
{
    int res;

    // Check enabled value
    if (enabled < RFS_PWM_DISABLED || enabled > RFS_PWM_ENABLED) {
        errno = EINVAL;
        return -1;
    }
    if (enabled) {
        res = write_sysfs_file(pwm->enable_file, "1");
    } else {
        res = write_sysfs_file(pwm->enable_file, "0");
    }
    if (res) {
        return -1;
    }
    return 0;
}

/* Set the period of the PWM signal.

   Parameters:
     * pwm: the PWM channel descriptor.
     * period: period of the PWM signal.

   Return 0 if the period was correctly set, -1 otherwise (and errno is
   set accordingly).
*/
int
rfs_pwm_set_period(struct pwm_t *pwm, pwm_time_t period)
{
    char periodstr[RFS_MAX_SYSFS_STR];

    // Check the period value
    if (period < 0) {
        errno = EINVAL;
        return -1;
    }
    // Write the period value
    snprintf(periodstr, RFS_MAX_SYSFS_STR, "%u", period);
    if (write_sysfs_file(pwm->period_file, periodstr))
    {
        return -1;
    }
    return 0;
}

/* Closes the PWM pin.

   The PWM channel is disabled. If the pin was exported, is unexported.

   Parameters:
     * pwm: the PWM channel descriptor.

   Return 0 if the PWM channel was correctly closed (disabled and unexported
   if necessary), -1 otherwise. In case of error, errno is set accordingly.
*/
int
rfs_pwm_close(struct pwm_t *pwm)
{
    char channelstr[RFS_MAX_SYSFS_STR];

    if (rfs_pwm_set_enabled(pwm, RFS_PWM_DISABLED)) {
        return -1;
    }
    // Unexport the file, if it was actually exported
    if (!(pwm->flags & RFS_DONT_EXPORT)) {
        snprintf(channelstr, RFS_MAX_SYSFS_STR, "%hhu", pwm->channel);
        if (write_sysfs_file(pwm->unexport_file, channelstr)) {
            return -1;
        }
    }
    return 0;
}

