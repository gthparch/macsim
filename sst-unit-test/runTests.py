#!/usr/bin/python

import os
import sys
import subprocess
import glob
import difflib
import re


def main():
  current_dir = os.getcwd()

  sst_exec_exist = False
  for path in os.environ['PATH'].split(':'):
    if os.path.exists('%s/sst' % (path)):
      sst_exec_exist = True
      break

  if not sst_exec_exist:
    print('sst executable does not exist.... exit')
    sys.exit(-1)


  trace_dir = '%s/traces' % current_dir
  refer_dir = '%s/references' % current_dir
  #print('trace directory: {}'.format(trace_dir))
  #print('reference directory: {}'.format(refer_dir))

  trace = 'vectoradd'
  for test in glob.glob('sdl*.py'):
    ## Run trace
    os.system('sst {0} > /dev/null'.format(test))

    ## Compare against reference results
    sdl_filename = os.path.splitext(os.path.basename(test))[0]
    golden_dir = '{0}/{1}/{2}'.format(refer_dir, trace, sdl_filename)
    result_dir = '{0}/results'.format(os.getcwd())
    #print('golden_dir: {}'.format(golden_dir))
    #print('result_dir: {}'.format(result_dir))

    match_fail = False
    stats = glob.glob('{0}/general.stat.out'.format(golden_dir))
    for stat in stats:
      golden_output = '{0}/{1}'.format(golden_dir, os.path.basename(stat))
      test_output = '{0}/{1}'.format(result_dir, os.path.basename(stat))

      if not os.path.exists(test_output):
        match_fail = True
        break

      diff = difflib.unified_diff(open(test_output).readlines(), open(golden_output).readlines())
      for line in diff:
        line = line.rstrip('\n')
        if line: ## Skip empty lines
          if line[0] == '-' and line[1] != '-':
            if re.match('-CYC_COUNT_TOT', line):
              match_fail = True
              break

      if match_fail:
        break

    if match_fail:
      print('{0} failed'.format(test))
    else:
      print('{0} success'.format(test))

    os.system('rm -f NULL trace_debug.out')
    os.system('rm -rf results')


if __name__ == '__main__':
  main()
