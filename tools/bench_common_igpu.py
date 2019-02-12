#!/usr/bin/python
#########################################################################################
# Author : Jaekyu Lee (kacear@gmail.com)
# Date : 7/14/2011
# Description
#   All benchmark suite lists for macsim simulations
# Note:
#   - How to add a new suite?
#     SUITES[suite_name] = ['xx', 'yy', 'zz']
#   - If you want to create list of benchmarks,
#     for example, SUITES['a'] = {SUITES['aa'], SUITES['bb'}, then you need to write it as
#     SUITES['a'] = sum([SUITES['aa'], SUITES['bb']], [])
#   - Core suites (apply to all users) before 'Local Copy tag'
#   - Add local suites after Local Copy tag
# TODO:
#   - remove @ref
#########################################################################################


from collections import defaultdict
import collections
import sys
import optparse
import argparse

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

## Suites
SUITES = defaultdict(list)

## Core Suites ###########################################################################

## Intel GPU
SUITES['igpu-single-prec'] = ['single-prec-32@ref', 'single-prec-64@ref', 'single-prec-96@ref', 'single-prec-128@ref', 'single-prec-160@ref', 'single-prec-192@ref', \
    'single-prec-224@ref', 'single-prec-256@ref', 'single-prec-288@ref', 'single-prec-320@ref', 'single-prec-352@ref', 'single-prec-384@ref', 'single-prec-416@ref', \
    'single-prec-448@ref', 'single-prec-480@ref', 'single-prec-512@ref', 'single-prec-544@ref', 'single-prec-576@ref', 'single-prec-608@ref', 'single-prec-640@ref', \
    'single-prec-672@ref', 'single-prec-704@ref', 'single-prec-736@ref', 'single-prec-768@ref', 'single-prec-800@ref', 'single-prec-832@ref', 'single-prec-864@ref', \
    'single-prec-896@ref', 'single-prec-928@ref', 'single-prec-960@ref', 'single-prec-992@ref', 'single-prec-1024@ref', 'single-prec-1056@ref', 'single-prec-1088@ref', \
    'single-prec-1120@ref', 'single-prec-1152@ref', 'single-prec-1184@ref', 'single-prec-1216@ref', 'single-prec-1248@ref', 'single-prec-1280@ref', 'single-prec-1312@ref', \
    'single-prec-1344@ref', 'single-prec-1376@ref', 'single-prec-1408@ref', 'single-prec-1440@ref', 'single-prec-1472@ref', 'single-prec-1504@ref', 'single-prec-1536@ref', \
    'single-prec-1568@ref', 'single-prec-1600@ref', 'single-prec-1632@ref', 'single-prec-1664@ref', 'single-prec-1696@ref', 'single-prec-1728@ref', 'single-prec-1760@ref', \
    'single-prec-1792@ref', 'single-prec-1824@ref', 'single-prec-1856@ref', 'single-prec-1888@ref', 'single-prec-1920@ref', 'single-prec-1952@ref', 'single-prec-1984@ref', \
    'single-prec-2016@ref', 'single-prec-2048@ref', 'single-prec-2080@ref', 'single-prec-2112@ref', 'single-prec-2144@ref', 'single-prec-2176@ref', 'single-prec-2208@ref', \
    'single-prec-2240@ref', 'single-prec-2272@ref', 'single-prec-2304@ref', 'single-prec-2336@ref', 'single-prec-2368@ref', 'single-prec-2400@ref', 'single-prec-2432@ref', \
    'single-prec-2464@ref', 'single-prec-2496@ref', 'single-prec-2528@ref', 'single-prec-2560@ref', 'single-prec-2592@ref', 'single-prec-2624@ref', 'single-prec-2656@ref', \
    'single-prec-2688@ref', 'single-prec-2720@ref', 'single-prec-2752@ref', 'single-prec-2784@ref', 'single-prec-2816@ref', 'single-prec-2848@ref', 'single-prec-2880@ref', \
    'single-prec-2912@ref', 'single-prec-2944@ref', 'single-prec-2976@ref', 'single-prec-3008@ref', 'single-prec-3040@ref', 'single-prec-3072@ref', 'single-prec-3104@ref', \
    'single-prec-3136@ref', 'single-prec-3168@ref', 'single-prec-3200@ref', 'single-prec-3232@ref', 'single-prec-3264@ref', 'single-prec-3296@ref', 'single-prec-3328@ref', \
    'single-prec-3360@ref', 'single-prec-3392@ref', 'single-prec-3424@ref', 'single-prec-3456@ref', 'single-prec-3488@ref', 'single-prec-3520@ref', 'single-prec-3552@ref', \
    'single-prec-3584@ref', 'single-prec-3616@ref', 'single-prec-3648@ref', 'single-prec-3680@ref', 'single-prec-3712@ref', 'single-prec-3744@ref', 'single-prec-3776@ref', \
    'single-prec-3808@ref', 'single-prec-3840@ref', 'single-prec-3872@ref', 'single-prec-3904@ref', 'single-prec-3936@ref', 'single-prec-3968@ref', 'single-prec-4000@ref', \
    'single-prec-4032@ref', 'single-prec-4064@ref', 'single-prec-4096@ref', 'single-prec-4128@ref', 'single-prec-4160@ref', 'single-prec-4192@ref', 'single-prec-4224@ref', \
    'single-prec-4256@ref', 'single-prec-4288@ref', 'single-prec-4320@ref', 'single-prec-4352@ref', 'single-prec-4384@ref', 'single-prec-4416@ref', 'single-prec-4448@ref', \
    'single-prec-4480@ref', 'single-prec-4512@ref', 'single-prec-4544@ref', 'single-prec-4576@ref', 'single-prec-4608@ref', 'single-prec-4640@ref', 'single-prec-4672@ref', \
    'single-prec-4704@ref', 'single-prec-4736@ref', 'single-prec-4768@ref', 'single-prec-4800@ref', 'single-prec-4832@ref', 'single-prec-4864@ref', 'single-prec-4896@ref', \
    'single-prec-4928@ref', 'single-prec-4960@ref', 'single-prec-4992@ref', 'single-prec-5024@ref', 'single-prec-5056@ref', 'single-prec-5088@ref', 'single-prec-5120@ref', \
    'single-prec-5152@ref', 'single-prec-5184@ref', 'single-prec-5216@ref', 'single-prec-5248@ref', 'single-prec-5280@ref', 'single-prec-5312@ref', 'single-prec-5344@ref', \
    'single-prec-5376@ref', 'single-prec-5408@ref', 'single-prec-5440@ref', 'single-prec-5472@ref', 'single-prec-5504@ref', 'single-prec-5536@ref', 'single-prec-5568@ref', \
    'single-prec-5600@ref', 'single-prec-5632@ref', 'single-prec-5664@ref', 'single-prec-5696@ref', 'single-prec-5728@ref', 'single-prec-5760@ref', 'single-prec-5792@ref', \
    'single-prec-5824@ref', 'single-prec-5856@ref', 'single-prec-5888@ref', 'single-prec-5920@ref', 'single-prec-5952@ref', 'single-prec-5984@ref', 'single-prec-6016@ref', \
    'single-prec-6048@ref', 'single-prec-6080@ref', 'single-prec-6112@ref', 'single-prec-6144@ref', 'single-prec-6176@ref', 'single-prec-6208@ref', 'single-prec-6240@ref', \
    'single-prec-6272@ref', 'single-prec-6304@ref', 'single-prec-6336@ref', 'single-prec-6368@ref']
