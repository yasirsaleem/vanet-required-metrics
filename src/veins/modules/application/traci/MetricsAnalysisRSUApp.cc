/**
 * @brief This is RSU class for the analysis of common required metrics in vehicular networks
 * @author Yasir Saleem (www.yasirsaleem.com)
 */

#include <veins/modules/application/traci/MetricsAnalysisRSUApp.h>

using namespace veins;

Define_Module(veins::MetricsAnalysisRSUApp);

void MetricsAnalysisRSUApp::initialize(int stage)
{
    MetricsAnalysisBaseApp::initialize(stage);
    if (stage == 0) {
        // Nothing is required here when stage=0
    }
    else if (stage == 1) {
        if (L2TocModule.find(myId) != L2TocModule.end() && strcmp("rsu[0]", L2TocModule[myId]->getFullName()) == 0) {

            // Initialize a timer for taking per second actions, e.g., count num vehicles, count num nbs, find max layer, find num avg children, find num gateways
            perSecondNbCountTimer = new cMessage("Per second nb counting timer", PER_SECOND_NB_COUNT_TIMER);
            simtime_t firstPerSecondNbCountMsg = simTime() + 1;
            scheduleAt(firstPerSecondNbCountMsg, perSecondNbCountTimer);

            // Add header in the log file of individual contact duration of vehicles
            std::string postfix = "-" + std::to_string(getTxPower()) + "mW-" + scenarioName + ".csv";

            perSecondNumVehiclesAndNumNbsFileDir = baseDirLogFiles + prefixLogFilename + "perSecondNumVehiclesAndNumAvgNbs" + postfix;
            indvContactDurationFileDir = baseDirLogFiles + prefixLogFilename + "indvContactDurationLog" + postfix;
            indvMeetingTimeFileDir = baseDirLogFiles + prefixLogFilename + "indvNewNbMeetingTimeLog" + postfix;
            indvPerSecondNbContactDurationLogFile = baseDirLogFiles + prefixLogFilename + "indvPerSecondNbContactDurationLog" + postfix;
            aggrContactDurationFileDir = baseDirLogFiles + prefixLogFilename + "aggrContactDurationLog" + postfix;
            aggrMeetingTimeFileDir = baseDirLogFiles + prefixLogFilename + "aggrMeetingTimeLog" + postfix;

            if (logPerSecondNumVehiclesAndNumNeighbors) {
                std::ifstream fileExits(perSecondNumVehiclesAndNumNbsFileDir.c_str());
                if (!fileExits.good()) {
                    std::ofstream perSecondLog;
                    perSecondLog.open(perSecondNumVehiclesAndNumNbsFileDir, std::ios::app);
                    perSecondLog << "seed,scenario,totalVehicles,maxSpeed,txPower,simTime,numVehicles,numAvgNbs,datetime" << endl;
                    perSecondLog.close();
                }
            }

            if (logIndividualContactDuration) {
                std::ifstream fileExits(indvContactDurationFileDir.c_str());
                if (!fileExits.good()) {
                    std::ofstream contactDurationLog;
                    contactDurationLog.open(indvContactDurationFileDir, std::ios::app);
                    contactDurationLog << "seed,scenario,maxSpeed,txPower,myId,nbId,startTime,endTime,contactDuration,datetime" << endl;
                    contactDurationLog.close();
                }
            }

            if (logIndividualMeetingTime) {
                std::ifstream fileExits(indvMeetingTimeFileDir.c_str());
                if (!fileExits.good()) {
                    std::ofstream newNbMeetingTimeLog;
                    newNbMeetingTimeLog.open(indvMeetingTimeFileDir, std::ios::app);
                    newNbMeetingTimeLog << "seed,scenario,maxSpeed,txPower,myId,newNbMeetTimeDiff,datetime" << endl;
                    newNbMeetingTimeLog.close();
                }
            }

            if (logIndividualPerSecondNeighborContactDuration) {
                std::ifstream fileExits(indvPerSecondNbContactDurationLogFile.c_str());
                if (!fileExits.good()) {
                    std::ofstream nbContactDurationLog;
                    nbContactDurationLog.open(indvPerSecondNbContactDurationLogFile, std::ios::app);
                    nbContactDurationLog << "seed,scenario,maxSpeed,txPower,simTime,myId,nbId,contactDuration,datetime" << endl;
                    nbContactDurationLog.close();
                }

            }

            if (logAggregatedContactuDuration) {
                std::ifstream fileExits(aggrContactDurationFileDir.c_str());
                if (!fileExits.good()) {
                    std::ofstream aggrContactDurationLog;
                    aggrContactDurationLog.open(aggrContactDurationFileDir, std::ios::app);
                    aggrContactDurationLog << "seed,scenario,totalVehicles,maxSpeed,txPower,simTime,contactTime,datetime" << endl;
                    aggrContactDurationLog.close();
                }
            }

            if (logAggregatedMeetingTime) {
                std::ifstream fileExits(aggrMeetingTimeFileDir.c_str());
                if (!fileExits.good()) {
                    std::ofstream aggrNewNbMeetingTimeLog;
                    aggrNewNbMeetingTimeLog.open(aggrMeetingTimeFileDir, std::ios::app);
                    aggrNewNbMeetingTimeLog << "seed,scenario,totalVehicles,maxSpeed,txPower,simTime,meetingTime,datetime" << endl;
                    aggrNewNbMeetingTimeLog.close();
                }
            }

        }
    }
}

