
#include "rfsgpio.h"

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
rfs_pwm_open(struct pwm_t pwm)
{
    return 0;
}