SUITES['igpu-cache-cl'] = ['cache-cl-1KB@ref', 'cache-cl-1024KB@ref', 'cache-cl-4096KB@ref', 'cache-cl-8192KB@ref']
SUITES['igpu-cachesize'] = ['cachesize-1KB@ref', 'cachesize-2KB@ref', 'cachesize-4KB@ref', 'cachesize-8KB@ref', 'cachesize-16KB@ref', 'cachesize-32KB@ref', 'cachesize-64KB@ref', 'cachesize-128KB@ref', 'cachesize-256KB@ref', 'cachesize-512KB@ref', 'cachesize-1024KB@ref', 'cachesize-2048KB@ref', 'cachesize-4096KB@ref', 'cachesize-8192KB@ref', 'cachesize-16384KB@ref', 'cachesize-32768KB@ref', 'cachesize-65536KB@ref', 'cachesize-131072KB@ref', 'cachesize-262144KB@ref', 'cachesize-524288KB@ref', 'cachesize-1048576KB@ref']
SUITES['igpu-mem'] = ['mem-gmem-coalesced@ref', 'mem-gmem-uncoalesced@ref', 'mem-localmem-coalesced@ref']
SUITES['igpu-gmembandwidth-local-offset'] = ['gmembandwidth-local-offset-32@ref', 'gmembandwidth-local-offset-64@ref', 'gmembandwidth-local-offset-128@ref', 'gmembandwidth-local-offset-256@ref', 'gmembandwidth-local-offset-512@ref', 'gmembandwidth-local-offset-1024@ref', 'gmembandwidth-local-offset-2048@ref', 'gmembandwidth-local-offset-4096@ref', 'gmembandwidth-local-offset-8192@ref', 'gmembandwidth-local-offset-16384@ref', 'gmembandwidth-local-offset-32768@ref', 'gmembandwidth-local-offset-65536@ref', 'gmembandwidth-local-offset-131072@ref', 'gmembandwidth-local-offset-262144@ref', 'gmembandwidth-local-offset-524288@ref', 'gmembandwidth-local-offset-1048576@ref', 'gmembandwidth-local-offset-2097152@ref', 'gmembandwidth-local-offset-4194304@ref', 'gmembandwidth-local-offset-8388608@ref', 'gmembandwidth-local-offset-16777216@ref']
SUITES['igpu-stride-interleave-mini'] = ['stride-interleave-mini-1@ref', 'stride-interleave-mini-2@ref', 'stride-interleave-mini-4@ref', 'stride-interleave-mini-8@ref', 'stride-interleave-mini-16@ref'] 

