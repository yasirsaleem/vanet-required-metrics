cplusplus {{
#include "veins/base/utils/Coord.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/base/utils/SimpleAddress.h"
#include <list>
}}

namespace veins;


class BaseFrame1609_4;
class noncobject Coord;
class LAddress::L2Type extends void;


//packet JoiningRequest extends BaseFrame1609_4 {
//    int status;
//    int layer; 
//}
//
//packet LeavingMessage extends BaseFrame1609_4 {}
//
//packet RequestDecline extends BaseFrame1609_4 {}
//
//packet JoiningReply extends BaseFrame1609_4 {
//    int status;
//    int layer; 
//    int numChildren;
//    bool isRequestAccepted;
//}


message NbConnectivityTimeoutMsg
{
    LAddress::L2Type offloadingVehicleId;
    LAddress::L2Type nbId;
    int nbDeviceType;
}