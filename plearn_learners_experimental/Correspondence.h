// -*- C++ -*-

// Correspondence.h
//
// Copyright (C) 2004 Pascal Lamblin 
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
   * $Id: Correspondence.h,v 1.2 2004/11/17 01:42:02 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file Correspondence.h */


#ifndef Correspondence_INC
#define Correspondence_INC

#include <plearn/base/Object.h>

namespace PLearn {
using namespace std;

class Correspondence;
typedef PP< Correspondence > Corresp;

class Correspondence: public Object
{

private:
  
  typedef Object inherited;

public:

  // ************************
  // * public build options *
  // ************************

  int sid;
  int mid;
  int model_id;
  real votes;
  real correlation;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  Correspondence();

  //! Copy constructor.
  Correspondence( const Correspondence& c );

  //! Virtual destructor.
  virtual ~Correspondence() {}

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:
  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(Correspondence);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  // (PLEASE IMPLEMENT IN .cc)
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  Correspondence& operator=( const Correspondence& c );

  static int compare( const Correspondence& p, const Correspondence& q );

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Correspondence);

bool operator<( const Correspondence& p, const Correspondence& q );

} // end of namespace PLearn

#endif
