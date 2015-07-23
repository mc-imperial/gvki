#!/usr/bin/python

import sys
import subprocess
import argparse
import json
import os

# ........................................
# filter not -I and -D arguments from list
def filterCppArguments(inputList):
    outputList = []
    i = 0
    while i < len(inputList):
        if len(inputList[i]) >= 2 and inputList[i][0:2] in ['-i','-I','-D']:
            outputList.append(inputList[i])
            i = i + 1
            if (i < len(inputList) and (len(inputList[i-1]) == 2)):
                if inputList[i][0] != '-':
                    outputList.append(inputList[i])
                    i = i + 1
        else:
            i = i + 1
    return outputList

# ======================================================
# MAIN SCRIPT
# ======================================================
def main(argv=None):
    # ............................
    # load command line arguments
    if argv is None:
        argv = sys.argv[1:]
    
    # .......................................
    # check number of arguments
    if (len(argv) < 2):
        print('run with -h to show help')
        exit(0)

    # ....................................................
    # parse command line arguments with argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--dir', help='The gvki-n directory to work with', required=True)
    parser.add_argument('--preprocessor', help='The preprocessor command to use. Default: cpp', default='cpp')
    args = parser.parse_args(argv)
    gvkiFolderPath = args.dir
    cPreProcessorExecutable = args.preprocessor
    logJsonFile = open(gvkiFolderPath + os.sep + 'log.json', 'r')

    # ...........................................
    # dictionary to store kernels to process
    processingQueue = {}
    
    # ..............................
    # process json file
    jsonfile = json.loads(logJsonFile.read())
    for elem in jsonfile:
        kernelName = elem['kernel_file']
        compilerFlags = elem['compiler_flags']
        processingQueue[kernelName] = compilerFlags
    
    # ..........................................................
    # preprocess entires in dictionary
    for dictionaryEntryKey in processingQueue:
        # dictionaryEntryKey is Kernel name
        # dictionaryEntryValue is Compiler flags
        dictionaryEntryValue = processingQueue[dictionaryEntryKey]

        # get CPP flags and split them
        cppArguments = dictionaryEntryValue.split()

        # remove non-preprocessor flags
        cppArguments = filterCppArguments(cppArguments)

        # construct call command
        callCommandList = [cPreProcessorExecutable] + [gvkiFolderPath + os.sep + dictionaryEntryKey] + cppArguments

        # open output stdout file
        preKernelFileName = dictionaryEntryKey.rsplit('.',1)[0] + '.pre.' + dictionaryEntryKey.rsplit('.',1)[1]
        processedKernelFile = open(gvkiFolderPath + os.sep + preKernelFileName, 'w')

        # call subprocess
        subprocess.call(callCommandList, stdout=processedKernelFile)
        
        processedKernelFile.close()

    logJsonFile.close()

# ===========================================
# call main if executed as script
if __name__ == '__main__':
    sys.exit(main())