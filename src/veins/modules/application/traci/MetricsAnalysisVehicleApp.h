/**
 * @brief This is vehicle class for the analysis of common required metrics in vehicular networks
 * @author Yasir Saleem (www.yasirsaleem.com)
 */

//#pragma once

#include <veins/modules/application/traci/MetricsAnalysisBaseApp.h>
#include "veins/veins.h"

using namespace omnetpp;

namespace veins {

class VEINS_API MetricsAnalysisVehicleApp : public MetricsAnalysisBaseApp {
public:
    void initialize(int stage) override;
    void finish() override;

protected:

    void onBSM(DemoSafetyMessage* bsm) override;

    void handlePositionUpdate(cObject* obj) override;

    void nbConnectivityTimeout(NbConnectivityTimeoutMsg *nbConnectivityTimeoutMsg) override;

    void createIndvContactDurationLogFile();
    void createIndvNewNbMeetingTimeLogFile();
    void forceStopConnectivityWithNbs();

public:
    std::map<LAddress::L2Type, NbConnectivityDetails> currentConnectedNbsMap;     // map<activeNb, ConnectionDetails>
    std::vector<NbConnectivityDetails> allConnectivityDetails;

    std::vector<simtime_t> newNbMeetTime;   // first element will be the time when vehicle entered the road
    std::vector<simtime_t> nextNbsMeetingDiffTime;
    cMessage* countNumNbsTimer;
    std::vector<int> numNbsOverTime;
    simtime_t maxNodeLocalSimTime;



};

}
