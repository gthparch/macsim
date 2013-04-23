#!/usr/bin/python


import os
import sys
import argparse
import datetime


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
  parsed_args['-pp'] = '0'


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


def pinpoint_tracegen(args):
  # Trace Generator Path Setting
  pin_dir        = '/home/jaekyu/software/pin-2.12-56759-gcc.4.4.7-linux' ### FIXME
  pin_home       = '%s/source/tools' % pin_dir
  pin_exe        = '%s/pin' % pin_dir
  isimpoint_path = '%s/PinPoints/obj-intel64/isimpoint.so' % pin_home
  simpoint_path  = '%s/PinPoints/bin/simpoint' % pin_home
  ppgen_path     = '%s/PinPoints/bin/ppgen.3' % pin_home
  current_dir    = os.getcwd() 
  current_pid    = os.getpid()

  current_time = str(datetime.datetime.now())
  current_time = current_time[:current_time.find('.')]
  current_time = current_time[0:current_time.find(' ')] + '_' + current_time[current_time.find(' ')+1:]
  
  output_dir     = '%s/trace_%s' % (current_dir, current_time)

  if args['-n'].rstrip(' ') == '0':
    args['-n'] = '200000000'
    

  # File open
  script_file = open('%s/%s.tracegen.py' % (current_dir, current_pid), 'w')
  script_file.write('#!/usr/bin/python\n\n')
  script_file.write('# -t : %s\n'   % (args['-t']))
  script_file.write('# -n : %s\n'   % (args['-n']))
  script_file.write('# -s : %s\n'   % (args['-s']))
  script_file.write('# -c : %s\n\n' % (args['-c']))
  script_file.write('import os\n')
  script_file.write('import time\n\n')


  # bb
  script_file.write('print(\'>>>>>>>>>>>>>>>>>>>> BB\')\n')
  script_file.write('os.system(\'mkdir -p %s/bbdata\')\n' % (output_dir))
  bb_file = '%s/bbdata/bbfile'% output_dir
  script_file.write('os.system(\'rm -rf %s.T.0.bb\')\n' % bb_file)
  script_file.write('os.system(\'%s -t %s -o %s -slice_size %s -- %s\')\n' 
      % (pin_exe, isimpoint_path, bb_file, args['-n'], args['-c']))
  script_file.write('time.sleep(10)\n\n')


  # simpoint
  bb_data = '%s.T.0.bb' % bb_file
  script_file.write('print(\'>>>>>>>>>>>>>>>>>>>> SimPoint\')\n')
  script_file.write('os.system(\'%s -loadFVFile %s -maxK 1 -coveragePct 0.9999 -saveSimpoints %s.simpoints -saveSimpointWeights %s.weights -saveLabels %s.labels > %s.simpointout\')\n' 
      % (simpoint_path, bb_data, bb_data, bb_data, bb_data, bb_data))
  script_file.write('time.sleep(10)\n\n')


  # pp
  pp_file = '%s/ppdata/ppoint' % output_dir
  script_file.write('print(\'>>>>>>>>>>>>>>>>>>>> PP\')\n')
  script_file.write('os.system(\'mkdir -p %s/ppdata\')\n' % output_dir) 
  script_file.write('os.system(\'%s %s %s %s.simpoints.lpt0.9999 %s.weights.lpt0.9999 %s.labels 0 &> %s.ppgenout\')\n'
      %(ppgen_path, pp_file, bb_data, bb_data, bb_data, bb_data, bb_data))
  script_file.write('time.sleep(10)\n\n')


  # Read PP file to get the starting point
  script_file.write('ppfile = open(\'%s.1.pp\', \'r\')\n' % pp_file)
  script_file.write('starting = \'0\'\n')
  script_file.write('length = \'200000000\'\n')
  script_file.write('for line in ppfile:\n')
  script_file.write('  line = line.rstrip(\'\\n\')\n')
  script_file.write('  tokens = filter(None, line.split(\' \'))\n')
  script_file.write('  if len(tokens) > 0 and tokens[0] == \'region\':\n')
  script_file.write('    starting = tokens[6]\n')
  script_file.write('    length = tokens[5]\n')
  script_file.write('ppfile.close()\n\n')


  # trace generation
  script_file.write('print(\'>>>>>>>>>>>>>>>>>>>> Trace Generation\')\n')
  script_file.write('print(\'>>>>>>>>>>>>>>>>>>>> PP : %s\' %s)\n' % ('%s', '% (starting)'))
  script_file.write('os.system(\'mkdir -p %s/pin_traces\')\n' % output_dir) 
  trace_file = '%s/pin_traces/trace' % output_dir
  script_file.write('os.system(\'%s -t %s -skip %s -max %s -tracename %s -dump 1 -dump_file %s.dump -- %s\' %s)\n\n'
      % (pin_exe, tracegen, '%s', '%s', trace_file, trace_file, args['-c'], '% (starting, length)'))

  # file close
  script_file.close()
  os.system('chmod +x %s/%s.tracegen.py' % (current_dir, current_pid))
      
#  os.system('qsub %s/scripts/%s.%d.tracegen.py' % (current_dir, bench, ii))


"""
Main function
"""
def main():
  args = process_options(sys.argv)

  if not os.path.exists(pin):
    print('pin doesn\'t exist')
    sys.exit(0)

  if args['-c'] == '':
    sys.exit(0)

  ## pinpoint trace generation
  if args['-pp'].rstrip(' ') == '1':
    pinpoint_tracegen(args)
  ## normal mode
  else:
    os.system('%s -t %s -thread %s -max %s -skip %s -- %s' % (pin, tracegen, args['-t'], args['-n'], args['-s'], args['-c']))


if __name__ == '__main__':
  main()
