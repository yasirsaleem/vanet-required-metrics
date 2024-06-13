package org.car2x.veins.modules.application.traci;
import org.car2x.veins.base.modules.IBaseApplLayer;

//
// network description file for your Veins Application. Add parameters here
//
simple MetricsAnalysisBaseApp like IBaseApplLayer
{
    parameters: 
        @class(veins::MetricsAnalysisBaseApp);
	    int headerLength = default(88bit) @unit(bit); //header length of the application

        bool sendBeacons = default(true); //tell the applayer to periodically send beacons
        int beaconLengthBits = default(256bit) @unit(bit); //the length of a beacon packet
        int beaconUserPriority = default(7); //the user priority (UP) of the beacon messages
        double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages

        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(false); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataUserPriority = default(7); //the default user priority (UP) for data packets

        bool avoidBeaconSynchronization = default(true); //don't start beaconing directly after node was created but delay to avoid artifical synchronization

        bool sendWSA = default(false);
        int wsaLengthBits = default(250bit) @unit(bit);
        double wsaInterval =  default(1s) @unit(s);
        
		int seed = default(-1);
		string baseDirLogFiles = default("");
		string prefixLogFilename = default("");
		string scenarioName = default("Default scenario"); 
	    
	    string appName = default("Base app for finding distribution and probability");
	    
	    bool logPerSecondNumVehiclesAndNumNeighbors = default(true);
	    bool logIndividualContactDuration = default(true);
	    bool logIndividualMeetingTime = default(false);
		bool logIndividualPerSecondNeighborContactDuration = default(false);
		bool logAggregatedMeetingTime = default(false);
		bool logAggregatedContactuDuration = default(false);
		
	gates:
        input lowerLayerIn; // from mac layer
        output lowerLayerOut; // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}
