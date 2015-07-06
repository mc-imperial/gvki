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
    inputLineSubString = inputLineString[0:MIN_COMPILER_FLAGS_LINE_LENGTH]
    #print(inputLineSubString)
    return inputLineSubString == '"compiler_flags"'

# checks if current line is kernel file name
def isKernelFileLine(inputLineString):
    inputLineStringLength = len(inputLineString)
    if (inputLineStringLength < MIN_KERNEL_FILE_LINE_LENGTH):
        return 0
    inputLineSubString = inputLineString[0:MIN_KERNEL_FILE_LINE_LENGTH]
    # print(inputLineSubString)
    return inputLineSubString == '"kernel_file"'

# extracts the name of the kernel file name
def getKernelFileName(inputLineString):
    return inputLineString[KERNEL_FILE_LINE_ENTRY_CHAR:-3]

# extracts the compiler flags string
def getCompilerFlags(inputLineString):
    return inputLineString[COMPILER_FLAGS_LINE_ENTRY_CHAR:-3]

# ###############################################
# MAIN SCRIPT
if (len(sys.argv) != 2):
    print('arguments: path-to-gvki-folder')
    exit(0)
else:
    gvkiFolderPath = sys.argv[1]
    # print(gvkiFolderPath)
    logJsonFile = open(gvkiFolderPath + '/log.json', 'r')

    #dictionary to store kernels to process
    processingQueue = {}

    for jsonCurrentLine in logJsonFile:
        #print(isCompilerArgumentsLine(jsonCurrentLine))
        #print(isKernelFileLine(jsonCurrentLine))
        if (isKernelFileLine(jsonCurrentLine)):
            #print(getKernelFileName(jsonCurrentLine))
            lastKernelFileName = getKernelFileName(jsonCurrentLine)
        if (isCompilerArgumentsLine(jsonCurrentLine)):
            #print(getCompilerFlags(jsonCurrentLine))
            # add to kernels to process
            processingQueue[lastKernelFileName] = getCompilerFlags(jsonCurrentLine)
    
    for dictionaryEntryKey in processingQueue:
        # dictionaryEntryKey is Kernel name
        # dictionaryEntryValue is Compiler flags
        dictionaryEntryValue = processingQueue[dictionaryEntryKey]
        #print(dictionaryEntryKey)
        #print(processingQueue[dictionaryEntryKey])
        cppArguments = dictionaryEntryValue.split()
        callCommandList = ["cpp"] + [gvkiFolderPath + '/' + dictionaryEntryKey] + cppArguments
        print(callCommandList)
        processedKernelFile = open(gvkiFolderPath + '/' + dictionaryEntryKey + '.pre', 'w')
        subprocess.call(callCommandList, stdout=processedKernelFile)
