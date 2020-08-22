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

#include <stdint.h>  // uint64_t

#ifndef CALLBACK_H
#define CALLBACK_H

namespace SST {
namespace MacSim {

template <typename ReturnT, typename... Params>
class CallbackBase
{
public:
  virtual ~CallbackBase() = 0;
  virtual ReturnT operator()(Params...) = 0;
};

template <typename ReturnT, typename... Params>
CallbackBase<ReturnT, Params...>::~CallbackBase() {
}

template <typename ConsumerT, typename ReturnT, typename... Params>
class Callback : public CallbackBase<ReturnT, Params...>
{
private:
  typedef ReturnT (ConsumerT::*PtrMember)(Params...);

public:
  Callback(ConsumerT* const object, PtrMember member)
    : object(object), member(member) {
  }
  Callback(const Callback<ConsumerT, ReturnT, Params...>& e)
    : object(e.object), member(e.member) {
  }
  ReturnT operator()(Params... params) {
    return (const_cast<ConsumerT*>(object)->*member)(params...);
  }

private:
  ConsumerT* const object;
  const PtrMember member;
};

}  // namespace MacSim
}  // namespace SST

#endif
