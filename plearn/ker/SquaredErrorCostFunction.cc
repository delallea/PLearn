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
   * $Id: SquaredErrorCostFunction.cc,v 1.2 2004/02/20 21:11:45 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SquaredErrorCostFunction.h"

// From Old Kernel.cc: all includes are putted in every file.
// To be revised manually 
#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"
//////////////////////////
namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(SquaredErrorCostFunction, "ONE LINE DESCR", "NO HELP");
string SquaredErrorCostFunction::info() const { return "squared_error"; }

real SquaredErrorCostFunction::evaluate(const Vec& output, const Vec& target) const
{
#ifdef BOUNDCHECK
  if(target.length()!=output.length() && classification==false)
    PLERROR("In SquaredErrorCostFunction::evaluate target.length() %d should be equal to output.length() %d",target.length(),output.length());
#endif

  real result = 0.0;
  if (targetindex>=0)
    result = square(output[targetindex]-target[targetindex]);
  else
    {
      real* outputdata = output.data();
      real* targetdata = target.data();
      if (classification) {
	if (target.length() != 1)
	  PLERROR("In SquaredErrorCostFunction::evaluate target.length() %s should be 1", target.length());

	for (int i = 0; i < output.length(); ++i)
	  if (i == targetdata[0])
	    result += square(outputdata[i] - hotvalue);
	  else
	    result += square(outputdata[i] - coldvalue);
      } else {
	for(int i=0; i<output.length(); i++)
	  result += square(outputdata[i]-targetdata[i]);
      }
    }
  return result;
}


void SquaredErrorCostFunction::write(ostream& out) const
{
  writeHeader(out,"SquaredErrorCostFunction");
  writeField(out,"targetindex",targetindex);
  writeField(out, "classification", classification);
  writeField(out, "coldvalue", coldvalue);
  writeField(out, "hotvalue", hotvalue);
  writeFooter(out,"SquaredErrorCostFunction");
}


void SquaredErrorCostFunction::oldread(istream& in)
{
  readHeader(in,"SquaredErrorCostFunction");
  readField(in,"targetindex",targetindex);
  if (in.peek() != '<') {
    // Newly added stuff, hack
    readField(in, "classification", classification); 
    readField(in, "coldvalue", coldvalue);
    readField(in, "hotvalue", hotvalue);
  }
  readFooter(in,"SquaredErrorCostFunction");

  if (hotvalue == -1)
    classification = true;
}

/*
// recognized option is "norm_to_use"
void SquaredErrorCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="targetindex")
    PLearn::read(in,targetindex);
  if (optionname == "hotvalue")
    PLearn::read(in,hotvalue);
  if (optionname == "coldvalue")
    PLearn::read(in,coldvalue);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void SquaredErrorCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "targetindex", &SquaredErrorCostFunction::targetindex, OptionBase::buildoption, "Index of target");
    declareOption(ol, "hotvalue", &SquaredErrorCostFunction::hotvalue, OptionBase::buildoption, "Hot value");
    declareOption(ol, "coldvalue", &SquaredErrorCostFunction::coldvalue, OptionBase::buildoption, "Cold value");
    declareOption(ol, "classification", &SquaredErrorCostFunction::classification, OptionBase::buildoption, "Used as classification cost");
    inherited::declareOptions(ol);
}



} // end of namespace PLearn