void MetricsAnalysisRSUApp::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind()) {
        case PER_SECOND_NB_COUNT_TIMER: {
            takePerSecondCountNbActionByRSU();
            break;
        }
        default: {
            MetricsAnalysisBaseApp::handleSelfMsg(msg);
        }
    }

}

void MetricsAnalysisRSUApp::finish()
{
    if (L2TocModule.find(myId) != L2TocModule.end() && strcmp("rsu[0]", L2TocModule[myId]->getFullName()) == 0) {

        if (logAggregatedContactuDuration) {
            std::ofstream contactDistrLog;
            contactDistrLog.open(aggrContactDurationFileDir, std::ios::app);

            int maxContactDurationInt = round(maxContactDuration.dbl());
            std::vector<int> distr_numVehicle_duration(maxContactDurationInt + 1);
            double numValidRecordsContactDistr = 0;

            for (int i=0; i<globalNbConnectivityDetails.size(); i++) {
                NbConnectivityDetails nbConnDetails = globalNbConnectivityDetails[i];
                int duration = round((nbConnDetails.duration).dbl());
    //            if (duration > maxContactDurationInt) {
    //                break;
    //            }
    //            else {
    //            if (duration > 0) {
                    distr_numVehicle_duration[duration]++;
                    numValidRecordsContactDistr++;
    //            }
            }



            for (int j=0; j<distr_numVehicle_duration.size(); j++) {
    //            if (distr_numVehicle_duration[j] > 0) {
        //            contactDistrLog << j << "," << distr_numVehicle_duration[j] << endl;
                contactDistrLog << seed <<
                        "," << scenarioName <<
                        "," << numVehicles <<
                        "," << laneMaxSpeed <<
                        "," << getTxPower() <<
                        "," << j <<
                        "," << (distr_numVehicle_duration[j]/numValidRecordsContactDistr)*100 <<
                        "," << currentDateTime() <<
                        endl;
        //            EV_INFO << j << ". " << distr_numVehicle_duration[j] << endl;
    //            }
            }

            contactDistrLog.close();
        }

        if (logAggregatedMeetingTime) {
            std::ofstream meetingDistrLog;
            meetingDistrLog.open(aggrMeetingTimeFileDir, std::ios::app);

            int maxMeetingTimeofNextVehiclenInt = round(maxMeetingTimeofNextVehicle.dbl());
            std::vector<int> distr_MeetingTime_numVehicles(maxMeetingTimeofNextVehiclenInt + 1);
            double numValidRecordsMeetingDistr = 0;

            for (int k=0; k<globalNewNbMeetingDiffTime.size(); k++) {
                int meetingDiffTime = round((globalNewNbMeetingDiffTime[k]).dbl());
    //            if (meetingDiffTime > 0) {
                    distr_MeetingTime_numVehicles[meetingDiffTime]++;
                    numValidRecordsMeetingDistr++;
    //            }

            }

            for (int l=0; l<distr_MeetingTime_numVehicles.size(); l++) {
    //            if (distr_MeetingTime_numVehicles[l] > 0) {
        //            meetingDistrLog << l << "," << distr_MeetingTime_numVehicles[l] << endl;
                    meetingDistrLog << seed <<
                            "," << scenarioName <<
                            "," << numVehicles <<
                            "," << laneMaxSpeed <<
                            "," << getTxPower() <<
                            "," << l <<
                            "," << (distr_MeetingTime_numVehicles[l]/numValidRecordsMeetingDistr)*100 <<
                            "," << currentDateTime() <<
                            endl;
    //            }
            }

            meetingDistrLog.close();
        }
    }

}

