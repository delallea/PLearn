// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org


/* *******************************************************      
   * $Id: QuadraticUtilityCostFunction.h,v 1.2 2004/02/20 21:11:45 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef QuadraticUtilityCostFunction_INC
#define QuadraticUtilityCostFunction_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;

/*!   cost function that takes (output,target) as arguments, transforms
  them into a profit using a given profit_function,
  and returns the negative of quadratic utility = - profit + risk_aversion * profit * profit;
*/
class QuadraticUtilityCostFunction : public Kernel
{
  protected:
    real risk_aversion;
    ProfitFunc profit_function;
    QuadraticUtilityCostFunction(){}
  public:
    QuadraticUtilityCostFunction(real the_risk_aversion, CostFunc the_profit)
      : risk_aversion(the_risk_aversion), profit_function(the_profit)
    {}

    PLEARN_DECLARE_OBJECT(QuadraticUtilityCostFunction);
    
    virtual string info() const;
    virtual real evaluate(const Vec& output, const Vec& target) const; 
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
    virtual void write(ostream& out) const;
    virtual void oldread(istream& in);
/*!       Recognized option: "risk_aversion".
      ALSO: options of the form "profit_function.XXX" are passed to the
      profit_function kernel as "XXX".
*/
    
};

inline CostFunc quadratic_risk(real risk_aversion, CostFunc profit_function)
{
  return new QuadraticUtilityCostFunction(risk_aversion, profit_function);
}

} // end of namespace PLearn

#endif

