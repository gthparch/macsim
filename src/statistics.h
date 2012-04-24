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
 * File         : statistics.h
 * Author       : Abderrahim Benquassmi
 * Date         : 3/11/2008 
 * CVS          : $Id: knob.h 868 2009-11-05 06:28:01Z kacear $:
 * Description  : stats framework
 *********************************************************************************************/


/*
  COUNT_TYPE_STAT, // stat is a simple counter  output is a number
  DIST_TYPE_STAT, // stat is the beginning of end of a distribution  output is a histogram
  PER_INST_TYPE_STAT, // stat is measured per instruction
  //   output is a ratio of count/inst_count
  PER_1000_INST_TYPE_STAT, // stat is measured per 1000 instructions
  //   output is a ratio of 1000*count/inst_count
  PER_1000_PRET_INST_TYPE_STAT, // stat is measured per 1000 pseudo-retired instructions
  //   output is a ratio of 1000*count/pret_inst_count
  PER_CYCLE_TYPE_STAT, // stat is measured per cycle
  //   output is a ratio of count/cycle_count
  RATIO_TYPE_STAT, // stat is measured per <some other stat>
  //   output is a ratio of count/other
  PERCENT_TYPE_STAT, // stat is measured per <some other stat>
  //   output is a percent of count/other
  LINE_TYPE_STAT,
*/


#ifndef STATS_H_INCLUDED
#define STATS_H_INCLUDED


#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "statsEnums.h"
#include "global_defs.h"

using namespace std;


///////////////////////////////////////////////////////////////////////////////////////////////


#define FILED1_LENGTH 45
#define FILED2_LENGTH 20
#define FILED3_LENGTH 30
#define FILED4_LENGTH 30


///////////////////////////////////////////////////////////////////////////////////////////////


unsigned int getInstructionCount();
unsigned int getPseudoRetiredInstructionCount();
unsigned int getCycleCount();
void init_per_core_stats(unsigned num_cores, macsim_c* simBase);

ofstream* getOutputStream(const string& filename);

class AbstractStat;

AbstractStat& getGLobalStat(long ID, ProcessorStatistics* m_ProcStat);
AbstractStat& getCoreWideStat(int coreID, long statID, ProcessorStatistics* m_ProcStat);


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Statistics base class
///////////////////////////////////////////////////////////////////////////////////////////////
class AbstractStat
{
  friend class StatisticsDumper;

  public:
    /**
     * Constructor.
     */
    AbstractStat(const string& str, const string& outputfilename, long ID, 
        bool corewide = false, bool isTemplate = true):
      m_name(str), m_count(0), m_total_count(0), m_pRatioStat(NULL), 
      m_fileName(outputfilename), m_ID(ID), m_bCoreWide(corewide), m_coreID(0),
      m_isTemplate(isTemplate), m_suffix("") {}

