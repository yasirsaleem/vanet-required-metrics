//package yasir_example;

import org.car2x.veins.nodes.RSU;
import org.car2x.veins.nodes.Scenario;
import org.car2x.veins.modules.mobility.traci.TraCIVehicleInserter;

network MetricsAnalysis extends Scenario
{
    parameters:
        int numberOfRSUs;
    submodules:
        rsu[numberOfRSUs]: RSU {
            @display("p=150,100;i=veins/sign/yellowdiamond;is=vs");
        }
        
        //vehiclesInserter: TraCIVehicleInserter {
        //    @display("p=100,0;i=veins/sign/yellowdiamond;is=vs");
        //}
}
