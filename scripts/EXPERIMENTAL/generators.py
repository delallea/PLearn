#generators2.py : re-implantation of generators1.py with
#                 numpy array module instead numarray

#generation procedures with 'TransformationLearner' distribution
import os, sys, time, matplotlib, math, numpy ,copy

from numpy import *
from math import *
from random import *

from plearn.io.server import *
from plearn.pyplearn import *
from copy import *

UNDEFINED = -1
BEHAVIOR_GENERATOR = 1
LINEAR = 0
LINEAR_INCREMENT = 1
EMPTY_BIAS = TMat()

#------------------- TREE GENERATOR --------------------------------------

class TreeGenerator(object):

    #DESCRIPTION:

    #generates a data set :
    #     equivalent in building a tree 
    #
    #            0      1        2     ...         
    #  
    #            r -> child1  -> child1  ...       
    #                         -> child2  ...
    #                             ...    ...
    #                         -> childn  ...
    #
    #              -> child2  -> child1  ...
    #                         -> child2  ...
    #                              ...   ...
    #                         -> childn  ...
    #                      ...
    #              -> childn  -> child1  ...
    #                         -> child2  ...
    #                              ...   ...
    #                         -> childn  ... 
    #
    # The child are generated by the same following process:
    #  1) choose a transformation  
    #  2) apply the transformation to the parent
    #  3) add noise to the result 
    


    #PARAMETERS OF THE  GENERATOR

    #parameters of the transformation learner used in the generation process:
    tl=UNDEFINED                   #c++ transformation learner object
    dim=2                          #dimension of input space
    nbTransforms = UNDEFINED       #number of transformations
    transformFamily=UNDEFINED      #transformation function family
    transforms = []                #transformation matrices
    withBias = False               #add a bias to the transformation function ?
    biasSet = UNDEFINED            #transformation bias (if any)
    noiseVariance = UNDEFINED      #noise variance 
    transformDistribution = []     #transformation distribution (in log form) 

    #shape of the tree data set to generate
    root = []
    deepness = 1
    branchingFactor = 1



    #CONSTRUCTOR
    
    def __init__(self,
                 tl,
                 dim=2,
                 builded=False,
                 deepness=1,
                 branchingFactor=1,
                 root=[]):
        #c++ transformation learner object
        self.tl = tl
        #number of transformations
        self.nbTransforms = self.tl.getOption("nbTransforms")
        #transformation function family
        self.transformFamily = self.tl.getOption("transformFamily")
        #add a bias to the transformation function ? 
        self.withBias = self.tl.getOption("withBias")
        #if the generator is not builded, some parameters might be undefined.
        #We call the default generator building procedure
        #(use default initialization procedures when initial values are needed)
        if(not builded):
            self.dim = dim
            self.tl.generatorBuild(self.dim,
                                   [],
                                   EMPTY_BIAS,
                                   UNDEFINED,
                                   [])
        else:
            self.dim = self.tl.getOption("trainingSetLength")
            assert(self.dim == dim)
        #transformation matrices 
        self.transforms=self.tl.getOption("transforms")
        #transformation bias (if any)
        if(self.withBias):
            self.biasSet= self.tl.getOption("biasSet")
        else:
            self.biasSet = EMPTY_BIAS
        #noise variance
        self.noiseVariance = self.tl.getOption("noiseVariance")
        #transformation distribution (in log form)
        self.transformDistribution  = self.tl.getOption("transformDistribution")
        #generation process default parameters
        if(len(root) == 0):
            self.root = array(zeros(self.dim), 'd')
            for i in range(self.dim):
                self.root[i]=uniform(0,10)
        else:
            self.setRoot(root)
        assert(deepness >=1)
        self.deepness = deepness
        assert(branchingFactor >= 1)
        self.branchingFactor = branchingFactor


    #changes the default root of the tree     
    def setRoot(self,root):
        assert(len(root) == self.dim)
        self.root = copy(root)
            

    #builds the TransformationLearner object (our generator) 
    #that is, call method 'TransformationLearner::generatorBuild'      
    def build(self,
              transforms=[],
              biasSet=EMPTY_BIAS,
              noiseVariance=UNDEFINED,
              transformDistribution=[]):
        #verifications fot the transformation matrices and bias
        if(len(transforms)>0):
            assert(self.nbTransforms == len(transforms))
            for i in range(self.nbTransforms):
                assert(transforms[i].shape[0] == self.dim)
                assert(transforms[i].shape[1] == self.dim)
            if(self.withBias):
                assert(biasSet.shape[0]==self.nbTransforms)
                assert(baisSet.shape[1]==self.dim)
        else:
            biasSet=EMPTY_BIAS
        #verifications for the transformation distribution
        if(len(transformDistribution)>0):
            assert(len(transformDistribution)==self.nbTransforms)
            sum=0
            w=0
            for i in range(self.nbTransforms):
                w = exp(transformDistribution[i])
                assert(0<=w<=1)
                sum += w
            assert(sum == 1)
        #we are ready to call the building method:
        self.tl.generatorBuild(self.dim,
                               transforms,
                               biasSet,
                               noiseVariance,
                               transformDistribution)
        #capture new parameters of the Transformation Learner
        self.transforms = self.tl.getOption("transforms")
        if(self.withBias):
            self.biasSet = self.tl.getOption("biasSet")
        else:
            self.biasSet= EMPTY_BIAS
        self.noiseVariance = self.tl.getOption("noiseVariance")
        self.transformDistribution = self.tl.getOption("transformDistribution")



    #sets the transformations matrices (and bias if any) of the TransformationLearner
    def setTransforms(self,
                      transforms,
                      biasSet=EMPTY_BIAS):
        assert(self.nbTransforms==len(transforms))
        for i in range(self.nbTransforms):
            assert(self.dim == transforms[i].shape[0])
            assert(self.dim == transforms[i].shape[1])
        self.transforms = copy(transforms)
        if(self.withBias):
            assert(biasSet.shape[0]==self.nbTransforms)
            assert(biasSet.shape[1]==self.dim)
            self.biasSet = biasSet.copy()
        else:
            biasSet = EMPTY_BIAS
        self.tl.setTransformsParameters(transforms,biasSet)
             

    #sets the noise variance of the TransformationLearner
    def setNoiseVariance(self,
                         noiseVariance):
        assert(noiseVariance > 0)
        self.noiseVariance = noiseVariance
        self.tl.setNoiseVariance(noiseVariance)
        

    #sets the transformation distribution of the TransformationLearner
    def setTransformDistribution(self,
                                 transformDistribution):
        assert(self.nbTransforms == len(transformDistribution))
        sum = 0
        for i in range(self.nbTransforms):
            p = exp(transformDistribution[i])
            assert(0<=p and p<=1)
            sum += p
        assert(sum == 1)
        self.transformDistribution  = copy(transformDistribution)
        self.tl.setTransformDistribution(transformDistribution)


    #computes the length of a generation tree 
    def computeTreeLength(self,
                          deepness,
                          branchingFactor):
        if(branchingFactor == 1):
            return deepness + 1
        else:
            return ((1.0 - pow(branchingFactor,deepness + 1))/(1.0 - b))


    #changes the default generation process parameters
    def setDefaultTreeParameters(self,
                                 deepness,
                                 branchingFactor,
                                 root):
        self.setRoot(root)
        assert(deepness >=1)
        self.deepness = deepness
        assert(branchingFactor >= 1)
        self.branchingFactor = branchingFactor
    
    #returns a rotation matrix 
    #(theta here must be given in Rad units)
    def get2DRotationMatrix(self,theta):
        M = array([cos(theta) ,-sin(theta),sin(theta) ,cos(theta)], 'd')
        M.resize(2,2)
        if(self.transformFamily == LINEAR_INCREMENT):
                M[0][0] -= 1;
                M[1][1] -= 1;
        return M

    
    #generates a data set and stores it in a file 
    def generateAndStoreDataSet(self,
                                filename,
                                deepness=UNDEFINED,
                                branchingFactor=UNDEFINED,
                                root =[]):
        dataSet = self.newDataSet( False, deepness,branchingFactor,root)
        self.writeDataSet(dataSet, filename)
    


    
    #stores a data set in a file 
    def writeDataSet(self,dataSet,filename):
        out = open(filename, 'w')
        out.write(str(dataSet.shape[1]) + " 0 0\n")
        out.write(str(dataSet))
        out.close()


    #creates a new data set
    #(use default generation parameters if the given generation parameters are not well defined
    # or unspecified)
    def newDataSet(self,
                   returnTransforms=False,
                   deepness=UNDEFINED,
                   branchingFactor=UNDEFINED,
                   root=[],
                   transformIndex = UNDEFINED):
        if(len(root)==0):
            root= copy(self.root)
        else:
            assert(len(root)==self.dim)
        if(deepness<1):
            deepness=self.deepness
        if(branchingFactor<1):
            branchingFactor=self.branchingFactor
        dataSet = self.tl.returnTreeDataSet(root,
                                            deepness,
                                            branchingFactor,
                                            transformIndex)
        if(returnTransforms):
            return (dataSet, self.transforms, self.biasSet)
        else:
            return dataSet 