SUITES['igpu-stride-part-mini'] = ['stride-part-mini-1@ref', 'stride-part-mini-2@ref', 'stride-part-mini-4@ref', 'stride-part-mini-8@ref', 'stride-part-mini-16@ref'] 
SUITES['igpu-st-hbw'] = ['st-intlv-ind-64-32-16@ref', 'st-intlv-ind-64-64-16@ref', 'st-intlv-ind-256-64-16@ref', 'st-intlv-ind-256-128-16@ref', 'st-intlv-ind-256-256-16@ref', 'st-intlv-ind-512-16-16@ref', 'st-intlv-ind-512-32-16@ref', 'st-intlv-ind-512-64-16@ref', 'st-intlv-ind-512-128-16@ref', 'st-intlv-ind-512-256-16@ref']
SUITES['igpu-st-1bw']=['st-intlv-ind-4-16-1@ref', 'st-intlv-ind-4-32-1@ref', 'st-intlv-ind-4-64-1@ref', 'st-intlv-ind-4-128-1@ref', 'st-intlv-ind-4-256-1@ref']
SUITES['igpu-st-lbw']=['st-intlv-ind-1-16-1@ref', 'st-intlv-ind-1-32-1@ref', 'st-intlv-ind-1-64-1@ref', 'st-intlv-ind-1-128-1@ref', 'st-intlv-ind-1-256-1@ref', 'st-intlv-ind-2-16-1@ref', 'st-intlv-ind-2-32-1@ref', 'st-intlv-ind-2-64-1@ref', 'st-intlv-ind-2-128-1@ref', 'st-intlv-ind-2-256-1@ref', 'st-intlv-ind-4-16-1@ref', 'st-intlv-ind-4-32-1@ref', 'st-intlv-ind-4-64-1@ref', 'st-intlv-ind-4-128-1@ref', 'st-intlv-ind-4-256-1@ref', 'st-intlv-ind-8-16-1@ref', 'st-intlv-ind-8-32-1@ref', 'st-intlv-ind-4-16-2@ref', 'st-intlv-ind-4-16-4@ref', 'st-intlv-ind-4-16-8@ref', 'st-intlv-ind-4-16-16@ref']
SUITES['igpu-st-bw']=sum([SUITES['igpu-st-hbw'],SUITES['igpu-st-lbw']],[])
SUITES['igpu-st-intlv-ind'] =  ['st-intlv-ind-1-16-1@ref', 'st-intlv-ind-1-32-1@ref', 'st-intlv-ind-1-64-1@ref', 'st-intlv-ind-1-128-1@ref', 'st-intlv-ind-1-256-1@ref', 'st-intlv-ind-2-16-1@ref', 'st-intlv-ind-2-32-1@ref', 'st-intlv-ind-2-64-1@ref', 'st-intlv-ind-2-128-1@ref', 'st-intlv-ind-2-256-1@ref', 'st-intlv-ind-4-16-1@ref', 'st-intlv-ind-4-32-1@ref', 'st-intlv-ind-4-64-1@ref', 'st-intlv-ind-4-128-1@ref', 'st-intlv-ind-4-256-1@ref', 'st-intlv-ind-8-16-1@ref', 'st-intlv-ind-8-32-1@ref', 'st-intlv-ind-8-64-1@ref', 'st-intlv-ind-8-128-1@ref', 'st-intlv-ind-8-256-1@ref', 'st-intlv-ind-16-16-1@ref', 'st-intlv-ind-16-32-1@ref', 'st-intlv-ind-16-64-1@ref', 'st-intlv-ind-16-128-1@ref', 'st-intlv-ind-16-256-1@ref', 'st-intlv-ind-32-16-1@ref', 'st-intlv-ind-32-32-1@ref', 'st-intlv-ind-32-64-1@ref', 'st-intlv-ind-32-128-1@ref', 'st-intlv-ind-32-256-1@ref', 'st-intlv-ind-64-16-1@ref', 'st-intlv-ind-64-32-1@ref', 'st-intlv-ind-64-64-1@ref', 'st-intlv-ind-64-128-1@ref', 'st-intlv-ind-64-256-1@ref', 'st-intlv-ind-128-16-1@ref', 'st-intlv-ind-128-32-1@ref', 'st-intlv-ind-128-64-1@ref', 'st-intlv-ind-128-128-1@ref', 'st-intlv-ind-128-256-1@ref', 'st-intlv-ind-256-16-1@ref', 'st-intlv-ind-256-32-1@ref', 'st-intlv-ind-256-64-1@ref', 'st-intlv-ind-256-128-1@ref', 'st-intlv-ind-256-256-1@ref', 'st-intlv-ind-512-16-1@ref', 'st-intlv-ind-512-32-1@ref', 'st-intlv-ind-512-64-1@ref', 'st-intlv-ind-512-128-1@ref', 'st-intlv-ind-512-256-1@ref', 'st-intlv-ind-1-16-2@ref', 'st-intlv-ind-1-32-2@ref', 'st-intlv-ind-1-64-2@ref', 'st-intlv-ind-1-128-2@ref', 'st-intlv-ind-1-256-2@ref', 'st-intlv-ind-2-16-2@ref', 'st-intlv-ind-2-32-2@ref', 'st-intlv-ind-2-64-2@ref', 'st-intlv-ind-2-128-2@ref', 'st-intlv-ind-2-256-2@ref', 'st-intlv-ind-4-16-2@ref', 'st-intlv-ind-4-32-2@ref', 'st-intlv-ind-4-64-2@ref', 'st-intlv-ind-4-128-2@ref', 'st-intlv-ind-4-256-2@ref', 'st-intlv-ind-8-16-2@ref', 'st-intlv-ind-8-32-2@ref', 'st-intlv-ind-8-64-2@ref', 'st-intlv-ind-8-128-2@ref', 'st-intlv-ind-8-256-2@ref', 'st-intlv-ind-16-16-2@ref', 'st-intlv-ind-16-32-2@ref', 'st-intlv-ind-16-64-2@ref', 'st-intlv-ind-16-128-2@ref', 'st-intlv-ind-16-256-2@ref', 'st-intlv-ind-32-16-2@ref', 'st-intlv-ind-32-32-2@ref', 'st-intlv-ind-32-64-2@ref', 'st-intlv-ind-32-128-2@ref', 'st-intlv-ind-32-256-2@ref', 'st-intlv-ind-64-16-2@ref', 'st-intlv-ind-64-32-2@ref', 'st-intlv-ind-64-64-2@ref', 'st-intlv-ind-64-128-2@ref', 'st-intlv-ind-64-256-2@ref', 'st-intlv-ind-128-16-2@ref', 'st-intlv-ind-128-32-2@ref', 'st-intlv-ind-128-64-2@ref', 'st-intlv-ind-128-128-2@ref', 'st-intlv-ind-128-256-2@ref', 'st-intlv-ind-256-16-2@ref', 'st-intlv-ind-256-32-2@ref', 'st-intlv-ind-256-64-2@ref', 'st-intlv-ind-256-128-2@ref', 'st-intlv-ind-256-256-2@ref', 'st-intlv-ind-512-16-2@ref', 'st-intlv-ind-512-32-2@ref', 'st-intlv-ind-512-64-2@ref', 'st-intlv-ind-512-128-2@ref', 'st-intlv-ind-512-256-2@ref', 'st-intlv-ind-1-16-4@ref', 'st-intlv-ind-1-32-4@ref', 'st-intlv-ind-1-64-4@ref', 'st-intlv-ind-1-128-4@ref', 'st-intlv-ind-1-256-4@ref', 'st-intlv-ind-2-16-4@ref', 'st-intlv-ind-2-32-4@ref', 'st-intlv-ind-2-64-4@ref', 'st-intlv-ind-2-128-4@ref', 'st-intlv-ind-2-256-4@ref', 'st-intlv-ind-4-16-4@ref', 'st-intlv-ind-4-32-4@ref', 'st-intlv-ind-4-64-4@ref', 'st-intlv-ind-4-128-4@ref', 'st-intlv-ind-4-256-4@ref', 'st-intlv-ind-8-16-4@ref', 'st-intlv-ind-8-32-4@ref', 'st-intlv-ind-8-64-4@ref', 'st-intlv-ind-8-128-4@ref', 'st-intlv-ind-8-256-4@ref', 'st-intlv-ind-16-16-4@ref', 'st-intlv-ind-16-32-4@ref', 'st-intlv-ind-16-64-4@ref', 'st-intlv-ind-16-128-4@ref', 'st-intlv-ind-16-256-4@ref', 'st-intlv-ind-32-16-4@ref', 'st-intlv-ind-32-32-4@ref', 'st-intlv-ind-32-64-4@ref', 'st-intlv-ind-32-128-4@ref', 'st-intlv-ind-32-256-4@ref', 'st-intlv-ind-64-16-4@ref', 'st-intlv-ind-64-32-4@ref', 'st-intlv-ind-64-64-4@ref', 'st-intlv-ind-64-128-4@ref', 'st-intlv-ind-64-256-4@ref', 'st-intlv-ind-128-16-4@ref', 'st-intlv-ind-128-32-4@ref', 'st-intlv-ind-128-64-4@ref', 'st-intlv-ind-128-128-4@ref', 'st-intlv-ind-128-256-4@ref', 'st-intlv-ind-256-16-4@ref', 'st-intlv-ind-256-32-4@ref', 'st-intlv-ind-256-64-4@ref', 'st-intlv-ind-256-128-4@ref', 'st-intlv-ind-256-256-4@ref', 'st-intlv-ind-512-16-4@ref', 'st-intlv-ind-512-32-4@ref', 'st-intlv-ind-512-64-4@ref', 'st-intlv-ind-512-128-4@ref', 'st-intlv-ind-512-256-4@ref', 'st-intlv-ind-1-16-8@ref', 'st-intlv-ind-1-32-8@ref', 'st-intlv-ind-1-64-8@ref', 'st-intlv-ind-1-128-8@ref', 'st-intlv-ind-1-256-8@ref', 'st-intlv-ind-2-16-8@ref', 'st-intlv-ind-2-32-8@ref', 'st-intlv-ind-2-64-8@ref', 'st-intlv-ind-2-128-8@ref', 'st-intlv-ind-2-256-8@ref', 'st-intlv-ind-4-16-8@ref', 'st-intlv-ind-4-32-8@ref', 'st-intlv-ind-4-64-8@ref', 'st-intlv-ind-4-128-8@ref', 'st-intlv-ind-4-256-8@ref', 'st-intlv-ind-8-16-8@ref', 'st-intlv-ind-8-32-8@ref', 'st-intlv-ind-8-64-8@ref', 'st-intlv-ind-8-128-8@ref', 'st-intlv-ind-8-256-8@ref', 'st-intlv-ind-16-16-8@ref', 'st-intlv-ind-16-32-8@ref', 'st-intlv-ind-16-64-8@ref', 'st-intlv-ind-16-128-8@ref', 'st-intlv-ind-16-256-8@ref', 'st-intlv-ind-32-16-8@ref', 'st-intlv-ind-32-32-8@ref', 'st-intlv-ind-32-64-8@ref', 'st-intlv-ind-32-128-8@ref', 'st-intlv-ind-32-256-8@ref', 'st-intlv-ind-64-16-8@ref', 'st-intlv-ind-64-32-8@ref', 'st-intlv-ind-64-64-8@ref', 'st-intlv-ind-64-128-8@ref', 'st-intlv-ind-64-256-8@ref', 'st-intlv-ind-128-16-8@ref', 'st-intlv-ind-128-32-8@ref', 'st-intlv-ind-128-64-8@ref', 'st-intlv-ind-128-128-8@ref', 'st-intlv-ind-128-256-8@ref', 'st-intlv-ind-256-16-8@ref', 'st-intlv-ind-256-32-8@ref', 'st-intlv-ind-256-64-8@ref', 'st-intlv-ind-256-128-8@ref', 'st-intlv-ind-256-256-8@ref', 'st-intlv-ind-512-16-8@ref', 'st-intlv-ind-512-32-8@ref', 'st-intlv-ind-512-64-8@ref', 'st-intlv-ind-512-128-8@ref', 'st-intlv-ind-512-256-8@ref', 'st-intlv-ind-1-16-16@ref', 'st-intlv-ind-1-32-16@ref', 'st-intlv-ind-1-64-16@ref', 'st-intlv-ind-1-128-16@ref', 'st-intlv-ind-1-256-16@ref', 'st-intlv-ind-2-16-16@ref', 'st-intlv-ind-2-32-16@ref', 'st-intlv-ind-2-64-16@ref', 'st-intlv-ind-2-128-16@ref', 'st-intlv-ind-2-256-16@ref', 'st-intlv-ind-4-16-16@ref', 'st-intlv-ind-4-32-16@ref', 'st-intlv-ind-4-64-16@ref', 'st-intlv-ind-4-128-16@ref', 'st-intlv-ind-4-256-16@ref', 'st-intlv-ind-8-16-16@ref', 'st-intlv-ind-8-32-16@ref', 'st-intlv-ind-8-64-16@ref', 'st-intlv-ind-8-128-16@ref', 'st-intlv-ind-8-256-16@ref', 'st-intlv-ind-16-16-16@ref', 'st-intlv-ind-16-32-16@ref', 'st-intlv-ind-16-64-16@ref', 'st-intlv-ind-16-128-16@ref', 'st-intlv-ind-16-256-16@ref', 'st-intlv-ind-32-16-16@ref', 'st-intlv-ind-32-32-16@ref', 'st-intlv-ind-32-64-16@ref', 'st-intlv-ind-32-128-16@ref', 'st-intlv-ind-32-256-16@ref', 'st-intlv-ind-64-16-16@ref', 'st-intlv-ind-64-32-16@ref', 'st-intlv-ind-64-64-16@ref', 'st-intlv-ind-64-128-16@ref', 'st-intlv-ind-64-256-16@ref', 'st-intlv-ind-128-16-16@ref', 'st-intlv-ind-128-32-16@ref', 'st-intlv-ind-128-64-16@ref', 'st-intlv-ind-128-128-16@ref', 'st-intlv-ind-128-256-16@ref', 'st-intlv-ind-256-16-16@ref', 'st-intlv-ind-256-32-16@ref', 'st-intlv-ind-256-64-16@ref', 'st-intlv-ind-256-128-16@ref', 'st-intlv-ind-256-256-16@ref', 'st-intlv-ind-512-16-16@ref', 'st-intlv-ind-512-32-16@ref', 'st-intlv-ind-512-64-16@ref', 'st-intlv-ind-512-128-16@ref', 'st-intlv-ind-512-256-16@ref']

