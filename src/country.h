//
// Created by woranhun on 2020. 10. 17..
//

#ifndef VIRUS_COUNTRY_H
#define VIRUS_COUNTRY_H


#include <vector>
#include <iostream>

class Country {
    size_t countryID{};
    int totalProductionCapacity{};
    int reserveVaccines{};
    std::vector<size_t> assignedDistrictIDs;
public:
    Country(size_t ID, std::vector<size_t> districts) {
        countryID=ID;
        assignedDistrictIDs = districts;
    }

    Country() {
        assignedDistrictIDs = std::vector<size_t>();
    }

    Country(const Country &c) {
        *this = c;
    }

    Country &operator=(const Country &c) {
        if (this != &c) {
            countryID = c.countryID;
            totalProductionCapacity = c.totalProductionCapacity;
            reserveVaccines = c.reserveVaccines;
            assignedDistrictIDs = c.assignedDistrictIDs;
        }
        return *this;
    }

    std::vector<size_t> getAssignedDistrictIDs() const { return assignedDistrictIDs; }

    friend std::ostream &operator<<(std::ostream &os, const Country &c);

    int getTotalProductionCapacity() const { return totalProductionCapacity; }

    void setTotalProductionCapacity(int tpc) { totalProductionCapacity = tpc; }

    int getReserveVaccines() const { return reserveVaccines; }

    void setReserveVaccines(int rv) { reserveVaccines = rv; }
};


#endif //VIRUS_COUNTRY_H