# ----------------- SEQUENTIAL GENERATOR -----------------------------------------

class SequentialGenerator(TreeGenerator):

    nbDataPoints = 40


    def __init__(self,
                 tl,
                 dim=2,
                 builded=False,
                 nbDataPoints=40,
                 root=[]):
        assert(nbDataPoints >=2)
        self.nbDataPoints = nbDataPoints
        TreeGenerator.__init__(self,
                               tl,
                               dim,
                               builded,
                               nbDataPoints - 1,
                               1,
                               root)


    def setTreeParameters(self,
                          deepness,
                          branchingFactor,
                          root):
        assert(branchingFactor == 1)
        TreeGenerator.setTreeParameters(self,
                                        deepness,
                                        branchingFactor,
                                        root)
        self.nbDataPoints=self.computeTreeLength(deepness,
                                                 branchingFactor)


    def setSequenceParameters(self,
                              nbDataPoints,
                              root):
        self.setTreeParameters(nbDataPoints - 1,1,root)


    def newDataSet(self,
                   returnTransforms=False,
                   nbDataPoints=UNDEFINED,
                   root=[],
                   transformIndex =UNDEFINED):
        if(nbDataPoints<2):
            nbDataPoints = self.nbDataPoints
        return TreeGenerator.newDataSet(self,
                                        returnTransforms,
                                        nbDataPoints - 1,
                                        1,
                                        root,
                                        transformIndex)


    
