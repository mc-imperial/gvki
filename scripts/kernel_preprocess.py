#!/usr/bin/python

import sys
import subprocess

MIN_KERNEL_FILE_LINE_LENGTH = 13
KERNEL_FILE_LINE_ENTRY_CHAR = 16
MIN_COMPILER_FLAGS_LINE_LENGTH = 16
COMPILER_FLAGS_LINE_ENTRY_CHAR = 19

# checks if current line is compiler arguments
def isCompilerArgumentsLine(inputLineString):
    inputLineStringLength = len(inputLineString)
    if (inputLineStringLength < MIN_COMPILER_FLAGS_LINE_LENGTH):
        return 0
    return inputLineString.split(':', 1)[0].rsplit(',',1)[0].strip().replace('"','') == 'compiler_flags'

# checks if current line is kernel file name
def isKernelFileLine(inputLineString):
    inputLineStringLength = len(inputLineString)
    if (inputLineStringLength < MIN_KERNEL_FILE_LINE_LENGTH):
        return 0
    return inputLineString.split(':', 1)[0].rsplit(',',1)[0].strip().replace('"','') == 'kernel_file'

# extract the json data
def getJsonData(inputLineString):
    return inputLineString.split(':', 1)[1].rsplit(',',1)[0].strip().replace('"','')

# filter not -I and -D arguments from list
def filterCppArguments(inputList):
    toBeRemoved = []
    for listEntry in inputList:
        if (not(listEntry[0:2] in ['-i', '-I', '-D'])):
            toBeRemoved = toBeRemoved + [listEntry]
    for removing in toBeRemoved:
        inputList.remove(removing)
    return inputList

# ###############################################
# MAIN SCRIPT
if (len(sys.argv) != 2):
    print('arguments: path-to-gvki-folder')
    exit(0)
else:
    gvkiFolderPath = sys.argv[1]
    logJsonFile = open(gvkiFolderPath + '/log.json', 'r')

    #dictionary to store kernels to process
    processingQueue = {}

    lastKernelFileName = None
    for jsonCurrentLine in logJsonFile:
        if (isKernelFileLine(jsonCurrentLine)):
            lastKernelFileName = getJsonData(jsonCurrentLine)
        if (isCompilerArgumentsLine(jsonCurrentLine)):
            # add to kernels to process
            processingQueue[lastKernelFileName] = getJsonData(jsonCurrentLine)
    
    for dictionaryEntryKey in processingQueue:
        # dictionaryEntryKey is Kernel name
        # dictionaryEntryValue is Compiler flags
        dictionaryEntryValue = processingQueue[dictionaryEntryKey]

        # get CPP flags and split them
        cppArguments = dictionaryEntryValue.split()

        # remove non-preprocessor flags
        cppArguments = filterCppArguments(cppArguments)

        # construct call command
        callCommandList = ["cpp"] + [gvkiFolderPath + '/' + dictionaryEntryKey] + cppArguments

        # open output stdout file
        processedKernelFile = open(gvkiFolderPath + '/' + dictionaryEntryKey + '.pre', 'w')

        # call subprocess
        print(callCommandList)
        subprocess.call(callCommandList, stdout=processedKernelFile)
        
        processedKernelFile.close()

    logJsonFile.close()
