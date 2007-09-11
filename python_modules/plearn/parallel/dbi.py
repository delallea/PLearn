#! /usr/bin/env python
import sys
import os
import getopt
import re
import string
import time
import traceback
import shutil
from subprocess import Popen,PIPE,STDOUT
from utils import *
from configobj import ConfigObj
from textwrap import dedent
import pdb
from threading import Thread,Lock
from time import sleep
import datetime

STATUS_FINISHED = 0
STATUS_RUNNING = 1
STATUS_WAITING = 2
STATUS_ERROR = 3

#original version from: http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/196618
class LockedIterator:
    def __init__( self, iterator ):
        self._lock     = Lock()
        self._iterator = iterator
        
    def __iter__( self ):
        return self
    
    def next( self ):
        try:
            self._lock.acquire()
            return self._iterator.next()
        finally:
            self._lock.release()
            
#original version from: http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/196618
class MultiThread:
    def __init__( self, function, argsVector, maxThreads=5, print_when_finished=None):
        self._function     = function
        self._argsIterator = LockedIterator( iter( argsVector ) )
        self._threadPool   = []
        self.print_when_finish = print_when_finished
        self.running = 0
        nb_thread=maxThreads
        if nb_thread>len(argsVector):
            nb_thread=len(argsVector)
        for i in range( nb_thread ):
            self._threadPool.append( Thread( target=self._tailRecurse ) )
            
    def _tailRecurse( self ):
        for args in self._argsIterator:
            self._function( args )
        self.running-=1
        if self.print_when_finish:
            print self.print_when_finish,"left running:",self.running
            
    def start( self  ):
        for thread in self._threadPool:
            time.sleep( 0 ) # necessary to give other threads a chance to run
            self.running+=1
            thread.start()
            
    def join( self, timeout=None ):
        for thread in self._threadPool:
            thread.join( timeout )
                    
class DBIBase:

    def __init__(self, commands, **args ):
        #generate a new unique id
        self.unique_id = get_new_sid('')

        # option is not used yet
        self.has_short_duration = 0

        # if all machines are full, run the jobs one by one on the localhost
        self_use_localhost_if_full = 1

        # the( human readable) time format used in log file
        self.time_format = "%Y-%m-%d/%H:%M:%S"

        # Commands to be executed once before the entire batch on the submit node
        self.pre_batch = []
        # Commands to be executed before every task in tasks
        self.pre_tasks = []
        # The main tasks to be dispatched
        self.tasks = []
        # Commands to be executed after each task in tasks
        self.post_tasks = []
        # Commands to be executed once after the entire batch on the submit node
        self.post_batch = []

        # the default directory where to keep all the log files
        self.log_dir = os.path.join( 'LOGS', self.unique_id )
        self.log_file = os.path.join( self.log_dir, 'log' )

        # the default directory where file generated by dbi will be stored
        # It should not take the "" or " " value. Use "." instead.
        self.tmp_dir = 'TMP_DBI'
        #
        self.file_redirect_stdout = True
        self.file_redirect_stderr = True
        self.redirect_stderr_to_stdout = False

        # Initialize the namespace
        self.requirements = ''
        self.test = False
        self.dolog = False
        self.temp_files = []
        self.arch = 0 # TODO, we should put the local arch: 32,64 or 3264 bits
        for key in args.keys():
            self.__dict__[key] = args[key]

        # check if log directory exists, if not create it
        if (not os.path.exists('LOGS')):
            os.mkdir('LOGS')
        if (not os.path.exists(self.log_dir)):
