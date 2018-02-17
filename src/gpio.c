
/* gpio.c
   Controls the pins of a GPIO port using the linux kernel interface sysfs.
   
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
#include <fcntl.h>  // open
#include <stdio.h>  // snprintf
#include <string.h> // strcmp
#include <unistd.h> // close

#define RFS_GPIO_BASE_DIR       "/sys/class/gpio/"
#define RFS_GPIO_EXPORT_FILE    RFS_GPIO_BASE_DIR "export"
#define RFS_GPIO_UNEXPORT_FILE  RFS_GPIO_BASE_DIR "unexport"
#define RFS_GPIO_PIN_DIR        RFS_GPIO_BASE_DIR "gpio%hhu/"
#define RFS_GPIO_DIRECTION_FILE RFS_GPIO_PIN_DIR "direction"
#define RFS_GPIO_VALUE_FILE     RFS_GPIO_PIN_DIR "value"
#define RFS_GPIO_EDGE_FILE      RFS_GPIO_PIN_DIR "edge"

// Possible values to write to the direction file
static const char *gpio_direction_str[] = {"in", "out", "low", "high"};

// Possible values to write to the value file
static const char *gpio_value_str[] = {"0", "1"};

// Possible values to write to the edge file
static const char *gpio_edge_str[] = {"none", "rising", "falling", "both"};

/* Open a GPIO pin.

   The pin is first exported and then the direction set. If the flag
   RFS_GPIO_DONT_EXPORT is set, the pin is not exported by the library and must
   be externally exported to use it.

   Parameters:
     * gpio: information to open the GPIO pin. Of this structure, only the
         fields pin and flags must be filled. The others are for internal use
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
rfs_gpio_open(struct gpio_t *gpio, enum gpio_direction_t direction)
{
    char pinstr[RFS_MAX_SYSFS_STR];

    // Check direction value
    if (direction < RFS_GPIO_IN || direction > RFS_GPIO_OUT_HIGH) {
        errno = EINVAL;
        return -1;
    }

    // Initialize the file names for this pin
    snprintf(gpio->direction_file, RFS_GPIO_FILENAME_MAX_LEN,
        RFS_GPIO_DIRECTION_FILE, gpio->pin);
    snprintf(gpio->value_file, RFS_GPIO_FILENAME_MAX_LEN, RFS_GPIO_VALUE_FILE,
        gpio->pin);
    snprintf(gpio->edge_file, RFS_GPIO_FILENAME_MAX_LEN, RFS_GPIO_EDGE_FILE,
        gpio->pin);

    // Export the pin, if necessary
    if (!(gpio->flags & RFS_DONT_EXPORT)) {
        snprintf(pinstr, RFS_MAX_SYSFS_STR, "%hhu", gpio->pin);
        if (write_sysfs_file(RFS_GPIO_EXPORT_FILE, pinstr)) {
            return -1;
        }
    }

    // Set the pin mode
    if (rfs_gpio_set_direction(gpio, direction)) {
        // In case of error, unexport the pin (if it was actually exported)
        if (!(gpio->flags & RFS_DONT_EXPORT)) {
            write_sysfs_file(RFS_GPIO_UNEXPORT_FILE, pinstr);
        }
        return -1;
    }
    return 0;
}

/* Return the current direction of a GPIO pin.

   Parameters:
     * gpio: GPIO pin descriptor.

   Return a value of the enum gpio_direction_t, or -1 in case of error. In the
   latter case, errno is set accordingly.
*/
int
rfs_gpio_get_direction(struct gpio_t *gpio)
{
    char dirstr[RFS_MAX_SYSFS_STR];

    if (read_sysfs_file(gpio->direction_file, dirstr, RFS_MAX_SYSFS_STR)) {
        return -1;
    }
    if (strcmp(dirstr, "out\n") == 0) {
        return RFS_GPIO_OUT;
    }
    return RFS_GPIO_IN;
}

/* Return the current edge of a GPIO pin.

   Parameters:
     * gpio: GPIO pin descriptor.

   Return a value of the enum gpio_edge_t, or -1 in case of error. In the
   latter case, errno is set accordingly.
*/
int
rfs_gpio_get_edge(struct gpio_t *gpio)
{
    char edgestr[RFS_MAX_SYSFS_STR];

    if (read_sysfs_file(gpio->edge_file, edgestr, RFS_MAX_SYSFS_STR)) {
        return -1;
    }
    if (strcmp(edgestr, "rising\n") == 0) {
        return RFS_GPIO_RISING;
    } else if (strcmp(edgestr, "falling\n") == 0) {
        return RFS_GPIO_FALLING;
    } else if (strcmp(edgestr, "both\n") == 0) {
        return RFS_GPIO_BOTH;
    }
    return RFS_GPIO_NONE;
}

