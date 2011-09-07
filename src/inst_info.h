/**********************************************************************************************
 * File         : inst_info.h
 * Author       : Hyesoon Kim
 * Date         : 4/8/2008 
 * SVN          : $Id: inst_info.h,v 1.5 2008-09-10 02:43:22 kacear Exp $:
 * Description  : trace generator inst info formation 
 *********************************************************************************************/

#ifndef INST_INFO_H_INCLUDED
#define INST_INFO_H_INCLUDED

#include "global_types.h"
#include "global_defs.h" 
#include "uop.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Register type enumerate
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Reg_Type_enum {
  INT_REG, /**< Integer register */
  FP_REG, /**< FP register */
  SPEC_REG, /**< Special register */
  EXTRA_REG, /**< Extra register */
  NUM_REG_MAPS,
} Reg_Type;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Register information data structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct reg_info_s {
  /**
   * Constructor
   */
  reg_info_s();

  uns16    m_reg;                //!< register number within the register set
  Reg_Type m_type;               //!< integer, floating point, extra
  uns16    m_id;                 //!< flattened register number (unique across sets)
} reg_info_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Trace information data structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct trace_info_sc_s {
  uns8 m_inst_size; //!< instruction size in x86 instructions 
  uns8 m_num_uop; //!< number of uop for x86 instructions 
  uns8 m_second_mem; /**< has second memory */ 
  bool m_bom; /**< first uop in an instruction */
  bool m_eom; /**< last uop in an instruction */
} trace_info_sc_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Static instruction information table
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct table_info_s {
  Uop_Type    m_op_type; //!< type of operation   
  Mem_Type    m_mem_type; //!< type of memory instruction
  Cf_Type     m_cf_type; //!< type of control flow instruction
  Bar_Type    m_bar_type; //!< type of barrier caused by instruction
  uns         m_num_dest_regs; //!< number of destination registers written
  uns         m_num_src_regs; //!< number of source registers read
  uns         m_mem_size; //!< number of bytes read/written by a memory instruction
//  char        m_name[256]; //!< Mnemonic of the instruction
  uns8        m_type; //!< the format type code for the instruction (see table)
  uns32       m_mask; /**< mask */
} table_info_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Instruction information data structure
///////////////////////////////////////////////////////////////////////////////////////////////
class inst_info_s
{
  public:
    Addr             m_addr;    //!< address of the instruction
    table_info_s    *m_table_info; //!< pointer into the table of static instruction information
    reg_info_s       m_srcs [MAX_SRCS]; //!< source register information
    reg_info_s       m_dests[MAX_DESTS]; //!< destination register information
    trace_info_sc_s  m_trace_info; //!< trace information

    /**
     * Constructor
     */
    inst_info_s()
    {
      m_table_info = new table_info_s;
    }

    /**
     * Destructor
     */
    ~inst_info_s() {}
};

#endif //INST_INFO_H_INCLUDED
