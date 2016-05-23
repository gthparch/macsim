#ifndef MACSIM_EVENT_H
#define MACSIM_EVENT_H

namespace SST { namespace MacSim {

enum MacSimEventType { NONE, START, FINISHED };

class MacSimEvent : public SST::Event {
	public:
    MacSimEvent(MacSimEventType type = NONE) : SST::Event(), m_type(type) {}
    ~MacSimEvent() {}

    MacSimEventType getType() { return m_type; }
    void setType(MacSimEventType type) { m_type = type; }

    // pure virtual functions overrides
    virtual const char* cls_name() const
    { return "macsim"; }

    virtual uint32_t cls_id() const
    { return 1; }

  private:
    MacSimEventType m_type;

}; // class MacSimEvent
}}
#endif //MACSIM_EVENT_H
