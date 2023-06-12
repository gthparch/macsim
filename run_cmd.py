#!/usr/bin/env python3

import os
import sys
import datetime
import numpy as np

def main():
    now = datetime.datetime.now()
    date = '%s%d' % (now.strftime("%b").lower(), now.day)
    #date='nov23'
    ptx_simulation = 0;
    igpu_simulation = 0;
    nvbit_simulation = 1;

    # bin = '/fast_data/jaewon/macsim_memsafety_mmu_eval/.opt_build/macsim'
    bin = '/fast_data/echung67/macsim/bin/macsim'
    ## ptx gpu simulation

    # tools = '/home/hyesoon/macsim_aos/internal/tools/run_batch_local.py'
    # tools= '/fast_data/jaewon/macsim_memsafety_mmu_eval/internal/tools/run_batch_local.py'
    tools = '/fast_data/echung67/macsim/internal/tools/run_batch_local.py'
    # get_data_tool = '/home/hyesoon/macsim_aos/internal/tools/get_data.py'
    # get_data_tool = '/fast_data/jaewon/macsim_memsafety_mmu_eval/internal/tools/get_data.py'
    get_data_tool = '/fast_data/echung67/macsim/internal/tools/get_data.py'

    if (ptx_simulation == 1):
        param = '/home/hyesoon/macsim_aos/bin/ptx_test/params.in'
        max_insts = 1000  ## PTX simulation
        #suite = 'ptx14-mb'
        #suite = 'ptx14-all'
        suite = 'aos-mb'
        #suite = 'aos-stream'

        #get_data_suite_list = ['ptx14-all']
        #get_data_suite_list = ['aos-mb']
        #get_data_suite_list = ['aos-stream']
        #suite = 'aos-mb'
        num_sim_cores = 16
    elif (igpu_simulation == 1):
        #param = '/home/hyesoon/macsim_aos/bin/igpu_test/params.in'
        param= '/fast_data/jaewon/macsim_memsafety_mmu_eval/bin/igpu_test/params.in'
        max_insts = 1000000 ## igpu simulation
        suite = 'aos-igpu-2'
        num_sim_cores = 24
        get_data_suite_list = ['aos-igpu-2']
    elif (nvbit_simulation == 1):
        param = '/fast_data/echung67/macsim/bin/params.in'
        max_insts = 1000000 ## nvbit simulation
        suite = 'rodinia31-nvbit-all'
        num_sim_cores = 16
        get_data_suite_list = ['rodinia31-nvbit-all']


   # max_insts = 100000


    max_cycle = 0
    sim_cycle_count= 0
    #max_cycle = 1000000
    forward_progress_limit = 50000000



    perfect_dcache = 0
    enable_physical_mapping = 1


    per_thread_frontend_q_size = 16
    per_thread_allocate_q_size = 16
    per_thread_schedule_q_size = 16


    desc = 'SIMPLE-LAT'
    #base_stat_list = '-stat INST_COUNT_TOT -stat CYC_COUNT_TOT '
    stat_list = '-stat INST_COUNT_TOT -stat CYC_COUNT_TOT -stat L3_HIT_GPU -stat TOTAL_DRAM'
    stat_core_list = ["OP_CAT_GED_SEND", "OP_CAT_GED_ADD", "OP_CAT_GED_ADDC", "OP_CAT_GED_OR", "OP_CAT_GED_AND"]
    #stat_core_list = ["NUM_OF_BOUNDS_CHECKING", "BOUNDS_L0_CACHE_HIT" , "BOUNDS_L1_CACHE_HIT" ,"BOUNDS_INFO_INSERT", "BOUNDS_CHECK_SKIP_STATIC" ]
    #stat_core_list = ["NUM_OF_BOUNDS_CHECKING", "BOUNDS_L0_CACHE_HIT" , "BOUNDS_L1_CACHE_HIT" ,"BOUNDS_INFO_INSERT", "BOUNDS_CHECK_SKIP_STATIC" ]
    for stat_name  in stat_core_list:
        # addr = '%s' %(stat_name)

        #print addr
        # for core_counts in range (0, 16):
        for core_counts in range (0, 1):
            new_stat_list = '-stat %s_CORE_%d' %(stat_name, core_counts)
            stat_list = '%s %s' %(stat_list, new_stat_list)
    base_cmd = '%s -bin %s -param %s -suite %s -cmd' % (tools, bin, param, suite)

    base_cmd = '%s \'num_sim_cores=%d --num_sim_small_cores=%d --sim_cycle_count=%d --max_insts=%d --forward_progress_limit=%d --perfect_dcache=%d --enable_physical_mapping=%d  ' % (base_cmd, num_sim_cores, num_sim_cores, max_cycle, max_insts, forward_progress_limit, perfect_dcache, enable_physical_mapping)

    # bounds_l0_cache_lat bounds_l1_cache_lat  1, 3, -- 3, 10 "

    base_dir = 'nvbit/%s/%d-%d' % (date, max_insts, max_cycle)
    file_name = 'get_data_cmd_%s.txt' %(date)

   # get_data_suite_list = ['aos-ML', 'aos-LA', 'aos-GT', 'aos-GI', 'aos-PS', 'aos-im', 'aos-dm']
    base_test = 0
    l1_lat_list = [ 1]
    l2_lat_list = [3 ]
    # l1_lat_list = [1,2,3]
    #l2_lat_list = [3, 10, 20]
   # l1_lat_list = [ 0, 1, 2, 3 ]
    #l2_lat_list = [ 0, 1, 3, 10, 20]
     #l2_lat_list = [1, 3]
    #l0_entry_list = [ 1, 2, 4, 8]j
    #l0_entry_list = [ 1,2, 4, 8, 16]
    l0_entry_list = [ 4 ]
    with open (file_name, 'w') as f:
        for suite in get_data_suite_list:
            for  enable_bounds_checking in range (0,1) :
                for enable_bounds_static_filter in range (0, 1):
                    for bounds_only_global_load_store in range (0, 1):
                    #for  enable_bounds_checking in range (1,2) :
                        for l0_entry_num in l0_entry_list:
                            for l1_lat  in l1_lat_list:
                                for l2_lat in l2_lat_list:
                                    bounds_l0_cache_lat = l1_lat
                                    bounds_l1_cache_lat = l2_lat
                                    if (base_test == 0 or enable_bounds_checking != 0):
                                        new_desc= '%s-bc-%s-l0lat%s-l1lat%s-l0entry%s-filter-%s-ldst-%s' % (desc, enable_bounds_checking, bounds_l0_cache_lat, bounds_l1_cache_lat, l0_entry_num,enable_bounds_static_filter,bounds_only_global_load_store)

                                        #new_cmd = '%s  --enable_bounds_ids_file=1 --enable_bounds_checking=%d --bounds_l0_cache_lat=%d --bounds_l1_cache_lat=%d --bounds_l0_cache_entry=%d --bounds_l0_insert_latency=%d' % (base_cmd, enable_bounds_checking, bounds_l0_cache_lat, bounds_l1_cache_lat, l0_entry_num, bounds_l1_cache_lat)

                                        new_cmd = '%s   --enable_bounds_ids_file=0  --bounds_only_global_load_store=%d  --enable_bounds_static_filter=%d --enable_bounds_checking=%d --bounds_l0_cache_lat=%d --bounds_l1_cache_lat=%d --bounds_l0_cache_entry=%d --bounds_l0_insert_latency=%d' % (base_cmd, bounds_only_global_load_store, enable_bounds_static_filter, enable_bounds_checking, bounds_l0_cache_lat, bounds_l1_cache_lat, l0_entry_num, bounds_l1_cache_lat)

                                        #new_dir = '%s/ptx14-all/%s' % (base_dir, new_desc)
                                        new_dir = '%s/%s/%s' %(base_dir,suite,new_desc)

                                        new_cmd = '%s\' -dir %s' % (new_cmd, new_dir)

                                    # get_data_cmd = '%s -d %s -widenames -disable-warning -amean -b base -prec 3 -suite %s %s >> summary_%s.txt\n'%  (get_data_tool, new_dir, suite, base_stat_list, desc)
                                        get_data_cmd = '%s -d %s -widenames -disable-warning -suite %s %s >> summary_%s.txt\n'%  (get_data_tool, new_dir, suite, stat_list, desc)

                                        #print(new_cmd)
                                        os.system(new_cmd)
                                        f.write(get_data_cmd)

                                    if (enable_bounds_checking == 0):
                                        base_test = 1


if __name__ == '__main__':
  main()
