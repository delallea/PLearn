// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: TmpFilenames.cc,v 1.4 2004/02/20 21:11:44 chrish42 Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#include "TmpFilenames.h"
#include "fileutils.h"

namespace PLearn {
using namespace std;

TmpFilenames::TmpFilenames(int nfiles, const string directory, const string prefix)
{
  for(int k=0; k<nfiles; k++)
    addFilename(directory, prefix);
}

TmpFilenames::TmpFilenames(const TmpFilenames& other)
{ operator=(other); }

void TmpFilenames::operator=(const TmpFilenames& other)
{
  resize(other.size());
  for(int k=0; k<size(); k++)
  {
    operator[](k) = other[k];
  }
}

string TmpFilenames::addFilename(const string directory, const string prefix)
{
  string newfilename = newFilename(directory,prefix);
  append(newfilename);
  return newfilename;
}

char* TmpFilenames::newFile(const string directory, const string prefix)
{
  PLERROR("TmpFilenames::newFile : DEPRECATED method.  Use addFilename instead.");
  char* dummy = 0; return dummy;  // for return compatibility
}


} // end of namespace PLearn