SUITES['igpu-wset-1-th'] =  ['wset-64@ref', 'wset-128@ref', 'wset-256@ref', 'wset-512@ref', 'wset-1024@ref', 'wset-2048@ref', 'wset-4096@ref', 'wset-8192@ref', 'wset-16384@ref', 'wset-32768@ref', 'wset-65536@ref', 'wset-131072@ref', 'wset-262144@ref', 'wset-524288@ref', 'wset-1048576@ref', 'wset-2097152@ref', 'wset-4194304@ref', 'wset-8388608@ref', 'wset-16777216@ref', 'wset-33554432@ref']

SUITES['igpu-wset-1-wi-1k'] =  ['wset-7-1024@ref', 'wset-14-1024@ref', 'wset-21-1024@ref', 'wset-28-1024@ref', 'wset-35-1024@ref', 'wset-42-1024@ref', 'wset-49-1024@ref', 'wset-56-1024@ref', 'wset-63-1024@ref', 'wset-70-1024@ref', 'wset-77-1024@ref', 'wset-84-1024@ref', 'wset-91-1024@ref', 'wset-98-1024@ref', 'wset-105-1024@ref', 'wset-112-1024@ref', 'wset-119-1024@ref', 'wset-126-1024@ref', 'wset-133-1024@ref', 'wset-140-1024@ref', 'wset-147-1024@ref', 'wset-154-1024@ref', 'wset-161-1024@ref', 'wset-168-1024@ref']

