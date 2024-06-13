/**
 * @brief This is base class for the analysis of common required metrics in vehicular networks
 * @author Yasir Saleem (www.yasirsaleem.com)
 */

#include <veins/modules/application/traci/MetricsAnalysisBaseApp.h>
#include <fstream> // for ofstream
#include <chrono>       // std::chrono::seconds

using namespace veins;

Define_Module(veins::MetricsAnalysisBaseApp);

/* Global static variables */
std::map<LAddress::L2Type, cModule*> veins::MetricsAnalysisBaseApp::L2TocModule;
std::map<cModule*, LAddress::L2Type> veins::MetricsAnalysisBaseApp::cModuleToL2;
std::map<LAddress::L2Type, const char*> veins::MetricsAnalysisBaseApp::L2ToFullname;
int veins::MetricsAnalysisBaseApp::numVehicles;
double veins::MetricsAnalysisBaseApp::laneMaxSpeed;
std::vector<veins::MetricsAnalysisBaseApp::NbConnectivityDetails> veins::MetricsAnalysisBaseApp::globalNbConnectivityDetails;
std::vector<simtime_t> veins::MetricsAnalysisBaseApp::globalNewNbMeetingDiffTime;
std::map<int, std::vector<int>> veins::MetricsAnalysisBaseApp::allNodesNumNbsOverTimeMap;
int veins::MetricsAnalysisBaseApp::numCurrentVehicles;
simtime_t veins::MetricsAnalysisBaseApp::maxContactDuration;
simtime_t veins::MetricsAnalysisBaseApp::maxMeetingTimeofNextVehicle;
int veins::MetricsAnalysisBaseApp::totalMaxVehicles;
std::string veins::MetricsAnalysisBaseApp::perSecondNumVehiclesAndNumNbsFileDir;
std::string veins::MetricsAnalysisBaseApp::indvContactDurationFileDir;
std::string veins::MetricsAnalysisBaseApp::indvMeetingTimeFileDir;
std::string veins::MetricsAnalysisBaseApp::indvPerSecondNbContactDurationLogFile;
std::string veins::MetricsAnalysisBaseApp::aggrContactDurationFileDir;
std::string veins::MetricsAnalysisBaseApp::aggrMeetingTimeFileDir;

