
/* rfsgpio.h
   Header for the library librfsgpio.
   
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

#ifndef RFSGPIO_H
#define RFSGPIO_H

// For an explanation about the linux sysfs interface for GPIO and PWM, see:
// https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
// https://www.kernel.org/doc/Documentation/pwm.txt

#include <poll.h>

#define RFS_GPIO_FILENAME_MAX_LEN   40
#define RFS_PWM_FILENAME_MAX_LEN    50

// Type that represents a GPIO pin number
typedef unsigned char gpio_pin_t;

// Type that represents a PWM chip number
typedef unsigned char pwm_chip_t;

// Type that represents a PWM channel number
typedef unsigned char pwm_channel_t;

// Type that represents a PWM signal period or duty cycle time
typedef int pwm_time_t;

// Fags to the opening functions
enum flags_t {
    RFS_DONT_EXPORT = 1
};

// Possible directions of a pin
enum gpio_direction_t {
    RFS_GPIO_IN,
    RFS_GPIO_OUT,
    RFS_GPIO_OUT_LOW,
    RFS_GPIO_OUT_HIGH
};

// Type for the value of a pin
enum gpio_value_t {RFS_GPIO_LOW, RFS_GPIO_HIGH};

// Type for the interrupt mode of a pin
enum gpio_edge_t {
    RFS_GPIO_NONE,
    RFS_GPIO_RISING,
    RFS_GPIO_FALLING,
    RFS_GPIO_BOTH
};

// Type for the PWM enable possible values
enum pwm_enable_t {RFS_PWM_DISABLED, RFS_PWM_ENABLED};

// Struct that contains information about a GPIO pin
struct gpio_t {
    // GPIO pin number
    gpio_pin_t pin;

    /* Opening flags, needed for closing operation. Possible flags are:
       RFS_GPIO_DONT_EXPORT
    */
    int flags;

    // File descriptor of the value sysfs file, in case it is left opened for
    // polling purposes
    int fd;

    // Some file names to avoid compute them at every operation
    char direction_file[RFS_GPIO_FILENAME_MAX_LEN];
    char value_file[RFS_GPIO_FILENAME_MAX_LEN];
    char edge_file[RFS_GPIO_FILENAME_MAX_LEN];
};

// Struct that contains information about a PWM channel.
struct pwm_t {
    // PWM chip number
    pwm_chip_t chip;

    // PWM channel number
    pwm_channel_t channel;

    /* Opening flags, needed for closing operation. Possible flags are:
       RFS_GPIO_DONT_EXPORT
    */
    int flags;

    // The PWM signal's period (in nanoseconds)
    pwm_time_t period;

    // Some file names to avoid compute them at every operation
    char export_file[RFS_PWM_FILENAME_MAX_LEN];
    char unexport_file[RFS_PWM_FILENAME_MAX_LEN];
    char period_file[RFS_PWM_FILENAME_MAX_LEN];
    char duty_cycle_file[RFS_PWM_FILENAME_MAX_LEN];
    char enable_file[RFS_PWM_FILENAME_MAX_LEN];
};

/* Open a GPIO pin using the linux sysfs interface.

   The pin is first exported and then the direction set. If the flag
   RFS_GPIO_DONT_EXPORT is set, the pin is not exported by the library and must
   be externally exported to use it.

   Parameters:
     * gpio: information to open the GPIO pin. Of this structure, only the
         fields pin and flags must be filled. the others are for internal use
         only.
         gpio.pin must contain the number of the pin to open.
         gpio.flags contains modifying flags.
         If flag RFS_GPIO_DONT_EXPORT is set, the pin is not exported by this
         library.
     * direction: Initial direction of the pin. May be one of RFS_GPIO_IN,
         RFS_GPIO_OUT, RFS_GPIO_OUT_LOW and RFS_GPIO_OUT_HIGH. The last two
         set the direction of the pin and the value in an atomic fashion.

   Return 0 if the pin was successfully opened, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_gpio_open(struct gpio_t *gpio, enum gpio_direction_t direction);

/* Return the current direction of a GPIO pin.

   Parameters:
     * gpio: GPIO pin descriptor.

   Return a value of the enum gpio_direction_t, or -1 in case of error. In the
   latter case, errno is set accordingly.
*/
int
rfs_gpio_get_direction(struct gpio_t *gpio);

/* Return the current edge of a GPIO pin.

   Parameters:
     * gpio: GPIO pin descriptor.

   Return a value of the enum gpio_edge_t, or -1 in case of error. In the
   latter case, errno is set accordingly.
*/
int
rfs_gpio_get_edge(struct gpio_t *gpio);

