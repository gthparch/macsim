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
 * File         : knob.h
 * Author       : Abderrahim Benquassmi
 * Date         : 3/11/2008 
 * CVS          : $Id: knob.h 868 2009-11-05 06:28:01Z kacear $:
 * Description  : knob template declaration 
 *********************************************************************************************/

#ifndef KNOB_H_INCLUDED
#define KNOB_H_INCLUDED


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "global_types.h"
#include "global_defs.h"

using namespace std;


///////////////////////////////////////////////////////////////////////////////////////////////


#define QUOTE(x) #x

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief name and value of the knob from the file
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct param_file_entry_s
{
  string m_name; /**< knob name */
  string m_value; /**< knob value */
} param_file_entry_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Abstract knob class : base class of all other knobs
///////////////////////////////////////////////////////////////////////////////////////////////
class abstract_knob_c
{
  public:
    /**
     * Constructor.
     */
    abstract_knob_c() {  }

    /**
     * Constructor.
     * @param name knob name
     * @param value initial value
     * @param parentName parent knob (for ratio)
     */
    abstract_knob_c(string name, string value, string parentName): m_name(name),
    m_valueString(value), m_parentName(parentName), m_valueProvided(false) {  }

    /**
     * Get knob name.
     */
    inline string getName() const { return m_name; }

    /**
     * Get knob value in string.
     */
    inline string getValueString() const { return m_valueString; }

    /**
     * Set the name of the knob.
     */
    inline void setName(const string& name) { m_name = name; m_valueProvided = true; }

    /**
     * Set the value of the knob in string.
     */
    inline void setValueString(const string& value)
    {
      m_valueString = value; 
      m_valueProvided = true;
    }

    /**
     * Get the parent knob name.
     */
    inline string getParentName() const {return m_parentName;}

    /**
     * Init knob from the string.
     */
    virtual void initFromString(const string& strVal) = 0;

    /**
     * Print knob name and its value.
     */
    virtual void display(ostream& os)
    {
      os << m_name << "   =   " << m_valueString;
    }

    /**
     * Was value provided?
     */
    bool wasValueProvided() { return m_valueProvided;}

  protected:
    string m_name; /**< knob name */
    string m_valueString; /**< knob value in string */
    string m_parentName; /**< parent knob name */
    bool   m_valueProvided; /**< value provided */
};



///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Knob template class with type T
///////////////////////////////////////////////////////////////////////////////////////////////
template<class T> class KnobTemplate : public abstract_knob_c
{
  public:
    /**
     * Constructor.
     */
    KnobTemplate():abstract_knob_c() { }

    /**
     * Constructor.
     * @param name name of the knob
     * @param val value of the knob
     * @param parentName parent knob name
     */
    KnobTemplate(const string& name, const T& val, const string& parentName = ""):
      abstract_knob_c(name, "", parentName), m_value(val) { }

    /**
     * Set the value of the knob.
     */
    inline KnobTemplate<T>& setValue(const T& val)
    {
      m_value = val;
      m_valueProvided = true;
      return (*this);
    }

    /**
     * Initialize a knob from the string.
     */
    virtual void initFromString(const string& strVal)
    {
      stringstream sstr;
      sstr << strVal;
      T temp_value;
      if (sizeof(T) == 1) {
        int temp_int;
        sstr >> temp_int;
        m_value = temp_int;
        temp_value = temp_int;
      }
      else {
        sstr >> m_value;
      }
      m_valueString = strVal;
      m_valueProvided = true;
    }

    /**
     * Get the value of the knob.
     */
    inline const T getValue() const { return m_value; }

    /**
     * Conversion operator ().
     * Return the value of the knob.
     */
    operator T() const { return m_value; }

    /**
     * Print knob name and its value.
     */
    virtual void display(ostream& os)
    {
      os << m_name << "   =   " << m_value;
    }

    /**
     * Get the value of the knob in string.
     */
    string GetValueString(void) {return m_valueString;}

  private:
    /** 
     * Copy constructor.
     */
    KnobTemplate(const KnobTemplate& rhs);

    /**
     * Overidden operator =.
     */
    const KnobTemplate& operator=(const KnobTemplate& rhs);

  private:
    T m_value; /**< knob value */

};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief string type knob class
///////////////////////////////////////////////////////////////////////////////////////////////
template<> class KnobTemplate<string> : public abstract_knob_c
{
  public:
    /**
     * Constructor.
     * @param name knob name
     * @param val knob value
     * @param parentName the name of parent knob
     */
    KnobTemplate(const string& name, const string& val, const string& parentName = ""):
      abstract_knob_c(name, "", parentName), m_value(val) { }

    /**
     * Set value of the knob.
     */
    inline KnobTemplate<string>& setValue(const string& val)
    {
      m_value = val;
      return (*this);
    }

    /**
     * Initialize a knob from the string.
     */
    virtual void initFromString(const string& strVal)
    {
      m_value = strVal;
      m_valueString = strVal;
    }

    /**
     * Get the value of the knob.
     */
    inline const string getValue()const {return m_value;}

    /**
     * Conversion operator to string.
     */
    operator string() const { return m_value; }

    /**
     * Conversion oeprator to bool.
     */
    operator bool() const
    {
      return (m_value.length() != 0);
    }

    /**
     * Print the name of a knob and its value.
     */
    virtual void display(ostream& os)
    {
      os << m_name << "   =   " << m_value;
    }

  private:
    /**
     * Copy constructor.
     */
    KnobTemplate(const KnobTemplate& rhs);

    /**
     * Overridden operator =
     */
    const KnobTemplate& operator=(const KnobTemplate& rhs);

  private:
    string m_value; /**< knob value */

};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Knob entry tokenizer
///////////////////////////////////////////////////////////////////////////////////////////////
class KnobEntryTokenizer 
{
  public:
    /**
     * Constructor.
     */
    KnobEntryTokenizer() { }

