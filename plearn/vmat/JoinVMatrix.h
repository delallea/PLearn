// -*- C++ -*-

// PLearn ("A C++ Machine Learning Library")
// Copyright (C) 2002 Julien Keable and Pascal Vincent
// 

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
   * $Id: JoinVMatrix.h,v 1.10 2004/07/07 17:30:48 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef JOINVMATRIX_H
#define JOINVMATRIX_H

#include "RowBufferedVMatrix.h"
#include "VMat.h"
#include <map>

#include <vector>

namespace PLearn {
using namespace std;


struct JoinFieldStat 
{
  enum statType { COUNT, NNONMISSING, NMISSING, SUM, SUMSQUARE, 
                  MEAN, VARIANCE, STDDEV, STDERR,
                  MIN, MAX };
  int from, to;
  statType stat;
  JoinFieldStat(){};
  JoinFieldStat(int f, int t, statType st)
    : from(f), to(t), stat(st)
      {}
};

/* TODO : preserve string mappings for fields that remain
 */


class JoinVMatrix : public RowBufferedVMatrix
{ 
  typedef RowBufferedVMatrix inherited;

  //! the fields generated by the join operation (declared with addStatField)
  typedef hash_multimap<Vec,int> Maptype;

  vector<JoinFieldStat> fld;
  VMat master,slave;
  Vec temp, tempkey;
  TVec<int> master_idx;
  TVec<int> slave_idx;
  Maptype mp;

public:

  JoinVMatrix(){};
  JoinVMatrix(VMat mas,VMat sla,TVec<int> mi, TVec<int> si);
  void addStatField(const string & statis,const string & namefrom,const string & nameto);

  PLEARN_DECLARE_OBJECT(JoinVMatrix);
  static void declareOptions(OptionList &ol);
    
  virtual void build();

protected:

  virtual void getNewRow(int idx, const Vec& v) const;

public:

  virtual string getValString(int col, real val) const;
  virtual const map<string,real>& getStringToRealMapping(int col) const;
  virtual const map<real,string>& getRealToStringMapping(int col) const;
  virtual real getStringVal(int col, const string & str) const;
  virtual string getString(int row,int col) const;

private:
    void build_();
};

DECLARE_OBJECT_PTR(JoinVMatrix);

} // end of namespace PLearn

#endif // JOINVMATRIX_H
