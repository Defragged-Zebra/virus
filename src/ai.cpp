//
// Created by lukac on 11/6/2020.
//

/*
Overview of AI
 -calculateDistrictScoresForNextRound()
 -chooseDistrictsToHeal()



*/

#include "ai.h"

uint64_t fuckCpp[4] = {0};
Grid AI::grid2 = Grid(0, 0, fuckCpp);


void AI::calculateDistrictScoresForNextRound(size_t countryID, std::vector<ScoreHolder> &districtScores) {
    if (grid2.getCurrentTick() == 1) {
        startFromGridBorder(countryID, districtScores);
    } else {
        startFromExistingDistricts(countryID, districtScores);
    }
}

void AI::startFromExistingDistricts(size_t countryID, std::vector<ScoreHolder> &districtScores) {
    auto districts = grid2.getCountryByID(countryID).getAssignedDistricts();
    for (auto d:districts) {
        for (auto neighbourD:d->getNeighbourDistricts()) {
            if (grid2.getDistrictByID(neighbourD).isClear()) {
                continue;
            } else {
                calculateScore(districtScores, grid2.getDistrictByID(neighbourD), countryID);
            }
        }
    }
}

void AI::startFromGridBorder(size_t countryID, std::vector<ScoreHolder> &districtScores) {
    for (size_t x = 1; x < grid2.getWidth() - 1; ++x) {
        District &district = grid2.getDistrictByPoint(Point(0, x));
        calculateScore(districtScores, district, countryID);

        District &district2 = grid2.getDistrictByPoint(Point(grid2.getHeight() - 1, x));
        calculateScore(districtScores, district2, countryID);
    }
    for (size_t y = 0; y < grid2.getHeight(); ++y) {
        District &district = grid2.getDistrictByPoint(Point(y, 0));
        calculateScore(districtScores, district, countryID);

        District &district2 = grid2.getDistrictByPoint(Point(y, grid2.getWidth() - 1));
        calculateScore(districtScores, district2, countryID);
    }
    //remove duplicates
    std::sort(districtScores.begin(), districtScores.end());
    std::unique(districtScores.begin(), districtScores.end());
}

void AI::calculateScore(std::vector<ScoreHolder> &districtScores, const District &district, size_t countryID) {
    if (!district.isClear()) {
        auto score = Utils::ScoreHolder(district.getDistrictID());
        int vaccinesNeededForTotalHealing = 0;
        for (auto fieldPointer:district.getAssignedFields()) {
            //assuming grid is 1 tick in the future, and no unhealed district in the country
            vaccinesNeededForTotalHealing += std::max((int) std::ceil(
                    (fieldPointer->getCurrentInfectionRate() - fieldPointer->getVaccinationRate()) /
                    fieldPointer->getPopulationDensity()), 6 - fieldPointer->getPopulationDensity());
        }
        int changeInVaccines = grid2.calculateChangeInProducedVaccinesByHealingDistrict(countryID,
                                                                                        district); //todo: +aStarPathVaccineCost;
        //todo: store path to district (prob in scoreHolder as well?)
        score = Utils::ScoreHolder(changeInVaccines, vaccinesNeededForTotalHealing, district.getDistrictID());
        districtScores.push_back(score);
    }
}


std::vector<VaccineData> &
AI::calculateBackVaccines(std::vector<VaccineData> &back, int &numberOfVaccinesToDistribute, size_t countryID) {
    //TODO: ezt az egeszet itt refactorolni kell majd
    //ToDo We should take back vaccines only if the district is healed.
    //ToDO We should check only the cells where we put vaccines in the past.
    for (int y = 0; y < grid2.getHeight(); ++y) {
        for (int x = 0; x < grid2.getWidth(); ++x) {
            std::map<size_t, int> allStoredVaccines = grid2.getFieldByPoint(Point(y, x)).getStoredVaccines();
            int countryStoredVaccines;
            try { countryStoredVaccines = allStoredVaccines.at(countryID); }
            catch (std::out_of_range &exc) { countryStoredVaccines = 0; }
            //Egy területről az összes tartalék vakcinát nem lehet visszavenni, legalább 1 egységnyit ott kell hagyni.
            if (countryStoredVaccines > 1) {
                back.emplace_back(Point(y, x), countryStoredVaccines - 1, countryID);
                //numberOfVaccinesToDistribute += countryStoredVaccines - 1;
            }
        }
    }
    return back;
}

std::vector<VaccineData> AI::chooseFieldsToVaccinate(int numberOfVaccinesToDistribute, size_t countryID) {
    std::vector<ScoreHolder> data;
    std::vector<VaccineData> fieldsToHealSendBack;
    Grid *originalGrid = Logic::getGrid();

    //simulate next round
    Logic::setGrid(&AI::grid2);
    Logic::simulateTO(0, grid2.getCurrentTick() + 1, countryID);
    Logic::setGrid(originalGrid);

    //check if grid is clear
    grid2.updateClearByFieldCheck();
    if (grid2.isClear()) return std::vector<VaccineData>();

    //calculate district scores
    AI::calculateDistrictScoresForNextRound(countryID, data);

    //calculate fields to heal
    //ToDo Filter out districts which cannot be reached. -- this is done I think
    //ToDO A* to make a path to all districts
    numberOfVaccinesToDistribute = modeA(numberOfVaccinesToDistribute, countryID, data, fieldsToHealSendBack);
    if (fieldsToHealSendBack.empty()) {
        modeB(numberOfVaccinesToDistribute, countryID, data, fieldsToHealSendBack);
    }

    //TODO: refactor start points after first round
    if (originalGrid->getCurrentTick() == 0) {
        auto district = originalGrid->getDistrictByPoint(fieldsToHealSendBack[0].getPoint());
        originalGrid->getCountryByID(countryID).addAssignedDistrict(&district);
    }
    return fieldsToHealSendBack;
}