#            if self.dolog or self.file_redirect_stdout or self.file_redirect_stderr:
            os.mkdir(self.log_dir)

        # If some arguments aren't lists, put them in a list
        if not isinstance(commands, list):
            commands = [commands]
        if not isinstance(self.pre_batch, list):
            self.pre_batch = [self.pre_batch]
        if not isinstance(self.pre_tasks, list):
            self.pre_tasks = [self.pre_tasks]
        if not isinstance(self.post_tasks, list):
            self.post_tasks = [self.post_tasks]
        if not isinstance(self.post_batch, list):
            self.post_batch = [self.post_batch]

    def n_avail_machines(self): raise NotImplementedError, "DBIBase.n_avail_machines()"
    
    def add_commands(self,commands): raise NotImplementedError, "DBIBase.add_commands()"
    
    def get_redirection(self,stdout_file,stderr_file):
        """Calcule the needed redirection based of the objects attribute
        return a tuple (stdout,stderr) that can be used with popen
        """
        output = PIPE
        error = PIPE
        if int(self.file_redirect_stdout):
            output = open(stdout_file, 'w')
        if self.redirect_stderr_to_stdout:
            error = STDOUT
        elif int(self.file_redirect_stderr):
            error = open(stderr_file, 'w')
        return (output,error)
            
    def exec_pre_batch(self):
        # Execute pre-batch
        pre_batch_command = ';'.join( self.pre_batch )

        (output,error)=self.get_redirection(self.log_file + '.out',self.log_file + '.err')
            
        if not self.test:
            self.pre = Popen(pre_batch_command, shell=True, stdout=output, stderr=error)
        else:
            print "[DBI] pre_batch_command:",pre_batch_command
            
    def exec_post_batch(self):
        # Execute post-batch
        post_batch_command = ';'.join( self.post_batch )
        
        (output,error)=self.get_redirection(self.log_file + '.out',self.log_file + '.err')
        
        if not self.test:
            self.post = Popen(post_batch_command, shell=True, stdout=output, stderr=error)
        else:
            print "[DBI] post_batch_command:",post_batch_command
            
    def clean(self):
        pass

    def run(self):
        pass

    def wait(self):
        print "[DBI] WARNING the wait function was not overrided by the sub class!"
        
class Task:

    def __init__(self, command, tmp_dir, log_dir, time_format, pre_tasks=[], post_tasks=[], dolog = True, gen_unique_id = True, args = {}):
        self.add_unique_id = 0
        # The "python utils.py..." command is not exactly the same for every
        # task in a batch, so it cannot be considered a "pre-command", and
        # has to be actually part of the command.  Since the user-provided
        # pre-command has to be executed afterwards, it also has to be part of
        # the command itself. Therefore, no need for pre- and post-commands in
        # the Task class

        utils_file = os.path.join(tmp_dir, 'utils.py')
        utils_file = os.path.abspath(utils_file)

        for key in args.keys():
            self.__dict__[key] = args[key]
        self.dolog = dolog
        
        formatted_command = re.sub( '[^a-zA-Z0-9]', '_', command );
        if gen_unique_id:
            self.unique_id = get_new_sid('')#compation intense
            self.log_file = truncate( os.path.join(log_dir, self.unique_id +'_'+ formatted_command), 200) + ".log"
        else:
            self.unique_id = formatted_command+'_'+str(datetime.datetime.now()).replace(' ','_')
            self.log_file = os.path.join(log_dir, self.unique_id) + ".log"

        if self.add_unique_id:
                command = command + ' unique_id=' + self.unique_id
        #self.before_commands = []
        #self.user_defined_before_commands = []
        #self.user_defined_after_commands = []
        #self.after_commands = []

        self.commands = []
        if len(pre_tasks) > 0:
            self.commands.extend( pre_tasks )

        if self.dolog == True:
            self.commands.append(utils_file + ' set_config_value '+
                string.join([self.log_file,'STATUS',str(STATUS_RUNNING)],' '))
            # set the current date in the field LAUNCH_TIME
            self.commands.append(utils_file +  ' set_current_date '+
                string.join([self.log_file,'LAUNCH_TIME',time_format],' '))


        self.commands.append( command )
        self.commands.extend( post_tasks )
        if self.dolog == True:
            self.commands.append(utils_file + ' set_config_value '+
                string.join([self.log_file,'STATUS',str(STATUS_FINISHED)],' '))
            # set the current date in the field FINISHED_TIME
            self.commands.append(utils_file + ' set_current_date ' +
                string.join([self.log_file,'FINISHED_TIME',time_format],' '))

        #print "self.commands =", self.commands

    def get_status(self):
        #TODO: catch exception if value not available
        status = get_config_value(self.log_file,'STATUS')
        return int(status)

    def get_stdout(self):
        try:
            if isinstance(self.p.stdout, file):
                return self.p.stdout
            else:
                return open(self.log_file + '.out','r')
        except:
            pass
        return None
        
    def get_stderr(self):
        try:
            if isinstance(self.p.stderr, file):
                return self.p.stderr
            else:
                return open(self.log_file + '.err','r')
        except:
            pass
        return None

    def set_scheduled_time(self):
        if self.dolog:
            set_config_value(self.log_file, 'STATUS',str(STATUS_WAITING))
            set_config_value(self.log_file, 'SCHEDULED_TIME',
                             time.strftime(self.time_format, time.localtime(time.time())))
            
    def get_waiting_time(self):
        # get the string representation
        str_sched = get_config_value(self.log_file,'SCHEDULED_TIME')
        # transform in seconds from the start of epoch
        sched_time = time.mktime(time.strptime(str_sched,ClusterLauncher.time_format))

        # get the string representation
        str_launch = get_config_value(self.log_file,'LAUNCH_TIME')
        # transform in seconds from the start of epoch
        launch_time = time.mktime(time.strptime(str_launch,ClusterLauncher.time_format))

        return launch_time - sched_time

    def get_running_time(self):
        #TODO: handle if job did not finish
        # get the string representation
        str_launch = get_config_value(self.log_file,'LAUNCH_TIME')
        # transform in seconds from the start of epoch
        launch_time = time.mktime(time.strptime(str_launch,ClusterLauncher.time_format))

        # get the string representation
        str_finished = get_config_value(self.log_file,'FINISHED_TIME')
        # transform in seconds from the start of epoch
        finished_time = time.mktime(time.strptime(str_finished,ClusterLauncher.time_format))

        return finished_time - launch_time

