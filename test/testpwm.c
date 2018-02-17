
#include "rfsgpio.h"

#include <err.h>    // err
#include <unistd.h> // access

void
pwm_open(struct pwm_t *p, int flags)
{
    p->flags = flags;
    if (rfs_pwm_open(p)) {
        err(1, "opening pwm (chan: %hhu, period: %d, flags: %d)",
            p->channel, p->period, p->flags);
    }
}

void
check_period(struct pwm_t *p, pwm_time_t period)
{
    pwm_time_t per;
    per = rfs_pwm_get_period(p);
    if (per != period) {
        err(1, "period different (channel %hhu, expected %d, read %d)",
            p->channel, period, per);
    }
}

void
check_duty_cycle(struct pwm_t *p, pwm_time_t duty_cycle)
{
    pwm_time_t dc;
    dc = rfs_pwm_get_duty_cycle(p);
    if (dc != duty_cycle) {
        err(1, "duty cycle different (channel %hhu, expected %d, read %d)",
            p->channel, duty_cycle, dc);
    }
}

void
check_enabled(struct pwm_t *p, enum pwm_enable_t enabled)
{
    enum pwm_enable_t e;
    e = rfs_pwm_get_enabled(p);
    if (e != enabled) {
        err(1, "enabled different (channel %hhu, expected %d, read %d)",
            p->channel, enabled, e);
    }
}

void
set_period(struct pwm_t *p, pwm_time_t period)
{
    if (rfs_pwm_set_period(p, period)) {
        err(1, "setting period");
    }
}

void
set_duty_cycle(struct pwm_t *p, pwm_time_t duty_cycle)
{
    if (rfs_pwm_set_duty_cycle(p, duty_cycle)) {
        err(1, "setting duty_cycle");
    }
}

void
set_enabled(struct pwm_t *p, enum pwm_enable_t enable)
{
    if (rfs_pwm_set_enabled(p, enable)) {
        err(1, "setting enabled");
    }
}

void
pwm_close(struct pwm_t *p)
{
    if (rfs_pwm_close(p)) {
        err(1, "close pwm");
    }
}

int
main()
{
    struct pwm_t p = {0, 0, 0, 50000}, p2, pbis;

    // Skip test if there's no PWM
    if (access("/sys/class/pwm", F_OK)) {
        return 77;
    }
    // Open the PWM channel 0
    pwm_open(&p, 0);
    check_period(&p, 50000);
    check_duty_cycle(&p, 0);
    check_enabled(&p, RFS_PWM_ENABLED);
    pwm_close(&p);
    // Open wrong PWM channel
    p2.chip = 0;
    p2.channel = 2;
    p2.flags = 0;
    p2.period = 0;
    if (rfs_pwm_open(&p2) != -1) {
        err(1, "open PWM 2 must give an error but does not");
    }
    // Double open, the second one without export
    pwm_open(&p, 0);
    pbis.chip = 0;
    pbis.channel = 0;
    pbis.period = 50000;
    pwm_open(&pbis, RFS_DONT_EXPORT);
    pwm_close(&pbis);
    pwm_close(&p);
    // Set duty cycle
    pwm_open(&p, 0);
    check_period(&p, 50000);
    check_duty_cycle(&p, 0);
    check_enabled(&p, RFS_PWM_ENABLED);
    set_duty_cycle(&p, 25000);
    check_duty_cycle(&p, 25000);
    // Set wrong duty cycle
    if (rfs_pwm_set_duty_cycle(&p, -1) != -1) {
        err(1, "setting wrong duty cycle must give an error but does not");
    }
    // Set enabled
    set_enabled(&p, RFS_PWM_DISABLED);
    check_enabled(&p, RFS_PWM_DISABLED);
    set_enabled(&p, RFS_PWM_ENABLED);
    // Set wrong enabled
    if (rfs_pwm_set_enabled(&p, -1) != -1) {
        err(1, "setting wrong enabled must give an error but does not");
    }
    if (rfs_pwm_set_enabled(&p, 10) != -1) {
        err(1, "setting wrong enabled must give an error but does not");
    }
    // Set period
    set_period(&p, 100000);
    check_period(&p, 100000);
    // Set wrong period
    if (rfs_pwm_set_period(&p, -1) != -1) {
        err(1, "setting wrong period must give an error but does not");
    }
    pwm_close(&p);
}

