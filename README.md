# Analysis of Common Required Metrics for Vehicular Networks
This file provides the usage guidelines of the code of "Analysis of Common Required Metrics for Vehicular Networks". 

The code is implemented in [Veins](http://veins.car2x.org) that is a framework of OMNeT++ network simulator to enable the functionality of vehicular networks. 

## How to Cite

This Git repository is linked to our research paper "Analysis of Common Required Metrics for Vehicular Networks". The repository includes all the required files to run the simulations and generate the results in our paper. You can find the instructions below about how to use the code and how to run the simulations to obtain the results presented in our paper. 

If you use this code, we would appreciate a citation of our following work. Thanks !
- Saleem Y, Mitton N, Loscri V. Analysis of Common Required Metrics for Vehicular Networks. *In International Conference on Wireless Networks and Mobile Communications (WINCOM)*, 2024 Jul 23. [PDF](https://inria.hal.science/hal-04601484/)

## Overview
The code provided in this repository analyses the number of vehicles over each simulation time, how long vehicles stay connected with other vehicles (contact duration), how long it takes for a vehicle to meet a new neighboring vehicle (time to meet new neighboring vehicle), how much connections of vehicle stay connected for less or more than contact duration and how much vehicles meet new neighboring vehicles before or after time *t*.  

## Class Hierarchy

**Base Appl Layer:** It is a base application class of veins framework located at (src/veins/base/modules/BaseApplLayer.cc) that handles the basic application layer funcationality of vehicular networks. We did not change anything in it. This is the ancestor of all our implemented classes. 

**Metrics Analysis Base App:** It is the base class for the implementation of analysis of common required metrics. It contains the variables and functions that are required for its subclasses of vehicle and RSU apps.  

**Metrics Analysis Vehicle App:** It is the main class that contains the implementation of analysis of common required metrics for vehicular networks. 
 
**Metrics Analysis RSU App:** RSUs are not involved in any communication but we included one RSU in our scenario that acts as a centralized entity. It is used for logging purpose.  

***Note:*** All the classes other than BaseApplLayer are located at *'src/veins/modules/application/'* directory.

## Scenario

We worked with real-world SUMO scenario publicly available at [SUMO scenarios website](https://sumo.dlr.de/docs/Data/Scenarios.html). We used the traffic mobility scenario of Ireland as follows:
**Ireland dataset:** The Ireland SUMO traffic mobility scenario provides the real-world traffic simulation of three types of networks in Ireland: highway, urban and motorway scenarios for a duration of 24 hours. It provides the duration of freeflow, saturated and congested traffic that represent the rush and non-rush hours.  We have used SUMO scenarios of Ireland's national highway N7 and Dublin's city centre for non-rush hours (defined as *freeflow* traffic) and rush hours (defined as *saturated* traffic). The complete details about Ireland scenarios are provided at [SUMO Ireleand scenario](https://github.com/maxime-gueriau/ITSC2020_CAV_impact).

**Other datasets** available at SUMO website can be used in a similar manner. 

## Usage
This simulation is conducted with OMNeT++ 5.5.1, Veins 5.0 and SUMO 1.7.0.
***Make sure that Veins is installed and sumo-launchd.py is currently running***. You can verify it by running the demo application at http://veins.car2x.org/tutorial/#finish.

The scenario of the analysis of common required metrics for vehicular networksis located at *'veins/scenarios/metrics-analysis'* directory. 

Set the directory where you want the log files to be saved (e.g., "./log-files/") by setting the parameter '*appl.baseDirLogFiles*'. The current directory is the directory of scenario that you are going to execute. 
To add a prefix to log file name (e.g., "metricsAnalysis-"), set the variable '*appl.prefixLogFilename*'.

There is a Python script *process-log-files-and-generate-results.py* (located at  '*veins/scripts/process-log-files-and-generate-results.py*') that reads the log files, process them and generate the results. 
***Important point for usage:*** In order to use this Python script, it is important to follow the template of the directory structure, i.e., create a directory with any name. Inside this directory, create two sub-folders with the exact following names: "data-files" and "results". Put the log files that are generated at the end of simulation into "data-files" folder. The graphs will be generated in "results" folder after the script is successfully executed . 

***Running example:***
 1. In the *metrics-analysis* scenario (*'veins/scenarios/metrics-analysis'* directory), execute the config file *IrelandNationalFreeFlowScenario*. When the simulation is finished, there will be six log files (*metricsAnalysis-aggrContactDurationLog-0.200000mW-IrelandNationalN7-FreeFlow.csv*, *metricsAnalysis-aggrMeetingTimeLog-0.200000mW-IrelandNationalN7-FreeFlow.csv*, *metricsAnalysis-indvContactDurationLog-0.200000mW-IrelandNationalN7-FreeFlow.csv*, *metricsAnalysis-indvNewNbMeetingTimeLog-0.200000mW-IrelandNationalN7-FreeFlow.csv*, *metricsAnalysis-indvPerSecondNbContactDurationLog-0.200000mW-IrelandNationalN7-FreeFlow.csv* and *metricsAnalysis-perSecondNumVehiclesAndNumAvgNbs-0.200000mW-IrelandNationalN7-FreeFlow.csv*).
 2. Navigate to the folder where the Python script is located (i.e., '*veins/scripts/process-log-files-and-generate-results.py*' directory). Create a folder in this directory with a name of your choice (e.g., '*2024-03-01-IrelandNational-FreeFlow*') and within this folder, create two sub-folders with exact names: *data-files* and *results*. 
 3. Copy log files generated in Step 1 into the *data-files* folder created in Step 2. 
 4. Now execute the script *process-log-files-and-generate-results.py*
 5. You will get all the results in *results* folder. Now, you can view the results.  
 6. Read the Python script to understand how log file is analysed.