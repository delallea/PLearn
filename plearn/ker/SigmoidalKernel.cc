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
   * $Id: SigmoidalKernel.cc,v 1.2 2004/02/20 21:11:45 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SigmoidalKernel.h"

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


PLEARN_IMPLEMENT_OBJECT(SigmoidalKernel, "ONE LINE DESCR", "NO HELP");
real SigmoidalKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return sigmoid(c*dot(x1,x2)); }


void SigmoidalKernel::write(ostream& out) const
{
  writeHeader(out,"SigmoidalKernel");
  inherited::oldwrite(out);
	writeField(out,"c",c);
  writeFooter(out,"SigmoidalKernel");
}

void SigmoidalKernel::oldread(istream& in)
{
  readHeader(in,"SigmoidalKernel");
  inherited::oldread(in);
  readField(in,"c",c);
  readFooter(in,"SigmoidalKernel");
}
// recognized option is "c"

/*
void SigmoidalKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="c")
    PLearn::read(in,c);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void SigmoidalKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "c", &SigmoidalKernel::c, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}



} // end of namespace PLearn

