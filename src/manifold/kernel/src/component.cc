// Component implementation for Manifold
// George F. Riley, (and others) Georgia Tech, Spring 2010

#include <vector>
#include <map>

#include "common-defs.h"
#include "manifold.h"
#include "component.h"

using namespace std;


namespace manifold {
namespace kernel {

// Static members
CompId_t                       Component::nextId = 0;
vector<ComponentLpMapping>     Component::allComponents;
std::map<CompName_t, CompId_t> Component::allNames;

// Static functions
bool Component::IsLocal(CompId_t id)
{ 
    return (Manifold::GetRank() == GetComponentLP(id)); 
}

bool Component::IsLocal(CompName_t name)
{
    return (Manifold::GetRank() == GetComponentLP(name)); 
}

// This is bogus; find the right spot for this
LpId_t GetRank()
{
  return 0;
}

Component::Component()
{
}

Component::~Component()
{ // Virtual destructor
}

void Component::SetComponentId(CompId_t newId)
{
  myId = newId;
}

void Component::SetComponentName(CompName_t newName)
{
  myName = newName; 
}

CompId_t Component::GetComponentId() const
{
  return myId;
}

CompName_t Component::GetComponentName() const
{
  return myName;
}

//void Component::AddInputLink(LinkBase* l)
//{
//  inLinks.push_back(l);
//}

// Get the component, returning base class
Component* Component::GetComponent(CompId_t id)
{
  if ( id >= (signed int)allComponents.size() || id < 0 ) return 0;	  
  return allComponents[id].component;
}

// Get the component, returning base class
Component* Component::GetComponent(CompName_t name)
{
  NameMap::iterator iter = allNames.begin();
  iter = allNames.find(name);  
  if ( name == "None" || iter == allNames.end() ) return 0;	  
  return allComponents[allNames[name]].component;
}

LpId_t Component::GetComponentLP(CompId_t id)
{
  if ( id >= (signed int)allComponents.size() || id < 0 ) return -1;	  
  return allComponents[id].lp;
}

LpId_t Component::GetComponentLP(CompName_t name)
{
  NameMap::iterator iter = allNames.begin();
  iter = allNames.find(name);
  if ( name == "None" || iter == allNames.end() ) return -1;	  
  return allComponents[allNames[name]].lp;
}

#ifndef NO_MPI
void Component::Recv_remote(int inputIndex, Ticks_t sendTick, Ticks_t recvTick,
                   Time_t sendTime, Time_t recvTime, unsigned char* data, int len)
{
    LinkInputBase* input = inLinks[inputIndex];
    assert(input != 0);

    input->Recv(recvTick, recvTime, data, len);
}
#endif //#ifndef NO_MPI

} //namespace kernel
} //namespace manifold
