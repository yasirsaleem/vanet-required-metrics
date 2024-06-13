/**
 * @brief This is base class for the analysis of common required metrics in vehicular networks
 * @author Yasir Saleem (www.yasirsaleem.com)
 */

//#pragma once

#include "veins/veins.h"

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/messages/DemoServiceAdvertisement_m.h"
#include "veins/modules/messages/DemoSafetyMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/DemoBaseApplLayerToMac1609_4Interface.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

#include <map>
#include <math.h>       /* sqrt */
#include "veins/modules/application/traci/MetricsAnalysisMessages_m.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"



namespace veins {

using veins::AnnotationManager;
using veins::AnnotationManagerAccess;
using veins::TraCICommandInterface;
using veins::TraCIMobility;
using veins::TraCIMobilityAccess;

using namespace omnetpp;

class VEINS_API MetricsAnalysisBaseApp : public BaseApplLayer {
public:
    ~MetricsAnalysisBaseApp() override;
    void initialize(int stage) override;
    void finish() override;

    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

    double getCurrentSpeed() {return currentSpeed;}

    std::string deviceName[2];
    int INFINITE_VALUE = 9999999;

    enum MetricsAnalysisMessageKinds {
            SEND_BEACON_EVT,
            SEND_WSA_EVT,
            NB_TIMEOUT_TIMER,
            PER_SECOND_NB_COUNT_TIMER
        };

        enum DeviceType
        {
            RSU = 0,
            VEHICLE = 1
        };
        enum Directions
        {
            EAST = 0,
            WEST = 1,
            NORTH = 2,
            SOUTH = 3
        };
        enum States
        {
            NONE = 0
        };

protected:
    struct NbConnectivityDetails
    {
        LAddress::L2Type myId;
        LAddress::L2Type nbId;
        simtime_t startTime;
        simtime_t endTime;
        simtime_t duration;  // seconds
    };

//    double bitsToMegabits = 0.000001;

    TraCIMobility* mobility;
    TraCICommandInterface* traci;
    TraCICommandInterface::Vehicle* traciVehicle;
    TraCICommandInterface::Lane* traciLane;

    AnnotationManager* annotations;
    DemoBaseApplLayerToMac1609_4Interface* mac;
    Mac1609_4* mac1609;

    bool isParked;

    /* BSM (beacon) settings */
    uint32_t beaconLengthBits;
    uint32_t beaconUserPriority;
    simtime_t beaconInterval;
    simtime_t beaconStartTime;
    bool sendBeacons;

    /* WSM (data) settings */
    uint32_t dataLengthBits;
    uint32_t dataUserPriority;
    bool dataOnSch;

    /* WSA settings */
    int currentOfferedServiceId;
    std::string currentServiceDescription;
    Channel currentServiceChannel;
    simtime_t wsaInterval;

    /* state of the vehicle */
    Coord curPosition;
    Coord curSpeed;
    LAddress::L2Type myId = 0;
    int myDeviceType;
    int mySCH;

    /* stats */
    uint32_t generatedWSMs;
    uint32_t generatedWSAs;
    uint32_t generatedBSMs;
    uint32_t receivedWSMs;
    uint32_t receivedWSAs;
    uint32_t receivedBSMs;
    static int numVehicles;
    static double laneMaxSpeed;

    /* messages for periodic events such as beacon and WSA transmissions */
    cMessage* sendBeaconEvt;
    cMessage* sendWSAEvt;

    int seed;
    std::string baseDirLogFiles;
    std::string prefixLogFilename;
    std::string scenarioName;

    std::map<LAddress::L2Type, NbConnectivityTimeoutMsg*> nbDeviceTimeoutTimerMap;

    double currentSpeed;
    cModule* host;
    static std::map<LAddress::L2Type, cModule*> L2TocModule;
    static std::map<cModule*, LAddress::L2Type> cModuleToL2;
    static std::map<LAddress::L2Type, const char*> L2ToFullname;

    static int totalMaxVehicles;
    static std::string perSecondNumVehiclesAndNumNbsFileDir;
    static std::string indvContactDurationFileDir;
    static std::string indvMeetingTimeFileDir;
    static std::string indvPerSecondNbContactDurationLogFile;
    static std::string aggrContactDurationFileDir;
    static std::string aggrMeetingTimeFileDir;