#-------------CIRCLE GENERATOR ----------------------------------
class CircleGenerator(SequentialGenerator):

    ray = 10
    center =array([0,0],'d')

    def __init__(self,
                 tl,
                 builded=False,
                 nbDataPoints=40,
                 center=array([0,0],'d'),
                 ray=10):
        assert(tl.getOption("nbTransforms") == 1)
        assert(ray>0)
        self.ray = ray
        assert(len(center)==2)
        root = copy(center)
        root[1] += ray
        SequentialGenerator.__init__(self,
                                     tl,
                                     2,
                                     builded,
                                     nbDataPoints,
                                     root)
        self.setTransforms(nbDataPoints)


    def setTreeParameters(self,
                          deepness,
                          branchingFactor,
                          root):
        SequentialGenerator.settreeParameters(self,
                                              deepness,
                                              branchingFactor,
                                              root)
        self.setTransforms(self.nbDataPoints) 



    def build(self,
              nbDataPoints,
              noiseVariance=UNDEFINED,
              transformDistribution=[]):
        assert(nbDataPoints>=2)
        self.nbDataPoints = nbDataPoints
        theta=2*pi/nbDataPoints
        M = self.get2DRotationMatrix(theta)
        transforms = [M]
        biasSet = EMPTY_BIAS
        if(self.withBias):
            biasSet = array([0,0],'d')
            biasSet.resize(1,2)
        SequentialGenerator.build(self,
                                  transforms,
                                  biasSet,
                                  noiseVariance,
                                  transformDistribution)

    def setRoot(self,
                root):
        SequentialGenerator.setRoot(self,
                                    root)
        self.center = copy(root)
        self.center[1] -= self.ray


    def setCircleParameters(self,
                            center,
                            ray,
                            nbDataPoints):
        assert(len(center)==2)
        assert(ray>0)
        assert(nbDataPoints >=2)
        self.center = copy(center)        #center
        self.ray = ray                    #ray
        self.root = copy(center)          #root
        self.root[1] += ray
        self.setTransforms(nbDataPoints)
    

    def setTransforms(self,
                      nbDataPoints):
        assert(nbDataPoints >=2)
        self.nbDataPoints = nbDataPoints
        self.defaultDeepness = nbDataPoints - 1
        theta = 2*pi/nbDataPoints
        transforms = [self.get2DRotationMatrix(theta)]
        biasSet = EMPTY_BIAS
        if(self.withBias):
            biasSet = array([0,0],'d')
            biasSet.resize(1,2)
        SequentialGenerator.setTransforms(self,
                                          transforms,
                                          biasSet)
        

    def newDataSet(self,
                   returnTransforms=False,
                   center=[],
                   ray=UNDEFINED,
                   nbDataPoints=UNDEFINED):
        if(len(center) != 2):
            center = copy(self.center)
        if(ray<=0):
            ray = self.ray
        if(nbDataPoints < 2):
            nbDataPoints = self.nbDataPoints
        self.setCircleParameters(center,
                                 ray,
                                 nbDataPoints)
        return SequentialGenerator.newDataSet(self,
                                              returnTransforms)

    
