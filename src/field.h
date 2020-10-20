//
// Created by woranhun on 2020. 10. 17..
//

#ifndef VIRUS_FIELD_H
#define VIRUS_FIELD_H


#include <map>
#include <queue>
#include <sstream>

class Field {
    int fieldID{};
    int currentInfectionValue{};
    int vaccinationRate{};
    int populationDensity{};
    std::map<int,int> storedVaccines;
    std::queue<int> lastInfectionValues;
public:
    Field()=default;
    Field(const int fieldID,const int currentInfectionValue, const int vaccinationRate, const int populationDensity){
        this->fieldID=fieldID;
        this->currentInfectionValue =currentInfectionValue;
        this->vaccinationRate =vaccinationRate;
        this->populationDensity = populationDensity;
        this->storedVaccines = std::map<int,int>();
        this->lastInfectionValues = std::queue<int>();
    }
    Field(const Field& f){
        *this = f;
    }
    Field& operator=(const Field& f){
        if(this != &f){
            this->fieldID=f.fieldID;
            this->lastInfectionValues = f.lastInfectionValues;
            this->currentInfectionValue = f.currentInfectionValue;
            this->storedVaccines = f.storedVaccines;
            this->populationDensity = f.populationDensity;
            this->vaccinationRate = f.vaccinationRate;
        }
        return *this;
    }
    int getFieldID()const{return this->fieldID;}
    friend std::ostream& operator<<(std::ostream & os,const Field& f);
};


#endif //VIRUS_FIELD_H
