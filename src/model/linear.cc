#include "linear.h"

/*
 *  A simple linear power band
 */

LinearModel::LinearModel(int p) {
    power = p;
}

int LinearModel::get_revs() {
    return revs;
}

void LinearModel::rev_up() {
    revs += power;
    if(revs >= 6000)
        revs = 6000;
}

void LinearModel::rev_down() {
    revs -= power;
    if(revs < 0)
        revs = 0;
}