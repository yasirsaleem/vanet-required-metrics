/**
 * @brief This is vehicle class for the analysis of common required metrics in vehicular networks
 * @author Yasir Saleem (www.yasirsaleem.com)
 */

#include <veins/modules/application/traci/MetricsAnalysisVehicleApp.h>

using namespace veins;

Define_Module(veins::MetricsAnalysisVehicleApp);

void MetricsAnalysisVehicleApp::initialize(int stage)
{
    MetricsAnalysisBaseApp::initialize(stage);
    if (stage == 0) {
        EV << "Initializing " << par("appName").stringValue() << std::endl;

        int simTimeLimitMax = atoi(getEnvir()->getConfig()->getConfigValue("sim-time-limit"));

        newNbMeetTime.push_back(simTime());  // first element will be the time when vehicle entered the road

        numNbsOverTime.resize(simTimeLimitMax+1);

    }
    else if (stage == 1) {
        numCurrentVehicles++;
        totalMaxVehicles++;
    }
}


void MetricsAnalysisVehicleApp::finish()
{
    numCurrentVehicles--;

    // For nb vehicles which are still connected, force stop their connectivity because the current vehicle reached the end of network.
    forceStopConnectivityWithNbs();

    if (logIndividualContactDuration) {
        // Create a log file of individual contact duration
        createIndvContactDurationLogFile();
    }

    if (logIndividualMeetingTime) {
        // Create a log file of individual meeting time with new neighbors
        createIndvNewNbMeetingTimeLogFile();
    }

    for (int j=1; j<newNbMeetTime.size(); j++) {
        simtime_t meetTime = newNbMeetTime[j] - newNbMeetTime[j-1];
        if (meetTime > maxMeetingTimeofNextVehicle) {
            maxMeetingTimeofNextVehicle = meetTime;
        }

        globalNewNbMeetingDiffTime.push_back(meetTime);
    }

    int maxNodeLocalSimTimeInt = round(maxNodeLocalSimTime.dbl());

    for (int simTime=0; simTime<=maxNodeLocalSimTimeInt; simTime++) {
        auto nbDeviceTimeoutIter = allNodesNumNbsOverTimeMap.find(simTime);

        std::vector<int> nbsList;

        if (nbDeviceTimeoutIter == allNodesNumNbsOverTimeMap.end()) {
            // First time
            nbsList.push_back(numNbsOverTime[simTime]);
            allNodesNumNbsOverTimeMap[simTime] = nbsList;
        }
        else {
            nbsList = nbDeviceTimeoutIter->second;
            nbsList.push_back(numNbsOverTime[simTime]);
            allNodesNumNbsOverTimeMap[simTime] = nbsList;
        }
    }

    allConnectivityDetails.clear();
    allConnectivityDetails.shrink_to_fit();

    newNbMeetTime.clear();
    newNbMeetTime.shrink_to_fit();

    numNbsOverTime.clear();
    numNbsOverTime.shrink_to_fit();

}

void MetricsAnalysisVehicleApp::onBSM(DemoSafetyMessage* bsm)
{
    // Received a beacon message from another vehicle

    LAddress::L2Type originatorAddress = bsm->getOriginatorAddress();

    EV_INFO << L2TocModule[myId]->getFullName() << " received beacon from: " << L2TocModule[originatorAddress]->getFullName()
                << "(" << deviceName[bsm->getDeviceType()] << ")" << " at time: " << simTime() << endl;

    EV_INFO << "I(" << myId <<") received a BSM from vehicle: " << originatorAddress << " at time: " << simTime() << endl;

    auto nbDeviceTimeoutIter = nbDeviceTimeoutTimerMap.find(originatorAddress);

    // This is the first time (or after long time) that current vehicle is receiving a beacon from a new neighboring vehicle
    if (nbDeviceTimeoutIter == nbDeviceTimeoutTimerMap.end()) {
        // For probability of meeting a new vehicle
        newNbMeetTime.push_back(simTime());

        // For distribution of contact time of vehicles
        NbConnectivityDetails nbConnDetails;
        nbConnDetails.myId = myId;
        nbConnDetails.nbId = originatorAddress;
        nbConnDetails.startTime = simTime();
        nbConnDetails.endTime = simTime();
        nbConnDetails.duration = 0;

        currentConnectedNbsMap[originatorAddress] = nbConnDetails;
    }

    // Current vehicle has already recently received a beacon from neighboring vehicle
    else {
        // Fetch the NbConnectivityDetails object of current neighbor
        auto currentConnectedNbIter = currentConnectedNbsMap.find(originatorAddress);
        NbConnectivityDetails nbConnDetails = currentConnectedNbIter->second;
        nbConnDetails.endTime = simTime();
        nbConnDetails.duration = simTime() - nbConnDetails.startTime;

        currentConnectedNbsMap[originatorAddress] = nbConnDetails;

        // Cancel the nb timeout timer and delete the entry
        NbConnectivityTimeoutMsg *nbConnectivityTimeoutMsg = nbDeviceTimeoutIter->second;
        ASSERT(nbConnectivityTimeoutMsg->isScheduled());
        cancelAndDelete(nbConnectivityTimeoutMsg);
        nbDeviceTimeoutTimerMap.erase(nbDeviceTimeoutIter);
    }

    // Schedule a new nb timeout timer
    scheduleNbTimoutTimer(originatorAddress, originatorAddress, VEHICLE);
}


