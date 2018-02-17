
#include "rfsgpio.h"

#include <err.h>    // err
#include <unistd.h> // access

void
gpio_open(struct gpio_t *g, gpio_pin_t pin, enum gpio_direction_t d, int flags)
{
    g->pin = pin;
    g->flags = flags;
    if (rfs_gpio_open(g, d)) {
        err(1, "open gpio %hhu", pin);
    }
}

void
check_direction(struct gpio_t *g, enum gpio_direction_t d)
{
    int dir = rfs_gpio_get_direction(g);
    if (dir != d) {
        err(1, "direction of pin %hhu is not %s (%hhu)", g->pin,
            (d == RFS_GPIO_IN) ? "in" : "out", dir);
    }
}

void
check_value(struct gpio_t *g, enum gpio_value_t v)
{
    int val = rfs_gpio_get_value(g);
    if (val != v) {
        err(1, "value of pin %hhu is not %s (%hhu)", g->pin, 
            (v == RFS_GPIO_LOW) ? "low" : "high", val);
    }
}

void
check_edge(struct gpio_t *g, enum gpio_edge_t e)
{
    int edge = rfs_gpio_get_edge(g);
    if (edge != e) {
        err(1, "edge of pin %hhu is not %hhu (%hhu)", g->pin, e, edge);
    }
}

void
check_poll_descriptors(struct gpio_t *g)
{
    struct pollfd pd;
    if (rfs_gpio_get_poll_descriptors(g, &pd)) {
        err(1, "error getting poll descriptors");
    }
    if (pd.fd != g->fd && pd.events != (POLLPRI | POLLERR)) {
        errx(1, "wrong value of poll descriptors");
    }
}

void
set_direction(struct gpio_t *g, enum gpio_direction_t d)
{
    if (rfs_gpio_set_direction(g, d)) {
        err(1, "setting direction");
    }
}

void
set_value(struct gpio_t *g, enum gpio_value_t v)
{
    if (rfs_gpio_set_value(g, v)) {
        err(1, "setting value");
    }
}

void
set_edge(struct gpio_t *g, enum gpio_edge_t e)
{
    if (rfs_gpio_set_edge(g, e)) {
        err(1, "setting edge");
    }
}

void
gpio_close(struct gpio_t *g)
{
    if (rfs_gpio_close(g)) {
        err(1, "close gpio %hhu", g->pin);
    }
}

