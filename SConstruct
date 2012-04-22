#!/usr/bin/python

#########################################################################################
# Author      : Jaekyu Lee (jq.lee17@gmail.com)
# Description : Scons top-level
#########################################################################################


import os
import sys


## Check c++0x support
def CheckCPP0x():
  def SimpleCall(context):
    cpp0x_test = '''
    #include <vector>
    #include <iostream>
  
    using namespace std;

    int main(void)
    {
      vector<int> test_vector(5);
      int sum = 0;
      for (auto itr = test_vector.begin(); itr != test_vector.end(); ++itr) {
        sum += (*itr);
      }
      cout << sum << "\N";
    }
    '''
    context.Message('Checking for c++0x conformance...')
    context.env.AppendUnique(CXXFLAGS=['-std=c++0x'])
    result = context.TryCompile(cpp0x_test, '.cpp')
    context.Result(result)
    return result
  return SimpleCall


## Check c++0x support
def pre_compile_check():
  conf = Configure(Environment(), custom_tests = {
    'CheckCPP0x' : CheckCPP0x()
  })

  if not conf.CheckCPP0x():
    print('Error: Your compiler does not support c++0x. Exit now...')
    sys.exit()


pre_compile_check()


## parse argument
flags = {}
flags['debug'] = ARGUMENTS.get('debug', 0)
flags['gprof'] = ARGUMENTS.get('gprof', 0)
flags['dram']  = ARGUMENTS.get('dram', 0)
flags['power'] = ARGUMENTS.get('power', 0)
flags['iris']  = ARGUMENTS.get('iris', 0)


if flags['dram'] == '1':
  if not os.path.exists('src/DRAMSim2'):
    os.system('git clone git://github.com/dramninjasUMD/DRAMSim2.git src/DRAMSim2')


## Create stat/knobs
SConscript('scripts/SConscript')


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


