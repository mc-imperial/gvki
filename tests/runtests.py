#!/usr/bin/env python
"""
Simple script to run test OpenCL programs
"""
import argparse

try:
    # Python 3
    import configparser as cp
except ImportError:
    # Python 2.x
    import ConfigParser as cp

import copy
import glob
import logging
import os
import re
import subprocess
import shutil
import sys

def printError(msg):
    logging.error('\033[0;31m*** {} ***\033[0m'.format(msg))

def printOk(msg):
    logging.info('\033[0;32m*** {} ***\033[0m'.format(msg))


class LibTest(object):
    def __init__(self, path):
        self.path = os.path.abspath(path)

    def _run(self, extra_env):
        logging.info('*** Running : {} ***'.format(self.path))
        env = copy.deepcopy(os.environ)
        env.update(extra_env)
        logging.debug('env: {}'.format(env))

        expectedOutputDir = os.path.join(os.path.dirname(self.path), "gvki-0")
        assert not os.path.exists(expectedOutputDir)

        retcode = subprocess.call(self.path, env=env, stdin=None, stdout=None, stderr=None, cwd=os.path.dirname(self.path), shell=False)
        if retcode != 0:
            printError('{} failed'.forat(self.path))
        else:
            printOk('{} passed'.format(self.path))

            if not os.path.exists(expectedOutputDir):
                printError('OutputDir missing')
                return 1
            else:
                expectedJSONFile = os.path.join(expectedOutputDir, 'log.json')

                if not os.path.exists(expectedJSONFile):
                    return 1

                # FIXME: Check the contents of the JSON file and kernel(s) look right

                # There should be at least one kernel
                recordedKernels=glob.glob(expectedOutputDir + os.path.sep + '*.cl')
                logging.info('Recorded kernels: {}'.format(recordedKernels))
                if len(recordedKernels) == 0:
                    printError('No kernels recorded')
                    return 1


        logging.info('')
        logging.info('')

        # FIXME: Check stuff was recorded

        return  0 if retcode == 0 else 1


    # So we can sort tests
    def __lt__(self, other):
        return self.path < other.path

class MacroLibTest(LibTest):
    def run(self):
        return self._run({})

class PreloadLibTest(LibTest):
    def __init__(self, path, libPath):
        super(PreloadLibTest, self).__init__(path)
        self.libPath = libPath

    def __lt__(self, other):
        return self.path < other.path

    def run(self):
        if sys.platform == 'darwin':
            return self._run({ 'DYLD_INSERT_LIBRARIES': self.libPath})
        else:
            return self._run({ 'LD_PRELOAD': self.libPath})

def main(args):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', help='Directory to scan for test OpenCL programs')
    parser.add_argument('-l', '--loglevel', type=str, default="info",choices=['debug','info','warning','error','critical'])
    parsedArgs = parser.parse_args(args)

    logging.basicConfig(level=getattr(logging, parsedArgs.loglevel.upper(), None))

    if not os.path.isdir(parsedArgs.directory):
        logging.error('"{}" is not a directory'.format(parsedArgs.directory))
        return 1

    directoryContainingConfig = os.path.dirname(__file__)
    # Load config file
    configFilePath = os.path.join(directoryContainingConfig, 'config.cfg')
    logging.info('Looking for config file "{}"'.format(configFilePath))
    config = cp.ConfigParser()
    config.read(configFilePath)

    preloadlibPath = config.get('settings', 'preloadLibPath')
    preloadlibPath = preloadlibPath.replace("//","/")
    logging.debug('preloadLibPath is "{}"'.format(preloadlibPath))

    if not os.path.exists(preloadlibPath):
        logging.error('preloadLibPath "{}" does not exist'.format(preloadlibPath))
        return 1

    

    logging.info('Scanning for tests in "{}"'.format(parsedArgs.directory))

    tests = [ ]

    for (dirpath, dirnames, filenames) in os.walk(parsedArgs.directory):
        for f in filenames:
            if f.endswith('_gvki_preload'):
                tests.append( PreloadLibTest( os.path.join(dirpath, f), preloadlibPath))
            elif f.endswith('_gvki_macro'):
                tests.append( MacroLibTest( os.path.join(dirpath, f)))

        # clean up any old output directories
        for directory in dirnames:
            if re.match(r'gvki-\d+', directory):
                toRemove = os.path.join(dirpath, directory)
                logging.info('Deleting {}'.format(toRemove))
                shutil.rmtree(toRemove)

    logging.info('Found {} tests'.format(len(tests)))
    tests.sort()

    count = 0
    for test in tests:
        logging.info('Running test: {}'.format(test.path))
        count += test.run()

    msg = '# of Failures {}'.format(count)
    if count == 0:
        printOk(msg)
    else:
        printError(msg)

    return count != 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