void MetricsAnalysisBaseApp::initialize(int stage)
{
    BaseApplLayer::initialize(stage);
    if (stage == 0) {

        // initialize pointers to other modules
        if (FindModule<TraCIMobility*>::findSubModule(getParentModule())) {
           mobility = TraCIMobilityAccess().get(getParentModule());
           traci = mobility->getCommandInterface();
           traciVehicle = mobility->getVehicleCommandInterface();
        }
        else {
           traci = nullptr;
           mobility = nullptr;
           traciVehicle = nullptr;
        }

        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        mac = FindModule<DemoBaseApplLayerToMac1609_4Interface*>::findSubModule(getParentModule());
        ASSERT(mac);
        mac1609 = FindModule<Mac1609_4*>::findSubModule(getParentModule());

        // read parameters
        headerLength = par("headerLength");
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits");
        beaconUserPriority = par("beaconUserPriority");
        beaconInterval = par("beaconInterval");

        dataLengthBits = par("dataLengthBits");
        dataOnSch = par("dataOnSch").boolValue();
        dataUserPriority = par("dataUserPriority");

        wsaInterval = par("wsaInterval").doubleValue();
        currentOfferedServiceId = -1;

        isParked = false;

        seed = par("seed");
        scenarioName = par("scenarioName").stringValue();
        baseDirLogFiles = par("baseDirLogFiles").stringValue();
        prefixLogFilename = par("prefixLogFilename").stringValue();


        findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        findHost()->subscribe(TraCIMobility::parkingStateChangedSignal, this);

        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendWSAEvt = new cMessage("wsa evt", SEND_WSA_EVT);

        generatedBSMs = 0;
        generatedWSAs = 0;
        generatedWSMs = 0;
        receivedBSMs = 0;
        receivedWSAs = 0;
        receivedWSMs = 0;

        logPerSecondNumVehiclesAndNumNeighbors = par("logPerSecondNumVehiclesAndNumNeighbors").boolValue();
        logIndividualContactDuration = par("logIndividualContactDuration").boolValue();
        logIndividualMeetingTime = par("logIndividualMeetingTime").boolValue();
        logIndividualPerSecondNeighborContactDuration = par("logIndividualPerSecondNeighborContactDuration").boolValue();
        logAggregatedContactuDuration = par("logAggregatedContactuDuration").boolValue();
        logAggregatedMeetingTime = par("logAggregatedMeetingTime").boolValue();

    }
    else if (stage == 1) {
        // Initializing members that require other modules initialization goes here

        // store MAC address for quick access
        myId = mac->getMACAddress();

        host = findHost();
        L2TocModule[myId] = host;
        cModuleToL2[host] = myId;
        L2ToFullname[myId] = host->getFullName();

        // simulate asynchronous channel access
        if (dataOnSch == true && !mac->isChannelSwitchingActive()) {
            dataOnSch = false;
            EV_ERROR << "App wants to send data on SCH but MAC doesn't use any SCH. Sending all data on CCH" << std::endl;
        }
        simtime_t firstBeacon = simTime();

        if (par("avoidBeaconSynchronization").boolValue() == true) {

            simtime_t randomOffset = dblrand() * beaconInterval;
            firstBeacon = simTime() + randomOffset;

            if (mac->isChannelSwitchingActive() == true) {
                if (beaconInterval.raw() % (mac->getSwitchingInterval().raw() * 2)) {
                    EV_ERROR << "The beacon interval (" << beaconInterval << ") is smaller than or not a multiple of  one synchronization interval (" << 2 * mac->getSwitchingInterval() << "). This means that beacons are generated during SCH intervals" << std::endl;
                }
                firstBeacon = computeAsynchronousSendingTime(beaconInterval, ChannelType::control);
            }

            if (sendBeacons) {
                EV_INFO << "Scheduling first beacon at: " << firstBeacon << "s for " << L2TocModule[myId]->getFullName()
                        << ". Current time: " << simTime() << endl;
                scheduleAt(firstBeacon, sendBeaconEvt);
            }
        }

        deviceName[RSU] = "RSU";
        deviceName[VEHICLE] = "Vehicle";

        /** to run only one time, below code runs on first vehicle (node[0])
         * because there must be at least one vehicle in the network */
        if (strcmp("node[0]", L2TocModule[myId]->getFullName()) == 0) {
            // set seed for random number generation
            srand(seed);

            std::string laneId = traciVehicle->getLaneId();
            laneMaxSpeed = (new TraCICommandInterface::Lane(traci, laneId))->getMaxSpeed();
        }

    }
}

void MetricsAnalysisBaseApp::finish()
{
    EV_INFO << L2TocModule[myId]->getFullName() << " left the network at " << simTime() << endl;
}

MetricsAnalysisBaseApp::~MetricsAnalysisBaseApp()
{
    if (sendBeaconEvt->isScheduled()) {
           cancelAndDelete(sendBeaconEvt);
       }

   cancelAndDelete(sendWSAEvt);

   findHost()->unsubscribe(BaseMobility::mobilityStateChangedSignal, this);
}

simtime_t MetricsAnalysisBaseApp::computeAsynchronousSendingTime(simtime_t interval, ChannelType chan)
{

    /*
     * avoid that periodic messages for one channel type are scheduled in the other channel interval
     * when alternate access is enabled in the MAC
     */

    simtime_t randomOffset = dblrand() * beaconInterval;
//    simtime_t randomOffset = dblrand() + interval;
    simtime_t firstEvent;
    simtime_t switchingInterval = mac->getSwitchingInterval(); // usually 0.050s
    simtime_t nextCCH;

    /*
     * start event earliest in next CCH (or SCH) interval. For alignment, first find the next CCH interval
     * To find out next CCH, go back to start of current interval and add two or one intervals
     * depending on type of current interval
     */

    if (mac->isCurrentChannelCCH()) {
        nextCCH = simTime() - SimTime().setRaw(simTime().raw() % switchingInterval.raw()) + switchingInterval * 2;
    }
    else {
        nextCCH = simTime() - SimTime().setRaw(simTime().raw() % switchingInterval.raw()) + switchingInterval;
    }

    firstEvent = nextCCH + randomOffset;

    // check if firstEvent lies within the correct interval and, if not, move to previous interval

    if (firstEvent.raw() % (2 * switchingInterval.raw()) > switchingInterval.raw()) {
        // firstEvent is within a sch interval
        if (chan == ChannelType::control) firstEvent -= switchingInterval;
    }
    else {
        // firstEvent is within a cch interval, so adjust for SCH messages
        if (chan == ChannelType::service) firstEvent += switchingInterval;
    }

    return firstEvent;
}