void MetricsAnalysisVehicleApp::handlePositionUpdate(cObject* obj)
{
    MetricsAnalysisBaseApp::handlePositionUpdate(obj);
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class
}

// This function is called when a neighbor is out of reach (i.e., moved outside coverage area)
void MetricsAnalysisVehicleApp::nbConnectivityTimeout(NbConnectivityTimeoutMsg *nbConnectivityTimeoutMsg) {
    LAddress::L2Type nbId = nbConnectivityTimeoutMsg->getNbId();

    // Fetch the NbConnectivityDetails object of neighbor
    auto nbIter = currentConnectedNbsMap.find(nbId);
    NbConnectivityDetails nbConnDetails = nbIter->second;

    // Add nb connectivity details in the vector of all nbs connectivity details (for local testing)
    allConnectivityDetails.push_back(nbConnDetails);

    // Add nb connectivity details in the vector of all vehicles' nbs connectivity details (to calculate contact duration of all vehicles)
    globalNbConnectivityDetails.push_back(nbConnDetails);

    // update the max contact duration
    if (nbConnDetails.duration > maxContactDuration) {
        maxContactDuration = nbConnDetails.duration;
    }

    // delete the entry from currentConnectedNbsMap
    currentConnectedNbsMap.erase(nbIter);

    // Cancel the message and delete the entry from the nb timout map
    auto nbTimeoutTimerIter = nbDeviceTimeoutTimerMap.find(nbId);
    if (nbTimeoutTimerIter != nbDeviceTimeoutTimerMap.end()) {
//        cancelEvent(nbTimeoutTimerIter->second);
        cancelAndDelete(nbTimeoutTimerIter->second);
        nbDeviceTimeoutTimerMap.erase(nbTimeoutTimerIter);
    }
}

void MetricsAnalysisVehicleApp::createIndvContactDurationLogFile() {
    std::ofstream contactDurationLog;
    contactDurationLog.open(indvContactDurationFileDir, std::ios::app);

    for (int i=0; i<allConnectivityDetails.size(); i++) {
        NbConnectivityDetails nbConnDetails = allConnectivityDetails[i];

        // seed,scenario,maxSpeed,txPower,myId,nbId,startTime,endTime,contactDuration,datetime
        contactDurationLog << seed <<
                "," << scenarioName <<
                "," << laneMaxSpeed <<
                "," << getTxPower() <<
                "," << nbConnDetails.myId <<
                "," << nbConnDetails.nbId <<
                "," << nbConnDetails.startTime <<
                "," << nbConnDetails.endTime <<
                "," << nbConnDetails.duration <<
                "," << currentDateTime() <<
                endl;
    }

    contactDurationLog.close();
}

void MetricsAnalysisVehicleApp::createIndvNewNbMeetingTimeLogFile() {
    std::ofstream newNbMeetingTimeLog;
    newNbMeetingTimeLog.open(indvMeetingTimeFileDir, std::ios::app);

    for (int j=1; j<newNbMeetTime.size(); j++) {

        simtime_t meetTime = newNbMeetTime[j] - newNbMeetTime[j-1];

        // seed,scenario,maxSpeed,txPower,myId,newNbMeetTimeDiff,datetime
        newNbMeetingTimeLog << seed <<
                "," << scenarioName <<
                "," << laneMaxSpeed <<
                "," << getTxPower() <<
                "," << myId <<
                "," << meetTime <<
                "," << currentDateTime() <<
                endl;
    }

    newNbMeetingTimeLog.close();
}

/**
 * @brief: Force stop the connectivity of vehicle with its current neighbors.
 * This is to be done when a vehicle leaves the network (i.e., in finish() function).
 */
void MetricsAnalysisVehicleApp::forceStopConnectivityWithNbs() {
    std::map<LAddress::L2Type, NbConnectivityDetails> currentConnectedNbsMapCopy = currentConnectedNbsMap;

    if (currentConnectedNbsMapCopy.size() > 0) {
        std::map<LAddress::L2Type, NbConnectivityDetails>::iterator iter;
        for (iter = currentConnectedNbsMapCopy.begin(); iter != currentConnectedNbsMapCopy.end(); iter++) {
            NbConnectivityDetails nbConnDetails = iter->second;

            nbConnDetails.endTime = simTime();
            nbConnDetails.duration = simTime() - nbConnDetails.startTime;

            currentConnectedNbsMap[nbConnDetails.nbId] = nbConnDetails;


            NbConnectivityTimeoutMsg *forceStopConnectivityMsg = new NbConnectivityTimeoutMsg("Forcing to stop connectivity because it reaches end of the network");
            forceStopConnectivityMsg->setOffloadingVehicleId(nbConnDetails.nbId);
            forceStopConnectivityMsg->setNbId(nbConnDetails.nbId);
            forceStopConnectivityMsg->setNbDeviceType(VEHICLE);

            nbConnectivityTimeout(forceStopConnectivityMsg);

            cancelAndDelete(forceStopConnectivityMsg);
        }
    }
}
