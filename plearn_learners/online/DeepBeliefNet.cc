// -*- C++ -*-

// DeepBeliefNet.cc
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file DeepBeliefNet.cc */


#include "DeepBeliefNet.h"
#include "RBMLayer.h"
#include "RBMMixedLayer.h"
#include "RBMMultinomialLayer.h"
#include "RBMParameters.h"
#include "RBMGenericParameters.h"
#include "RBMJointGenericParameters.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DeepBeliefNet,
    "Does the same thing as Hinton's deep belief nets",
    ""
);

//////////////////
// DeepBeliefNet //
//////////////////
DeepBeliefNet::DeepBeliefNet() :
    learning_rate(0.),
    weight_decay(0.)
{
}

////////////////////
// declareOptions //
////////////////////
void DeepBeliefNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "learning_rate", &DeepBeliefNet::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate");

    declareOption(ol, "weight_decay", &DeepBeliefNet::weight_decay,
                  OptionBase::buildoption,
                  "Weight decay");

    declareOption(ol, "training_schedule", &DeepBeliefNet::training_schedule,
                  OptionBase::buildoption,
                  "Number of epochs for training each RBMParameters during\n"
                  "greedy learning phase.\n");

    declareOption(ol, "fine_tuning_method", &DeepBeliefNet::fine_tuning_method,
                  OptionBase::buildoption,
                  "Method for fine-tuning the whole network after greedy"
                  " learning.\n"
                  "One of:\n"
                  "  - \"none\"\n"
                  "  - \"CD\" or \"contrastive_divergence\"\n"
                  "  - \"EGD\" or \"error_gradient_descent\"\n"
                  "  - \"WS\" or \"wake_sleep\".\n");

    declareOption(ol, "layers", &DeepBeliefNet::layers,
                  OptionBase::buildoption,
                  "Layers that learn representations of the input,"
                  " unsupervisedly.\n"
                  "layers[0] is input layer.\n");

    declareOption(ol, "target_layer", &DeepBeliefNet::target_layer,
                  OptionBase::buildoption,
                  "Target (or label) layer");

    declareOption(ol, "params", &DeepBeliefNet::params,
                  OptionBase::buildoption,
                  "RBMParameters linking the unsupervised layers.\n"
                  "params[i] links layers[i] and layers[i+1], except for"
                  "params[n_layers-1],\n"
                  "that links layers[n_layers-1] and last_layer.\n");

    declareOption(ol, "target_params", &DeepBeliefNet::target_params,
                  OptionBase::buildoption,
                  "Parameters linking target_layer and last_layer");

    declareOption(ol, "n_layers", &DeepBeliefNet::n_layers,
                  OptionBase::learntoption,
                  "Number of unsupervised layers, including input layer");

    declareOption(ol, "last_layer", &DeepBeliefNet::last_layer,
                  OptionBase::learntoption,
                  "Last layer, learning joint representations of input and"
                  " target");

    declareOption(ol, "joint_layer", &DeepBeliefNet::joint_layer,
                  OptionBase::learntoption,
                  "Concatenation of target_layer and layers[n_layers-1]");

    declareOption(ol, "joint_params", &DeepBeliefNet::joint_params,
                  OptionBase::learntoption,
                  "Parameters linking joint_layer and last_layer");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void DeepBeliefNet::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void DeepBeliefNet::build_()
{
    n_layers = layers.length();
    if( n_layers <= 1 )
        return;

    if( training_schedule.length() != n_layers )
        training_schedule = TVec<int>( n_layers, 30 );

    // check value of fine_tuning_method
    string ftm = lowerstring( fine_tuning_method );
    if( ftm == "" | ftm == "none" )
        fine_tuning_method = "";
    else if( ftm == "cd" | ftm == "contrastive_divergence" )
        fine_tuning_method = "CD";
    else if( ftm == "egd" | ftm == "error_gradient_descent" )
        fine_tuning_method = "EGD";
    else if( ftm == "ws" | ftm == "wake_sleep" )
        fine_tuning_method = "WS";
    else
        PLERROR( "DeepBeliefNet::build_ - fine_tuning_method \"%s\"\n"
                 "is unknown.\n", fine_tuning_method.c_str() );

    build_layers();
    build_params();
}

