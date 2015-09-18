/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/



#include <stdint.h> // uint64_t

#ifndef CALLBACK_H
#define CALLBACK_H

namespace SST { namespace MacSim {

template <typename ReturnT, typename Param1T>
class CallbackBase1
{
public:
	virtual ~CallbackBase1() = 0;
	virtual ReturnT operator()(Param1T) = 0;
};

template <typename ReturnT, typename Param1T> MacSim::CallbackBase1<ReturnT,Param1T>::~CallbackBase1() {}

template <typename ConsumerT, typename ReturnT, typename Param1T> 
class Callback1: public CallbackBase1<ReturnT,Param1T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T);

public:
	Callback1( ConsumerT* const object, PtrMember member) : object(object), member(member) { } 
	Callback1( const Callback1<ConsumerT,ReturnT,Param1T>& e ) : object(e.object), member(e.member) { } 
	ReturnT operator()(Param1T param1) { return (const_cast<ConsumerT*>(object)->*member) (param1); }

private:
	ConsumerT* const object;
	const PtrMember  member;
};



template <typename ReturnT, typename Param1T, typename Param2T>
class CallbackBase2
{
public:
	virtual ~CallbackBase2() = 0;
	virtual ReturnT operator()(Param1T,Param2T) = 0;
};

template <typename ReturnT, typename Param1T, typename Param2T> MacSim::CallbackBase2<ReturnT,Param1T,Param2T>::~CallbackBase2() {}

template <typename ConsumerT, typename ReturnT, typename Param1T, typename Param2T> 
class Callback2: public CallbackBase2<ReturnT,Param1T,Param2T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T,Param2T);

public:
	Callback2( ConsumerT* const object, PtrMember member) : object(object), member(member) { }
	Callback2( const Callback2<ConsumerT,ReturnT,Param1T,Param2T>& e ) : object(e.object), member(e.member) { }
	ReturnT operator()(Param1T param1, Param2T param2) { return (const_cast<ConsumerT*>(object)->*member) (param1,param2); }

private:
	ConsumerT* const object;
	const PtrMember  member;
};



template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T>
class CallbackBase3
{
public:
	virtual ~CallbackBase3() = 0;
	virtual ReturnT operator()(Param1T,Param2T,Param3T) = 0;
};

template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T> MacSim::CallbackBase3<ReturnT,Param1T,Param2T,Param3T>::~CallbackBase3() {}

template <typename ConsumerT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T> 
class Callback3: public CallbackBase3<ReturnT,Param1T,Param2T,Param3T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T,Param2T,Param3T);

public:
	Callback3( ConsumerT* const object, PtrMember member) : object(object), member(member) { }
	Callback3( const Callback3<ConsumerT,ReturnT,Param1T,Param2T,Param3T>& e ) : object(e.object), member(e.member) { }
	ReturnT operator()(Param1T param1, Param2T param2, Param3T param3) { return (const_cast<ConsumerT*>(object)->*member) (param1,param2,param3); }

private:
	ConsumerT* const object;
	const PtrMember  member;
};




template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T>
class CallbackBase4
{
public:
	virtual ~CallbackBase4() = 0;
	virtual ReturnT operator()(Param1T,Param2T,Param3T,Param4T) = 0;
};

template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T> MacSim::CallbackBase4<ReturnT,Param1T,Param2T,Param3T,Param4T>::~CallbackBase4() {}

template <typename ConsumerT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T> 
class Callback4: public CallbackBase4<ReturnT,Param1T,Param2T,Param3T,Param4T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T,Param2T,Param3T,Param4T);

public:
	Callback4( ConsumerT* const object, PtrMember member) : object(object), member(member) { }
	Callback4( const Callback4<ConsumerT,ReturnT,Param1T,Param2T,Param3T,Param4T>& e ) : object(e.object), member(e.member) { }
	ReturnT operator()(Param1T param1, Param2T param2, Param3T param3, Param4T param4) { return (const_cast<ConsumerT*>(object)->*member) (param1,param2,param3,param4); }

private:
	ConsumerT* const object;
	const PtrMember  member;
};




template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T>
class CallbackBase5
{
public:
	virtual ~CallbackBase5() = 0;
	virtual ReturnT operator()(Param1T,Param2T,Param3T,Param4T,Param5T) = 0;
};

template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T> MacSim::CallbackBase5<ReturnT,Param1T,Param2T,Param3T,Param4T,Param5T>::~CallbackBase5() {}

template <typename ConsumerT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T> 
class Callback5: public CallbackBase5<ReturnT,Param1T,Param2T,Param3T,Param4T,Param5T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T,Param2T,Param3T,Param4T,Param5T);

public:
	Callback5( ConsumerT* const object, PtrMember member) : object(object), member(member) { }
	Callback5( const Callback5<ConsumerT,ReturnT,Param1T,Param2T,Param3T,Param4T,Param5T>& e ) : object(e.object), member(e.member) { }
	ReturnT operator()(Param1T param1, Param2T param2, Param3T param3, Param4T param4, Param5T param5) { return (const_cast<ConsumerT*>(object)->*member) (param1,param2,param3,param4,param5); }

private:
	ConsumerT* const object;
	const PtrMember  member;
};

template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T>
class CallbackBase6
{
public:
	virtual ~CallbackBase6() = 0;
	virtual ReturnT operator()(Param1T,Param2T,Param3T,Param4T,Param5T,Param6T) = 0;
};
template <typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T> MacSim::CallbackBase6<ReturnT,Param1T,Param2T,Param3T,Param4T,Param5T,Param6T>::~CallbackBase6() {}


template <typename ConsumerT, typename ReturnT, typename Param1T, typename Param2T, typename Param3T, typename Param4T, typename Param5T, typename Param6T> 
class Callback6: public CallbackBase6<ReturnT,Param1T,Param2T,Param3T,Param4T,Param5T,Param6T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T,Param2T,Param3T,Param4T,Param5T,Param6T);

public:
	Callback6( ConsumerT* const object, PtrMember member) : object(object), member(member) { }
	Callback6( const Callback6<ConsumerT,ReturnT,Param1T,Param2T,Param3T,Param4T,Param5T,Param6T>& e ) : object(e.object), member(e.member) { }
	ReturnT operator()(Param1T param1, Param2T param2, Param3T param3, Param4T param4, Param5T param5, Param6T param6) { return (const_cast<ConsumerT*>(object)->*member) (param1,param2,param3,param4,param5,param6); }


private:
	ConsumerT* const object;
	const PtrMember  member;
};

}}

#endif