class DBICluster(DBIBase):

    def __init__(self, commands, **args ):
        self.duree=None
        self.arch=None
        self.cluster_wait=None
        self.threads=[]
        self.started=0
        self.nb_proc=50
        self.mt=None
        DBIBase.__init__(self, commands, **args)
        self.add_commands(commands)
        self.nb_proc=int(self.nb_proc)

    def add_commands(self,commands):
        if not isinstance(commands, list):
            commands=[commands]

        # create the information about the tasks
        for command in commands:
            self.tasks.append(Task(command, self.tmp_dir, self.log_dir,
                                   self.time_format,self.pre_tasks,
                                   self.post_tasks,self.dolog,False))


    def run_one_job(self, task):
        DBIBase.run(self)
        
        command = "cluster" 
        if self.arch == "32":
            command += " --typecpu 32bits"
        elif self.arch == "64":
            command += " --typecpu 64bits"
        elif self.arch == "3264":
            command += " --typecpu all"
        if self.duree:
            command += " --duree "+self.duree
        if self.cluster_wait:
            command += " --wait"
        command += " --execute '"+string.join(task.commands,';') + "'"
        self.started+=1
        started=self.started# not thread safe!!!
        print "[DBI,%d/%d,%s] %s"%(started,len(self.tasks),time.ctime(),command)
        if self.test:
            return

        task.launch_time = time.time()
        task.set_scheduled_time()

        (output,error)=self.get_redirection(task.log_file + '.out',task.log_file + '.err')
        task.p = Popen(command, shell=True,stdout=output,stderr=error)
        ret=task.p.wait()
        if task.p.returncode!=0:
            print "[DBI,%d/%d,%s] Failed to launch: '%s' returned %d,%d"%(started,len(self.tasks),time.ctime(),command,task.p.returncode,ret)
            
    def run(self):
        print "[DBI] The Log file are under %s"%self.log_dir
        if self.test:
            print "[DBI] Test mode, we only print the command to be executed, we don't execute them"
        # Execute pre-batch
        if len(self.pre_batch)>0:
            exec_pre_batch()

        # Execute all Tasks (including pre_tasks and post_tasks if any)
        self.mt=MultiThread(self.run_one_job,self.tasks,self.nb_proc,"[DBI,%s]"%time.ctime())
        self.mt.start()

        # Execute post-batchs
        if len(self.post_batch)>0:
            exec_post_batch()

        print "[DBI] The Log file are under %s"%self.log_dir

    def clean(self):
        #TODO: delete all log files for the current batch
        pass

    def wait(self):
        if self.mt:
            self.mt.join()
        else:
            print "[DBI] WARNING jobs not started!"
                
