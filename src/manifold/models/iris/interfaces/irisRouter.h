/*
 * =====================================================================================
 *
 *       Filename:  IrisRouter.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/04/2011 02:26:36 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  IRISROUTER_H_INC
#define  IRISROUTER_H_INC

#include        "genericHeader.h"
#include        "../iris_srcs/data_types/linkData.h"
//#include	"manifoldTile.h"
#include	"irisInterface.h"


class IrisRouter:public manifold::kernel::Component
{
	public:
		IrisRouter(){}
		IrisRouter(uint nid ):node_id(nid){}

		/* ====================  Event handlers at the router-NI interface    ======================================= */
		virtual void handle_link_arrival( int inputId, LinkData* data ) = 0;

		/* ====================  Event handlers at the interface-router interface    ======================================= */

		/* ====================  Clocked funtions ======================================= */
		virtual void tick (void) = 0;
		virtual void tock (void) = 0;
		virtual void init (void) = 0;
		virtual void parse_config(map<string,string>&) = 0;


		/* ====================== variables  */
		/*  Used enum definitions from genericHeader for port descriptions */
		uint node_id;
		vector<uint> input_connections;
		vector<uint> output_connections;
		vector<uint> signal_outports;
		vector<uint> data_outports;

		//        vector<manifold::kernel::Component*> output_link_connections;
		//        vector<manifold::kernel::Component*> input_link_connections;

	protected:

	private:

}; /* -----  end of class IrisRouter  ----- */

#endif   /* ----- #ifndef IRISROUTER_H_INC  ----- */

