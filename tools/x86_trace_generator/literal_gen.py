#!/usr/bin/python


import argparse
import sys
import os
import re

#########################################################################################

#########################################################################################
def process_options():
  parser = argparse.ArgumentParser(description='xxx')
  parser.add_argument('-pinhome', action='store', dest='pinhome', help='')

  return parser


def get_opcode_literal(pinhome):
  xed_list = []
  xed_category_file = pinhome + '/extras/xed2-intel64/include/xed-category-enum.h'

  file = open(xed_category_file, 'r')
  for line in file:
    if re.match('  XED_CATEGORY', line):
      xed_list.append(line.lstrip(' ').rstrip('\n').rstrip(','))
  file.close()

  print('string tr_opcode_names[%d] = {' % (len(xed_list) - 1 + 9))
  for ii in range(0, len(xed_list) - 1):
    print('  "%s",' % xed_list[ii])
  print('  "TR_MUL",')
  print('  "TR_DIV",')
  print('  "TR_FMUL",')
  print('  "TR_FDIV",')
  print('  "TR_NOP",')
  print('  "PREFETCH_NTA",')
  print('  "PREFETCH_T0",')
  print('  "PREFETCH_T1",')
  print('  "PREFETCH_T2",')
  print('};')


def get_reg_literal(pinhome):
  xed_reg_file = pinhome + '/source/include/gen/reg_ia32.PH'
  file = open(xed_reg_file, 'r')

  ## #if #else is not closed
  flag_open = False
  for line in file:
    if re.search('REG_APPLICATION_LAST', line):
      break

    #if #endif
    if re.match('^#if', line) or re.match('#else', line):
      print line.rstrip('\n')
      flag_open = True
      continue

    if re.match('^#if', line) or re.match('^#endif', line) or re.match('#else', line):
      print line.rstrip('\n')
      flag_open = False
      continue

    if re.match('^\s+REG_', line) and not re.search('=', line) and not re.match('^\s+REG_PIN', line):
#      print(line.rstrip('\n'))
#      print(line.find(','))
#      print(len(line))
      if line.find(',') == -1 or line.find(',') != len(line) - 2:
        continue

      line = line[:line.find(',')].lstrip(' ')
      ##line = line[:line.find(',')].lstrip(' ')
      # R[A,B,C,D]X, E[A,B,C,D]X, [A,B,C,D]X, [A,B,C,D]H, [A,B,C,D]L
      if re.search('REG_RAX', line) or re.search('REG_EAX', line) or re.search('REG_AL', line) or re.search('REG_AH', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_EAX;' % line)
      elif re.search('REG_RBX', line) or re.search('REG_EBX', line) or re.search('REG_BL', line) or re.search('REG_BH', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_EBX;' % line)
      elif re.search('REG_RCX', line) or re.search('REG_ECX', line) or re.search('REG_CL', line) or re.search('REG_CH', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_ECX;' % line)
      elif re.search('REG_RDX', line) or re.search('REG_EDX', line) or re.search('REG_DL', line) or re.search('REG_DH', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_EDX;' % line)

      # R[S,B]P, E[S,B]P, [S,B]P, [S,B]PL
      elif re.search('REG_RSP', line) or re.search('REG_ESP', line) or re.search('REG_SP', line) or re.search('REG_SPL', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_ESP;' % line)
      elif re.search('REG_RBP', line) or re.search('REG_EBP', line) or re.search('REG_BP', line) or re.search('REG_BPL', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_EBP;' % line)

      # R[S,D]I, E[S,D]I, [S,D]I, [S,D]IL
      elif re.search('REG_RSI', line) or re.search('REG_ESI', line) or re.search('REG_SI', line) or re.search('REG_SIL', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_ESI;' % line)
      elif re.search('REG_RDI', line) or re.search('REG_EDI', line) or re.search('REG_DI', line) or re.search('REG_DIL', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_EDI;' % line)
      
      # RIP, EIP, IP
      elif re.search('REG_RIP', line) or re.search('REG_EIP', line) or re.search('REG_IP', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::REG_EIP;' % line)

      # MM, EMM, XMM, YMM
      elif re.search('REG_MM', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::%s;' % (line, line.replace('MM', 'XMM')))
      elif re.search('REG_EMM', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::%s;' % (line, line.replace('EMM', 'XMM')))
      elif re.search('REG_YMM', line):
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::%s;' % (line, line.replace('YMM', 'XMM')))


      # R8~R15 
      elif re.search('REG_R[0-9]', line):
        if line[5] == '1': 
          print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::%s;' % (line, line[:line.find('_')+4]))
        else:
          print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::%s;' % (line, line[:line.find('_')+3]))
      else:
        print('  reg_compress[LEVEL_BASE::%s] = LEVEL_BASE::%s;' % (line, line))
  file.close()
  if flag_open:
    print('#endif')

      
#########################################################################################
# main function
#########################################################################################
def main():
  # parse arguments
  parser = process_options()
  args = parser.parse_args()

  if not args.pinhome and not 'PINHOME' in os.environ:
    print('error: PINHOME -pinhome not specified')
    exit(0)

  get_opcode_literal(args.pinhome)
#  get_reg_literal(args.pinhome)


#########################################################################################
if __name__ == '__main__':
  main()


#########################################################################################
# End of file
#########################################################################################
