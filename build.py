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

    if options.dramsim:
      cmd += 'dram=1 '

    cmd += '-j %s ' % options.thread

  ## run scons command
  os.system(cmd)



if __name__ == '__main__':
  main()
