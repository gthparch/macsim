#!/usr/bin/env python
#########################################################################################
# Description:
# #   download trace files from Dropbox URL
# # Note:
# #   - Download all trace files if there is no argument
# #     Give trace file name as argument if you want to download specific trace file
# #########################################################################################
import os
import tarfile
import urllib
from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("-trace_file", dest="trace_file",  help="Trace file to download")
args = parser.parse_args()

URL = {}
URL['stride'] = 'https://www.dropbox.com/s/98jplcxoox3cyv5/stride.tar.gz?dl=1'
URL['cache_cl'] = 'https://www.dropbox.com/s/8n20vxsz8194m6q/cache_cl.tar.gz?dl=1'
URL['cachesize'] = 'https://www.dropbox.com/s/igl3sjmzvzf5sum/cachesize.tar.gz?dl=1'
URL['gmembandwidth_local_offset'] = 'https://www.dropbox.com/s/fhll1a9pz9ijo88/gmembandwidth_local_offset.tar.gz?dl=1'
URL['mem'] = 'https://www.dropbox.com/s/2g0mwq7zpp08q5u/mem.tar.gz?dl=1'
#URL['single_prec'] = ''
#URL['stride_independent_interleaved'] = ''
URL['working_set_1_thread'] = 'https://www.dropbox.com/s/subugv2u0yahwl0/working_set_1_thread.tar.gz?dl=1'
#URL['working_set_1_wi_region_1k'] = ''
URL['intel_sample'] = 'https://www.dropbox.com/s/l89ts8v8yy5gyi6/intel_sample.tar.gz?dl=1'
URL['rodinia'] = 'https://www.dropbox.com/s/vcgql5yc3s55kko/rodinia.tar.gz?dl=1'

try:
  os.system('mkdir ../traces')
except:
  pass

if args.trace_file: # Download specific trace file
  fn = args.trace_file
  print('Downloading '+fn+'.tar.gz...')
  file_tmp = urllib.urlretrieve(url, filename=fn)[0]
  with tarfile.open(file_tmp, 'r:*') as f:
    f.extractall("../traces/"+fn)
  print(fn+'.tar.gz is downloaded. (../traces/'+fn+')')
  os.system('rm -rf '+fn)

else: # Download all trace files one by one
  for fn, url in URL.iteritems():
    print('Downloading '+fn+'.tar.gz...')
    file_tmp = urllib.urlretrieve(url, filename=fn)[0]
    with tarfile.open(file_tmp, 'r:*') as f:
      f.extractall("../traces/"+fn)
    print(fn+'.tar.gz is downloaded. (../traces/'+fn+')')
    os.system('rm -rf '+fn)
