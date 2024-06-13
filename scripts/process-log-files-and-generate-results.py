"""
USAGE STEPS: 
1. Create a folder with any name (e.g., 'Ireland-Scenario') in your machine. 
2. Within this created folder, create two folders with the following exact names: 'data-files' and 'results'. 
3. Place the data files in the folder 'data-files'.
4. Now set the variables names in this program as follows (the variables are defined near the end of this file):
4a. Set the variable 'baseFolder' to be the name of folder that you created in Step 1 (e.g., 'Ireland-Scenario')
4b. Set the variable 'perSecondFileName' to be the name of log file of per second number of vehicles log (for number of vehicles over simulation time)
4c. Set the variable 'indvContactDuration02mWFileName' to be the name of log file of individual contact duration log (for contact duration and time to meet new nbs)
4d. Set the variable 'indvPerSecondNbContactDuration02mWFileName' to be the name of log file of individual per second contact duration log (for nb degree)
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from itertools import cycle
import statistics as stats
import matplotlib.dates as mdates
import copy

    
def calculateMeanPerSimTime(perSecondDf, columnName, **kwargs):
    xAxis_simTime = []
    yAxis_avgNumVehicleList = []
    newDf = perSecondDf

    for colName, value in kwargs.items():
        newDf = newDf[newDf[colName] == value]
    
    maxSimTime = perSecondDf["simTime"].max()
    
#     for simTime in range(maxSimTime+1):
    for simTime in range(int(maxSimTime)+1):
        newDf2 = newDf[newDf.simTime == simTime]
        xAxis_simTime.append(simTime)
        yAxis_avgNumVehicleList.append(newDf2[columnName].mean())
        
    
    return xAxis_simTime, yAxis_avgNumVehicleList


def calculateMean(dataframe, columnName, **kwargs):
    mean = []
    newDf = dataframe
    
    for colName, value in kwargs.items():
        newDf = newDf[newDf[colName] == value]
    
#     newDf = perSecondDf[perSecondDf.totalVehicles == totalVehicles]
    
    return newDf[columnName].mean();
    

def plotGraph(perSecond=True, withMarker=False, withXTicks=False, xTicksInterval=1.0, formatTime=False, legendLoc='upper right', **kwargs):
# def plotGraph(perSecond=True, **kwargs):
    
    xAxisMin = 999
    xAxisMax = 0
    yAxisMax = 0
    
    lines = ["-","dotted","--","-.",":"]
    linecycler = cycle(lines)
      
    fig = plt.figure(figsize=(18,12))
    ax = fig.add_subplot()
    plt.title(Config.title, fontsize=20)
    plt.xlabel(Config.xlabel, fontsize=16)
    plt.ylabel(Config.ylabel, fontsize=16)
#     ax.set_ylim(top=4.5)
    
    for legend, data in kwargs.items():
        xAxis = data[0]
        yAxis = data[1]
        
        xAxisFormatted = xAxis
        
        if formatTime:
            xAxisFormatted = pd.to_datetime(xAxis, unit='s') # convert to datetime
        
        if withMarker:
#             plt.plot(xAxis, yAxis, label=legend, ls=next(linecycler), marker='.')
            plt.plot(xAxisFormatted, yAxis, label=legend, marker='.')
        else:
#             plt.plot(xAxis, yAxis, label=legend, ls=next(linecycler))
            plt.plot(np.array(xAxisFormatted), np.array(yAxis), label=legend)

        yAxisMax_temp = max(yAxis)
        
        if (min(xAxis) < xAxisMin):
            xAxisMin = min(xAxis)
        if (max(xAxis) > xAxisMax):
            xAxisMax = max(xAxis)
        
            
        if (yAxisMax_temp > yAxisMax):
            yAxisMax = yAxisMax_temp

 
    
    plt.yticks(fontsize=16)
    plt.legend(fontsize=16, loc=legendLoc)
    plt.grid()
    
    if formatTime:
        ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
        fig.autofmt_xdate() # auto format
    
    if withXTicks:
        plt.xticks(np.arange(xAxisMin, xAxisMax+1, xTicksInterval))

    ax.set_ylim(bottom=0)
#     plt.ylim(ymin=0)

#     ax.ticklabel_format(useOffset=False, style='plain') # to avoid scientific notation
#     plt.savefig(Config.resultsFolder + '/' + Config.filenameToSave +'.eps', format='eps', bbox_inches = 'tight')
    plt.savefig(Config.resultsFolder + '/' + Config.scenarioPrefix + '-' + Config.filenameToSave +'.png', format='png', bbox_inches = 'tight')

#     plt.show()  



    
def numCurrentVehicles(perSecondDf):

    maxSimTime = perSecondDf["simTime"].max() 
    
    xAxis_simTime, yAxis_avgNumVehicles = calculateMeanPerSimTime(perSecondDf, 'numVehicles')
    
    Config.title = 'Number of Vehicles Over Each Simulation Time'
    Config.xlabel = 'Simulation Time (seconds)'
    Config.ylabel = 'Number of Vehicles'
    Config.filenameToSave = 'num-vehicles-over-sim-time'
        
    plotGraph(formatTime=True, **{Config.scenario  : [xAxis_simTime, yAxis_avgNumVehicles]})
    


# This function calculates the average contact duration 
def calculateAverageContactDuration(contactDurationDf, minContactDuration, **kwargs):
    np.set_printoptions(suppress=True)
    
    dataframe = contactDurationDf
    
    # apply filter to the dataframe (i.e., select only the column values specified in the **kwargs, such as txPower = 0.2)
    for colName, value in kwargs.items():
        dataframe = dataframe[dataframe[colName] == value]
    
    # remove the records of contact duration that are less than 1
    dataframe = dataframe[dataframe['contactDuration'] >= minContactDuration]
    
    # get ids of vehicles ('myId' column) of all vehicles and then select the distinct ones. 
    vehiclesIdArray = dataframe[['myId']].values.ravel()
    distinctVehiclesIds = pd.unique(vehiclesIdArray)
    
    # calculate the average contact duration of each vehicle individually
    indvVehicleContactDurationMean = []
    for vehicleId in distinctVehiclesIds:
        indvVehicleDf = dataframe[dataframe['myId'] == vehicleId]
        # take average and round it off
        indvVehicleContactDurationMean.append(round(indvVehicleDf['contactDuration'].mean(), 0))
    
    # apply filter if required (e.g., select only those contact duration which are greater than zero).
    # limit the max contact duration to 80 because almost all the vehicles have contact duration less than 80 seconds, 
    # otherwise, the x-axis in the graph gets many values that make it difficult to interpret the graph.
    filteredIndvVehicleContactDurationMean = [duration for duration in indvVehicleContactDurationMean if duration < 80]
#     filteredIndvVehicleContactDurationMean = indvVehicleContactDurationMean    # uncomment this if do not want to apply the filter
    
    # count total vehicles after filter (i.e., the length of filteredIndvVehicleMeetingDurationMean)
    totalVehicles = len(filteredIndvVehicleContactDurationMean)
    
    minContatDuration = min(filteredIndvVehicleContactDurationMean)
    maxContactDuration = max(filteredIndvVehicleContactDurationMean)
    
    print("Total vehicles: " + str(totalVehicles))   
    print("Min contact duration: " + str(minContatDuration))
    print("Max contact duration: " + str(maxContactDuration))
    
    # find distinct meeting duration for running the loop on
    distinctContactDurations = np.unique(filteredIndvVehicleContactDurationMean)
    
    xAxis_contactDurations = []
    yAxis_percentageOfVehicles = []
    
    for contactDuration in distinctContactDurations:
        numVehicles = filteredIndvVehicleContactDurationMean.count(contactDuration)
        percentageOfVehicles = round(numVehicles/totalVehicles * 100, 0)
        
        xAxis_contactDurations.append(contactDuration)
        yAxis_percentageOfVehicles.append(percentageOfVehicles)
        
        print("contact duration: " + str(contactDuration) + "\t numVehicles: " + str(numVehicles) + "\t % of vehicles: " + str(percentageOfVehicles))
        
    return xAxis_contactDurations, yAxis_percentageOfVehicles
    
    

def calculateDurationOfMeetingNewNb(nbContactDurationDf, minContactDurationThreshold, **kwargs):
    np.set_printoptions(suppress=True)
    
    dataframe = nbContactDurationDf
    
    # apply the filters on columns received as **kwargs
    for colName, value in kwargs.items():
        dataframe = dataframe[dataframe[colName] == value]
    
        
    dataframe = dataframe[dataframe['contactDuration'] >= minContactDurationThreshold]
    
    # get distinct vehicles ids
    vehiclesIdArray = dataframe[['myId']].values.ravel()
    distinctVehiclesIds = pd.unique(vehiclesIdArray)
    
    avgMeetingTimeList = []
    # loop on each vehicle
    for vehicleId in distinctVehiclesIds:
        newDf = dataframe[dataframe['myId'] == vehicleId]
        # sort dataframe by start time
        newDf = newDf.sort_values(by=['startTime'])
        # convert start time in data frame to numpy list
        startTimeList = newDf['startTime'].tolist()

        meetTimeList = []
#         for startTime in startTimeList:
        for index in range(1, len(startTimeList)):
            # calculate meeting time as the difference of two consecutive time stamps
            meetingTime = startTimeList[index] - startTimeList[index-1]
            meetTimeList.append(meetingTime)
            
        if len(meetTimeList) > 0:
            avgMeetTime = round(stats.mean(meetTimeList), 0)
            avgMeetingTimeList.append(avgMeetTime)
        
    totalVehicles = len(avgMeetingTimeList)
    distinctMeetingTimesList = np.unique(avgMeetingTimeList)
    
    xAxis_meetingTime = []
    yAxis_percentageOfVehicles = []
    for duration in distinctMeetingTimesList:
        # Based on the observations from experiments, almost all vehicles meet new nbs within 20 seconds. 
        # therefore, only considering the meeting duration less than 20 for easy interpreting the graphs
        if duration < 20:
            numVehicles = avgMeetingTimeList.count(duration)
            xAxis_meetingTime.append(duration)
            percentageOfVehicles = round(numVehicles/totalVehicles * 100, 0)
            yAxis_percentageOfVehicles.append(percentageOfVehicles)
            print("avgMeetingTime " + str(duration) + "\t numVehicles: " + str(numVehicles) + "\t % of vehicles: " + str(percentageOfVehicles))
        
    print()
    
    return xAxis_meetingTime, yAxis_percentageOfVehicles
   

# Count number of neighbors before or after the meeting time
def calculateNumNbsBeforeOrAfterMeetingTime(nbContactDurationDf, minContactDurationThreshold, numNbsBeforeOrAfter="Before", **kwargs):
    print("\t\t *** Function: Calculate Number of Neighbors At and " + numNbsBeforeOrAfter + " Meeting Time for minContactDurationThreshold: " + str(minContactDurationThreshold) + " ***")
    np.set_printoptions(suppress=True)
    
    dataframe = nbContactDurationDf
    
    # apply the filters on columns received as **kwargs
    for colName, value in kwargs.items():
        dataframe = dataframe[dataframe[colName] == value]
    
        
    dataframe = dataframe[dataframe['contactDuration'] >= minContactDurationThreshold]
    
    # get distinct vehicles ids
    vehiclesIdArray = dataframe[['myId']].values.ravel()
    distinctVehiclesIds = pd.unique(vehiclesIdArray)
    
    avgMeetingTimeList = []
    numNbsDict = {}
    totalNeighbors = 0
    # loop on each vehicle
    for vehicleId in distinctVehiclesIds:
        newDf = dataframe[dataframe['myId'] == vehicleId]
        # sort dataframe by start time
        newDf = newDf.sort_values(by=['startTime'])
        # convert start time in data frame to numpy list
        startTimeList = newDf['startTime'].tolist()

        meetTimeList = []
#         for startTime in startTimeList:
        for index in range(1, len(startTimeList)):
            # calculate meeting time as the difference of two consecutive time stamps
            meetingTime = startTimeList[index] - startTimeList[index-1]
            meetTimeList.append(meetingTime)
            
        if len(meetTimeList) > 0:
            avgMeetTime = round(stats.mean(meetTimeList), 0)
            avgMeetingTimeList.append(avgMeetTime)
            currentNumNbs = len(newDf)  # number of neighbors are equal to the size of newDf (i.e., filtered dataset)
            totalNeighbors += len(newDf)
            if avgMeetTime in numNbsDict:
#                 print('numNbsDict for ' + str(avgMeetTime) + ' for vehicle ' + str(vehicleId) + ' is currently empty')
                numNbsDict[avgMeetTime] += currentNumNbs
            else:
                numNbsDict[avgMeetTime] = currentNumNbs
        
    totalVehicles = len(avgMeetingTimeList)
    distinctMeetingTimesList = np.unique(avgMeetingTimeList)
    
    xAxis_meetingTime = []
    yAxis_numNbs = []
    yAxis_avgNumNbs = []
    yAxis_percentageOfNbs = []
    for duration in distinctMeetingTimesList:
        # Based on the observations from experiments, almost all vehicles meet new nbs within 20 seconds. 
        # therefore, only considering the meeting duration less than 20 for easy interpreting the graphs
        if duration < 21:
            xAxis_meetingTime.append(duration)
            numNbs = 0
            minMeetingTime = min(distinctMeetingTimesList)
            maxMeetingTime = max(distinctMeetingTimesList)
#             print('minMeetingTime: ' + str(minMeetingTime) + ', maxMeetingTime: ' + str(maxMeetingTime) + ', duration: ' + str(duration))
            if numNbsBeforeOrAfter == "Before":
                for meetingTime in range(int(minMeetingTime), int(duration)+1):
                    if meetingTime in numNbsDict:
                        numNbs += numNbsDict[meetingTime]
            elif numNbsBeforeOrAfter == "After":
                for meetingTime in range(int(duration), int(maxMeetingTime)+1):
                    if meetingTime in numNbsDict:
                        numNbs += numNbsDict[meetingTime]
            avgNumNbs = round(numNbs/totalVehicles, 0)
            percentageOfNbs = round(numNbs/totalNeighbors * 100, 0)
#             yAxis_numNbs.append(avgNumNbs)
            yAxis_numNbs.append(numNbs)
            yAxis_avgNumNbs.append(avgNumNbs)
            yAxis_percentageOfNbs.append(percentageOfNbs)
            print("avgMeetingTime " + str(duration) + "\t numNbs: " + str(numNbs) + "\t avgNumNbs: " + str(avgNumNbs))
        
    print()
    
    return xAxis_meetingTime, yAxis_numNbs, yAxis_avgNumNbs, yAxis_percentageOfNbs
    
# This function calculates the average contact duration 
def calculatePercenCumVehiclesBeforeOrAfterContactDuration(contactDurationDf, minContactDuration, connLastingLessOrMore="Less", **kwargs):
    np.set_printoptions(suppress=True)
    
    dataframe = contactDurationDf
    
    # apply filter to the dataframe (i.e., select only the column values specified in the **kwargs, such as txPower = 0.2)
    for colName, value in kwargs.items():
        dataframe = dataframe[dataframe[colName] == value]
    
    # remove the records of contact duration that are less than 1
    dataframe = dataframe[dataframe['contactDuration'] >= minContactDuration]
    
    # get ids of vehicles ('myId' column) of all vehicles and then select the distinct ones. 
    vehiclesIdArray = dataframe[['myId']].values.ravel()
    distinctVehiclesIds = pd.unique(vehiclesIdArray)
    
    # calculate the average contact duration of each vehicle individually
    indvVehicleContactDurationMean = []
    for vehicleId in distinctVehiclesIds:
        indvVehicleDf = dataframe[dataframe['myId'] == vehicleId]
        # take average and round it off
        indvVehicleContactDurationMean.append(round(indvVehicleDf['contactDuration'].mean(), 0))
    
    # apply filter if required (e.g., select only those contact duration which are greater than zero).
    # limit the max contact duration to 80 because almost all the vehicles have contact duration less than 80 seconds, 
    # otherwise, the x-axis in the graph gets many values that make it difficult to interpret the graph.
    filteredIndvVehicleContactDurationMean = [duration for duration in indvVehicleContactDurationMean if duration < 81]
#     filteredIndvVehicleContactDurationMean = indvVehicleContactDurationMean    # uncomment this if do not want to apply the filter
    
    # count total vehicles after filter (i.e., the length of filteredIndvVehicleMeetingDurationMean)
    totalVehicles = len(filteredIndvVehicleContactDurationMean)
    
    minContatDuration = min(filteredIndvVehicleContactDurationMean)
    maxContactDuration = max(filteredIndvVehicleContactDurationMean)
    
    print("Total vehicles: " + str(totalVehicles))   
    print("Min contact duration: " + str(minContatDuration))
    print("Max contact duration: " + str(maxContactDuration))
    
    # find distinct meeting duration for running the loop on
    distinctContactDurations = np.unique(filteredIndvVehicleContactDurationMean)
    
    xAxis_contactDurations = []
    yAxis_percentageOfVehicles = []
    
    for contactDuration in distinctContactDurations:
        numVehicles = 0
        if connLastingLessOrMore == "Less":
            for participatingDuration in range(int(minContactDuration), int(contactDuration)+1):
                numVehicles += filteredIndvVehicleContactDurationMean.count(participatingDuration)
        elif connLastingLessOrMore == "More":
            for participatingDuration in range(int(contactDuration), int(maxContactDuration)+1):
                numVehicles += filteredIndvVehicleContactDurationMean.count(participatingDuration)                
#         numVehicles = filteredIndvVehicleContactDurationMean.count(contactDuration)
        percentageOfVehicles = round(numVehicles/totalVehicles * 100, 0)
        
        xAxis_contactDurations.append(contactDuration)
        yAxis_percentageOfVehicles.append(percentageOfVehicles)
        
        print("contact duration: " + str(contactDuration) + "\t numVehicles: " + str(numVehicles) + "\t % of vehicles: " + str(percentageOfVehicles))
        
    return xAxis_contactDurations, yAxis_percentageOfVehicles

def avgContactDuration(indvContactDuration02mWFileName):
    
    # indvContactDurationDf02mW = pd.read_csv(indvContactDuration02mWFileName, dtype={
    #     'seed': 'string',
    #     'scenario':'string',
    #     'maxSpeed': 'float32',
    #     'txPower': 'float64',
    #     'myId': 'int64',
    #     'nbId': 'int64',
    #     'startTime': 'float64',
    #     'endTime': 'float64',
    #     'contactDuration': 'float64',
    #     'datetime': 'string'
    #     })
    indvContactDurationDf02mW = pd.read_csv(indvContactDuration02mWFileName)
    
    txPower = 0.2
    avgContactDurationDict = {}
    minContactDurationThresholdList = [0,3,10,25,40]
    
    for minContactDurationThreshold in minContactDurationThresholdList:
        xAxis_contactDurations, yAxis_percentageOfVehicles = calculateAverageContactDuration(indvContactDurationDf02mW, minContactDurationThreshold, **{'txPower':txPower})
        
        avgContactDurationDict['min contact duration thres >= ' + str(minContactDurationThreshold) + 'sec']  = [xAxis_contactDurations, yAxis_percentageOfVehicles]
    
    Config.title = 'Contact Time of Vehicles (' + Config.scenario + ') for '  + str(txPower) + 'mW'
    Config.xlabel = 'Contact Duration(seconds)'
    Config.ylabel = 'Percentage of Vehicles (%)'
    Config.filenameToSave = 'dist-contact-time-vehicles'
        
    plotGraph(withMarker=True, **avgContactDurationDict) 



def avgNewNbMeetingTime(indvContactDuration02mWFileName):
    indvContactDurationDf02mW = pd.read_csv(indvContactDuration02mWFileName)
    txPower = 0.2
    
    minContactDurationThresholdList = [0,3,10,25,40]
    
    newNbMeetingTimeDict = {}
    
    for minContactDurationThreshold in minContactDurationThresholdList:
        xAxis_meetingTime02MwThres0, yAxis_PercentageOfVehicles02mWThres0 = calculateDurationOfMeetingNewNb(indvContactDurationDf02mW, minContactDurationThreshold)
        
        newNbMeetingTimeDict['min contact duration thres >= ' + str(minContactDurationThreshold) + 'sec'] = [xAxis_meetingTime02MwThres0, yAxis_PercentageOfVehicles02mWThres0]
    
    Config.title = 'Meeting Duration of Next Neighbor (' + Config.scenario + ') for '  + str(txPower) + 'mW'
    Config.xlabel = 'Duration of Meeting a New Vehicle (seconds)'
    Config.ylabel = 'Percentage of Vehicles (%)'
    Config.filenameToSave = 'meeting-duration-next-nb'

    plotGraph(withMarker=True, withXTicks=True, **newNbMeetingTimeDict)
#     Config.withMarker = True
#     Config.withXTicks = True
#     plotGraph(**newNbMeetingTimeDict)
    

    

    
    
 


#     plotGraph(withMarker=True, withXTicks=True, **newNbMeetingTimeDict)
#     Config.withMarker = True
#     Config.withXTicks = True
#     plotGraph(**newNbMeetingTimeDict)
    
def numNbsBeforeOrAfterMeetingTime(indvContactDuration02mWFileName, numNbsBeforeOrAfter="Before"):
    indvContactDurationDf02mW = pd.read_csv(indvContactDuration02mWFileName)
    txPower = 0.2
    
    minContactDurationThresholdList = [0,3,10,25,40]
    
    numNbsBeforeOrAfterMeetingTimeDict = {}
    avgNumNbsBeforeOrAfterMeetingTimeDict = {}
    percentNbsBeforeOrAfterMeetingTimeDict = {}
    
    for minContactDurationThreshold in minContactDurationThresholdList:
        xAxis_meetingTime02MwThres0, yAxis_numNbs02mWThres0, yAxis_avgNumNbs02mWThres0, yAxis_percentNbs02mWThres0 = calculateNumNbsBeforeOrAfterMeetingTime(indvContactDurationDf02mW, minContactDurationThreshold, numNbsBeforeOrAfter)
        
        numNbsBeforeOrAfterMeetingTimeDict['min contact duration thres >= ' + str(minContactDurationThreshold) + 'sec'] = [xAxis_meetingTime02MwThres0, yAxis_numNbs02mWThres0]
        avgNumNbsBeforeOrAfterMeetingTimeDict['min contact duration thres >= ' + str(minContactDurationThreshold) + 'sec'] = [xAxis_meetingTime02MwThres0, yAxis_avgNumNbs02mWThres0]
        percentNbsBeforeOrAfterMeetingTimeDict['min contact duration thres >= ' + str(minContactDurationThreshold) + 'sec'] = [xAxis_meetingTime02MwThres0, yAxis_percentNbs02mWThres0]
    
    
    Config.title = 'Number of Neighbors At and ' + numNbsBeforeOrAfter + ' Meeting Time (' + Config.scenario + ') for '  + str(txPower) + 'mW'
    Config.xlabel = 'Duration of Meeting a New Vehicle (seconds)'
    Config.ylabel = 'Number of Neighbors'
    Config.filenameToSave = 'num-nb-' + numNbsBeforeOrAfter + '-meeting-duration'
    

#     plotGraph(withMarker=True, withXTicks=True, **numNbsBeforeOrAfterMeetingTimeDict)
    
    
    
    Config.title = 'Avg Number of Neighbors At and ' + numNbsBeforeOrAfter + ' Meeting Time (' + Config.scenario + ') for '  + str(txPower) + 'mW'
    Config.xlabel = 'Duration of Meeting a New Vehicle (seconds)'
    Config.ylabel = 'Avg Number of Neighbors'
    Config.filenameToSave = 'num-nb-' + numNbsBeforeOrAfter + '-meeting-duration-avg'
    

#     plotGraph(withMarker=True, withXTicks=True, **avgNumNbsBeforeOrAfterMeetingTimeDict)
    
    
    Config.title = '% of Nodes Met ' + numNbsBeforeOrAfter + ' Time t (' + Config.scenario + ') for '  + str(txPower) + 'mW'
    Config.xlabel = 'Time of Meeting a New Vehicle (seconds)'
    Config.ylabel = '% of Nodes Met ' + numNbsBeforeOrAfter + ' Time t'
    Config.filenameToSave = 'percent-node-met-' + numNbsBeforeOrAfter + '-time-t-multilines'
    
    
    if numNbsBeforeOrAfter == "Before":
        plotGraph(withXTicks=True, legendLoc='center right', **percentNbsBeforeOrAfterMeetingTimeDict)
    else:
        plotGraph(withXTicks=True, **percentNbsBeforeOrAfterMeetingTimeDict)
    
    
 
# def percentVehiclesBeforeOrAfterContactDuration(indvContactDuration02mWFileName, vehiclesBeforeOrAfter="Before"):
def percentVehiclesBeforeOrAfterContactDuration(indvContactDuration02mWFileName, connLastingLessOrMore="Less"):
    indvContactDurationDf02mW = pd.read_csv(indvContactDuration02mWFileName)
    txPower = 0.2
    
    minContactDurationThresholdList = [0,3,10,25,40]
    

    percentCumVehiclesBeforeOrAfterContactDurationDict = {}
    
    for minContactDurationThreshold in minContactDurationThresholdList:
        xAxis_meetingTime02MwThres0, yAxis_percentVehicles02mWThres0 = calculatePercenCumVehiclesBeforeOrAfterContactDuration(indvContactDurationDf02mW, minContactDurationThreshold, connLastingLessOrMore)
        
        percentCumVehiclesBeforeOrAfterContactDurationDict['min contact duration thres >= ' + str(minContactDurationThreshold) + 'sec'] = [xAxis_meetingTime02MwThres0, yAxis_percentVehicles02mWThres0]
    
    
    
#     Config.title = 'Percentage of Cumulative Vehicles At and ' + connLastingLessOrMore + ' Contact Duration (' + Config.scenario + ') for '  + str(txPower) + 'mW'
    Config.title = '% of Connections Lasting ' + connLastingLessOrMore + ' than 'r'$\Delta$t (' + Config.scenario + ') for '  + str(txPower) + 'mW'
    Config.xlabel = 'Contact Duration (seconds)'
    Config.ylabel = '% of Connections Lasting ' + connLastingLessOrMore + ' than Contact Duration 'r'$\Delta$t'
    Config.filenameToSave = 'percent-connections-lasting-' + connLastingLessOrMore + '-than-t-multilines'
    
    
    if connLastingLessOrMore == "Less":
        plotGraph(withXTicks=True, xTicksInterval=2.0, legendLoc='lower right', **percentCumVehiclesBeforeOrAfterContactDurationDict)
    else:
        plotGraph(withXTicks=True, xTicksInterval=2.0, **percentCumVehiclesBeforeOrAfterContactDurationDict)
            
    
       
    
    
def perSecondFunctions(perSecondFileName):
    perSecondDf = pd.read_csv(perSecondFileName)
    
    
    numCurrentVehicles(perSecondDf)

##########################################
############### Global Parameters  #######
##########################################

class Config:
    # ************* Configurations of Graphs Plot ***************
    title = ''
    xlabel = 'Simulation Time (seconds)'
    ylabel = ''
    filenameToSave = ''
    scenario = ''
    scenarioPrefix = ''
#     legendLoc = 'upper right'
#     formatTime = False
#     withMarker = False
#     withXTicks = False
#     xTicksInterval = 1.0

    # ************* Configuratations of Data Files ************
    baseFolder = '2021-10-28-Ireland-Urban-FreeFlow/'
    dataFilesFolder = baseFolder + 'data-files/'
    resultsFolder = baseFolder + 'results'
            
    perSecondFileName = dataFilesFolder + 'distrContactMeetTest-perSecondNumVehiclesAndNumAvgNbs-0.200000mW-IrelandNationalN7-FreeFlow.csv'    # for number of vehicles over simulation time
    indvContactDuration02mWFileName = dataFilesFolder + 'distr-prob-indvContactDurationLog-0.200000mW-IrelandUrban-FreeFlow.csv'   # for contact duration and time of meeting new nb
    indvPerSecondNbContactDuration02mWFileName = dataFilesFolder + 'distr-prob-indvPerSecondNbContactDurationLog-0.200000mW-IrelandNationalN7-FreeFlow.csv'  # for avg nb degree

    Ropt = 0
    Fopt = 0
    X = 0


# aggregatedFileName = dataFilesFolder + 'aggregatedLog.csv'
# perSecondFileName = dataFilesFolder + 'perSecondLog.csv'    # for number of vehicles over simulation time

def executeMetricsAnalysisMultipleScenarios():
    scenarioDict = {}
    
    scenario1 = "Ireland Urban (Dublin City Centre) (Non-Rush Hours)"
    scenario1Prefix = "ireland-urban-freeflow"
    baseFolder = '2024-03-01-IrelandUrban-FreeFlow/'
    indvContactDurationFilename1 = baseFolder + 'data-files/' + 'metricsAnalysis-indvContactDurationLog-0.200000mW-IrelandUrban-FreeFlow.csv'   # for contact duration and time of meeting new nb
    perSecondFileName1 = baseFolder + 'data-files/' + 'metricsAnalysis-perSecondNumVehiclesAndNumAvgNbs-0.200000mW-IrelandUrban-FreeFlow.csv' # for number of vehicles over simulation time
    scenarioDict[scenario1] = [baseFolder, indvContactDurationFilename1, perSecondFileName1, scenario1Prefix]
    
    
    scenario2 = "Ireland Urban (Dublin City Centre) (Rush Hours)"
    scenario2Prefix = "ireland-urban-saturated"
    baseFolder = '2024-03-01-IrelandUrban-Saturated/'
    indvContactDurationFilename2 = baseFolder + 'data-files/' + 'metricsAnalysis-indvContactDurationLog-0.200000mW-IrelandUrban-Saturated.csv'   # for contact duration and time of meeting new nb
    perSecondFileName2 = baseFolder + 'data-files/' + 'metricsAnalysis-perSecondNumVehiclesAndNumAvgNbs-0.200000mW-IrelandUrban-Saturated.csv' # for number of vehicles over simulation time
#     scenarioDict[scenario2] = [baseFolder, indvContactDurationFilename2, perSecondFileName2, scenario2Prefix]
    
    
    scenario3 = "Ireland National Highway (Non-Rush Hours)"
    scenario3Prefix = "ireland-national-freeflow"
    baseFolder = '2024-03-01-IrelandNational-FreeFlow/'
    indvContactDurationFilename3 = baseFolder + 'data-files/' + 'metricsAnalysis-indvContactDurationLog-0.200000mW-IrelandNationalN7-FreeFlow.csv'   # for contact duration and time of meeting new nb
    perSecondFileName3 = baseFolder + 'data-files/' + 'metricsAnalysis-perSecondNumVehiclesAndNumAvgNbs-0.200000mW-IrelandNationalN7-FreeFlow.csv' # for number of vehicles over simulation time
    scenarioDict[scenario3] = [baseFolder, indvContactDurationFilename3, perSecondFileName3, scenario3Prefix]
    
    scenario4 = "Ireland National Highway (Rush Hours)"
    scenario4Prefix = "ireland-national-saturated"
    baseFolder = '2024-03-01-IrelandNational-Saturated/'
    indvContactDurationFilename4 = baseFolder + 'data-files/' + 'metricsAnalysis-indvContactDurationLog-0.200000mW-IrelandNationalN7-Saturated.csv'   # for contact duration and time of meeting new nb
    perSecondFileName4 = baseFolder + 'data-files/' + 'metricsAnalysis-perSecondNumVehiclesAndNumAvgNbs-0.200000mW-IrelandNationalN7-Saturated.csv' # for number of vehicles over simulation time
#     scenarioDict[scenario4] = [baseFolder, indvContactDurationFilename4, perSecondFileName4, scenario4Prefix]
    
    
    for scenario, data in scenarioDict.items():
        Config.baseFolder = data[0]
        Config.dataFilesFolder = Config.baseFolder + 'data-files/'
        Config.resultsFolder = Config.baseFolder + 'results'
        indvContactDuration02mWFileName = data[1]
        perSecondFileName = data[2]
        Config.scenarioPrefix = data[3]
        Config.scenario = scenario

        avgContactDuration(indvContactDuration02mWFileName)
        avgNewNbMeetingTime(indvContactDuration02mWFileName)
                 
        numNbsBeforeOrAfterMeetingTime(indvContactDuration02mWFileName, numNbsBeforeOrAfter="Before")
        # numNbsBeforeOrAfterMeetingTime(indvContactDuration02mWFileName, numNbsBeforeOrAfter="After")
   
        # percentVehiclesBeforeOrAfterContactDuration(indvContactDuration02mWFileName, connLastingLessOrMore="Less")
        percentVehiclesBeforeOrAfterContactDuration(indvContactDuration02mWFileName, connLastingLessOrMore="More")
                   
          
        perSecondFunctions(perSecondFileName)   
        
    

def main():   
    executeMetricsAnalysisMultipleScenarios()
    

if __name__ == "__main__":
    main()
    
    