int AI::modeA(int numberOfVaccinesToDistribute, size_t countryID, std::vector<ScoreHolder> &data,
              std::vector<VaccineData> &fieldsToHealSendBack) {
    std::priority_queue<ScoreHolder, std::vector<ScoreHolder>, Compare::ProfIndex> districtScores(data.begin(),
                                                                                                  data.end());
    while (!districtScores.empty()) {
        Utils::ScoreHolder maxScoredDistrict = districtScores.top();
        std::set<Field *> fieldsToHeal = grid2.getDistrictByID(maxScoredDistrict.getDistrictID()).getAssignedFields();
        Point fromWhere = calculateStartPoint(fieldsToHeal, countryID);
        std::vector<Field *> fieldsToHealContinuous;
        floodDistrict(fromWhere, fieldsToHeal, fieldsToHealContinuous);
        //ToDo check if fieldsToHeal is not empty --> nem volt folytonos a terület
        //check proposed by woranhun WARNING in extreme cases it can make problem
        if (maxScoredDistrict.ChangeInVaccines() < 0) break;
        if (numberOfVaccinesToDistribute >= maxScoredDistrict.getVaccinesNeededForHealing()) {
            //get the fields of the district
            for (const auto &field:fieldsToHealContinuous) {
                int vaccines = std::max((int) std::ceil(
                        (field->getCurrentInfectionRate() - field->getVaccinationRate()) /
                        field->getPopulationDensity()), 6 - field->getPopulationDensity());
                if (vaccines > 0) {
                    VaccineData vc = VaccineData(grid2.getCoordinatesByID(field->getFieldID()), vaccines, countryID);
                    //TODO: a-star algo modifies here too
                    fieldsToHealSendBack.push_back(vc);
                }
            }
            numberOfVaccinesToDistribute -= districtScores.top().getVaccinesNeededForHealing();
        }
        if (numberOfVaccinesToDistribute == 0) break;
        districtScores.pop();
    }
    return numberOfVaccinesToDistribute;
}

void AI::modeB(int numberOfVaccinesToDistribute, size_t countryID, std::vector<ScoreHolder> &data,
               std::vector<VaccineData> &fieldsToHealSendBack) {//mode B - get the easiest district and try to heal it
    std::priority_queue<ScoreHolder, std::vector<ScoreHolder>, Compare::TotalHealing> districtScores2(data.begin(),
                                                                                                      data.end());
    Utils::ScoreHolder maxScoredDistrict = districtScores2.top();
    std::set<Field *> fieldsToHeal = grid2.getDistrictByID(maxScoredDistrict.getDistrictID()).getAssignedFields();
    Point fromWhere = calculateStartPoint(fieldsToHeal, countryID);
    std::vector<Field *> fieldsToHealContinuous;
    floodDistrict(fromWhere, fieldsToHeal, fieldsToHealContinuous);
    for (auto field:fieldsToHealContinuous) {
        int vaccines = std::max((int) std::ceil(
                (field->getCurrentInfectionRate() - field->getVaccinationRate()) /
                field->getPopulationDensity()), 6 - field->getPopulationDensity());
        if (vaccines > 0) {
            if (numberOfVaccinesToDistribute - vaccines < 0) break;
            VaccineData vc = VaccineData(grid2.getCoordinatesByID(field->getFieldID()), vaccines, countryID);
            //TODO: a-star algo modifies here too
            fieldsToHealSendBack.push_back(vc);
        }
        numberOfVaccinesToDistribute -= vaccines;
    }
}

Point AI::calculateStartPoint(const std::set<Field *> &fieldsToCalc, size_t countryID) {
    Grid *g = Logic::getGrid();
    for (const auto field:fieldsToCalc) {
        const Point &p = g->getCoordinatesByID(field->getFieldID());
        if (g->getCountryByID(countryID).isNeighbourVaccinatedFields(p)) return p;
    }
    throw std::runtime_error("calculateStartPointFailed -- you tried to heal an invalid district");
    //return g->getCoordinatesByID((*fieldsToCalc.begin())->getFieldID());
}


std::vector<VaccineData> &
AI::calculatePutVaccines(std::vector<VaccineData> &put, int numberOfVaccinesToDistribute, size_t countryID) {
    put = chooseFieldsToVaccinate(numberOfVaccinesToDistribute, countryID);
    Country &c = Logic::getGrid()->getCountryByID(countryID);
    for (const auto &vd:put) c.addToVaccinatedFields(vd.getPoint());
    return put;
}

//floood kap egy Field tömböt + egy pontot, ezt olyan sorrandbe rendezni, hogy lerakható legyen.
void
AI::floodDistrict(const Point &p, std::set<Field *> &notVisitedFields, std::vector<Field *> &orderedFields) {
    if (!p.withinBounds())return;
    //Ez itt egy lamda függvény :(
    auto centerIter = std::find_if(notVisitedFields.begin(), notVisitedFields.end(), [p](Field *const &obj) {
        return grid2.getCoordinatesByID(obj->getFieldID()) == p;
    });
    if (centerIter == notVisitedFields.end()) return;
    orderedFields.emplace_back(*centerIter);
    notVisitedFields.erase(centerIter);
    for (const auto &selected:p.getNeighbours()) {
        if (!selected.withinBounds())continue;
        if (grid2.getDistrictByPoint(p) == grid2.getDistrictByPoint(selected)) {
            floodDistrict(selected, notVisitedFields, orderedFields);
        }
    }
}