SUITES['igpu-interference'] = ['stream_cachesize-1KB@ref', 'stream_cachesize-2KB@ref', 'stream_cachesize-4KB@ref', 'stream_cachesize-8KB@ref', 'stream_cachesize-16KB@ref', 'stream_cachesize-32KB@ref', 'stream_cachesize-64KB@ref', 'stream_cachesize-128KB@ref', 'stream_cachesize-256KB@ref', 'stream_cachesize-512KB@ref', 'stream_cachesize-1024KB@ref', 'stream_cachesize-2048KB@ref', 'stream_cachesize-4096KB@ref', 'stream_cachesize-8192KB@ref', 'stream_cachesize-16384KB@ref', 'stream_cachesize-32768KB@ref', 'stream_cachesize-65536KB@ref', 'stream_cachesize-131072KB@ref', 'stream_cachesize-262144KB@ref', 'stream_cachesize-524288KB@ref', 'stream_cachesize-1048576KB@ref']

SUITES['igpu-intel-sdk'] = ['bitonic-sort@ref', 'gemm@ref', 'median-filter@ref', 'monte-carlo@ref']
SUITES['igpu-rodinia'] = ['backprop@ref', 'bfs@ref', 'hotspot@ref', 'lavaMD@ref', 'pathfinder@ref', 'streamcluster@ref']

