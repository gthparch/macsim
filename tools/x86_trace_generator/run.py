#!/usr/bin/python


import os
import sys
import argparse


"""
User-specific variables
"""
pin = '/home/jaekyu/software/pin-2.12-56759-gcc.4.4.7-linux/pin'
tracegen = os.getcwd() + '/obj-intel64/trace_generator.so'


"""
Parse argument
  -t : number of thread in an application (defautl 1)
  -n : number of instructions to generate (default 0 - until end)
  -s : number of instructions to skip the beginning of a binary (default 0 - nothing to skip)
  -c : cmd including binary
"""
def process_options(args):
  temp_args = list(args)
  temp_args.pop(0)

  parsed_args = {}
  parsed_args['-t'] = '1'
  parsed_args['-n'] = '0'
  parsed_args['-s'] = '0'
  parsed_args['-c'] = ''


  current_key = ''
  current_arg = ''
  for token in temp_args:
    if token in parsed_args:
      if current_key != '':
        parsed_args[current_key] = current_arg
        current_arg = ''
      current_key = token
    elif current_key != '':
      current_arg = current_arg + token + ' '

  if current_key != '':
    parsed_args[current_key] = current_arg

  return parsed_args


"""
Main function
"""
def main():
  args = process_options(sys.argv)

  if not os.path.exists(pin):
    print('pin doesn\'t exist')
    sys.exit(0)


  os.system('%s -t %s -thread %s -max %s -skip %s -- %s' % (pin, tracegen, args['-t'], args['-n'], args['-s'], args['-c']))


if __name__ == '__main__':
  main()