void MetricsAnalysisRSUApp::createPerSecondNumNbLogFile() {

    std::ofstream perSecondLogFile;
    perSecondLogFile.open(perSecondNumVehiclesAndNumNbsFileDir, std::ios::app);

    std::map<int, PerSecondLogStruct>::iterator iter;
    for (iter = perSecondLogMap.begin(); iter != perSecondLogMap.end(); iter++) {
        int simTime = iter->first;
        PerSecondLogStruct perSecondValue = iter->second;

        // seed,scenario,totalVehicles,maxSpeed,txPower,simTime,numVehicles,NumAvgNbs
        perSecondLogFile << seed
                << "," << scenarioName
                << "," << perSecondValue.totalVehicles
                << "," << laneMaxSpeed
                << "," << getTxPower()
                << "," << simTime
                << "," << perSecondValue.numCurrentVehicles
                << "," << perSecondValue.numAvgNbs
                << "," << currentDateTime()
                << endl;
    }

    perSecondLogFile.close();
}

void MetricsAnalysisRSUApp::createPerSecondNumNbLogFile_IndividualRecord(PerSecondLogStruct perSecondValue) {

    std::ofstream perSecondLogFile;
    perSecondLogFile.open(perSecondNumVehiclesAndNumNbsFileDir, std::ios::app);


    // seed,scenariototalVehicles,maxSpeed,txPower,simTime,numVehicles,NumAvgNbs,datetime
    perSecondLogFile << seed
            << "," << scenarioName
            << "," << perSecondValue.totalVehicles
            << "," << laneMaxSpeed
            << "," << getTxPower()
            << "," << round(simTime().dbl())
            << "," << perSecondValue.numCurrentVehicles
            << "," << perSecondValue.numAvgNbs
            << "," << currentDateTime()
            << endl;

    perSecondLogFile.close();
}

void MetricsAnalysisRSUApp::createPerSecondNbContactDurationLogFile_IndividualRecords(std::vector<NbContactDurationLogStruct> nbContactDurationsVector) {
    std::ofstream nbContactDurationLog;
    nbContactDurationLog.open(indvPerSecondNbContactDurationLogFile, std::ios::app);

    for (int i=0; i<nbContactDurationsVector.size(); i++) {

        NbContactDurationLogStruct nbContactDuration = nbContactDurationsVector[i];

        // seed,scenariomaxSpeed,txPower,simTime,myId,nbId,contactDuration,datetime
        nbContactDurationLog << seed <<
                "," << scenarioName <<
                "," << laneMaxSpeed <<
                "," << getTxPower() <<
                "," << round(simTime().dbl()) <<
                "," << nbContactDuration.myId <<
                "," << nbContactDuration.nbId <<
                "," << nbContactDuration.contactDuration <<
                "," << currentDateTime() <<
                endl;
    }

    nbContactDurationLog.close();
}