int
main()
{
    struct gpio_t g4, g99, g4bis, g17;
    struct pollfd pd;

    // Skip test if there's no GPIO
    if (access("/sys/class/gpio", F_OK)) {
        return 77;
    }
    // Open the GPIO pin 4 as input
    gpio_open(&g4, 4, RFS_GPIO_IN, 0);
    check_direction(&g4, RFS_GPIO_IN);
    gpio_close(&g4);
    // Open the GPIO pin 4 as output
    gpio_open(&g4, 4, RFS_GPIO_OUT, 0);
    check_direction(&g4, RFS_GPIO_OUT);
    check_value(&g4, RFS_GPIO_LOW);
    gpio_close(&g4);
    // Open the GPIO pin 4 as output low
    gpio_open(&g4, 4, RFS_GPIO_OUT_LOW, 0);
    check_direction(&g4, RFS_GPIO_OUT);
    check_value(&g4, RFS_GPIO_LOW);
    gpio_close(&g4);
    // Open the GPIO pin 4 as output high
    gpio_open(&g4, 4, RFS_GPIO_OUT_HIGH, 0);
    check_direction(&g4, RFS_GPIO_OUT);
    check_value(&g4, RFS_GPIO_HIGH);
    gpio_close(&g4);
    // Open wrong GPIO pin
    g99.pin = 99;
    g99.flags = 0;
    if (rfs_gpio_open(&g99, RFS_GPIO_IN) != -1) {
        err(1, "open GPIO 99 must give an error but does not");
    }
    return 0;
    // Use wrong direction in open
    g4.pin = 4;
    g4.flags = 0;
    if (rfs_gpio_open(&g99, -1) != -1) {
        err(1, "opening with wrong direction must give an error but does not");
    }
    g4.pin = 4;
    g4.flags = 0;
    if (rfs_gpio_open(&g99, 10) != -1) {
        err(1, "opening with wrong direction must give an error but does not");
    }
    // Double open, the second one without export
    gpio_open(&g4, 4, RFS_GPIO_IN, 0);
    gpio_open(&g4bis, 4, RFS_GPIO_IN, RFS_DONT_EXPORT);
    gpio_close(&g4bis);
    gpio_close(&g4);
    // Set direction
    gpio_open(&g4, 4, RFS_GPIO_IN, 0);
    check_direction(&g4, RFS_GPIO_IN);
    set_direction(&g4, RFS_GPIO_OUT);
    check_direction(&g4, RFS_GPIO_OUT);
    set_direction(&g4, RFS_GPIO_IN);
    check_direction(&g4, RFS_GPIO_IN);
    // Set wrong direction
    if (rfs_gpio_set_direction(&g4, -1) != -1) {
        err(1, "setting wrong direction must give an error but does not");
    }
    if (rfs_gpio_set_direction(&g4, 10) != -1) {
        err(1, "setting wrong direction must give an error but does not");
    }
    // Read direction of an unopened GPIO pin
    g17.pin = 17;
    g17.flags = 0;
    if (rfs_gpio_get_direction(&g17) != -1) {
        err(1, "reading direction must give an error but does not");
    }
    // Set direction of an unopened GPIO pin
    if (rfs_gpio_set_direction(&g17, RFS_GPIO_IN) != -1) {
        err(1, "setting direction must give an error but does not");
    }
    // Set value
    set_direction(&g4, RFS_GPIO_OUT_LOW);
    check_direction(&g4, RFS_GPIO_OUT);
    check_value(&g4, RFS_GPIO_LOW);
    set_value(&g4, RFS_GPIO_HIGH);
    check_value(&g4, RFS_GPIO_HIGH);
    set_value(&g4, RFS_GPIO_LOW);
    check_value(&g4, RFS_GPIO_LOW);
    // Set wrong value
    if (rfs_gpio_set_value(&g4, -1) != -1) {
        err(1, "setting wrong value must give an error but does not");
    }
    if (rfs_gpio_set_value(&g4, 10) != -1) {
        err(1, "setting wrong value must give an error but does not");
    }
    // Read value of an unopened GPIO pin
    if (rfs_gpio_get_value(&g17) != -1) {
        err(1, "reading value must give an error but does not");
    }
    // Set value of an unopened GPIO pin
    if (rfs_gpio_set_value(&g17, RFS_GPIO_HIGH) != -1) {
        err(1, "setting value must give an error but does not");
    }
    // Set edge
    set_direction(&g4, RFS_GPIO_IN);
    check_direction(&g4, RFS_GPIO_IN);
    set_edge(&g4, RFS_GPIO_NONE);
    check_edge(&g4, RFS_GPIO_NONE);
    set_edge(&g4, RFS_GPIO_FALLING);
    check_edge(&g4, RFS_GPIO_FALLING);
    set_edge(&g4, RFS_GPIO_RISING);
    check_edge(&g4, RFS_GPIO_RISING);
    set_edge(&g4, RFS_GPIO_BOTH);
    check_edge(&g4, RFS_GPIO_BOTH);
    // Set wrong edge
    if (rfs_gpio_set_edge(&g4, -1) != -1) {
        err(1, "setting wrong edge must give an error but does not");
    }
    if (rfs_gpio_set_edge(&g4, 10) != -1) {
        err(1, "setting wrong edge must give an error but does not");
    }
    // Read edge of an unopened GPIO pin
    if (rfs_gpio_get_edge(&g17) != -1) {
        err(1, "reading edge must give an error but does not");
    }
    // Set edge of an unopened GPIO pin
    if (rfs_gpio_set_edge(&g17, RFS_GPIO_RISING) != -1) {
        err(1, "setting edge must give an error but does not");
    }
    // Check poll descriptors
    check_poll_descriptors(&g4);
    // Get poll descriptors of an unopened pin
    if (rfs_gpio_get_poll_descriptors(&g17, &pd) != -1) {
        err(1, "getting poll descriptors must give an error but does not");
    }
}

