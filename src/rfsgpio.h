
#ifndef RFSGPIO_H
#define RFSGPIO_H

// For an explanation about the linux sysfs interface for GPIO and PWM, see:
// https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
// https://www.kernel.org/doc/Documentation/pwm.txt

// Type that represents a GPIO pin number
typedef unsigned int gpio_pin_t;

// Type that represents a PWM channel number
typedef unsigned int pwm_channel_t;

// Type that represents a PWM signal period or duty cycle time
typedef unsigned int pwm_time_t;

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
};

// Struct that contains information about a PWM channel.
struct pwm_t {
    // PWM channel number
    pwm_channel_t channel;

    /* Opening flags, needed for closing operation. Possible flags are:
       RFS_GPIO_DONT_EXPORT
    */
    int flags;

    // The PWM signal's period (in nanoseconds)
    pwm_time_t period;
};

/* Open a GPIO pin using the linux sysfs interface.

   The pin is first exported and then the direction set. If the flag
   RFS_GPIO_DONT_EXPORT is set, the pin is not exported by the library and must
   be externally exported to use it.

   Parameters:
     * gpio: information to open the GPIO pin. Of this structure, only the
         fields pin and flags must be filled. fd is for internal use only.
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
int rfs_gpio_open(struct gpio_t gpio, enum gpio_direction_t direction);

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
int rfs_pwm_open(struct pwm_t pwm);

#endif

