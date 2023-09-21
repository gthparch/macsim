#!/usr/bin/env python3

#########################################################################################
# Author      : Jaekyu Lee (jq.lee17@gmail.com)
# Description : wrapper script for scons build
#########################################################################################


import os
import sys
import itertools
from optparse import OptionParser
import subprocess

#########################################################################################
# argument parsing
#########################################################################################
def parse_arg():
  parser = OptionParser(usage="usage: %prog [options] filename", version="%prog 1.0")
  parser.add_option("-j", "--thread", action="store", dest="thread", default=1, help="-j option for the parallel build")
  parser.add_option("-d", "--debug", action="store_true", dest="debug", default=False, help="debug build")
  parser.add_option("-p", "--gprof", action="store_true", dest="gprof", default=False, help="gprof build")
  parser.add_option("-c", "--clean", action="store_true", dest="clean", default=False, help="clean")
  parser.add_option("-t", "--test", action="store_true", dest="test", default=False, help="clean")
  parser.add_option("-v", "--val", action="store_true", dest="val", default=False, help="build version used for gpu validation")
  parser.add_option("--newpin", action="store_true", dest="pin_3_13_trace", default=False, help="trace based on pin 3.13 ")
  parser.add_option("-q", "--qsim", action="store_true", dest="qsim", default=False, help="use qsim to drive macsim")
  parser.add_option("--dramsim", action="store_true", dest="dramsim", default=False, help="DRAMSim2")
  parser.add_option("--dramsim3", action="store_true", dest="dramsim3", default=False, help="DRAMSim3")

  parser.add_option("--power", action="store_true", dest="power", default=False, help="EI Power")
  parser.add_option("--iris", action="store_true", dest="iris", default=False, help="IRIS")
  parser.add_option("--ramulator", action="store_true", dest="ramulator", default=False, help="Ramulator")
  parser.add_option("--sst", action="store_true", dest="sst", default=False, help="Build Macsim SST Element")
  parser.add_option("--sst-install", action="store_true", dest="sst_install", default=False, help="Install Macsim SST Element")

  return parser


#########################################################################################
# build test for all possible build combinations
#########################################################################################
def build_test():
  build_option = ['', 'debug=1', 'gprof=1', 'qsim=1', 'pin_3_13_trace=1']
  build_dir    = ['.opt_build', '.dbg_build', '.gpf_build']
  build_libs   = ['dram=1', 'power=1', 'iris=1', 'ramulator=1', 'dram3=1']

  for ii in range(0, len(build_option)):
    os.system('rm -rf %s' % build_dir[ii])
    for jj in range(0, len(build_libs)+1):
      for opt in itertools.combinations(build_libs, jj):
        cmd = 'scons -j 4 %s %s' % (build_option[ii], ' '.join(opt))
        redir = '> /dev/null 2>&1'

        if os.path.exists('%s/macsim' % build_dir[ii]):
          os.system('rm -f %s/macsim' % build_dir[ii])

        os.system('%s %s' % (cmd, redir))
        if os.path.exists('%s/macsim' % build_dir[ii]):
          print('%s %s successful' % (build_option[ii], ' '.join(opt)))
        else:
          print('%s %s failed' % (build_option[ii], ' '.join(opt)))

#########################################################################################
# Util Functions
#########################################################################################
def get_cmd_output(cmd:list, abort_on_error:bool=True):
  """
  Run a command and return the output.
  :param cmd: The command to run as a list of tokens.
  :param abort_on_error: If True, abort on error.
  :return: The stdout, stderr, and return code of the command.
  """
  process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
  stdout, stderr = process.communicate()
  stdout = stdout.decode('utf-8', errors='replace').strip() if stdout is not None else ''
  stderr = stderr.decode('utf-8', errors='replace').strip() if stderr is not None else ''
  if abort_on_error and process.returncode != 0:
    print(f'Error: Shell command failed (return code: {process.returncode}): ', ' '.join(cmd), file=sys.stderr)
    print('\tstdout:', stdout, file=sys.stderr)
    print('\tstderr:', stderr, file=sys.stderr)
    sys.exit(1)
  return stdout


#########################################################################################
# main function
#########################################################################################
def main():
  parser = parse_arg()
  (options, args) = parser.parse_args()

  ## Build test
  if options.test:
    build_test()
    sys.exit(0)


  ## Prepare scons command
  cmd = 'scons '

  ## Main build options (opt, dbg, gpf)
  if options.debug:
    cmd += 'debug=1 '
  elif options.gprof:
    cmd += 'gprof=1 '

  if options.val:
    cmd += 'val=1 '

  ## External libraries (dramsim, ei, iris)
  # DRAMSim2
  if options.dramsim:
    cmd += 'dram=1 '

  if options.dramsim3:
    cmd+= 'dram3=1 '
    print('dramsim3 enabled!')
    
  if options.sst:
    cmd += 'sst=1 '

  if options.sst_install:
    component = 'macsimComponent'
    component_lib_dir = os.path.abspath('.sst_build')
    component_src_dir = os.getcwd()
    component_tests_dir = os.path.abspath('sst-unit-test')

    # Check if macsim component exists
    if not os.path.exists(f'{component_lib_dir}/lib{component}.so'):
      print(f"ERROR: {component} not found in {component_lib_dir}, build with sst=1 option first")
      exit(0)

    print(f"Registering SST element: {component}")
    print(f"  SRCDIR: {component_src_dir}")
    print(f"  LIBDIR: {component_lib_dir}")
    print(f"  TESTDIR: {component_tests_dir}")
    os.system(f'sst-register {component} {component}_LIBDIR={component_lib_dir}')
    os.system(f'sst-register SST_ELEMENT_SOURCE {component}={component_src_dir}')
    os.system(f'sst-register SST_ELEMENT_TESTS {component}={component_tests_dir}')

    # Check if component is registered successfully
    sst_info_out = get_cmd_output(['sst-info', component])
    if 'Component 0: macsimComponent' in sst_info_out:
      print(f"Successfully registered SST element: {component}")
      exit(0)
    else:
      print(f"ERROR: Failed to register SST element: {component}")
      exit(1)

  # EI power
  if options.power:
    cmd += 'power=1 '

  # IRIS
  if options.iris:
    cmd += 'iris=1 '

  # Qsim
  if options.qsim:
    cmd += 'qsim=1 '

  # NEW PIN 
  if options.pin_3_13_trace:
      cmd += 'pin_3_13_trace=1 '

  # Ramulator
  if options.ramulator:
    cmd += 'ramulator=1 '

  ## Parallel building 
  cmd += '-j %s ' % options.thread
  
  if options.clean:
    cmd += '-c'


  ## run scons command
  os.system(cmd)


  ## Create a symbolic link
  if not options.clean:
    if options.debug:
      build_dir = '.dbg_build'
    elif options.gprof:
      build_dir = '.gpf_build'
    else:
      build_dir = '.opt_build'

    if os.path.exists('%s/macsim' % build_dir):
      os.chdir('bin')

      if os.path.exists('macsim'):
        os.system('rm -f macsim')
      os.system('ln -s ../%s/macsim' % build_dir)



if __name__ == '__main__':
  main()