    /**
     * Destructor.
     */
    virtual ~AbstractStat() {}

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID) = 0;

    /**
     * Not a distribution stat.
     */
    virtual bool memberOfDistribution()
    {
      return false;
    }

    /**
     * Set the core id.
     */
    inline void setCoreID(unsigned int coreID)
    {
      char suffix[20];

      m_coreID = coreID;
      m_bCoreWide = true;

      sprintf(suffix, "_CORE_%d", m_coreID);
      m_suffix = suffix;
    }

    /**
     * Get output file name.
     */
    const string& getOutputFilename()
    {
      return m_fileName;
    }

    /**
     * Get the name of stat.
     */
    const string& getName()
    {
      return m_name;
    }

    /**
     * Increment the counter.
     */
    inline void inc()
    {
      m_count += 1;
    }

    /**
     * Increase the counter
     */
    inline void inc(unsigned int delta)
    {
      m_count += delta;
    }

    /**
     * Operator ++ : increment the counter.
     */
    inline void operator++(int)
    {
      m_count += 1;
    }

    /**
     * Operator -- : decrement the counter.
     */
    inline void operator--(int)
    {
      m_count -= 1;
    }

    /**
     * Operator += : increase the counter.
     */
    inline void operator+=(unsigned int delta)
    {
      m_count += delta;
    }

    /**
     * Get the value of the counter.
     */
    inline unsigned long long getCount()
    {
      return m_count;
    }

    /**
     * Dump out all stats to the file.
     */
    virtual void writeTo(ofstream& stream)
    {
      string name = m_name;
      name.append(m_suffix);
      stream.setf( ios::left, ios::adjustfield );
      stream << setw(FILED1_LENGTH) << name;

      stream.setf ( ios::right, ios::adjustfield );
      stream << setw(FILED2_LENGTH) << m_count << setw(FILED3_LENGTH) << m_count << 
        endl << endl;
    }

  protected:
    string m_name; /**< name of stat */
    unsigned long long m_count; /**< count during the current stat interval */
    unsigned long long m_total_count; /**< total count from beginning of run */
    AbstractStat* m_pRatioStat; /**< stat that to use in the ratio */
    string m_fileName; /**< name of file to print stats */
    long m_ID; /**< stat id */
    bool m_bCoreWide; /**< when set, add suffix to the name of a stat */
    unsigned int m_coreID; /**< core id */
    bool m_isTemplate; /**< is template */
    string m_suffix; /**< stat suffix */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief COUNT type stats
///////////////////////////////////////////////////////////////////////////////////////////////
class COUNT_Stat : public AbstractStat
{
  public:
    /**
     * Constructor.
     */
    COUNT_Stat(const string& str, const string& outputfilename, long ID):
      AbstractStat(str, outputfilename, ID) {}

    /**
     * Destructor.
     */
    virtual ~COUNT_Stat() {}

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      COUNT_Stat* pStat = new COUNT_Stat(m_name, m_fileName, this->m_ID);
      pStat->m_isTemplate = false;
      pStat->setCoreID(coreID);

      return pStat;
    }
};


class DIST_Stat;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Distribution member class
///////////////////////////////////////////////////////////////////////////////////////////////
class DISTMember_Stat : public AbstractStat
{
  friend class DIST_Stat;

  long m_parentDistroID; /**< parent distribution stat id */

 public:
    /**
     * Constructor.
     */
    DISTMember_Stat(const string& str,  const string& outputfilename, long ID, long distID,
        bool coreWide = false, bool isTemplate = false):
      AbstractStat(str, outputfilename, ID, coreWide, isTemplate), m_parentDistroID(distID) {}

    /**
     * Destructor.
     */
    virtual ~DISTMember_Stat()
    {
    }

    /**
     * Test this is the member of dist stat
     */
    virtual bool memberOfDistribution()
    {
      return true;
    }

    /**
     * set parent dist id
     */
    inline void setParentDistroID(long ID)
    {
      m_parentDistroID = ID;
    }

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      DISTMember_Stat* pStat = new DISTMember_Stat(m_name, m_fileName, m_ID, true, 
          m_isTemplate);
      pStat->m_isTemplate = false;
      pStat->setCoreID(coreID);
      pStat->m_parentDistroID = m_parentDistroID;

      return pStat;
    }
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief DIST type stat
///////////////////////////////////////////////////////////////////////////////////////////////
class DIST_Stat : public AbstractStat
{
  vector<long>  m_distributionMembers; /**< stat table */

  public:
    /**
     * Constructor.
     */
    DIST_Stat(const string& str, const string& outputfilename, long ID, ProcessorStatistics* procStat):
      AbstractStat(str, outputfilename, ID) 
    {
      m_ProcStat = procStat;
    }

    /**
     * Destructor.
     */
    virtual ~DIST_Stat()
    {
      if(!m_isTemplate)
      {
      }

      m_distributionMembers.clear();
    }

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      DIST_Stat* pClone = new DIST_Stat(this->m_name, this->m_fileName, this->m_ID, this->m_ProcStat);

      pClone->setCoreID(coreID);
      pClone->m_bCoreWide = true;
      pClone->m_isTemplate = false;

