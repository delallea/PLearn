// -*- C++ -*-

// HyperLearner.cc 
// Copyright (C) 2003-2004 ApSTAT Technologies Inc.
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

// Author: Pascal Vincent

/* *******************************************************      
   * $Id: HyperLearner.cc,v 1.2 2005/01/12 14:43:14 tihocan Exp $
   ******************************************************* */
// Author: Pascal Vincent

#include "HyperLearner.h"
#include <plearn/base/stringutils.h>
#include <plearn/io/load_and_save.h>
#include <plearn/vmat/FileVMatrix.h>

namespace PLearn {


// ###### HyperLearner ########################################################

PLEARN_IMPLEMENT_OBJECT(HyperLearner, "ONE LINE DESCR", "NO HELP");


TVec<string> HyperLearner::getTrainCostNames() const
{
  if (strategy.size() > 0)
    return strategy.lastElement()->getResultNames();
  else
    return TVec<string>();
}

HyperLearner::HyperLearner()
  : provide_strategy_expdir(true), save_final_learner(true)
{
}

void
HyperLearner::declareOptions(OptionList &ol)
{
    declareOption(ol, "tester", &HyperLearner::tester, OptionBase::buildoption,
                  "A model for the kind of train/test to be performed for each \n"
                  "combination of hyper-parameters considered. \n"
                  "The tester's options are used as follows: \n"
                  "  - You don't need to provide tester.learner, it will be set from \n"
                  "    the HyperLearner's learner.\n"
                  "  - tester.statnames is the list of cost stats to be computed, \n"
                  "    reported in the results table, and used as criteria for the hyper-optimization \n"
                  "  - tester.splitter is the default splitter to use for validation (although this may be \n"
                  "    overridden locally in HyperOptimize, see help on HyperOptimize) \n"
                  "  - Similarly, no need to provide tester.learner tester.expdir, \n"
                  "    and tester.provide_learner_expdir. \n"
                  "    They will be set by HyperOptimize as it deems appropriate\n"
                  "    for each hyper-parameter combination it tries. \n"
                  "  - *If* HyperOptimize does set a non-empty expdir, then the kind of\n"
                  "    report and result files to be generated is taken from the remaining \n"
                  "    report_stats and save_... options of the tester \n");

    declareOption(ol, "option_fields", &HyperLearner::option_fields, OptionBase::buildoption,
                  "learner option names to be reported in results table");

    declareOption(ol, "dont_restart_upon_change", &HyperLearner::dont_restart_upon_change, OptionBase::buildoption,
                  "a list of names of options, used in the strategy, but that do not require \n"
                  "calling build() and forget() each time they are modified. \n"
                  "Note that this is almost always the case of the 'nstages' option \n"
                  "typically modified incrementally in an 'early stopping' fashion... \n");

    declareOption(ol, "strategy", &HyperLearner::strategy, OptionBase::buildoption,
                  "The strategy to follow to optimize the hyper-parameters.\n"
                  "It's a list of hyper-optimization commands to call sequentially,"
                  "(mostly HyperOptimize and SetOptions commands).");

    declareOption(ol, "provide_strategy_expdir", &HyperLearner::provide_strategy_expdir, OptionBase::buildoption,
                  "should each strategy step be provided a directory expdir/Step# to report its results");

    declareOption(ol, "save_final_learner", &HyperLearner::save_final_learner, OptionBase::buildoption,
                  "should final learner be saved in expdir/final_learner.psave");

    inherited::declareOptions(ol);
}

void HyperLearner::setLearnerOptions(const TVec<string>& option_names,
                                     const TVec<string>& option_vals)
{
  // This function has been modified to call changeOptions on the learner
  // rather than a sequence of setOptions.  Learner options are accumulated
  // into a map suitable for calling changeOptions.  TESTER options are
  // still handled as a set of discrete setOptions.

  map<string,string> new_learner_options;
  int l = option_names.length();
  bool do_restart = false;
  for(int i=0; i<l; i++) {
    string optname = option_names[i];
    string newval = option_vals[i];

    // Decide if we should perform build after setting the options
    do_restart = do_restart ||
      ! dont_restart_upon_change.contains(optname);

    // Here, we incorporate the capability to change an option in the
    // tester (rather than the embedded learner) by prefixing the option
    // name with the special keyword TESTER.
    if (optname.substr(0,7) == "TESTER.") {
      // This is the old control flow for banging tester only
      optname = optname.substr(7);
      if (tester->getOption(optname) != newval)
        tester->setOption(optname, newval);
    }
    else {
      // New control flow: accumulate changes into the map
      if (learner_->getOption(optname) != newval)
        new_learner_options[optname] = newval;
    }
  }
  learner_->changeOptions(new_learner_options);
  if(do_restart)
    {
      learner_->build();
      learner_->forget();
    }
}

void HyperLearner::setTrainingSet(VMat training_set, bool call_forget)
{ 
  train_set = training_set;
  if (call_forget)
  {
    build();
    forget();
  }
  // no need to forget a second time
  learner_->setTrainingSet(training_set,false); // just to make sure the underlying learner knows the targetsize
}

void HyperLearner::train()
{
  tester->dataset = getTrainingSet();  
  Vec results;

  if (!train_stats)
    PLERROR("HyperLearner::train train_stats was not set");

  if(stage==0 && nstages>0)
    {
      for(int commandnum=0; commandnum<strategy.length(); commandnum++)
        {
          if(expdir!="" && provide_strategy_expdir)
            strategy[commandnum]->setExperimentDirectory(expdir+"Strat"+tostring(commandnum));
          results = strategy[commandnum]->optimize();
        }

      train_stats->update(results);

      if(save_final_learner)
        {
          if(expdir=="")
            PLERROR("Cannot save final model: no experiment directory has been set");
          PLearn::save(expdir+"final_learner.psave",*getLearner());
        }

      stage = 1;
    }
}

void HyperLearner::forget()
{
  learner_->forget();
  stage = 0;
}

void HyperLearner::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.

  // Set the Tester's learner to point to the same as the HyperLearner's learner
  tester->learner = learner_;

  for(int commandnum=0; commandnum<strategy.length(); commandnum++)
    strategy[commandnum]->setHyperLearner(this);
}

// ### Nothing to add here, simply calls build_
void HyperLearner::build()
{
  inherited::build();
  build_();
}


void HyperLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  
  deepCopyField(tester, copies);
  deepCopyField(option_fields, copies);
  deepCopyField(dont_restart_upon_change, copies);
  deepCopyField(strategy, copies);
}

} // end of namespace PLearn
