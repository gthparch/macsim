/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of 
conditions and the following disclaimer in the documentation and/or other materials provided 
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/


/**********************************************************************************************
 * File         : statistics.cc
 * Author       : Abderrahim Benquassmi
 * Date         : 3/11/2008 
 * CVS          : $Id: knob.h 868 2009-11-05 06:28:01Z kacear $:
 * Description  : stats framework
 *********************************************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "all_knobs.h"
#include "macsim.h"
#include "statistics.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////


// get the output stream
ofstream* getOutputStream(const string& filename, macsim_c* m_simBase)
{
  string fullpath = *m_simBase->m_knobs->KNOB_STATISTICS_OUT_DIRECTORY;
  fullpath = fullpath + "/" + filename;

  // stat name already encountered
  // check if the output stream is opened then do nothing
  // otherwise open the stream
  map<string, ofstream*>::iterator iter = m_simBase->m_AllStatsOutputStreams.find(filename);

  ofstream* stream = NULL;

  // first time this filename is encounterd
  // create the ofstream object
  if (iter == m_simBase->m_AllStatsOutputStreams.end()) {
    stream = new ofstream();
    stream->exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );

    try {
      stream->open(fullpath.c_str(), ios_base::out);
    
      if (stream->is_open())
        m_simBase->m_AllStatsOutputStreams[filename] = stream;
      else
      {
        delete stream;
        stream = NULL;
      }
    }
    catch(ofstream::failure exp) {
      string strException = exp.what();
      cout << strException;
      delete stream;
      stream = NULL;
    }
  }
  else {
    stream = m_simBase->m_AllStatsOutputStreams[filename];
  }

  return stream;
}


// get global stats
AbstractStat& getGlobalStat(long statID, ProcessorStatistics* m_ProcStat)
{
  AbstractStat& pStat = (*m_ProcStat)[statID];

  return pStat;
}


// get core stats
AbstractStat& getCoreWideStat(int coreID, long statID, ProcessorStatistics* m_ProcStat)
{
  CoreStatistics& coreStats = m_ProcStat->core(coreID);
  AbstractStat& pStat = coreStats[statID - PER_CORE_STATS_ENUM_FIRST];

  return pStat;
}


unsigned int getInstructionCount()
{
  return 1200;
}


unsigned int getPseudoRetiredInstructionCount()
{
  return 1200;
}


unsigned int getCycleCount()
{
  return 20000;
}


// initialize core stats
void init_per_core_stats(unsigned num_cores, macsim_c* simBase)
{
  simBase->m_ProcessorStats->setNumCores(num_cores);
}


///////////////////////////////////////////////////////////////////////////////////////////////


// dump out all stats to the file
void DIST_Stat::writeTo(ofstream& stream)
{
  vector<long>::iterator iter = m_distributionMembers.begin();
  vector<long>::iterator end = m_distributionMembers.end();
	
  long memberID = 0;

  m_total_count = 0;

  //compute the total first
  while (iter != end) {
    memberID = (*iter);
    if (m_bCoreWide) {
      AbstractStat& memberStat = getCoreWideStat(m_coreID, memberID, m_ProcStat);
      m_total_count += memberStat.getCount();
    }
    else {
      AbstractStat& memberStat = getGlobalStat(memberID, m_ProcStat);
      m_total_count += memberStat.getCount();
    }
    iter++;
  }

    
  iter = m_distributionMembers.begin();
  end = m_distributionMembers.end();

  int count = 0;
  float percent = 0;

  while (iter != end) {
    memberID = (*iter);
    if (m_bCoreWide) {
      AbstractStat& memberStat = getCoreWideStat(m_coreID, memberID, m_ProcStat);
      count = memberStat.getCount();
      const string& name = memberStat.getName();

      string display_name(name);
      display_name.append(m_suffix);

      stream.setf( ios::left, ios::adjustfield );
      stream << setw(FILED1_LENGTH) << display_name;
    }
    else {
      AbstractStat& memberStat = getGlobalStat(memberID, m_ProcStat);
      count = memberStat.getCount();
      const string& name = memberStat.getName();

      stream.setf( ios::left, ios::adjustfield );
      stream << setw(FILED1_LENGTH) << name;
    }

    stream.setf ( ios::right, ios::adjustfield );
    stream <<  setw(FILED2_LENGTH);
		
    if (0 == m_total_count) {
      stream << 0 << setw(FILED3_LENGTH) << " NaN " << endl;
    }
    else {
      percent = 100*((float)count)/m_total_count;

      stream.setf ( ios::right, ios::adjustfield );
      stream << count << setw(FILED3_LENGTH) << percent << '%' << endl;
    }

    iter++;
  }

  stream.setf( ios::left, ios::adjustfield );
  stream << setw(FILED1_LENGTH) << " ";
	
  stream.setf ( ios::right, ios::adjustfield );
  stream << setw(FILED2_LENGTH) << m_total_count;
  stream << setw(FILED3_LENGTH) << "100" << "%" << endl << endl;
}


///////////////////////////////////////////////////////////////////////////////////////////////


// dump out all stats to the file
void GlobalStatistics::saveStats()
{
  saveStats("");
}


// dump out all stats to the file
void GlobalStatistics::saveStats(string ext)
{
  vector<AbstractStat*>::iterator iter = m_globalStats.begin();
  vector<AbstractStat*>::iterator end = m_globalStats.end();

  bool newFile = true;
  string oldFileName = "";

  while (iter != end) {
    AbstractStat* pStat = (*iter);

    // ignore distribution members, they will be printed by
    // the distributions they belong to
    if (false == pStat->memberOfDistribution()) {
      const string& filename = pStat->getOutputFilename() + ext;

      newFile = (oldFileName != filename);
      if (newFile) {
        newFile = true;
        oldFileName = filename;
      }

      ofstream* pStream = getOutputStream(filename, m_simBase);
      if (NULL != pStream) {
        ofstream& stream = (*pStream);
				
        if (newFile) {
          stream << setfill ('#');
          stream << setw(100);
          stream << '#'<< endl;
          stream << setfill (' ');
          stream.setf( ios::left, ios::adjustfield );
          stream << "Interval" << endl;
          stream << setfill ('#');
          stream << setw(100);
          stream << '#' << endl;

          stream << setfill (' ');
          stream.setf( ios::left, ios::adjustfield );
          stream << setw(20) << "Cumulative:" << setw(10) << "Cycles:  ";

          stream.setf( ios::right, ios::adjustfield );
          stream << setw(20) << getCycleCount();
					
          stream.setf( ios::left, ios::adjustfield );
          stream << setw(20) << "     Instructions:";

          stream.setf( ios::right, ios::adjustfield );
          stream << setw(20) << getInstructionCount();

          stream << endl;
        }

        stream << setfill (' ');
        pStat->writeTo(stream);
      }
    }

    iter++;
  }
    
  // save the distributions
  vector<DIST_Stat*>::iterator iter2 =  m_distributions.begin();
  vector<DIST_Stat*>::iterator end2 =  m_distributions.end();

  while (iter2 != end2) {
    DIST_Stat* pDistribution = (*iter2);

    const string& filename = pDistribution->getOutputFilename() + ext;
    
    ofstream* pStream = getOutputStream(filename, m_simBase);
    if (NULL != pStream) {
      ofstream& stream = (*pStream);
      pDistribution->writeTo(stream);
    }

    iter2++;
  }
}


// dump out all stats to the file
void GlobalStatistics::writeTo(ofstream& stream)
{
  vector<AbstractStat*>::iterator iter = m_globalStats.begin();
  vector<AbstractStat*>::iterator end = m_globalStats.end();

  while (iter != end) {
    AbstractStat* pStat = (*iter);
    // ignore distribution members, they will be printed by the distributions they belong to
    if (false == pStat->memberOfDistribution()) {	
      stream << "GS ## ";
      pStat->writeTo(stream);
    }

    iter++;
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////

// destructor
CoreStatistics::~CoreStatistics()
{
  m_CoreStats.clear();
	
}


// dumpt out all stats to the file
void CoreStatistics::saveStats()
{
  saveStats("");
}


// dumpt out all stats to the file
void CoreStatistics::saveStats(string ext)
{
  vector<AbstractStat*>::iterator iter = m_CoreStats.begin();
  vector<AbstractStat*>::iterator end = m_CoreStats.end();

  while (iter != end) {
    AbstractStat* pStat = (*iter);
    const string& filename = pStat->getOutputFilename() + ext;

    ofstream& stream = (*getOutputStream(filename, m_simBase));

    // ignore distribution members, they will be printed by
    // the distributions they belong to
    if (false == pStat->memberOfDistribution()) {
      pStat->writeTo(stream);
    }

    iter++;
  }

  // save the distributions
  vector<DIST_Stat*>::iterator iter2 =  m_distributions.begin();
  vector<DIST_Stat*>::iterator end2 =  m_distributions.end();

  while (iter2 != end2) {
    DIST_Stat* pDistribution = (*iter2);

    const string& filename = pDistribution->getOutputFilename() + ext;
    
    ofstream* pStream = getOutputStream(filename, m_simBase);
    if (NULL != pStream) {
      ofstream& stream = (*pStream);
      pDistribution->writeTo(stream);
    }

    iter2++;
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////


// constructor
ProcessorStatistics::ProcessorStatistics(macsim_c* simBase)
{
  m_simBase = simBase;
  m_globalStatistics = new GlobalStatistics(simBase);
}


// destructor
ProcessorStatistics::~ProcessorStatistics()
{
  vector<CoreStatistics*>::iterator iter = m_allCoresStats.begin();
  vector<CoreStatistics*>::iterator end = m_allCoresStats.end();

  while (iter != end) {
    CoreStatistics* pCoreModel = (*iter);
    delete pCoreModel;
		
    iter++;
  }

  m_allCoresStats.clear();
}


// return global stats
GlobalStatistics* ProcessorStatistics::globalStats()
{
  return m_globalStatistics;
}


// set number of cores
void ProcessorStatistics::setNumCores(unsigned int numCores)
{
  if (numCores < 1)
    numCores = 1;

  vector<CoreStatistics*>::iterator iter = m_allCoresStats.begin();
  vector<CoreStatistics*>::iterator end = m_allCoresStats.end();

  while (iter != end) {
    CoreStatistics* pCoreModel = (*iter);
    delete pCoreModel;
		
    iter++;
  }

  m_allCoresStats.clear();

  // generate enough core power model objects
  for (unsigned int coreId = 0; coreId < numCores; ++coreId) {
    CoreStatistics* pCoreModel = m_simBase->m_coreStatsTemplate->clone(coreId, m_simBase);
    m_allCoresStats.push_back(pCoreModel);
  }
}


// dump out all stats
void ProcessorStatistics::saveStats()
{
  saveStats("");
}


// dump out all stats with file extension ext
void ProcessorStatistics::saveStats(string ext)
{
  // char* Path = "statistics";
  string stat_path = *m_simBase->m_knobs->KNOB_STATISTICS_OUT_DIRECTORY; 
  const char *Path = stat_path.c_str(); 

  int check = mkdir(Path, S_IRWXU);
  check = check; // avoid "unused variable" warning
  //if (chdir(Path) != 0)
  //  exit(0);

  // svae global statistics
  m_globalStatistics->saveStats(ext);

  // save core-wide statistics
  // iterate through each core, ask the core to save its stats
  vector<CoreStatistics*>::iterator iterCoreStats = m_allCoresStats.begin();
  vector<CoreStatistics*>::iterator endCoreStats = m_allCoresStats.end();

  while (iterCoreStats != endCoreStats) {
    CoreStatistics* pCoreStats = (*iterCoreStats);
    pCoreStats->saveStats(ext);

    iterCoreStats++;
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////