      vector<long>::iterator iter = m_distributionMembers.begin();
      vector<long>::iterator end = m_distributionMembers.end();

      while(iter != end)
      {
        long memberID = (*iter);

        pClone->addMember(memberID);

        iter++;
      }

      return pClone;
    }

    /**
     * Add a new stat.
     */
    void addMember(long memberID)
    {
      m_distributionMembers.push_back(memberID);
    }

    /**
     * Dump out all stats to the file.
     */
    virtual void writeTo(ofstream& stream);

    private:
      ProcessorStatistics* m_ProcStat; /**< reference to simulation-scoped processor stats */


};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief PER_INST type stat
///////////////////////////////////////////////////////////////////////////////////////////////
class PER_INST_Stat : public AbstractStat
{
  public:
    /**
     * Constructor
     */
    PER_INST_Stat(const string& str, const string& outputfilename, long ID):
      AbstractStat(str, outputfilename, ID) {}

    /**
     * Destructor.
     */
    virtual ~PER_INST_Stat() {}

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      PER_INST_Stat* pStat = new PER_INST_Stat(this->m_name, this->m_fileName, this->m_ID);
      pStat->setCoreID(coreID);
      pStat->m_isTemplate = false;
      return pStat;
    }

    /**
     * Dump out all stats to the file.
     */
    virtual void writeTo(ofstream& stream)
    {
      unsigned int numInstructions = getInstructionCount();
      per_inst_value = (float)m_count / numInstructions;
      string name = m_name;
      name.append(m_suffix);

      stream.setf ( ios::left, ios::adjustfield );
      stream << setw(FILED1_LENGTH) << name;
      stream.setf ( ios::right, ios::adjustfield );
      stream << setw(FILED2_LENGTH) << m_count << setw(FILED3_LENGTH) 
        << per_inst_value << endl << endl;
    }
  
  private:
    float per_inst_value; /**< stat value */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief PER_CYCLE stat
///////////////////////////////////////////////////////////////////////////////////////////////
class PER_CYCLE_Stat : public AbstractStat
{
  public:
    /**
     * Constructor.
     */
    PER_CYCLE_Stat(const string& str, const string& outputfilename, long ID):
      AbstractStat(str, outputfilename, ID) {}

    /**
     * Destructor.
     */
    virtual ~PER_CYCLE_Stat()
    {
    }

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      PER_CYCLE_Stat* pStat = new PER_CYCLE_Stat(this->m_name, this->m_fileName, this->m_ID);
      pStat->setCoreID(coreID);
      pStat->m_isTemplate = false;
      return pStat;
    }

    /**
     * Dump out all stats to the file.
     */
    virtual void writeTo(ofstream& stream)
    {
      unsigned int cycleCount = getCycleCount();
      if (cycleCount > 0) {
        string name = m_name;
        name.append(m_suffix);

        per_cycle = (float)m_count/cycleCount;

        stream.setf ( ios::left, ios::adjustfield );
        stream << setw(FILED1_LENGTH) << name;
        stream.setf ( ios::right, ios::adjustfield );
        stream << setw(FILED2_LENGTH) << m_count << setw(FILED3_LENGTH) 
          << per_cycle << endl << endl;
      }
    }

  private:
    float per_cycle; /**< stat value */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief RATIO type stat
///////////////////////////////////////////////////////////////////////////////////////////////
class RATIO_Stat : public AbstractStat
{

  public:
    /**
     * Constructor.
     */
    RATIO_Stat(const string& str, const string& outputfilename, long ID,
               long RatioID, ProcessorStatistics* procStat):
      AbstractStat(str, outputfilename, ID), m_RatioID(RatioID)
    {
      m_ProcStat = procStat;
    }

