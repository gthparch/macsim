#!/usr/bin/python

"""
Author  : Jaekyu Lee (jq.lee17@gmail.com)
Date    : 8/9/2012
Purpose : This script file is intended for generating traces for GPU application
          using GPUOcelot. 

If you find bugs, please report to the author.
"""


import os
import re
import sys
import glob
import argparse
import subprocess


"""
<Usage>

./gpu_tracegen.py -cmd command_to_run -inc -I dir_to_include

0. Assume you've built the binary to run.
1. Specify original command to run a benchmark. 
2. If you have to change the version of compute capability, please modify
   ver and ver_env in this file.
3. In some cases, you may need to add compiler flag (e.g. -I../../sdk) to get
   kernel information. Please edit 'inc' in get_kernel_information().
   e.g. inc = '-I../../sdk'.

* All possible changes that a user needs to make is marked with TO_CHANGE.
"""


## global variables
cwd = os.getcwd()
ver = 'sm_20' ## TO_CHANGE
ver_env = '2.0' ## TO_CHANGE


"""
Process arguments
"""
def process_options():
  parser = argparse.ArgumentParser(description='GPU trace generation for MacSim')
  parser.add_argument('-cmd', action='append', nargs='*', dest='cmd', 
      help='command to run a benchmark')

  return parser


"""
Generate kernel information file
"""
def get_kernel_information():
  inc = '' ## TO_CHANGE
  cmd = 'nvcc --cubin --ptxas-options=-v -arch %s *.cu -I. %s' % (ver, inc)
  print(cmd)
  p = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
                       stderr=subprocess.PIPE, close_fds=True)     
  (fi, fo, fe) = (p.stdin, p.stdout, p.stderr)

  pattern_kernel = re.compile('ptxas info\s+: Compiling entry function \'(\S+)\' for \'%s\'' % ver)
  pattern_info = re.compile('ptxas info\s+: Used ([0-9]+) registers, ([0-9\+]+) bytes smem')

  filename = 'occupancy.txt'
  ofile = open(filename, 'w')

  kernel_dict = {}
  kernel_name = ''
  smem = ''
  register = ''
  for line in fe.readlines():
    match_kernel = pattern_kernel.match(line)
    match_info = pattern_info.match(line)

    if match_kernel != None:
      kernel_name = match_kernel.group(1)

    if match_info != None:
      register = match_info.group(1)
      smem = match_info.group(2)
      smem = smem.split('+')
      if len(smem) > 1:
        smem = str(int(smem[0]) + int(smem[1]));
      else:
        smem = smem[0]

      if kernel_name not in kernel_dict:
        ofile.write('%s %s %s\n' % (kernel_name, register, smem))
        kernel_dict[kernel_name] = True

  ofile.close()


"""
set environmental variables
"""
def set_env():
  trace_path = '%s/trace' % cwd
  if os.path.exists(trace_path):
    os.system('rm -rf %s' % trace_path)
  os.system('mkdir -p %s' % trace_path)

  os.environ['TRACE_PATH']       = trace_path
  os.environ['USE_KERNEL_NAME']  = '1'
  os.environ['KERNEL_INFO_PATH'] = '%s/occupancy.txt' % cwd
  os.environ['COMPUTE_VERSION']  = ver_env


"""
main routine
"""
def main():
  global args

  ## get arguments 
  parser = process_options()
  args = parser.parse_args()

  ## sanity check for command
  if args.cmd == None:
    print('please specify command to run')
    sys.exit(-1)

  ## setting for GPUOcelot trace generator
  get_kernel_information()
  set_env()

  ## execute the binary
  cmd = ' '.join(sum(args.cmd, []))
  os.system(cmd)


if __name__ == '__main__':
  main()