class DBIbqtools(DBIBase):

    def __init__( self, commands, **args ):
        self.nb_proc = 1
        self.clean_up = True
        self.micro = 1
        self.queue = "qwork@ms"
        self.long = False
        self.duree = "12:00:00"

        DBIBase.__init__(self, commands, **args)

        self.nb_proc = int(self.nb_proc)
        self.micro = int(self.micro)
        
### We can't accept the symbols "," as this cause trouble with bqtools
        if self.log_dir.find(',')!=-1 or self.log_file.find(',')!=-1:
            print "[DBI] ERROR: The log file and the log dir should not have the symbol ','"
            print "[DBI] log file=",self.log_file
            print "[DBI] log dir=",self.log_dir
            sys.exit(1)

        # create directory in which all the temp files will be created
        self.temp_dir = 'bqtools_tmp_' + os.path.split(self.log_dir)[1]
        print "[DBI] All bqtools file will be in ",self.temp_dir
        os.mkdir(self.temp_dir)
        os.chdir(self.temp_dir)
        
        if self.long:
            self.queue = "qlong@ms"
            # Get max job duration from environment variable if it is set.
            max = os.getenv("BQ_MAX_JOB_DURATION")
            if max:
                self.duree = max
            else:
                self.duree = "1200:00:00" #50 days

        # create the information about the tasks
        args['temp_dir'] = self.temp_dir
        self.args=args
        self.add_commands(commands)

    def add_commands(self,commands):
        if not isinstance(commands, list):
            commands=[commands]

        # create the information about the tasks
        for command in commands:
            self.tasks.append(Task(command, self.tmp_dir, self.log_dir,
                                   self.time_format,self.pre_tasks,
                                   self.post_tasks,self.dolog,False,self.args))
    def run(self):
        pre_batch_command = ';'.join( self.pre_batch );
        post_batch_command = ';'.join( self.post_batch );

        # create one (sh) script that will launch the appropriate ~~command~~
        # in the right environment


        launcher = open( 'launcher', 'w' )
        bq_cluster_home = os.getenv( 'BQ_CLUSTER_HOME', '$HOME' )
        bq_shell_cmd = os.getenv( 'BQ_SHELL_CMD', '/bin/sh -c' )
        launcher.write( dedent('''\
                #!/bin/sh

                HOME=%s
                export HOME

                cd ../../../
                (%s '~~task~~')'''
                % (bq_cluster_home, bq_shell_cmd)
                ) )

        if int(self.file_redirect_stdout):
            launcher.write( ' >> ~~logfile~~.out' )
        if int(self.file_redirect_stderr):
            launcher.write( ' 2>> ~~logfile~~.err' )
        launcher.close()

        # create a file containing the list of commands, one per line
        # and another one containing the log_file name associated
        tasks_file = open( 'tasks', 'w' )
        logfiles_file = open( 'logfiles', 'w' )
        for task in self.tasks:
            tasks_file.write( ';'.join(task.commands) + '\n' )
            logfiles_file.write( task.log_file + '\n' )
        tasks_file.close()
        logfiles_file.close()

        # create the bqsubmit.dat, with
        bqsubmit_dat = open( 'bqsubmit.dat', 'w' )
        bqsubmit_dat.write( dedent('''\
                batchName = dbi_%s
                command = sh launcher
                templateFiles = launcher
                submitOptions = -q %s -l walltime=%s
                param1 = (task, logfile) = load tasks, logfiles
                linkFiles = launcher
                concurrentJobs = %d
                preBatch = rm -f _*.BQ
                microJobs = %d
                '''%(self.unique_id[1:12],self.queue,self.duree,self.nb_proc,self.micro)) )
        print self.unique_id
        if self.clean_up:
            bqsubmit_dat.write('postBatch = rm -rf dbi_batch*.BQ ; rm -f logfiles tasks launcher bqsubmit.dat ;')
        bqsubmit_dat.close()

        # Execute pre-batch
        if len(self.pre_batch)>0:
            exec_pre_batch()

        print "[DBI] All the log will be in the directory: ",self.log_dir
        # Launch bqsubmit
        if not self.test:
            task.set_scheduled_time()
            self.p = Popen( 'bqsubmit', shell=True)
            self.p.wait()
        else:
            print "[DBI] Test mode, we generate all the file, but we do not execute bqsubmit"
            if self.dolog:
                print "[DBI] The scheduling time will not be logged when you will submit the generated file" 

        # Execute post-batchs
        if len(self.post_batch)>0:
            exec_post_batch()

    def wait(self):
        print "[DBI] WARNING cannot wait until all jobs are done for bqtools, use bqwatch or bqstatus"
                