    /**
     * Destructor.
     */
    virtual ~RATIO_Stat()
    {
      if(!m_isTemplate)
      {
      }
    }

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      RATIO_Stat* pStat = new RATIO_Stat(this->m_name, this->m_fileName, this->m_ID,
                                         m_RatioID, this->m_ProcStat);
      pStat->setCoreID(coreID);
      pStat->m_isTemplate = false;
      return pStat;
    }

    /**
     * Set ratio reference id.
     */
    void setRatioReference(long ID)
    {
      m_RatioID = ID;
    }

    /**
     * Dump out all stats to the file.
     */
    virtual void writeTo(ofstream& stream)
    {
      unsigned int refValue = 0;
      string refName = "";

      string name = m_name;
      if (m_bCoreWide) {
        name.append(m_suffix);
        AbstractStat& refStat = getCoreWideStat(m_coreID, m_RatioID, m_ProcStat);
        refValue = refStat.getCount();
        refName = refStat.getName();
      }
      else {
        AbstractStat& refStat = getGLobalStat(m_RatioID, m_ProcStat);
        refValue = refStat.getCount();
        refName = refStat.getName();
      }

      stream.setf ( ios::left, ios::adjustfield );
      stream << setw(FILED1_LENGTH) << name;

      stream.setf ( ios::right, ios::adjustfield );
      stream << setw(FILED2_LENGTH) << m_count;

      stream.setf ( ios::right, ios::adjustfield );

      stream << setw(FILED3_LENGTH);

      if (refValue) {
        ratio = (float)m_count / (float)refValue;
        stream << ratio << endl;
      }
      else {
        stream << "NaN" << endl;
      }

      stream << endl;
    }

  private:
    long m_RatioID; /**< ratio id */
    float ratio; /**< ratio */
    ProcessorStatistics* m_ProcStat; /**< reference to simulation-scoped processor stats */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief PERCENT type stat
///////////////////////////////////////////////////////////////////////////////////////////////
class PERCENT_Stat : public AbstractStat
{
  public:
    /**
     * Constructor.
     */
    PERCENT_Stat(const string& str, const string& outputfilename, long ID, 
                 long denominatorID, ProcessorStatistics* procStat):
      AbstractStat(str, outputfilename, ID), m_denominatorID(denominatorID) 
    {
      m_ProcStat = procStat;
    }

    /**
     * Destructor
     */
    virtual ~PERCENT_Stat() {}

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      PERCENT_Stat* pStat = new PERCENT_Stat(m_name, m_fileName, m_ID, m_denominatorID, m_ProcStat);
      pStat->setCoreID(coreID);
      pStat->m_isTemplate = false;
      return pStat;
    }

    /**
     * Dump out all stats to the file.
     */
    virtual void writeTo(ofstream& stream)
    {
      unsigned int refValue = 0;
      string refName = "";

      string name = m_name;
      if (m_bCoreWide) {
        name.append(m_suffix);
        AbstractStat& refStat = getCoreWideStat(m_coreID, m_denominatorID, m_ProcStat);
        refValue = refStat.getCount();
        refName = refStat.getName();
      }
      else {
        AbstractStat& refStat = getGLobalStat(m_denominatorID, m_ProcStat);
        refValue = refStat.getCount();
        refName = refStat.getName();
      }

      stream.setf ( ios::left, ios::adjustfield );
      stream << setw(FILED1_LENGTH) << name;

      stream.setf ( ios::right, ios::adjustfield );
      stream << setw(FILED2_LENGTH) << m_count;

      stream.setf ( ios::right, ios::adjustfield );
      stream << setw(FILED3_LENGTH) ;

      if (refValue) {
        ratio = (float)m_count / (float)refValue;
        percent = 100 * ratio;

        stream << percent << endl;
      }
      else {
        stream << "NaN" << endl;
      }

      stream << endl;
    }


  private:
    long m_denominatorID; /**< stat id */
    float ratio; /**< ratio */
    float percent; /**< percent */
    ProcessorStatistics* m_ProcStat; /**< reference to simulation-scoped processor stats */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief PER_1000_INST type stat