void MetricsAnalysisBaseApp::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method_Silent();
    if (signalID == BaseMobility::mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == TraCIMobility::parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}

void MetricsAnalysisBaseApp::handleLowerMsg(cMessage* msg)
{
    BaseFrame1609_4* wsm = dynamic_cast<BaseFrame1609_4*>(msg);
    ASSERT(wsm);

    if (DemoSafetyMessage* bsm = dynamic_cast<DemoSafetyMessage*>(wsm)) {
        receivedBSMs++;
        onBSM(bsm);
    }
    else if (DemoServiceAdvertisment* wsa = dynamic_cast<DemoServiceAdvertisment*>(wsm)) {
        receivedWSAs++;
        onWSA(wsa);
    }
    else {
        receivedWSMs++;
        onWSM(wsm);
    }

    delete (msg);
}

// This method is for self messages (mostly timers)
void MetricsAnalysisBaseApp::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind()) {
        case SEND_BEACON_EVT: {
            DemoSafetyMessage* bsm = new DemoSafetyMessage();
            populateWSM(bsm);
            sendDown(bsm);
            scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
            break;
        }
        case SEND_WSA_EVT: {
            DemoServiceAdvertisment* wsa = new DemoServiceAdvertisment();
            populateWSM(wsa);
            sendDown(wsa);
            scheduleAt(simTime() + wsaInterval, sendWSAEvt);
            break;
        }
        case NB_TIMEOUT_TIMER: {
            auto nbConnectivityTimeoutMsg = dynamic_cast<NbConnectivityTimeoutMsg *>(msg);
            nbConnectivityTimeout(nbConnectivityTimeoutMsg);
            break;
        }

        default: {
            if (msg) EV_WARN << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
}

void MetricsAnalysisBaseApp::handlePositionUpdate(cObject* obj)
{
    EV_INFO << "Inside MetricsAnalysisBaseApp::handlePositionUpdate() function" << endl;

    if (mobility) {
        currentSpeed = mobility->getSpeed();
    }
    else {
        currentSpeed = 0;
    }

    ChannelMobilityPtrType const channelMobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = channelMobility->getPositionAt(simTime());
    curSpeed = channelMobility->getCurrentSpeed();
}

void MetricsAnalysisBaseApp::handleParkingUpdate(cObject* obj)
{
    isParked = mobility->getParkingState();
}

void MetricsAnalysisBaseApp::populateWSM(BaseFrame1609_4* wsm, LAddress::L2Type rcvId, int serial) {

    wsm->setRecipientAddress(rcvId);
    wsm->setBitLength(headerLength);

    if (DemoSafetyMessage* bsm = dynamic_cast<DemoSafetyMessage*>(wsm)) {
        bsm->setSenderPos(curPosition);
        bsm->setSenderSpeed(curSpeed);

        bsm->setOriginatorAddress(myId);
        bsm->setOriginatorSpeed(currentSpeed);

        bsm->setPsid(-1);
        bsm->setChannelNumber(static_cast<int>(Channel::cch));
        bsm->addBitLength(beaconLengthBits);
        wsm->setUserPriority(beaconUserPriority);
    }
    else if (DemoServiceAdvertisment* wsa = dynamic_cast<DemoServiceAdvertisment*>(wsm)) {
        wsa->setChannelNumber(static_cast<int>(Channel::cch));
        wsa->setTargetChannel(static_cast<int>(currentServiceChannel));
        wsa->setPsid(currentOfferedServiceId);
        wsa->setServiceDescription(currentServiceDescription.c_str());
    }
    else {
        if (dataOnSch)
            wsm->setChannelNumber(static_cast<int>(Channel::sch1)); // will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        else
            wsm->setChannelNumber(static_cast<int>(Channel::cch));
        wsm->addBitLength(dataLengthBits);
        wsm->setUserPriority(dataUserPriority);
    }
}

const std::string MetricsAnalysisBaseApp::currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}