class DBICondor(DBIBase):

    def __init__( self, commands, **args ):
        DBIBase.__init__(self, commands, **args)
        if not os.path.exists(self.log_dir):
            os.mkdir(self.log_dir) # condor log are always generated
        
        if not os.path.exists(self.tmp_dir):
            os.mkdir(self.tmp_dir)

        self.add_commands(commands)

    def add_commands(self,commands):
        if not isinstance(commands, list):
            commands=[commands]

        # create the information about the tasks
        for command in commands:
            pos = string.find(command,' ')
            if pos>=0:
                c = command[0:pos]
                c2 = command[pos:]
            else:
                c=command
                c2=""

            # We use the absolute path so that we don't have corner case as with ./ 
            c = os.path.normpath(os.path.join(os.getcwd(), c))
            command = "".join([c,c2])
            
                # We will execute the command on the specified architecture
                # if it is specified. If the executable exist for both
                # architecture we execute on both. Otherwise we execute on the
                # same architecture as the architecture of the launch computer
            self.cplat = get_condor_platform()
            if c.endswith('.32'):
                self.targetcondorplatform='INTEL'
                self.targetplatform='linux-i386'
                newcommand=command
            elif c.endswith('.64'):
                self.targetcondorplatform='X86_64'
                self.targetplatform='linux-x86_64'
                newcommand=command
            elif os.path.exists(c+".32") and os.path.exists(c+".64"):
                self.targetcondorplatform='BOTH'
                self.targetplatform='linux-i386'
                #newcommand=c+".32"+c2
                newcommand='if [ $CPUTYPE == \'x86_64\' ]; then'
                newcommand+='  '+c+'.64'+c2
                newcommand+='; else '
                newcommand+=c+".32"+c2+'; fi'
                if not os.access(c+".64", os.X_OK):
                    raise Exception("The command '"+c+".64' do not have execution permission!")