///////////////////////////////////////////////////////////////////////////////////////////////
class PER_1000_INST_Stat : public AbstractStat
{
  public:
    /**
     * Constructor.
     */
    PER_1000_INST_Stat(const string& str, const string& outputfilename, long ID):
      AbstractStat(str, outputfilename, ID) {}

    /**
     * Destructor.
     */
    virtual ~PER_1000_INST_Stat()
    {
    }

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      PER_1000_INST_Stat* pStat = new PER_1000_INST_Stat(m_name, m_fileName, m_ID);
      pStat->setCoreID(coreID);
      pStat->m_isTemplate = false;
      return pStat;
    }

    /**
     * Dump all stats to the file.
     */
    virtual void writeTo(ofstream& stream)
    {
      unsigned int numInstructions = getInstructionCount();
      float ratio = (float)m_count / (float)numInstructions;
      per_1000_inst_value = 1000 * ratio;

      string name = m_name;
      name.append(m_suffix);
      stream.setf(ios::left, ios::adjustfield);
      stream << setw(FILED1_LENGTH) << name;

      stream.setf (ios::right, ios::adjustfield);
      stream << setw(FILED2_LENGTH) << m_count;
      stream << setw(FILED3_LENGTH) << per_1000_inst_value << endl << endl;
    }

  private:
    float per_1000_inst_value; /**< per 1000 instruction value */
};



///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief PER_1000_PRET_INST type stat
///////////////////////////////////////////////////////////////////////////////////////////////
class PER_1000_PRET_INST_Stat : public AbstractStat
{
  public:
    /**
     * Constructor.
     */
    PER_1000_PRET_INST_Stat(const string& str, const string& outputfilename, long ID):
      AbstractStat(str, outputfilename, ID) {}

    /**
     * Destructor.
     */
    virtual ~PER_1000_PRET_INST_Stat() {}

    /**
     * Clone a stat.
     */
    virtual AbstractStat* clone(unsigned int coreID)
    {
      PER_1000_PRET_INST_Stat* pStat = new PER_1000_PRET_INST_Stat(m_name, m_fileName, m_ID);
      pStat->setCoreID(coreID);
      pStat->m_isTemplate = false;
      return pStat;
    }

    /**
     * Dump stats to the file.
     */
    virtual void writeTo(ofstream& stream)
    {
      unsigned int PRET = getPseudoRetiredInstructionCount();
      float ratio = (float)m_count / (float)PRET;
      per_1000_pret_inst_value = 1000 * ratio;

      string name = m_name;
      name.append(m_suffix);
      stream.setf ( ios::left, ios::adjustfield );
      stream << setw(FILED1_LENGTH) << name;

      stream.setf ( ios::right, ios::adjustfield );
      stream << setw(FILED2_LENGTH) << m_count;
      stream << setw(FILED3_LENGTH) << per_1000_pret_inst_value << endl << endl;
    }

  private:
    float per_1000_pret_inst_value; /**< stat value */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief LINE stat
///////////////////////////////////////////////////////////////////////////////////////////////
class LINE_Stat : public AbstractStat
{
  public:
    /**
     * Constructor.
     */
    LINE_Stat(const string& str, const string& outputfilename, long ID):
      AbstractStat(str, outputfilename, ID) {}

    /**
     * Destructor.
     */
    virtual ~LINE_Stat() {}
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Global statistics
///////////////////////////////////////////////////////////////////////////////////////////////
class GlobalStatistics
{
  public:
    /**
     * Constructor.
     */
    GlobalStatistics(macsim_c* simBase)
    {
      m_simBase = simBase;
    }

    /**
     * Destructor.
     */
    ~GlobalStatistics()
    {
      m_globalStats.clear();
    }

    /**
     * Add a new stat.
     */
    void addStatistic(AbstractStat* pStat)
    {
      m_globalStats.push_back(pStat);
    }

    /**
     * Add a distribution stat.
     */
    void addDistribution(DIST_Stat* pStat)
    {
      m_distributions.push_back(pStat);
    }

    /**
     * operator []. Return corresponding stat.
     */
    AbstractStat& operator[](int index) const
    {
      AbstractStat* pStat =  m_globalStats[index];
      return (*pStat);
    }