    bool logPerSecondNumVehiclesAndNumNeighbors;
    bool logIndividualContactDuration;
    bool logIndividualMeetingTime;
    bool logIndividualPerSecondNeighborContactDuration;
    bool logAggregatedContactuDuration;
    bool logAggregatedMeetingTime;

    static std::vector<NbConnectivityDetails> globalNbConnectivityDetails;
    static std::vector<simtime_t> globalNewNbMeetingDiffTime;
    static int numCurrentVehicles;
    static std::map<int, std::vector<int>> allNodesNumNbsOverTimeMap;
    static simtime_t maxContactDuration;
    static simtime_t maxMeetingTimeofNextVehicle;

    /** @brief handle messages from below and calls the onWSM, onBSM, and onWSA functions accordingly */
    void handleLowerMsg(cMessage* msg) override;

    /** @brief handle self messages */
    void handleSelfMsg(cMessage* msg) override;

    /** @brief this function is called upon receiving a DemoSafetyMessage, also referred to as a beacon  */
    virtual void onBSM(DemoSafetyMessage* bsm) {};

    /** @brief this function is called upon receiving a BaseFrame1609_4 */
    virtual void onWSM(BaseFrame1609_4* wsm) {};

    /** @brief this function is called upon receiving a DemoServiceAdvertisement */
    virtual void onWSA(DemoServiceAdvertisment* wsa) {};

    /** @brief this function is called every time the vehicle receives a position update signal */
    virtual void handlePositionUpdate(cObject* obj);

    /** @brief sets all the necessary fields in the WSM, BSM, or WSA. */
    virtual void populateWSM(BaseFrame1609_4* wsm, LAddress::L2Type rcvId = LAddress::L2BROADCAST(), int serial = 0);

    /** @brief this function is called every time the vehicle parks or starts moving again */
    virtual void handleParkingUpdate(cObject* obj);

    /** @brief This will start the periodic advertising of the new service on the CCH
     *
     *  @param channel: the channel on which the service is provided
     *  @param serviceId: a service ID to be used with the service
     *  @param serviceDescription: a literal description of the service
     */
    virtual void startService(Channel channel, int serviceId, std::string serviceDescription);

    /** @brief stopping the service and advertising for it */
    virtual void stopService();

    /** @brief compute a point in time that is guaranteed to be in the correct channel interval plus a random offset
     *
     * @param interval: the interval length of the periodic message
     * @param chantype: the type of channel, either type_CCH or type_SCH
     */
    virtual simtime_t computeAsynchronousSendingTime(simtime_t interval, ChannelType chantype);

    /**
     * @brief overloaded for error handling and stats recording purposes
     *
     * @param msg: the message to be sent. Must be a WSM/BSM/WSA
     */
    virtual void sendDown(cMessage* msg);

    /**
     * @brief overloaded for error handling and stats recording purposes
     *
     * @param msg the message to be sent. Must be a WSM/BSM/WSA
     * @param delay the delay for the message
     */
    virtual void sendDelayedDown(cMessage* msg, simtime_t delay);

    /**
     * @brief helper function for error handling and stats recording purposes
     *
     * @param msg: the message to be checked and tracked
     */
    virtual void checkAndTrackPacket(cMessage* msg);

    /** @brief this function is called when a vehicle does not receive beacon from its neighbor within a pre-defined timer */
    virtual void nbConnectivityTimeout(NbConnectivityTimeoutMsg *nbConnectivityTimeoutMsg) {};

    /** @brief after receiving a beacon from a neighboring vehicle, schedule a neighbor timeout timer to identify
     * if the vehicle is still neighbor */
    virtual void scheduleNbTimoutTimer(LAddress::L2Type offloadingVehicleAddr, LAddress::L2Type destAddress, int nbDeviceType);

    /** @brief obtain the transmission power of node (vehicle/RSU) */
    virtual double getTxPower();

    /** @brief obtain current date time for recording into log (csv) files */
    const std::string currentDateTime();
};

} // namespace veins
