// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2001 Yoshua Bengio and University of Montreal
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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file Profiler.h */

#ifndef PROFILER_INC
#define PROFILER_INC


#ifndef WIN32
#define PROFILE
#include <sys/times.h>
#endif

#include <plearn/base/general.h>
#include <plearn/base/plerror.h>
#include <plearn/base/stringutils.h>
#include <plearn/io/PStream.h>

namespace PLearn {
using namespace std;

/**
 *  Profiling tools, to count average time elapsed and number of times 
 *  traversed for pieces of code delimited by two calls to the static
 *  functions.
 *
 *  @code
 *  Profiler::start("name_of_piece_of_code");
 *  // ...
 *  Profiler::end("name_of_piece_of_code");
 *  @endcode
 *    
 *  A static field of Profiler is used to maintain the statistics of 
 *  occurence and durations for each piece of code. Calls to start/end
 *  for the same name cannot be nested.  Three different durations are measured
 *  for a piece of code:
 *
 *  - Wall duration (how long of "actual time" was measured, in clock ticks)
 *  - User duration ("User" time, in clock ticks)
 *  - System duration ("System" time, in clock ticks)
 *    
 *  Before the above calls, usually in the main program, the user
 *  must call
 *
 *  @code
 *  Profiler::activate();
 *  @endcode
 *    
 *  and after all the above calls, a report for all such pieces of code can
 *  then be obtained by calling
 *
 *  @code
 *  Profiler::report(cout);
 *  @endcode
 *    
 *  on an output stream. 
 *    
 *  This code is based on statistical averaging (using the C "times" function)
 *  because the individual measurements of elapsed time with times are too
 *  coarse (e.g. 100th of a second).
 */

class Profiler {
public:
    class Stats {
    public:
        long frequency_of_occurence;         //!< Number of start/stop cycles
        long wall_duration;                  //!< Wall-so-far, in clock ticks
        long user_duration;                  //!< User-so-far, in clock ticks
        long system_duration;                //!< System-so-far, in clock ticks
        clock_t wall_last_start;             //!< Wall when last started
        clock_t user_last_start;             //!< User when last started
        clock_t system_last_start;           //!< System when last started
        int nb_going;                       //!< Whether we have started this stat
      
        Stats()
            : frequency_of_occurence(0),
              wall_duration(0),
              user_duration(0),
              system_duration(0),
              wall_last_start(0),
              user_last_start(0),
              system_last_start(0),
              nb_going(0)
        { }
    };

public:

    //! Enable profiling
    static void activate()
    {
#ifdef WIN32
        PLERROR("In Profiler::activate - Profiling is not currently supported "
                "under Windows");
#endif
        active=true;
    }

    //! Disable profiling
    static void deactivate() { active=false; }

    //! Return activation status
    static bool isActive() { return active; }
    
    //!  Start recording time for named piece of code
#ifdef PROFILE
    static void start(const string& name_of_piece_of_code, const int max_nb_going=1);
#else
    static inline void start(const string& name_of_piece_of_code, const int max_nb_going=1) { }
#endif

    //!  End recording time for named piece of code, and increment
    //!  frequency of occurence and total duration of this piece of code.
#ifdef PROFILE
    static void end(const string& name_of_piece_of_code);
#else
    static inline void end(const string& name_of_piece_of_code) { } 
#endif

    //!  call start if if PL_PROFILE is set
#if defined(PROFILE) && defined(PL_PROFILE)
    static void pl_profile_start(const string& name_of_piece_of_code, const int max_nb_going=1);
#else
    static inline void pl_profile_start(const string& name_of_piece_of_code, const int max_nb_going=1) {}
#endif

    //!  call end() if if PL_PROFILE is set
#if defined(PROFILE) && defined(PL_PROFILE)
    static void pl_profile_end(const string& name_of_piece_of_code);
#else
    static inline void pl_profile_end(const string& name_of_piece_of_code) { } 
#endif

    //!  call activate() if if PL_PROFILE is set
#if defined(PROFILE) && defined(PL_PROFILE)
    static void pl_profile_activate();
#else
    static inline void pl_profile_activate() {}
#endif

    //!  call deactivate() if if PL_PROFILE is set
#if defined(PROFILE) && defined(PL_PROFILE)
    static void pl_profile_deactivate();
#else
    static inline void pl_profile_deactivate() {}
#endif

    //!  call report() if if PL_PROFILE is set
#if defined(PROFILE) && defined(PL_PROFILE)
    static void pl_profile_report(ostream& out);
    static void pl_profile_report(PStream out);
#else
    static inline void pl_profile_report(ostream& out) {}
    static inline void pl_profile_report(PStream out) {}
#endif

    //!  call reportwall() if if PL_PROFILE is set
#if defined(PROFILE) && defined(PL_PROFILE)
    static void pl_profile_reportwall(ostream& out);
    static void pl_profile_reportwall(PStream out);
#else
    static inline void pl_profile_reportwall(ostream& out) {}
    static inline void pl_profile_reportwall(PStream out) {}
#endif

    //! Return the number of clock ticks per second on this computer.
#ifdef PROFILE
    static long ticksPerSecond() { return sysconf(_SC_CLK_TCK); }
#else
    static long ticksPerSecond() { return 0; }
#endif


    //! Return the statistics related to a piece of code.  This is useful
    //! for aggregators that collect and report a number of statistics
    static const Stats& getStats(const string& name_of_piece_of_code);

    //! Reset the statistics associated with a piece of code.  The piece of
    //! code may not yet exist, this is fine.
    static void reset(const string& name_of_piece_of_code);
    
    //!  Output a report on the output stream, giving
    //!  the statistics recorded for each of the named pieces of codes.
    static void report(ostream& out);
    static void report(PStream out);

    //!  Output a report on the output stream, giving the
    //!  wall time statistics recorded for each of the named pieces of codes.
    static void reportwall(ostream& out);
    static void reportwall(PStream out);

protected:
    static map<string,Stats> codes_statistics;
    static struct tms t;
    static bool active;
    //return a thread save name. usefull for openmp code.
    static string get_omp_save_name(const string name_of_piece_of_code);
};

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
