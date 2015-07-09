#!/usr/bin/python

import sys
import subprocess
import platform
import os
import argparse

import kernel_preprocess

def printUsage():
    print('Please run ' + sys.argv[0] + ' -h for help')
    exit(0)

ENV_GVKI_LIMIT_INTERCEPTIONS = 'GVKI_LIMIT_INTERCEPTIONS'
ENV_GVKI_LOG_DATA = 'GVKI_LOG_DATA'
ENV_GVKI_WORKING_DIR = 'GVKI_ROOT'

# ==============================================================
# add environment variable for preload library
def addPreloadEnv(optionsUsePreloadLibrary, optionsPreloadLibrary):
    if (optionsUsePreloadLibrary):
        if (platform.system() == 'Windows'):
            # windows
            print("Preloading is unsupported on Windows.")
            exit(0)
        elif (platform.system() == 'Linux'):
            # linux
            os.environ['LD_PRELOAD'] = optionsPreloadLibrary
        else:
            # mac
            os.environ['DYLD_FORCE_FLAT_NAMESPACE'] = '1'
            os.environ['DYLD_INSERT_LIBRARIES'] = optionsPreloadLibrary
            
# ================================================================
# add environment variable for GVKI_LOG_DATA
def addLogDataEnv(optionsLogData):
    if (optionsLogData):
        os.environ[ENV_GVKI_LOG_DATA] = '1'
    else:
        os.environ[ENV_GVKI_LOG_DATA] = '0'
    
# =========================================================
# add environment variable for GVKI_LIMIT_INTERCEPTIONS
def addInterceptionsEnv(optionsInterceptionsPerKernel):
    os.environ[ENV_GVKI_LIMIT_INTERCEPTIONS] = optionsInterceptionsPerKernel

# ===============================================
# add environment variable for GVKI_WORKING_DIR
def addWorkingDirEnv(workingDir):
    if (workingDir != ''):
        os.environ[ENV_GVKI_WORKING_DIR] = workingDir

# ===================================
# MAIN METHOD
def main(argv=None):
    if argv is None:
        argv = sys.argv
    
    # ....................................................
    # options and defaults
    optionsUsePreloadLibrary = False
        
    # ...................................................
    # parse command line arguments (argparse)
    parser = argparse.ArgumentParser()
    parser.add_argument('--log-data', help='gvki will log data to binary files', action='store_true', default=False)
    parser.add_argument('--interceptions-per-kernel', help='The maximum number of kernels to intercept. 0 is unlimited. Default: 0', default='0')
    parser.add_argument('--preprocess', help='Specifies to preprocess the intercepted kernels', action='store_true', default=False)
    parser.add_argument('--preprocessor', help='Specifies the preprocessor command to use', default='cpp')
    parser.add_argument('--preload-library', help='Specifies to use preload library.', default='')
    parser.add_argument('--working-dir', help='Specifies where the gvki-n folders will be created', default='')
    parser.add_argument('--verbose', help='Prints script debug messages', action='store_true', default=False)
    parser.add_argument('programcommand', nargs='*', help='The program to run')
    args = parser.parse_args(argv[1:])
    if (args.preload_library != ''):
        optionsUsePreloadLibrary = True
        
    # ..............................................
    # check for missing arguments and prepare variables
    if len(args.programcommand) == 0:
        print('Please specify program to run')
        printUsage()
    if (args.working_dir == ''):
        scriptWorkingDir = os.getcwd()
    else:
        scriptWorkingDir = args.working_dir
    
    # ..........................................................
    # debug: print parsed arguments
    if (args.verbose):
        print("log data " + str(args.log_data))
        print("preprocess " + str(args.preprocess))
        print("interceptions " + str(args.interceptions_per_kernel))
        print("preprocessor " + args.preprocessor)
        print("preload " + str(optionsUsePreloadLibrary))
        print("preloadlib " + args.preload_library)
        print("workdir " + args.working_dir)
        print(args.programcommand)
    
    # ..............................................................
    # get initial directory structure
    initialDirectoriesList = []
    for walkroot, walkdirs, walkfiles in os.walk(scriptWorkingDir):
        for dirnames in walkdirs:
            initialDirectoriesList.append(dirnames)
    
    # .........................................................
    # set environment variables for gvki and preloading
    addPreloadEnv(optionsUsePreloadLibrary, args.preload_library)
    addLogDataEnv(args.log_data)
    addInterceptionsEnv(args.interceptions_per_kernel)
    addWorkingDirEnv(args.working_dir)
    commandToRun = args.programcommand
    if args.verbose:
        print('Env variables: LD, WORKDIR, LOGDATA, LIMITINTERC')
        os.system('printenv LD_PRELOAD')
        os.system('printenv ' + ENV_GVKI_WORKING_DIR)
        os.system('printenv ' + ENV_GVKI_LOG_DATA)
        os.system('printenv ' + ENV_GVKI_LIMIT_INTERCEPTIONS)
        print("running command " + str(commandToRun))
    
    # ........................................................
    # run program
    subprocess.call(commandToRun, shell=True)
    
    # ...........................................................
    # get final directory structure and make difference
    finalDirectoriesList = []
    gvkiDirectoriesList = []
    for walkroot, walkdirs, walkfiles in os.walk(scriptWorkingDir):
        for dirnames in walkdirs:
            finalDirectoriesList.append(dirnames)
    for dirname in initialDirectoriesList:
        finalDirectoriesList.remove(dirname)
    for dirname in finalDirectoriesList:
        if (dirname[0:5] == 'gvki-'):
            gvkiDirectoriesList.append(dirname)
    
    if args.verbose:
        print(gvkiDirectoriesList)
    if (len(gvkiDirectoriesList) == 0):
        print("\nNo gvki folders generated. Did you recompile with the gvki folder or did you run using the gvki preload library?\n")
    
    # .............................................................
    # run preprocessor

    if (args.preprocess):
        for gvkiDirName in gvkiDirectoriesList:
            kernel_preprocess.main(['preprocess', '--dir', scriptWorkingDir + os.sep + gvkiDirName, '--preprocessor', args.preprocessor])
    
# ===========================================
# call main if executed as script
if __name__ == '__main__':
    sys.exit(main())