# ------------- Spiral Generator ---------------------------------------

class SpiralGenerator(SequentialGenerator):

    alpha = 1.01
    theta = 0.1


    def __init__(self,
                 tl,
                 builded=False,
                 nbDataPoints=40,
                 root=[],
                 alpha = 1.01,
                 theta = 0.1
                 ):

        assert(tl.getOption("nbTransforms") ==1)
        assert(alpha > 0)
        assert(theta > 0)
        self.alpha = alpha
        self.theta = theta
        SequentialGenerator.__init__(self,
                                     tl,
                                     2,
                                     builded,
                                     nbDataPoints,
                                     root)
        self.setTransforms(alpha,theta)


    def setTransforms(self,
                      alpha,
                      theta):
        assert(alpha > 0)
        assert(theta > 0)
        self.alpha = alpha
        self.theta = theta
        transforms = self.get2DRotationMatrix(theta)
        transforms = alpha*transforms
        biasSet=EMPTY_BIAS
        if(self.withBias):
            biasSet = array([0,0], 'd')
            biasSet.resize(1,2)
        SequentialGenerator.setTransforms(self,[transforms],biasSet) 


    def setSpiralParameters(self,
                            nbDataPoints,
                            root,
                            alpha,
                            theta):
        SequentialGenerator.setDefaultSequenceParameters(self,
                                                         nbDataPoints,
                                                         root)
        self.setTransforms(alpha,theta)


    def build(self,
              alpha,
              theta,
              noiseVariance=UNDEFINED,
              transformDistribution=[]):
        self.setTransforms(alpha,theta)
        SequentialGenerator.build(self,
                                  self.transforms,
                                  self.biasSet,
                                  noiseVariance,
                                  transformDistribution)


    def newDataSet(self,
                   returnTransforms=False,
                   nbDataPoints=UNDEFINED,
                   root=[],
                   alpha=UNDEFINED,
                   theta=UNDEFINED):
        if(alpha <= 0):
            alpha = self.alpha
        if(theta <= 0):
            theta = self.theta
        if(nbDataPoints < 2):
            nbDataPoints = self.nbDataPoints
        if(len(root) != self.dim):
            root = self.root
        self.setSpiralParameters(self,nbDataPoints,root,alpha,theta)        
        return SequentialGenerator.newDataSet(self,
                                              returnTransforms,
                                              nbDataPoints,
                                              root) 

#--------------- LINE GENERATOR ------------------------------------
class LineGenerator(SequentialGenerator):
    

    
    def __init__(self,
                 tl,
                 dim=2,
                 builded=False,
                 nbDataPoints=40,
                 increment=[]
                 ):
        assert(tl.getOption("nbTransforms") == 1)
        assert(tl.getOption("withBias"))
        t = array(zeros(dim*dim),'d')
        t.resize(dim,dim)
        transforms = [t]
        self.transforms = transforms
        biasSet = array(increment, 'd')
        biasSet.resize(1,dim)
        SequentialGenerator__init__(self,
                                    tl,
                                    dim,
                                    builded,
                                    transforms,
                                    biasSet)

    def setTransforms(self,
                      increment):
        t = array(zeros(self.dim*self.dim),d)
        t.resize(self.dim,self.dim)
        transforms = [t]
        biasSet = array(increment, 'd')
        biasSet.resize(1,self.dim)
        SequentialGenerator.setTransforms(self,
                                          transforms,
                                          biasSet)

    def build(self,
              increment,
              noiseVariance=UNDEFINED,
              transformDistribution=[]):
        assert(len(increment)==self.dim)
        biasSet = array(increment, 'd')
        biasSet.resize(1,self.dim)
        SequentialGenerator.build(self,
                                  [zeros(self.dim,self.dim)],
                                  biasSet,
                                  noiseVariance,
                                  transformDistribution)
        
        
    def newDataSet(self,
                   returnDataSet,
                   root=[],
                   nbDataPoints=UNDEFINED,
                   increment=array([])):
        if(len(increment)!= self.dim):
            increment = copy(self.biasSet[0,:])
        self.setTransforms(increment)
        return SequentialGenerator.newDataSet(self,
                                              returnDataSet,
                                              root,
                                              nbDataPoints) 
