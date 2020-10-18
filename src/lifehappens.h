//
// Created by lukac on 10/18/2020.
//

#ifndef VIRUS_LIFEHAPPENS_H
#define VIRUS_LIFEHAPPENS_H

#include <vector>
#include "grid.h"
#include "random.h"
#include <algorithm>
//for debug purposes
#include <iostream>
#include <stdexcept>

class LifeHappens {
private:
    int floor(int n) {
        return n - (n % 1);
    }
    int ceil(int n){
        return floor(n)+1;
    }
    int min(int n, int m) {
        return n < m ? n : m;
    }
    int distance(coord, c);
public:
    //separate functions, so values can be changed from the visualization directly
    int calculateSpontaneousHealing(Field field, int currentTick, int healStartTick, unsigned long random1);

    void applySpontaneousHealing(int value, Field field);

    int calculateSpontaneousInfection(Grid *grid, unsigned long random2, unsigned long random3, unsigned long random4);

    void applySpontaneousInfection(int value);
};


#endif //VIRUS_LIFEHAPPENS_H
