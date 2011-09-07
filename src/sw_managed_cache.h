/**********************************************************************************************
 * File         : sw_managed_cache.h
 * Author       : Nagesh BL
 * Date         : 03/03/2010 
 * SVN          : $Id: cache.h,
 * Description  : software managed cache 
 *               (each instance of this class encapsulates an instance of class cache_c)
 *********************************************************************************************/

#ifndef SW_MANAGED_CACHE_H
#define SW_MANAGED_CACHE_H

#include "cache.h"
#include "global_defs.h"
#include "global_types.h"


///////////////////////////////////////////////////////////////////////////////////////////////
///  \brief class for software managed cache
///
/// Scaratchpad memory class (model GPU's shared memory)
///////////////////////////////////////////////////////////////////////////////////////////////
class sw_managed_cache_c
{
  public:
   /*! \fn sw_managed_cache_c(string name, int c_id, uns32 c_size, 
    *  uns8 c_assoc, uns8 c_line_size, uns8 c_banks, uns8 c_cycles, 
    *  bool by_pass, Cache_Type c_type, uns n_read_ports, 
    *  uns n_write_ports, int c_data_size)
    *  \brief constructor to create a software managed cache
    */
    sw_managed_cache_c(string name, int c_id, uns32 c_size, uns8 c_assoc,
        uns8 c_line_size, uns8 c_banks, uns8 c_cycles,
        bool by_pass, Cache_Type c_type, uns n_read_ports,
        uns n_write_ports, int c_data_size, macsim_c* simBase);

   /*! \fn ~sw_managed_cache_c()
    *  \brief destructor for class sw_managed_cache_c
    */
    ~sw_managed_cache_c();

   /*! \fn load(uop_c *uop)
    *  \brief access software managed cache
    *  \param uop - pointer to uop which requires access to the sw managed cache
    *  \return int - 0 for bank conflict, x > 0 for successful cache access and 
    *   x is the cache access latency
    */
    int load(uop_c *uop);

   /*! \fn base_cache_line(Addr addr)
    *  \brief get the address of the cache line containing the specified address
    *  \param addr - address whose cache line address is required
    *  \return Addr - requested cache line address
    */
    Addr base_cache_line(Addr addr);

   /*! \fn cache_line_size(void)
    *  \brief returns the cache block size for this cache
    *  \return uns8 cache block size
    */
    uns8 cache_line_size(void);

  private:
    int       m_core_id;        /**< core id */
    cache_c  *m_cache;          /**< cache structure */
    port_c  **m_ports;          /**< cache ports */
    uns8      m_cache_cycles;   /**< latency */
    uns8      m_cache_line_size; /**< cache line size */
    uns8      m_num_banks;      /**< number of cache banks */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};

#endif //SW_MANAGED_CACHE_H_INCLUDED
