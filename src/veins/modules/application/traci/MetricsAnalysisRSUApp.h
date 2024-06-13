/**
 * @brief This is RSU class for the analysis of common required metrics in vehicular networks
 * @author Yasir Saleem (www.yasirsaleem.com)
 */

//#pragma once

#include <veins/modules/application/traci/MetricsAnalysisVehicleApp.h>

namespace veins {

class VEINS_API MetricsAnalysisRSUApp : public MetricsAnalysisBaseApp {
protected:

    struct PerSecondLogStruct
    {
        int totalVehicles = 0;
        int numCurrentVehicles = 0;
        int numAvgNbs = 0;
    };

    struct NbContactDurationLogStruct
    {
        LAddress::L2Type myId = -1;
        LAddress::L2Type nbId = -1;
        simtime_t contactDuration = 0.0;
    };

    void initialize(int stage) override;
    void finish() override;

    void handleSelfMsg(cMessage* msg) override;

    void takePerSecondCountNbActionByRSU();
    void createPerSecondNumNbLogFile();
    void createPerSecondNumNbLogFile_IndividualRecord(PerSecondLogStruct perSecondValue);
    void createPerSecondNbContactDurationLogFile_IndividualRecords(std::vector<NbContactDurationLogStruct> nbContactDurationsVector);

protected:
    cMessage* perSecondNbCountTimer;

    std::map<int, PerSecondLogStruct> perSecondLogMap;
};

}