/* Return the poll descriptors, used to call poll over this pin.

   The pin is leaved opened, of course, to be able to poll it.

   Parameters:
     * gpio: GPIO pin descriptor.
     * descriptors: values to use with the poll function.

   Return 0 if the pin was successfully opened, -1 otherwise. In case of error,
   errno is set accordingly.
*/
int
rfs_gpio_get_poll_descriptors(struct gpio_t *gpio, struct pollfd *descriptors)
{
    close(gpio->fd);
    gpio->fd = open(gpio->value_file, O_RDONLY | O_SYNC);
    if (gpio->fd < 0) {
        return -1;
    }
    descriptors->fd = gpio->fd;
    descriptors->events = POLLPRI | POLLERR;
    return 0;
}

/* Return the current value of a GPIO pin.

   Parameters:
     * gpio: GPIO pin descriptor.

   Return a value of the enum gpio_value_t, or -1 in case of error. In the
   latter case, errno is set accordingly.
*/
int
rfs_gpio_get_value(struct gpio_t *gpio)
{
    char valstr[RFS_MAX_SYSFS_STR];

    close(gpio->fd);
    if (read_sysfs_file(gpio->value_file, valstr, RFS_MAX_SYSFS_STR)) {
        return -1;
    }
    if (strcmp(valstr, "0\n") == 0) {
        return RFS_GPIO_LOW;
    }
    return RFS_GPIO_HIGH;
}

/* Set the direction of a GPIO pin (input or output).

   Parameters:
     * gpio: GPIO pin descriptor.
     * direction: direction to give to the GPIO pin.

   Return 0 if the direction was successfully changed, -1 in case of error. In
   this last case, errno is set accordingly.
*/
int
rfs_gpio_set_direction(struct gpio_t *gpio, enum gpio_direction_t direction)
{
    // Check direction value
    if (direction < RFS_GPIO_IN || direction > RFS_GPIO_OUT_HIGH) {
        errno = EINVAL;
        return -1;
    }
    // Write the direction value
    if (write_sysfs_file(gpio->direction_file, gpio_direction_str[direction]))
    {
        return -1;
    }
    return 0;
}

/* Set the edge of a GPIO pin (input or output).

   Parameters:
     * gpio: GPIO pin descriptor.
     * edge: edge to give to the GPIO pin.

   Return 0 if the edge was successfully changed, -1 in case of error. In
   this last case, errno is set accordingly.
*/
int
rfs_gpio_set_edge(struct gpio_t *gpio, enum gpio_edge_t edge)
{
    // Check edge value
    if (edge < RFS_GPIO_NONE || edge > RFS_GPIO_BOTH) {
        errno = EINVAL;
        return -1;
    }
    // Write the direction value
    if (write_sysfs_file(gpio->edge_file, gpio_edge_str[edge]))
    {
        return -1;
    }
    return 0;
}

/* Set the value of a GPIO pin (low or high).

   Parameters:
     * gpio: GPIO pin descriptor.
     * value: value to give to the GPIO pin.

   Return 0 if the value was successfully changed, -1 in case of error. In
   this last case, errno is set accordingly.
*/
int
rfs_gpio_set_value(struct gpio_t *gpio, enum gpio_value_t value)
{
    // Check value range
    if (value < RFS_GPIO_LOW || value > RFS_GPIO_HIGH) {
        errno = EINVAL;
        return -1;
    }
    close(gpio->fd);
    // Write the direction value
    if (write_sysfs_file(gpio->value_file, gpio_value_str[value]))
    {
        return -1;
    }
    return 0;
}

/* Closes the GPIO pin.

   The direction is set to RFS_GPIO_IN. If the pin was exported, is unexported.

   Parameters:
     * gpio: GPIO pin descriptor.

   Return 0 if the pin was correctly closed (direction changed and unexported
   if necessary), -1 otherwise. In case of error, errno is set accordingly.
*/
int
rfs_gpio_close(struct gpio_t *gpio)
{
    char pinstr[RFS_MAX_SYSFS_STR];

    // Close the file descriptor of the value file in case was opened for
    // polling purposes
    close(gpio->fd);
    if (rfs_gpio_set_direction(gpio, RFS_GPIO_IN)) {
        return -1;
    }
    // Unexport the file, if it was actually exported
    if (!(gpio->flags & RFS_DONT_EXPORT)) {
        snprintf(pinstr, RFS_MAX_SYSFS_STR, "%hhu", gpio->pin);
        if (write_sysfs_file(RFS_GPIO_UNEXPORT_FILE, pinstr)) {
            return -1;
        }
    }
    return 0;
}