void DeepBeliefNet::build_layers()
{
    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->random_gen = random_gen;
    target_layer->random_gen = random_gen;

    last_layer = layers[n_layers-1];

    // concatenate target_layer and layers[n_layers-2] into joint_layer
    TVec< PP<RBMLayer> > the_sub_layers( 2 );
    the_sub_layers[0] = target_layer;
    the_sub_layers[1] = layers[n_layers-2];
    joint_layer = new RBMMixedLayer( the_sub_layers );
    joint_layer->random_gen = random_gen;
}

void DeepBeliefNet::build_params()
{
    if( params.length() == 0 )
    {
        params.resize( n_layers-1 );
        for( int i=0 ; i<n_layers-1 ; i++ )
            params[i] = new RBMGenericParameters();
    }
    else if( params.length() != n_layers-1 )
        PLERROR( "DeepBeliefNet::build_params - params.length() should be"
                 " equal to\n"
                 "layers.length()-1 (%d != %d).\n",
                 params.length(), n_layers-1 );

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        params[i]->down_units_types = layers[i]->units_types;
        params[i]->up_units_types = layers[i+1]->units_types;
        params[i]->learning_rate = learning_rate;
        params[i]->build();
    }

    if( target_layer && !target_params )
    {
        target_params = new RBMGenericParameters();
        target_params->down_units_types = target_layer->units_types;
        target_params->up_units_types = last_layer->units_types;
        target_params->learning_rate = learning_rate;
        target_params->build();
    }

    // build joint_params from params[n_layers-1] and target_params
    joint_params = new RBMJointGenericParameters( target_params,
                                                  params[n_layers-2] );
}

/////////
// cdf //
/////////
real DeepBeliefNet::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for DeepBeliefNet"); return 0;
}

/////////////////
// expectation //
/////////////////
void DeepBeliefNet::expectation(Vec& mu) const
{
    PLERROR("expectation not implemented for DeepBeliefNet");
}

// ### Remove this method if your distribution does not implement it.
////////////
// forget //
////////////
void DeepBeliefNet::forget()
{
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    PLERROR("forget method not implemented for DeepBeliefNet");
}

//////////////
// generate //
//////////////
void DeepBeliefNet::generate(Vec& y) const
{
    PLERROR("generate not implemented for DeepBeliefNet");
}

// ### Default version of inputsize returns learner->inputsize()
// ### If this is not appropriate, you should uncomment this and define
// ### it properly here:
// int DeepBeliefNet::inputsize() const {}

/////////////////
// log_density //
/////////////////
real DeepBeliefNet::log_density(const Vec& y) const
{
    PLERROR("density not implemented for DeepBeliefNet"); return 0;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DeepBeliefNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    deepCopyField(layers, copies);
    deepCopyField(last_layer, copies);
    deepCopyField(target_layer, copies);
    deepCopyField(joint_layer, copies);
    deepCopyField(params, copies);
    deepCopyField(joint_params, copies);
    deepCopyField(target_params, copies);
    deepCopyField(training_schedule, copies);
}

//////////////////
// setPredictor //
//////////////////
void DeepBeliefNet::setPredictor(const Vec& predictor, bool call_parent) const
{
    if (call_parent)
        inherited::setPredictor(predictor, true);
    // ### Add here any specific code required by your subclass.
}

////////////////////////////////
// setPredictorPredictedSizes //
////////////////////////////////
bool DeepBeliefNet::setPredictorPredictedSizes(int the_predictor_size,
                                               int the_predicted_size,
                                               bool call_parent)
{
    if (call_parent)
        inherited::setPredictorPredictedSizes(the_predictor_size,
                                              the_predicted_size, true);
    // ### Add here any specific code required by your subclass.
    return false;
}

/////////////////
// survival_fn //
/////////////////
real DeepBeliefNet::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for DeepBeliefNet"); return 0;
}

// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void DeepBeliefNet::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */

    Vec input;
    Vec target;

}

//////////////
// variance //
//////////////
void DeepBeliefNet::variance(Mat& covar) const
{
    PLERROR("variance not implemented for DeepBeliefNet");
}

} // end of namespace PLearn


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
