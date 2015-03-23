#ifndef MACSIM_EVENT_H
#define MACSIM_EVENT_H

namespace SST { namespace MacSim {

enum class MacSimEventType { NULL, START, FINISHED };

class MacSimEvent
{
	public:
    MacSimEvent(MacSimEventType type = MacSimEventType::NULL) : m_type(type);
    ~MacSimEvent();

    MacSimEventType getType() { return m_type; }
    void setType(MacSimEventType type) { m_type = type; }

  private:
    MacSimEventType m_type;

}; // class MacSimEvent
}}
#endif //MACSIM_EVENT_H
