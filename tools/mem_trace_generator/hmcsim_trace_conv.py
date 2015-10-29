#!/usr/bin/python
import sys


cmdargs = str(sys.argv) 

fname = "dram_"+sys.argv[1]
f = open(fname,'w')
print ("output file name is %s " %fname)
for line in open (sys.argv[1]):
	columns  = line.split(':')
	if "RD64" in columns[2]: 
                f.write ("%s R\n" %(columns[7]))
#                print columns[7],
#                print  "R"
	elif "WR64" in columns[2]:
                f.write("%s W\n" %(columns[7]))
			
f.close()
	    # print columns[7],
	    # print  "W"  
	
		