/* Return the poll descriptors, used to call poll over this pin.

   The pin is leaved opened, of course, to be able to poll it.

   Parameters:
     * gpio: GPIO pin descriptor.
     * descriptors: values to use with the poll function.

   Return 0 if the pin was successfully opened, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_gpio_get_poll_descriptors(struct gpio_t *gpio, struct pollfd *descriptors);

/* Return the current value of a GPIO pin.

   Parameters:
     * gpio: GPIO pin descriptor.

   Return a value of the enum gpio_value_t, or -1 in case of error. In the
   latter case, errno is set accordingly.
*/
int
rfs_gpio_get_value(struct gpio_t *gpio);

/* Set the direction of a GPIO pin (input or output).

   Parameters:
     * gpio: GPIO pin descriptor.
     * direction: direction to give to the GPIO pin.

   Return 0 if the direction was successfully changed, -1 in case of error. In
   this last case, errno is set accordingly.
*/
int
rfs_gpio_set_direction(struct gpio_t *gpio, enum gpio_direction_t direction);

/* Set the edge of a GPIO pin (input or output).

   Parameters:
     * gpio: GPIO pin descriptor.
     * edge: edge to give to the GPIO pin.

   Return 0 if the edge was successfully changed, -1 in case of error. In
   this last case, errno is set accordingly.
*/
int
rfs_gpio_set_edge(struct gpio_t *gpio, enum gpio_edge_t edge);

/* Set the value of a GPIO pin (low or high).

   Parameters:
     * gpio: GPIO pin descriptor.
     * value: value to give to the GPIO pin.

   Return 0 if the value was successfully changed, -1 in case of error. In
   this last case, errno is set accordingly.
*/
int
rfs_gpio_set_value(struct gpio_t *gpio, enum gpio_value_t value);

/* Closes the GPIO pin.

   The direction is set to RFS_GPIO_IN. If the pin was exported, is unexported.

   Parameters:
     * gpio: GPIO pin descriptor.

   Return 0 if the pin was correctly closed (direction changed and unexported
   if necessary), -1 otherwise. In case of error, errno is set accordingly.
*/
int
rfs_gpio_close(struct gpio_t *gpio);

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
rfs_pwm_open(struct pwm_t *pwm);

/* Get the PWM signal's duty cycle

   Parameters:
     * pwm: the PWM channel descriptor.

   Return the duty cycle in case of succes, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_pwm_get_duty_cycle(struct pwm_t *pwm);

/* Get the enabled state of the PWM channel

   Parameters:
     * pwm: the PWM channel descriptor.

   Return the enabled status in case of succes, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_pwm_get_enabled(struct pwm_t *pwm);

/* Get the PWM signal's period

   Parameters:
     * pwm: the PWM channel descriptor.

   Return the period in case of succes, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_pwm_get_period(struct pwm_t *pwm);

/* Set the duty cycle of the PWM signal.

   Parameters:
     * pwm: the PWM channel descriptor.
     * duty_cycle: the new duty cycle.

   Return 0 if the duty cycle was correctly set, -1 otherwise (and errno is
   set accordingly).
*/
int
rfs_pwm_set_duty_cycle(struct pwm_t *pwm, pwm_time_t duty_cycle);

/* Set the enabled state of the PWM channel.

   Parameters:
     * pwm: the PWM channel descriptor.
     * enable: RFS_PWM_DISABLED to disable, RFS_PWM_ENABLED to enable.

   Return 0 if the pwm channel is correctly enabled/disabled, -1 otherwise (and
   errno is set accordingly).
*/
int
rfs_pwm_set_enabled(struct pwm_t *pwm, enum pwm_enable_t enabled);

/* Set the period of the PWM signal.

   Parameters:
     * pwm: the PWM channel descriptor.
     * period: period of the PWM signal.

   Return 0 if the period was correctly set, -1 otherwise (and errno is
   set accordingly).
*/
int
rfs_pwm_set_period(struct pwm_t *pwm, pwm_time_t period);

/* Closes the PWM pin.

   The PWM channel is disabled. If the pin was exported, is unexported.

   Parameters:
     * pwm: the PWM channel descriptor.

   Return 0 if the PWM channel was correctly closed (disabled and unexported
   if necessary), -1 otherwise. In case of error, errno is set accordingly.
*/
int
rfs_pwm_close(struct pwm_t *pwm);

#endif