#                newcommand=command
                c+=".32"
            elif self.cplat=="INTEL" and os.path.exists(c+".32"):
                self.targetcondorplatform='INTEL'
                self.targetplatform='linux-i386'
                c+=".32"
                newcommand=c+c2
            elif self.cplat=="X86_64" and os.path.exists(c+".64"):
                self.targetcondorplatform='X86_64'
                self.targetplatform='linux-x86_64'
                c+=".64"
                newcommand=c+c2
            else:
                self.targetcondorplatform=self.cplat
                if self.cplat=='INTEL':
                    self.targetplatform='linux-i386'
                else:
                    self.targetplatform='linux-x86_64'
                newcommand=command
            
            if not os.path.exists(c):
                raise Exception("The command '"+c+"' do not exist!")
            elif not os.access(c, os.X_OK):
                raise Exception("The command '"+c+"' do not have execution permission!")

            self.tasks.append(Task(newcommand, self.tmp_dir, self.log_dir,
                                   self.time_format, self.pre_tasks,
                                   self.post_tasks,self.dolog,False,args))

            #keeps a list of the temporary files created, so that they can be deleted at will            

    def run_all_job(self):
        if len(self.tasks)==0:
            return #no task to run
        # create the bqsubmit.dat, with
        condor_datas = []

        #we supose that each task in tasks have the same number of commands
        #it should be true.
        if len(self.tasks[0].commands)>1:
            for task in self.tasks:
                condor_data = os.path.join(self.tmp_dir,self.unique_id +'.'+ task.unique_id + '.data')
                condor_datas.append(condor_data)
                self.temp_files.append(condor_data)
                param_dat = open(condor_data, 'w')
                
                param_dat.write( dedent('''\
                #!/bin/bash
                %s''' %('\n'.join(task.commands))))
                param_dat.close()
        

        condor_file = os.path.join(self.tmp_dir, self.unique_id + ".condor")
        self.temp_files.append(condor_file)
        condor_dat = open( condor_file, 'w' )
        
        req=""
        if self.targetcondorplatform == 'BOTH':
            req="((Arch == \"INTEL\")||(Arch == \"X86_64\"))"
        elif self.targetcondorplatform == 'INTEL':
            req="(Arch == \"INTEL\")"
        elif self.targetcondorplatform == 'X86_64':
            req="(Arch == \"X86_64\")"
            

        tplat=self.targetplatform

        if self.requirements != "":
            req = req+'&&('+self.requirements+')'

        condor_dat.write( dedent('''\
                executable     = %s/launch.sh
                universe       = vanilla
                requirements   = %s
                output         = %s/condor.%s.$(Process).out
                error          = %s/condor.%s.$(Process).error
                log            = %s/condor.log
                ''' % (self.tmp_dir,req,
                       self.log_dir,self.unique_id,
                       self.log_dir,self.unique_id,
                       self.log_dir)))

        if len(condor_datas)!=0:
            for i in condor_datas:
                condor_dat.write("arguments      = sh "+i+" $$(Arch) \nqueue\n")
        else:
            for task in self.tasks:
                condor_dat.write("arguments      = %s \nqueue\n" %(' ; '.join(task.commands)))
        condor_dat.close()

        launch_file = os.path.join(self.tmp_dir, 'launch.sh')
        dbi_file=get_plearndir()+'/python_modules/plearn/parallel/dbi.py'
        overwrite_launch_file=False
        if not os.path.exists(dbi_file):
            print '[DBI] WARNING: Can\' locate dbi.py file. Meaby the file "'+launch_file+'" is not up to date!'
        else:
            if os.path.exists(launch_file):
                mtimed=os.stat(dbi_file)[8]
                mtimel=os.stat(launch_file)[8]
                if mtimed>mtimel:
                    print '[DBI] WARNING: We overwrite the file "'+launch_file+'" with a new version. Update it to your need!'
                    overwrite_launch_file=True
        
        if not os.path.exists(launch_file) or overwrite_launch_file:
            self.temp_files.append(launch_file)
            launch_dat = open(launch_file,'w')
            launch_dat.write(dedent('''\
                #!/bin/sh
                PROGRAM=$1
                shift\n'''))
            if None != os.getenv("CONDOR_LOCAL_SOURCE"):
                launch_dat.write('source ' + os.getenv("CONDOR_LOCAL_SOURCE") + '\n')
            launch_dat.write(dedent('''\
                    echo "Executing on ${HOSTNAME}" 1>&2
                    echo "PATH: $PATH" 1>&2
                    echo "PYTHONPATH: $PYTHONPATH" 1>&2
                    echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH" 1>&2
                    #which python 1>&2
                    #echo -n python version: 1>&2
                    #python -V 1>&2
                    #echo -n /usr/bin/python version: 1>&2
                    #/usr/bin/python -V 1>&2
                    echo ${PROGRAM} $@ 1>&2
                    ${PROGRAM} "$@"'''))
            launch_dat.close()
            os.chmod(launch_file, 0755)

        utils_file = os.path.join(self.tmp_dir, 'utils.py')
        if not os.path.exists(utils_file):
            shutil.copy( get_plearndir()+
                         '/python_modules/plearn/parallel/utils.py', utils_file)
            self.temp_files.append(utils_file)
            os.chmod(utils_file, 0755)

        configobj_file = os.path.join(self.tmp_dir, 'configobj.py')
        if not os.path.exists('configobj.py'):
            shutil.copy( get_plearndir()+
                         '/python_modules/plearn/parallel/configobj.py',  configobj_file)
            self.temp_files.append(configobj_file)            
            os.chmod(configobj_file, 0755)

        # Launch condor
        if self.test == False:
            (output,error)=self.get_redirection(self.log_file + '.out',self.log_file + '.err')
            print "[DBI] Executing: condor_submit " + condor_file
            task.set_scheduled_time()
            self.p = Popen( 'condor_submit '+ condor_file, shell=True , stdout=output, stderr=error)
        else:
            print "[DBI] Created condor file: " + condor_file
            if self.dolog:
                print "[DBI] The scheduling time will not be logged when you will submit the condor file" 
            
    def clean(self):
        if len(self.temp_files)>0:
            sleep(20)
            for file_name in self.temp_files:
                try:
                    os.remove(file_name)
                except os.error:
                    pass
                pass    


    def run(self):
        print "[DBI] The Log file are under %s"%self.log_dir

        if len(self.pre_batch)>0:
            exec_pre_batch()
        self.run_all_job()
        if len(self.post_batch)>0:
            exec_post_batch()

    def wait(self):
        print "[DBI] WARNING no waiting for all job to finish implemented for condor, use 'condor_q' or 'condor_wait %s/condor.log'"%(self.log_dir)
                
    def clean(self):
        pass