void MetricsAnalysisBaseApp::startService(Channel channel, int serviceId, std::string serviceDescription)
{
    if (sendWSAEvt->isScheduled()) {
        throw cRuntimeError("Starting service although another service was already started");
    }

    mac->changeServiceChannel(channel);
    currentOfferedServiceId = serviceId;
    currentServiceChannel = channel;
    currentServiceDescription = serviceDescription;

    simtime_t wsaTime = computeAsynchronousSendingTime(wsaInterval, ChannelType::control);
    scheduleAt(wsaTime, sendWSAEvt);
}

void MetricsAnalysisBaseApp::stopService()
{
    cancelEvent(sendWSAEvt);
    currentOfferedServiceId = -1;
}

void MetricsAnalysisBaseApp::sendDown(cMessage* msg)
{
    checkAndTrackPacket(msg);
    BaseApplLayer::sendDown(msg);
}

void MetricsAnalysisBaseApp::sendDelayedDown(cMessage* msg, simtime_t delay)
{
    checkAndTrackPacket(msg);
    BaseApplLayer::sendDelayedDown(msg, delay);
}

void MetricsAnalysisBaseApp::checkAndTrackPacket(cMessage* msg)
{
    if (dynamic_cast<DemoSafetyMessage*>(msg)) {
        EV_TRACE << "sending down a BSM" << std::endl;
        generatedBSMs++;
    }
    else if (dynamic_cast<DemoServiceAdvertisment*>(msg)) {
        EV_TRACE << "sending down a WSA" << std::endl;
        generatedWSAs++;
    }
    else if (dynamic_cast<BaseFrame1609_4*>(msg)) {
        EV_TRACE << "sending down a wsm" << std::endl;
        generatedWSMs++;
    }
}

double MetricsAnalysisBaseApp::getTxPower() {
    Mac1609_4* mac1609 = FindModule<Mac1609_4*>::findSubModule(getParentModule());
    double txPower = mac1609->getTxPower();

    return txPower;
}


/** This function is called when an offloading vehicle or offloaded RSU/Vehicle are out of coverage.
 * This function is also used when granted data is completely offloaded */
//
//void MetricsAnalysisBaseApp::nbConnectivityTimeout(NbConnectivityTimeoutMsg *nbConnectivityTimeoutMsg) {
//    LAddress::L2Type nbId = nbConnectivityTimeoutMsg->getNbId();
//
//    // Cancel the message and delete the entry from the map
//    auto nbTimeoutTimerIter = nbDeviceTimeoutTimerMap.find(nbId);
//    if (nbTimeoutTimerIter != nbDeviceTimeoutTimerMap.end()) {
////        cancelEvent(nbTimeoutTimerIter->second);
//        cancelAndDelete(nbTimeoutTimerIter->second);
//        nbDeviceTimeoutTimerMap.erase(nbTimeoutTimerIter);
//    }
//}

void MetricsAnalysisBaseApp::scheduleNbTimoutTimer(LAddress::L2Type offloadingVehicleAddr, LAddress::L2Type nbAddress, int nbDeviceType) {
    NbConnectivityTimeoutMsg *nbConnectivityTimer = new NbConnectivityTimeoutMsg("Nb device timeout", NB_TIMEOUT_TIMER);
    nbConnectivityTimer->setOffloadingVehicleId(offloadingVehicleAddr);
    nbConnectivityTimer->setNbId(nbAddress);
    nbConnectivityTimer->setNbDeviceType(nbDeviceType);

    nbDeviceTimeoutTimerMap[nbAddress] = nbConnectivityTimer;
    scheduleAt(simTime() + 3*beaconInterval, nbConnectivityTimer);
}
