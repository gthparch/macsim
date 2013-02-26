/*
 * =====================================================================================
 *
 *       Filename:  linkData.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/14/2011 02:35:25 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  LINKDATA_H_INC
#define  LINKDATA_H_INC

#include	"../../interfaces/genericHeader.h"
//#include	"flit.h"
class Flit; //forward declaration
#include	<cstring>

class LinkData
{
    public:
        LinkData();
        ~LinkData();

        link_arrival_data_type type;
        uint vc;
        Flit *f;
        uint src;

        static int Serialize(const LinkData& p, unsigned char* buf);
        static LinkData* Deserialize(const unsigned char* data);
        static LinkData Deserialize(const unsigned char* data, int len );
        std::string toString(void) const;
};

#endif   /* ----- #ifndef LINKDATA_H_INC  ----- */
