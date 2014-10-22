#!/usr/bin/env python
"""
Simple script to run test OpenCL programs
"""
import argparse
import configparser as cp
import copy
import logging
import os
import subprocess
import sys

class LibTest(object):
    def __init__(self, path):
        self.path = os.path.abspath(path)

    def _run(self, extra_env):
        logging.info('*** Running : {} ***'.format(self.path))
        env = copy.deepcopy(os.environ)
        env.update(extra_env)
        logging.debug('env: {}'.format(env))
        retcode = subprocess.call(self.path, env=env, stdin=None, stdout=None, stderr=None, cwd=os.path.dirname(self.path), shell=False)
        if retcode != 0:
            logging.error('*** {} failed ***'.format(self.path))

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
    logging.basicConfig(level=logging.DEBUG)
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', help='Directory to scan for test OpenCL programs')
    parsedArgs = parser.parse_args(args)

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

    logging.info('Found {} tests'.format(len(tests)))
    tests.sort()

    count = 0
    for test in tests:
        logging.info('Running test: {}'.format(test.path))
        count += test.run()

    logging.info('Failures {}'.format(count))

    return count != 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