class DBILocal(DBIBase):

    def __init__( self, commands, **args ):
        self.nb_proc=1
        DBIBase.__init__(self, commands, **args)
        self.args=args
        self.threads=[]
        self.mt = None
        self.started=0
        self.nb_proc=int(self.nb_proc)
        self.add_commands(commands)
            
    def add_commands(self,commands):
        if not isinstance(commands, list):
            commands=[commands]

        #We copy the variable localy as an optimisation for big list of commands
        #save around 15% with 100 commands
        tmp_dir=self.tmp_dir
        log_dir=self.log_dir
        time_format=self.time_format
        pre_tasks=self.pre_tasks
        post_tasks=self.post_tasks
        dolog=self.dolog
        args=self.args
        for command in commands:
            pos = string.find(command,' ')
            if pos>=0:
                c = command[0:pos]
                c2 = command[pos:]
            else:
                c=command
                c2=""
                
            # We use the absolute path so that we don't have corner case as with ./ 
            c = os.path.normpath(os.path.join(os.getcwd(), c))
            command = "".join([c,c2])
            
            # We will execute the command on the specified architecture
            # if it is specified. If the executable exist for both
            # architecture we execute on both. Otherwise we execute on the
            # same architecture as the architecture of the launch computer
            
            if not os.access(c, os.X_OK):
                raise Exception("The command '"+c+"' do not exist or have execution permission!")
            self.tasks.append(Task(command, tmp_dir, log_dir,
                                   time_format, pre_tasks,
                                   post_tasks,dolog,False,args))
        #keeps a list of the temporary files created, so that they can be deleted at will            

    def run_one_job(self,task):
        c = (';'.join(task.commands))
        task.set_scheduled_time()

        if self.test:
            print "[DBI] "+c
            return

        (output,error)=self.get_redirection(task.log_file + '.out',task.log_file + '.err')

        self.started+=1
        print "[DBI,%d/%d,%s] %s"%(self.started,len(self.tasks),time.ctime(),c)
        p = Popen(c, shell=True,stdout=output,stderr=error)
        p.wait()
            
    def clean(self):
        if len(self.temp_files)>0:
            sleep(20)
            for file_name in self.temp_files:
                try:
                    os.remove(file_name)
                except os.error:
                    pass
                pass    

    def run(self):
        if self.test:
            print "[DBI] Test mode, we only print the command to be executed, we don't execute them"
        if not self.file_redirect_stdout and self.nb_proc>1:
            print "[DBI] WARNING: many process but all their stdout are redirected to the parent"
        if not self.file_redirect_stderr and self.nb_proc>1:
            print "[DBI] WARNING: many process but all their stderr are redirected to the parent"
        print "[DBI] The Log file are under %s"%self.log_dir

        # Execute pre-batch
        if len(self.pre_batch)>0:
            exec_pre_batch()

        # Execute all Tasks (including pre_tasks and post_tasks if any)
        self.mt=MultiThread(self.run_one_job,self.tasks,self.nb_proc,"[DBI,%s]"%time.ctime())
        self.mt.start()
        
        # Execute post-batchs
        if len(self.post_batch)>0:
            exec_post_batch()
            
        print "[DBI] The Log file are under %s"%self.log_dir
        
    def clean(self):
        pass

    def wait(self):
        if self.mt:
            self.mt.join()
        else:
            print "[DBI] WARNING jobs not started!"
                
