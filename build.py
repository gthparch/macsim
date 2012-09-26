#!/usr/bin/python

#########################################################################################
# Author      : Jaekyu Lee (jq.lee17@gmail.com)
# Description : wrapper script for scons build
#########################################################################################


import os
import sys
import itertools
from optparse import OptionParser


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
  parser.add_option("--dramsim", action="store_true", dest="dramsim", default=False, help="DRAMSim2")
  parser.add_option("--power", action="store_true", dest="power", default=False, help="EI Power")
  parser.add_option("--iris", action="store_true", dest="iris", default=False, help="IRIS")

  return parser


#########################################################################################
# build test for all possible build combinations
#########################################################################################
def build_test():
  build_option = ['', 'debug=1', 'gprof=1']
  build_dir    = ['.opt_build', '.dbg_build', '.gpf_build']
  build_libs   = ['dram=1', 'power=1', 'iris=1']

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

  # EI power
  if options.power:
    cmd += 'power=1 '

  # IRIS
  if options.iris:
    cmd += 'iris=1 '

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
