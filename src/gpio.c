
#include "rfsgpio.h"

/* Open a GPIO pin.

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
int
rfs_gpio_open(struct gpio_t gpio, enum gpio_direction_t direction)
{
    return 0;
}