class SshHost:
    def __init__(self, hostname):
        self.hostname= hostname
        self.lastupd= -16
        self.getAvailability()
        
    def getAvailability(self):
        # simple heuristic: mips / load
        t= time.time()
        if t - self.lastupd > 15: # min. 15 sec. before update
            self.bogomips= self.getBogomips()
            self.loadavg= self.getLoadavg()
            self.lastupd= t
            #print  self.hostname, self.bogomips, self.loadavg, (self.bogomips / (self.loadavg + 0.5))
        return self.bogomips / (self.loadavg + 0.5)
        
    def getBogomips(self):
        cmd= ["ssh", self.hostname ,"cat /proc/cpuinfo"]
        p= Popen(cmd, stdout=PIPE)
        bogomips= 0.0
        for l in p.stdout:
            if l.startswith('bogomips'):
                s= l.split(' ')
                bogomips+= float(s[-1])
        return bogomips

    def getLoadavg(self):
        cmd= ["ssh", self.hostname,"cat /proc/loadavg"]
        p= Popen(cmd, stdout=PIPE)
        l= p.stdout.readline().split(' ')
        return float(l[0])
        
    def addToLoadavg(self,n):
        self.loadavg+= n
        self.lastupd= time.time()

    def __str__(self):
        return "SshHost("+self.hostname+" <"+str(self.bogomips) \
               +','+str(self.loadavg) +','+str(self.getAvailability()) \
               +','+str(self.lastupd) + '>)'

    def __repr__(self):
        return str(self)
        
def find_all_ssh_hosts():
    return [SshHost(h) for h in set(pymake.get_distcc_hosts())]

def cmp_ssh_hosts(h1, h2):
    return cmp(h2.getAvailability(), h1.getAvailability())

class DBISsh(DBIBase):

    def __init__(self, commands, **args ):
        print "[DBI] WARNING: The SSH DBI is not fully implemented!"
        print "[DBI] Use at your own risk!"
        DBIBase.__init__(self, commands, **args)

        self.add_commands(commands)
        self.hosts= find_all_ssh_hosts()
        
    def add_commands(self,commands):
        if not isinstance(commands, list):
            commands=[commands]
            
        # create the information about the tasks
        for command in commands:
            self.tasks.append(Task(command, self.tmp_dir, self.log_dir,
                                   self.time_format, self.pre_tasks,
                                   self.post_tasks,self.dolog,False))
            
    def getHost(self):
        self.hosts.sort(cmp= cmp_ssh_hosts)
        #print "hosts= "
        #for h in self.hosts: print h
        self.hosts[0].addToLoadavg(1.0)
        return self.hosts[0]
    
    def run_one_job(self, task):
        DBIBase.run(self)

        host= self.getHost()


        cwd= os.getcwd()
        command = "ssh " + host.hostname + " 'cd " + cwd + "; " + string.join(task.commands,';') + "'"
        print "[DBI] "+command

        if self.test:
            return
        
        task.launch_time = time.time()
        task.set_scheduled_time()
        
        (output,error)=self.get_redirection(task.log_file + '.out',task.log_file + '.err')
        
        task.p = Popen(command, shell=True,stdout=output,stderr=error)

    def run(self):
        print "[DBI] The Log file are under %s"%self.log_dir

        # Execute pre-batch
        if len(self.pre_batch)>0:
            exec_pre_batch()

        if self.test:
            print "[DBI] In testmode, we only print the command that would be executed."
            
        # Execute all Tasks (including pre_tasks and post_tasks if any)
        print "[DBI] tasks= ", self.tasks
        for task in self.tasks:
            self.run_one_job(task)

        # Execute post-batchs
        if len(self.post_batch)>0:
            exec_post_batch()

    def clean(self):
        #TODO: delete all log files for the current batch
        pass

    def wait(self):
        #TODO
        print "[DBI] WARNING the wait function was not implement for the ssh backend!"

# creates an object of type ('DBI' + launch_system) if it exists
def DBI(commands, launch_system, **args):
    """The Distributed Batch Interface is a collection of python classes
    that make it easy to execute commands in parallel using different
    systems like condor, bqtools on Mammoth, the cluster command or localy.
    """
    try:
        jobs = eval('DBI'+launch_system+'(commands,**args)')
    except NameError:
        print 'The launch system ',launch_system, ' does not exists. Available systems are: Cluster, Ssh, bqtools and Condor'
        traceback.print_exc()
        sys.exit(1)
    return jobs

def main():
    if len(sys.argv)!=2:
        print "Usage:dbi.py {Condor|Cluster|Ssh|Local|bqtools} < joblist"
        print "Where joblist is a file containing one exeperiement by line"
        sys.exit(0)
    DBI([ s[0:-1] for s in sys.stdin.readlines() ], sys.argv[1]).run()
#    jobs.clean()

#    config['LOG_DIRECTORY'] = 'LOGS/'
if __name__ == "__main__":
    main()