    /**
     * Return number of total stats.
     */
    int size()const
    {
      return m_globalStats.size();
    }

    /**
     * Dump out all stats in the standrad output.
     */
    void displayAll()
    {
      vector<AbstractStat*>::iterator iter = m_globalStats.begin();
      vector<AbstractStat*>::iterator end = m_globalStats.end();

      while (iter != end) {
        AbstractStat* pStat = (*iter);
        cout << pStat->getName() << endl;

        iter++;
      }
    }

    /**
     * Dump out all stats to the file.
     */
    void saveStats(string);

    /**
     * Dump out all stats to the file.
     */
    void saveStats();

    /**
     * Dump out all stats to the file.
     */
    void writeTo(ofstream& stream);

  private:
    vector<AbstractStat*> m_globalStats; /**< global stats */
    vector<DIST_Stat*> m_distributions; /**< distribution stats */
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
};



///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Per core stats
///////////////////////////////////////////////////////////////////////////////////////////////
class CoreStatistics
{
  public:
    /**
     * Constructor.
     */
    CoreStatistics(macsim_c* simBase, unsigned int coreID = 0, bool isTemplate = false):
      m_coreID(coreID),
      m_isTemplate(isTemplate)
    {
      m_simBase = simBase;
    }

    /**
     * Desturctor
     */
    ~CoreStatistics();

    /**
     * Set core id.
     */
    inline void setID(unsigned int coreID)
    {
      m_coreID = coreID;
    }

    /**
     * Add a new stat.
     */
    void addStatistic(AbstractStat* pStat)
    {
      m_CoreStats.push_back(pStat);
    }

    /**
     * Add a new distribution stat.
     */
    void addDistribution(DIST_Stat* pStat)
    {
      m_distributions.push_back(pStat);
    }

    /**
     * Operator []. Return corresponding core stat.
     */
    AbstractStat& operator[](int index) const
    {
      AbstractStat* pStat =  m_CoreStats[index];
      return (*pStat);
    }

    /**
     * Make a clone stats for the core.
     */
    CoreStatistics* clone(unsigned int cloneCoreID, macsim_c* simBase)
    {
      CoreStatistics* pClone = new CoreStatistics(simBase, cloneCoreID);

      vector<AbstractStat*>::iterator iter = m_CoreStats.begin();
      vector<AbstractStat*>::iterator end = m_CoreStats.end();
      
      while (iter != end) {
        AbstractStat* pStat = (*iter);
        AbstractStat* pStatClone = pStat->clone(cloneCoreID);

        pClone->addStatistic(pStatClone);

        iter++;
      }

      vector<DIST_Stat*>::iterator dist_iter = m_distributions.begin();
      vector<DIST_Stat*>::iterator dist_end = m_distributions.end();

      while (dist_iter != dist_end) {
        AbstractStat* pStat = (*dist_iter);
        DIST_Stat* pStatClone = static_cast<DIST_Stat*>(pStat->clone(cloneCoreID));

        pClone->addDistribution(pStatClone);

        dist_iter++;
      }

      return pClone;
    }

    /**
     * Display all stats in the standard output.
     */
    void displayAll()
    {
      vector<AbstractStat*>::iterator iter = m_CoreStats.begin();
      vector<AbstractStat*>::iterator end = m_CoreStats.end();

      while (iter != end) {
        AbstractStat* pStat = (*iter);
        cout << pStat->getName() << endl;

        iter++;
      }
    }

    /**
     * Dump accumulated stats to the file.
     */
    void saveStats(string);

    /**
     * Dump accumulated stats to the file.
     */
    void saveStats();

  private:
    unsigned int m_coreID; /**< core id */
    vector<AbstractStat*> m_CoreStats; /**< core stats */
    vector<DIST_Stat*> m_distributions; /**< distribution stats */
    bool m_isTemplate; /**< is template? */
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Processor stats
/// 
/// This class has two types of stats; global and core stats
///////////////////////////////////////////////////////////////////////////////////////////////
class ProcessorStatistics
{
  public:
    /**
     * Default constructor.
     */
    ProcessorStatistics(macsim_c* simBase);

