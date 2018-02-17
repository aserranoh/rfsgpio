
# rfsgpio

rfsgpio is a library to use the hardware interfaces **GPIO** and **PWM** in
Linux. It is implemented used the `sysfs` interface that the kernel provides.

## Getting Started

These instructions will get you a copy of the project up and running on your
local machine for development and testing purposes.

### Prerequisites

For this library to be of any use, your machine must have:

* Some **GPIO** and/or **PWM** ports (for example, the **Raspberry PI**s).
* You must have kernel support activated for the `sysfs` interfaces that the
  kernel provides for these hardware interfaces (check the next options in
  your `.config` file):

```
CONFIG_GPIO_SYSFS=y
CONFIG_PWM_SYSFS=y
```

### Installing

To install this library, just clone the repository (or download the release
tarball) and execute the three known steps:

```
./configure
make
make install
```

### Using the `rfsgpio` library

Here are some small examples on how to use this library. The first one shows
basic input/output **GPIO** operations:

```c
#include <err.h>
#include <rfsgpio.h>

int main() {
    struct gpio_t gpio;

    gpio.pin = 5;
    gpio.flags = 0;
    // Open the GPIO pin
    if (rfs_gpio_open(&gpio, RFS_GPIO_IN)) {
        err(1, "some error opening the GPIO pin #5");
    }
    // Read the value from the GPIO pin
    printf("pin value: %d\n", rfs_gpio_get_value(&gpio));
    // Change pin direction
    rfs_gpio_set_direction(&gpio, RFS_GPIO_OUT);
    // Write a value to the GPIO pin
    rfs_gpio_set_value(&gpio, RFS_GPIO_HIGH);
    // Close the GPIO pin
    rfs_gpio_close(&gpio);
}

```

When linking, use the flag `-lrfsgpio`.

The next example shows how to wait for a change in value of a **GPIO** pin.
Note that if you want to repeat again the wait cycle you have to call again
`rfs_gpio_get_poll_descriptors`, because when the value is read the file is
closed and the previous descriptor doesn't make sense anymore.

```c
#include <err.h>
#include <poll.h>
#include <rfsgpio.h>

int main() {
    struct gpio_t gpio;
    struct pollfs pd;

    gpio.pin = 5;
    gpio.flags = 0;
    // Open the GPIO pin
    if (rfs_gpio_open(&gpio, RFS_GPIO_IN)) {
        err(1, "some error opening the GPIO pin #5");
    }
    // Tell the kernel that you want to be notified when the GPIO value
    // falls or rises
    rfs_gpio_set_edge(&gpio, RFS_GPIO_BOTH);
    // Obtain the poll descriptor
    rfs_gpio_get_poll_descriptors(&gpio, &pd);
    // Waiting for a change in value
    if (poll(&pd, 1, -1) > 0) {
        printf("new value in GPIO pin #5: %d\n", rfs_gpio_get_value(&gpio));
    } else {
        err(1, "some error polling the GPIO pin #5");
    }
    // Close the GPIO pin
    rfs_gpio_close(&gpio);
}

```

And finally, basic use of the **PWM** signals:

```c
#include <err.h>
#include <rfsgpio.h>

int main() {
    struct pwm_t pwm;

    pwm.chip = 0;       // This is the number N in /sys/class/pwm/pwmchipN
    pwm.channel = 0;
    pwm.flags = 0;
    pwm.period = 50000; // The period of the PWM signal, in nanoseconds

    // Open the PWM channel
    if (rfs_pwm_open(&pwm)) {
        err(1, "some error opening the PWM channel #0");
    }
    // Set full power signal
    rfs_pwm_set_duty_cycle(&pwm, 50000);
    // Set half power signal
    rfs_pwm_set_duty_cycle(&pwm, 25000);
    // Zero power signal
    rfs_pwm_set_duty_cycle(&pwm, 0);
    // Close the PWM channel
    rfs_pwm_close(&pwm);
}
```

Usually the calls to `rfs_gpio_open` and `rfs_pwm_open` will export the pin
before use it and `rfs_gpio_close` and `rfs_pwm_close` will unexport them.
Usually only root can do this, so in normal use, the program that uses this
library will have to run with superuser privileges. You can use the flag
`RFS_DONT_EXPORT` to start using the pins without exporting them, assuming
off course that somebody exported them before (if the pins are not exported
then nothing will work). This can be used, for example, to allow an initscript
to export some pins at boot time, change the permissions and/or group of some
**GPIO/PWM** files (actually `value`, `direction` and `edge` for the **GPIO**
and `period`, `enable` and `duty_cycle` for the **PWM**) and then a program
that uses this library could use the **GPIO** and **PWM** without running as
superuser.

## Authors

**Antonio Serrano Hernandex**.

## License

This project is licensed under the GPLv3 License - see the `COPYING` file for
details.

