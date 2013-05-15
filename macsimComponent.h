#ifndef _MACSIM_COMPONENT_H
#define _MACSIM_COMPONENT_H

#include <sst/core/event.h>
#include <sst/core/sst_types.h>
#include <sst/core/component.h>
#include <sst/core/timeConverter.h>

#include "src/macsim.h"

class macsimComponent : public SST::Component {
	
	public:
		macsimComponent(SST::ComponentId_t id, SST::Component::Params_t& params);
		void setup();
		void finish();
		
	private:
		macsimComponent();   // for serialization only
		macsimComponent(const macsimComponent&);   // do not implement
		void operator=(const macsimComponent&); // do not implement
	
		virtual bool ticReceived(SST::Cycle_t);
		
		//MEMBER VARIABLES
		std::string  paramPath;
		std::string  tracePath;
		std::string outputPath;
		macsim_c* macsim;
		bool simRunning;
		
		//FOR SERIALIZATION
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const 
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) 
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
		}
		
		BOOST_SERIALIZATION_SPLIT_MEMBER()
};
#endif /* _MACSIM_COMPONENT_H */