/**
 * @brief: This is a centralized function executed at RSU 0.
 * It counts the number of neighbors of each vehicle and stores in a struct.
 */
void MetricsAnalysisRSUApp::takePerSecondCountNbActionByRSU() {

    EV_INFO << "Inside takePerSecondCountNbActionByRSU() function" << endl;


    // If current node is not RSU 0, then return because this is a centralized function executed only at RSU 0.
    if (L2TocModule.find(myId) != L2TocModule.end() && strcmp("rsu[0]", L2TocModule[myId]->getFullName()) != 0) {
        return;
    }

    int currentTime = round(simTime().dbl());
    int sumNeighbors = 0;

    std::cout << "Per Second Action at simTime: " << currentTime << " and currentTime: " << currentDateTime() << endl;

    // a vector of NbContactDurationLogStruct for storing the information of my id, nb id, contact duration of all the vehicles at current sim time
    std::vector<NbContactDurationLogStruct> nbContactDurationLogVector;

    // Run a loop on current number of vehicles to fetch the pointer of each vehicle
    for (int i=0; i<totalMaxVehicles; i++) {
        std::string vehicleName = "node[" + std::to_string(i) + "]";
        cModule *vehiclePointer = getModuleByPath(vehicleName.c_str());

        // if node pointer is null, then continue to the next vehicle in the loop
        if (vehiclePointer == nullptr) {
           continue;
        }

//        EV_INFO << "I am " << vehiclePointer->getFullName() << endl;

        MetricsAnalysisVehicleApp* vehicleModule = FindModule<MetricsAnalysisVehicleApp*>::findSubModule(vehiclePointer);

        // find number of average neighbors
        sumNeighbors += vehicleModule->currentConnectedNbsMap.size();

        // add current vehicle id (my id), all its nbs id and contact duration into the vector nbContactDurationLogVector
        std::map<LAddress::L2Type, NbConnectivityDetails>::iterator iter;

        for (iter = vehicleModule->currentConnectedNbsMap.begin(); iter != vehicleModule->currentConnectedNbsMap.end(); iter++) {
            NbContactDurationLogStruct nbContactDurationTemp;
            NbConnectivityDetails nbConnectivityTemp = iter->second;
            nbContactDurationTemp.myId = nbConnectivityTemp.myId;
            nbContactDurationTemp.nbId = nbConnectivityTemp.nbId;
            nbContactDurationTemp.contactDuration = nbConnectivityTemp.duration;

            nbContactDurationLogVector.push_back(nbContactDurationTemp);
        }

    }

    PerSecondLogStruct perSecondLog;
    perSecondLog.totalVehicles = totalMaxVehicles;
    perSecondLog.numCurrentVehicles = numCurrentVehicles;
    if (numCurrentVehicles > 0) {
        perSecondLog.numAvgNbs = sumNeighbors/numCurrentVehicles;
    }

    // Add the log into the map of each simulation time
    perSecondLogMap[currentTime] = perSecondLog;

    if (totalMaxVehicles > 0) {
        if (logPerSecondNumVehiclesAndNumNeighbors) {
            // Write the log of current second into log file
            createPerSecondNumNbLogFile_IndividualRecord(perSecondLog);
        }

        if (logIndividualPerSecondNeighborContactDuration) {
            // Write the contact duration of each neighbor for all vehicles into log file
            createPerSecondNbContactDurationLogFile_IndividualRecords(nbContactDurationLogVector);
        }
    }
    // Schedule the timer again after one second
    scheduleAt(simTime() + 1, perSecondNbCountTimer);
}