#SUITES['igpu-all'] = sum([SUITES['igpu-single-prec'], SUITES['igpu-cache-cl'], SUITES['igpu-mem']],[])
SUITES['igpu-real'] = sum([SUITES['igpu-intel-sdk'], SUITES['igpu-rodinia'],],[])
SUITES['igpu-other']=sum([SUITES['igpu-real'], SUITES['igpu-wset-1-th'], SUITES['igpu-wset-1-wi-1k']],[])


#########################################################################################
# create an argument parser
#########################################################################################
def process_options():
  parser = argparse.ArgumentParser(description='bench_common.py')
  parser.add_argument('-s', action='store', dest='suite', help='suite name - print contents of a suite')
  parser.add_argument('-p', action='store_true', dest='suite_help', help='print all suite names')
  parser.set_defaults(suite_help=False)

  return parser

#########################################################################################
# help message
#########################################################################################
def help_message():
  print('List of suites')
  for key, val in SUITES.items():
    print('  %s: %d benchmarks' % (key, len(val)))

#########################################################################################
# print suite names
#########################################################################################
def print_suites():
  SUITES_ORDERED = collections.OrderedDict(sorted(SUITES.items()))
  print bcolors.HEADER + 'List of suites:' + bcolors.ENDC
  for key, val in SUITES_ORDERED.items():
    print( bcolors.BOLD + ' %s ' % key + bcolors.ENDC)



#########################################################################################
# main function
#########################################################################################
def main(argv):
  # parse arguments
  parser = process_options()
  args = parser.parse_args()

  if args.suite:
    if args.suite in SUITES:
      print bcolors.OKBLUE + "%s:" % (args.suite) + bcolors.ENDC
      for item in SUITES[args.suite]:
        print bcolors.BOLD + "\t%s" % (item) + bcolors.ENDC
    else:
      print bcolors.WARNING + 'There is not SUITES with "%s" name' % (args.suite) + bcolors.ENDC
  elif args.suite_help:
    print_suites()
  else:
    pass


#########################################################################################
if __name__ == '__main__':
  main(sys.argv)


#########################################################################################
# OLD SUTIES
#########################################################################################
'''
SUITES['nacl']=['bzip2-1@nacl', 'bzip2-2@nacl', 'bzip2-3@nacl', 'bzip2-4@nacl', 'bzip2-5@nacl', 'gobmk@nacl','gobmk-2@nacl','gobmk-3@nacl','gobmk-4@nacl', 'lbm@nacl','libquantum@nacl','mcf@nacl','milc@nacl','namd@nacl']
'''
#########################################################################################
# End of file
#########################################################################################



