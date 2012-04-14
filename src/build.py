#!/usr/bin/python

#########################################################################################
# Author      : Jaekyu Lee (jq.lee17@gmail.com)
# Description : wrapper script for scons build
#########################################################################################


import os
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
  parser.add_option("--dramsim", action="store_true", dest="dramsim", default=False, help="DRAMSim2")
  parser.add_option("--power", action="store_true", dest="power", default=False, help="EI Power")

  return parser


#########################################################################################
# main function
#########################################################################################
def main():
  parser = parse_arg()
  (options, args) = parser.parse_args()

  ## prepare scons command
  cmd = 'scons '

  if options.clean:
    cmd += '-c'
  else:
    if options.debug:
      cmd += 'debug=1 '
    elif options.gprof:
      cmd += 'gprof=1 '
    
    # DRAMSim2
    if options.dramsim:
      cmd += 'dram=1 '

    # EI power
    if options.power:
      cmd += 'power=1 '

    cmd += '-j %s ' % options.thread


  ## run scons command (in the top level)
  os.chdir('../')
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
