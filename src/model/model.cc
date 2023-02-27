#include "model.h"
#include "config.h"

/*
 *  A simple linear power band
 */

static int revs = 0;

void rev_up() {
    revs += power;
    if(revs >= 6000)
        revs = 6000;
}

void rev_down() {
    revs -= power;
    if(revs < 0)
        revs = 0;
}

int get_revs() {
    return revs;
}
