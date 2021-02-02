#!/usr/bin/env python3

#########################################################################################
# Author      : Jaekyu Lee (jq.lee17@gmail.com)
# Description : Scons top-level
#########################################################################################


import os
import sys
import configparser


## Check c++14 support
def CheckCPP14():
  def SimpleCall(context):
    cpp14_test = '''
    #include <vector>
    #include <iostream>
    #include <memory>
  
    using namespace std;

    int main(void)
    {
      vector<int> test_vector(5);
      int sum = 0;
      for (auto itr = test_vector.begin(); itr != test_vector.end(); ++itr) {
        sum += (*itr);
      }
      cout << sum;

      unique_ptr<int> test_int = make_unique<int>(2347);
      cout << *test_int;
    }
    '''
    context.Message('Checking for c++14 conformance...')
    context.env.AppendUnique(CXXFLAGS=['-std=c++14'])
    result = context.TryCompile(cpp14_test, '.cpp')
    context.Result(result)
    return result
  return SimpleCall


## Check c++14 support
def pre_compile_check():
  ## Environment
  env = Environment()
  custom_vars = set(['AS', 'AR', 'CC', 'CXX', 'HOME', 'LD_LIBRARY_PATH', 'PATH', 'RANLIB'])

  for key,val in os.environ.items():
    if key in custom_vars:
      env[key] = val

  conf = Configure(env, custom_tests = {'CheckCPP14' : CheckCPP14()})

  # if not conf.CheckCPP14():
  #   print('Error: Your compiler does not support c++14. Exit now...')
  #   os.system('cat config.log')
  #   sys.exit()


pre_compile_check()


## Configuration
flags = {}


## Configuration from file
Config = configparser.ConfigParser()
Config.read('macsim.config')
flags['dram']          = Config.get('Library', 'dram', fallback='0')
flags['power']         = Config.get('Library', 'power', fallback='0')
flags['iris']          = Config.get('Library', 'iris', fallback='0')
flags['qsim']          = Config.get('Library', 'qsim', fallback='0')
flags['debug']         = Config.get('Build', 'debug', fallback='0')
flags['gprof']         = Config.get('Build', 'gprof', fallback='0')
flags['pin_3_13_trace'] = Config.get('Build', 'pin_3_13_trace', fallback='0')
flags['val']           = Config.get('Build_Extra', 'val', fallback='0')
flags['ramulator']     = Config.get('Library', 'ramulator', fallback='0')

## Configuration from commandline
flags['debug']         = ARGUMENTS.get('debug', flags['debug'])
flags['gprof']         = ARGUMENTS.get('gprof', flags['gprof'])
flags['pin_3_13_trace'] = ARGUMENTS.get('pin_3_13_trace', flags['pin_3_13_trace'])
flags['power']         = ARGUMENTS.get('power', flags['power'])
flags['iris']          = ARGUMENTS.get('iris', flags['iris'])
flags['dram']          = ARGUMENTS.get('dram', flags['dram'])
flags['val']           = ARGUMENTS.get('val', flags['val'])
flags['qsim']          = ARGUMENTS.get('qsim', flags['qsim'])
flags['ramulator']     = ARGUMENTS.get('ramulator', flags['ramulator'])


## Checkout DRAMSim2 copy
if flags['dram'] == '1':
  if not os.path.exists('src/DRAMSim2'):
    os.system('git clone git://github.com/dramninjasUMD/DRAMSim2.git src/DRAMSim2')

## Checkout Ramulator copy
if flags['ramulator'] == '1':
  if not os.path.exists('src/ramulator'):
    os.system('git clone https://github.com/CMU-SAFARI/ramulator.git src/ramulator')

## Create stat/knobs
SConscript('scripts/SConscript', exports='flags')


## debug build
if flags['debug'] == '1':
  SConscript('SConscript', variant_dir='.dbg_build', duplicate=0, exports='flags')
  Clean('.', '.dbg_build')
## gprof build
elif flags['gprof'] == '1':
  SConscript('SConscript', variant_dir='.gpf_build', duplicate=0, exports='flags')
  Clean('.', '.gpf_build')
## opt build
else:
  SConscript('SConscript', variant_dir='.opt_build', duplicate=0, exports='flags')
  Clean('.', '.opt_build')


