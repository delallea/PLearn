// -*- C++ -*-

// plearn_main.cc
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: plearn_main.cc,v 1.17 2004/12/01 01:28:16 dorionc Exp $
   ******************************************************* */

//#include "general.h"
#include "plearn_main.h"
#include "PLearnCommandRegistry.h"
//#include "stringutils.h"
#include <plearn/math/random.h>
#include <plearn/sys/PLMPI.h>
//#include "Object.h"
//#include "RunCommand.h"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

static bool is_command( string& possible_command )
{
  if(PLearnCommandRegistry::is_registered(possible_command))
    return true; 

  if( file_exists(possible_command) )
  {
    possible_command = "run";
    return false;
  }
  
  PLERROR( "%s appears to neither be a known PLearn command, "
           "nor an existing .plearn script", possible_command.c_str() );
  return false;
}

static void output_version( int major_version, int minor_version, int fixlevel )
{
  cerr << prgname()
       << " "      << major_version
       << "."      << minor_version
       << "."      << fixlevel
       << "  ("    << __DATE__ << " "
       << __TIME__ << ")"      << endl;
}

static string global_options( vector<string>& command_line,
                              int major_version, int minor_version, int fixlevel )
{
  int argc                 = command_line.size();

  int no_version_pos       = findpos( command_line, "--no-version" );  

  int verbosity_pos        = findpos( command_line, "--verbosity"  );
  int verbosity_value      = VLEVEL_NORMAL;
  int verbosity_value_pos  = -1; // IMPORTANT FOR THE TEST BELOW (for loop)
  if ( verbosity_pos != -1 )
  {
    verbosity_value_pos = verbosity_pos+1;
    if ( verbosity_value_pos < argc )
      verbosity_value = toint( command_line[verbosity_value_pos] );
    else
      PLERROR("Option --verbosity must be followed by an integer value.");
  }

  
  int    cleaned     = 0;
  string the_command = "";
  vector<string> old( command_line );
  
  for ( int c=0; c < argc; c++ )
    if ( c != no_version_pos   &&
         c != verbosity_pos    &&
         c != verbosity_value_pos )
    {
      if ( the_command == "" )
      {
        the_command = old[c];
        if ( !is_command( the_command ) )
          command_line[cleaned++] = old[c];
      }
      else
        command_line[cleaned++] = old[c];
    }
  command_line.resize( cleaned );
  
  if ( no_version_pos == -1 )
    output_version( major_version, minor_version, fixlevel );
  PL_Log::instance().verbosity( verbosity_value );

  return the_command;
}

int plearn_main( int argc, char** argv,
                 int major_version, int minor_version, int fixlevel )
{
  try {

  PLMPI::init(&argc, &argv);

  seed();

  // set program name
  prgname(argv[0]);

  if(argc<=1)
    {
      cerr << "Type '" << prgname() << " help' for help" << endl;
      return 0;
    }
  else 
    {
      vector<string> command_line = stringvector(argc-1, argv+1);
      string command = global_options( command_line, major_version, minor_version, fixlevel );
      PLearnCommandRegistry::run(command, command_line);				
    }

  PLMPI::finalize();
  
  } // end of try
  catch(const PLearnError& e)
  {
    cerr << "FATAL ERROR: " << e.message() << endl;
  }
  catch (...) 
  {
    cerr << "FATAL ERROR: uncaught unknown exception" << endl;
  }

  return 0;
}

} // end of namespace PLearn
