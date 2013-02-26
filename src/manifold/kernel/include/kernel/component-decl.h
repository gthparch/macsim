/** @file component-decl.h
 *  Common definitions for Manifold
 *
 *
 * This file contains the declarations (not the implementation)
 * of the Component class.  The actual implementation is in 
 * component.h. We split them apart since we have circular dependencies
 * with component requiring manifold, and manifold requiring component.
 */
 
 //George F. Riley, (and others) Georgia Tech, Fall 2010
 

#ifndef __COMPONENT_DECL_H__
#define __COMPONENT_DECL_H__

#include "common-defs.h"
#include "link-decl.h"
#include <string>
#include <map>

namespace manifold {
namespace kernel {

class Component;

/** \class ComponentLpMapping Stores the logical process id for each
 *  component in the system with associated name. For distributed simulations, 
 *  all components are created on all LP's; only those "mapped" to that LP
 *  are actually created, other use null pointers. 
 */
class ComponentLpMapping 
{
public:

  /** ComponentLpMapping Constructor
   *  @arg \c c Component ptr
   *  @arg \c l LpId
   */
  ComponentLpMapping(Component* c, LpId_t l)
    : component(c), lp(l) {}
public:
  /** Pointer to the component being mapped, 
   *  nil if it is on a remote Lp
   */
  Component* component;
  
  /** Lp id for the component this mapping refers to
   */
  LpId_t     lp;
};


/** \class LinkTypeMismatchException component-decl.h
 *
 * A LinkTypeMismatchException is thrown when trying to set up a link
 * to a component input where a link with a different data type already
 * exists.
 */
class LinkTypeMismatchException
{
};


/** \class Component The base class for all models.
 */
class Component 
{
public:
  
  /** Empty base constructor
   */
  Component();
  
  /** Virtual destructor
   */
  virtual ~Component();
  
  /** Sets the id of the component
   *  @arg \c newId The id of the component 
   */
  void SetComponentId(CompId_t newId);
  
  /** Set the name of the component
   * @arg \c name The unique name of the component
   */
  void SetComponentName(CompName_t newName);

  /** Returns the component id
   */
  int GetComponentId() const;    

  /** Returns the component name
   */
  CompName_t GetComponentName() const; 
  
  /** Creates a complex link that sends arbitrary type T
   * @arg \c outIndex The index(port) number of the Link
   */
  template <typename T> Link<T>* AddOutputLink(int outIndex) throw (LinkTypeMismatchException);
  
  /** Adds an input for the component to keep track of.
   *  @arg Ptr to the input Link 
   */
  //void AddInputLink(LinkBase*);

  /** Add an input for the component.
   */
  template<typename T, typename T2>
  void AddInput(int inputIndex, void (T::*handler)(int, T2), T* obj, Clock* c, bool isTimed, bool isHalf);

  /** Used to send data on an output link of any type
   *  @arg The output index(port) number to send the data on. 
   *  @arg The actual data being sent over the Link.
   */
  template <typename T>
    void Send(int, T);
    
  template <typename T>
    void SendTick(int, T, Ticks_t);
    
  /** This is called by the scheduler to handler an incoming message.
   */
  template <typename T>
  void Recv_remote(int inputIndex, Ticks_t sendTick, Ticks_t recvTick,
                   Time_t sendTime, Time_t recvTime, T data);

  void Recv_remote(int inputIndex, Ticks_t sendTick, Ticks_t recvTick,
                   Time_t sendTime, Time_t recvTime, unsigned char* data, int len);


  // Static functions
  
  /** Return true if component is local to this LP
   *  @arg Component id
   */
  static bool IsLocal(CompId_t);

  /** Return true if component is local to this LP
   * @arg Component name
   */ 
  static bool IsLocal(CompName_t);

  /** Variation on Create, 0 arguments for the constructor.
   *  The first argument is always type LpId_t, and indicates which LP
   * is desired; subsequent parameters are passed to the constructor.
   * @arg Component id
   * @arg Component name (optional)
   */
  template <typename T>
    static CompId_t Create(LpId_t, CompName_t name = "None");

  /** Variation on Create, 1 arugment for the constructor.
   *  The first argument is always type LpId_t, and indicates which LP
   * is desired; subsequent parameters are passed to the constructor.
   * @arg Lp id
   * @arg Any type argument
   * @arg Component name (optional)
   */    
  template <typename T, typename T1> 
    static CompId_t Create(LpId_t, const T1&, CompName_t name = "None");

  /** Variation on Create, 2 arugments for the constructor.
   *  The first argument is always type LpId_t, and indicates which LP
   * is desired; subsequent parameters are passed to the constructor.
   * @arg Lp id
   * @arg Any type argument
   * @arg Any type argument
   * @arg Component name (optional)
   */        
  template <typename T, typename T1, typename T2> 
    static CompId_t Create(LpId_t, const T1&, const T2&, CompName_t name = "None");

  /** Variation on Create, 3 arugments for the constructor.
   *  The first argument is always type LpId_t, and indicates which LP
   * is desired; subsequent parameters are passed to the constructor.
   * @arg Lp id
   * @arg Any type argument
   * @arg Any type argument
   * @arg Any type argument   
   * @arg Component name (optional)
   */        
  template <typename T, typename T1, typename T2, typename T3> 
    static CompId_t Create(LpId_t, const T1&, const T2&, const T3&, CompName_t name = "None");

  /** Variation on Create, 4 arugments for the constructor.
   *  The first argument is always type LpId_t, and indicates which LP
   *  is desired; subsequent parameters are passed to the constructor.
   * @arg Lp id
   * @arg Any type argument
   * @arg Any type argument
   * @arg Any type argument
   * @arg Any type argument   
   * @arg Component name (optional)
   */        
  template <typename T, typename T1, typename T2, typename T3, typename T4> 
    static CompId_t Create(LpId_t, const T1&, const T2&, const T3&, const T4&, CompName_t name = "None");

  /** Static function to get the original component pointer by component id
   *  @arg CompId_t
   */ 
  template <typename T>
    static T* GetComponent(CompId_t);
   
  /** Static function to get the original component pointer by component name
   *  @arg CompName_t
   */ 
  template <typename T>
    static T* GetComponent(CompName_t);
 
  /** Static function to get the component, returning base class pointer
   *  by component id
   *  @arg Component id
   */
  static Component* GetComponent(CompId_t);
  
  /** Static function to get the component, returning base class pointer
   *  by component name
   *  @arg Component name
   */
  static Component* GetComponent(CompName_t);
 
  /** Static function to get the component LP by component id
   *  @arg Component id
   */
  static LpId_t GetComponentLP(CompId_t);
  
  /** Static function to get the component LP by component name
   *  @arg Component name
   */
  static LpId_t GetComponentLP(CompName_t);

protected:
  
  /** Vector of input link ptrs for this component
   */
  //std::vector<LinkBase*> inLinks;

#ifndef NO_MPI
  std::vector<LinkInputBase*> inLinks;
#endif //#ifndef NO_MPI
  
  /** Vector of output link ptrs for this component
   */
  std::vector<LinkBase*> outLinks;
private:
  
  /** Component id for this component.
   */
  CompId_t myId;
 
  /** Component name for this component.
   */
  CompName_t myName; 

  /** Next available component id
   */
  static CompId_t nextId;
  
  /** vector of all componentlpmappings for all components in the system
   */ 
  static std::vector<ComponentLpMapping> allComponents;

  /** vector of all componentlpmappings for all components in the system
   */ 
  static std::map<CompName_t, CompId_t> allNames;
};

} //namespace kernel
} //namespace manifold


#endif