    /**
     * Destructor.
     */
    ~KnobEntryTokenizer() { }

    /**
     * Clear all tokens.
     */
    void clear() {m_TokensVector.clear();}

    /**
     * Tokenize the string with delimiter.
     */
    void tokenizeString(string str, char delim);

    /**
     * Return number of tokens.
     */
    int numTokens();

    /**
     * Get tokens from the m_TokensVector.
     */
    void getTokens(vector<string>& array);

  private:
    vector<string> m_TokensVector; /**< token vector */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief provide comparator to KnobsContainer clas
/// @see KnobsContainer
///////////////////////////////////////////////////////////////////////////////////////////////
struct ltstr_s
{
  /**
   * operator() - sort comparator
   */
  bool operator()(const string& s1, const string& s2) const
  {
    return (s1 < s2);
  }
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Knob container class
///
/// Singleton object
///////////////////////////////////////////////////////////////////////////////////////////////
class KnobsContainer
{
  public:
    /**
     * Constructor.
     */
    KnobsContainer();
    /**
     * Destructor.
     */
    ~KnobsContainer();

    /**
     * Clear all knobs.
     */
    void clear();

    /**
     * Apply new values from the parameter file.
     */
    void applyParamFile(const string& filename);

    /**
     * Apply new values from the command line.
     */
    bool applyComandLineArguments(int argc, char** argv, char** invalidParam);

    /**
     * Change the value of a single knob.
     */
    void updateKnob(string key, string value);

    /**
     * Adjust the value of a knob.
     */
    void adjustKnobValues();

    /**
     * Insert a new knob.
     */
    void insertKnob(abstract_knob_c* pKnob);

    /**
     * Get the pointer to all the knobs
     */
    all_knobs_c* getAllKnobs();

    /**
     * Save all knob contents to the file.
     */
    void saveToFile(const string& filename);

    /**
     * Overridden operator << : Provide output stream.
     */
    friend ostream& operator<<(ostream& os, KnobsContainer& container)
    {
      auto iter = container.m_theKnobs.begin();
      auto end = container.m_theKnobs.end();
      abstract_knob_c* pKnob = NULL;
      while (iter != end) {
        pKnob = iter->second;
        if (pKnob) {
          pKnob->display(os);
          os << endl;
        }
        ++iter;
      }
      return os;
    }

  private:
    /**
     * Create knob-value pair from the string.
     */
    void createEntryFromtext(const string& text);
    
    /**
     * Apply values to the all knobs.
     */
    void applyValuesToKnobs(map<string, string, ltstr_s>& ValuesMap);

  private:
    static const char MAGIC_COMMENTS = '#'; /**< enable comment in parameter file */
    all_knobs_c *m_allKnobs; /**< the knobs for this component instance **/
    map<string, abstract_knob_c*, ltstr_s> m_theKnobs; /**< knob table */
    map<string, string, ltstr_s> m_valuesFromFile; /**< knob values from the file */
    map<string, string, ltstr_s> m_valuesFromCommandLineSwitches; /**< values from command */
    KnobEntryTokenizer m_theTokenizer; /**< tokenizer */

};

#endif // KNOB_H_INCLUDED