    /**
     * Default destructor.
     */
    ~ProcessorStatistics();

    /**
     * Overridden operator [].
     * Return corresponding stats (enum value for the index).
     */
    inline AbstractStat& operator[](int index) const
    {
      AbstractStat& stat = (*m_globalStatistics)[index];
      return stat;
    }

    /**
     * Return global stats.
     */
    GlobalStatistics* globalStats();


    /**
     * Get a core stat.
     * @param coreID core id
     */
    inline CoreStatistics& core(unsigned int coreID) const
    {
      // if invalid coreID, default to the last one
      if(coreID >= m_allCoresStats.size())
        coreID = m_allCoresStats.size() - 1;

      CoreStatistics* pCoreModel = m_allCoresStats[coreID];
      return (*pCoreModel);
    }

    /**
     * Set number of cores.
     */
    void setNumCores(unsigned int numCores);

    /**
     * Print all stats after the simulation.
     * @param ext extension to the stat.out file
     */
    void saveStats(string ext);

    /**
     * Print all stats after the simulation.
     */
    void saveStats();

  private:
    GlobalStatistics*  m_globalStatistics; /**< global stats */
    vector<CoreStatistics*> m_allCoresStats; /**< core stats table */
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
};


///////////////////////////////////////////////////////////////////////////////////////////////

// Macros to accumulate stats

#define WATTCH_CORE_EVENT(coreID, UnitName, ComponentName)              \
  TheProcessorPowerModel.core(coreID)[E_##UnitName][E_##ComponentName]++;

#define WATTCH_CORE_EVENT_N(coreID, UnitName, ComponentName, delta)     \
  TheProcessorPowerModel.core(coreID)[E_##UnitName][E_##ComponentName]+=delta;

#define WATTCH_GLOBAL_EVENT(UnitName, ComponentName)            \
  TheProcessorPowerModel[E_##UnitName][E_##ComponentName]++;

#define WATTCH_GLOBAL_EVENT_N(UnitName, ComponentName, delta)           \
  TheProcessorPowerModel[E_##UnitName][E_##ComponentName]+=delta;

#define STAT_CORE_EVENT(coreID, Event)                                  \
  m_simBase->m_ProcessorStats->core(coreID)[Event - PER_CORE_STATS_ENUM_FIRST]++;


#define STAT_CORE_EVENT_M(coreID, Event)                                \
  m_simBase->m_ProcessorStats->core(coreID)[Event - PER_CORE_STATS_ENUM_FIRST]--;


#define STAT_CORE_EVENT_N(coreID, Event, delta)                         \
  m_simBase->m_ProcessorStats->core(coreID)[Event  - PER_CORE_STATS_ENUM_FIRST]+=delta;


// increment a stat
#define STAT_EVENT(ID)                                                   \
  (*m_simBase->m_ProcessorStats)[ID]++


// decrement a stat
#define STAT_EVENT_M(ID)                                                 \
  (*m_simBase->m_ProcessorStats)[ID]--


// increat a stat with delta value
#define STAT_EVENT_N(ID, delta)                                          \
  (*m_simBase->m_ProcessorStats)[ID] += delta



// POWER EVENT
#define POWER_CORE_EVENT(coreID, Event) STAT_CORE_EVENT(coreID, Event)
#define POWER_CORE_EVENT_M(coreID, Event) STAT_CORE_EVENT_M(coreID, Event)
#define POWER_CORE_EVENT_N(coreID, Event, delta) STAT_CORE_EVENT_N(coreID, Event, delta)
#define POWER_EVENT(ID) STAT_EVENT(ID)
#define POWER_EVENT_M(ID) STAT_EVENT_M(ID)
#define POWER_EVENT_N(ID, delta) STAT_EVENT_N(ID, delta)




#endif //STATS_H_INCLUDED
