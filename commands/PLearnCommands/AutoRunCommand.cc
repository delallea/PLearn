
// -*- C++ -*-

// AutoRunCommand.cc
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: AutoRunCommand.cc,v 1.3 2004/02/20 21:11:40 chrish42 Exp $ 
   ******************************************************* */

/*! \file AutoRunCommand.cc */

#include "AutoRunCommand.h"
#include "RunCommand.h"
#include "TVec.h"
//#include "fileutils.h"
//#include <unistd.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'AutoRunCommand' command in the command registry
PLearnCommandRegistry AutoRunCommand::reg_(new AutoRunCommand);

//! The actual implementation of the 'AutoRunCommand' command 
void AutoRunCommand::run(const vector<string>& args)
{
  string scriptname = args[0];
  int nargs = args.size();

  vector<string> runargs(1);
  runargs[0] = scriptname;

  TVec<int> times(nargs);

  for(;;)
    {
      try 
        {
          RunCommand rc;
          cerr << ">>> Running script " << scriptname << endl; 
          rc.run(runargs);
        }
      catch(const PLearnError& e)
        {
          cerr << "FATAL ERROR running script " << scriptname << "\n"
               << e.message() << endl;
        }
      
      sleep(1);

      // Store times
      for(int k=0; k<nargs; k++)
        times[k] = mtime(args[k]);

      bool up_to_date = true;
      while(up_to_date)
        {
          sleep(1);
          for(int k=0; k<nargs; k++)
            if(times[k] < mtime(args[k]))
              {
                cerr << " File " << args[k] << " has changed." << endl;
                up_to_date = false;
              }
        }
    }

}

} // end of namespace PLearn

