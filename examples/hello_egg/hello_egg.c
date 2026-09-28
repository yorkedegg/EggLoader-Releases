#include "host.h"

static int on_blocks(const EggHost *host, void *argument) {
    (void)argument;
    host->log("hello_egg: blocks event");
    return 0;
}

int egg_module_entry(const EggHost *host) {
    if (!host || host->magic != EGG_HOST_MAGIC || host->api < 15 ||
        host->size < sizeof(*host) || !host->log || !host->register_event)
        return 1;
    host->log("hello_egg: loaded");
    return host->register_event(host, EGG_EVENT_BLOCKS, on_blocks);
}
