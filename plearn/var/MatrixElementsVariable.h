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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef MatrixElementsVariable_INC
#define MatrixElementsVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


class MatrixElementsVariable: public NaryVariable
{
    typedef NaryVariable inherited;

protected:
    Var i;
    Var j;
    int ni;
    int nj;
    Var expression;
    VarArray parameters;

    VarArray full_fproppath; //!<  output(inputs&parameters)
    VarArray fproppath; //!<  output(inputs)
    VarArray bproppath; //!<  output(parameters)

public:
    //!  protected default constructor for persistence
    MatrixElementsVariable()
        : i(), j(), ni(), nj(), expression(), parameters(), full_fproppath(), fproppath(),
          bproppath()
    {}
    MatrixElementsVariable(Variable* the_expression, const Var& i_index, const Var& j_index,
                           int number_of_i_values, int number_of_j_values, const VarArray& the_parameters);
                         
    PLEARN_DECLARE_OBJECT(MatrixElementsVariable);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual void fprop();
    virtual void bprop();
    virtual void fbprop();

protected:
    void build_();
};

DECLARE_OBJECT_PTR(MatrixElementsVariable);

inline Var matrixElements(Var expression, const Var& i, const Var& j,
                          int ni, int nj, const VarArray& parameters)
{ return new MatrixElementsVariable(expression, i, j, ni, nj, parameters); }

} // end of namespace PLearn

#endif 


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
