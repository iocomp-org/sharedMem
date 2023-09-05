import pandas as pd
import os
import json 
import numpy as np 
from plots import *

"""
Function goes through directories trawling for csv files
Takes in input directories
Returns json data 
"""
def trawl_files(file_dir):

    fileSize='128MiB'

    IO_list = next(os.walk(f"{file_dir}"))[1]

    data_io = {} 

    AVG_FLAG = 0 # averaging flag to establish averaging needed or not  

    for ioLayer in IO_list:
        fileSize_list = next(os.walk(f"{file_dir}/{ioLayer}"))[1]

        data_fileSize = {} 

        for fileSize in fileSize_list:
            nodeSize_list = next(os.walk(f"{file_dir}/{ioLayer}/{fileSize}"))[1]

            data_numprocs = {}

            for node in nodeSize_list:
                ppn_list = next(os.walk(f"{file_dir}/{ioLayer}/{fileSize}/{node}"))[1]

                for ppn in ppn_list:
                    num_procs = int(node)*int(ppn) # number of MPI processes  
                    mydata = {}
                    
                    log(f"directory: {ioLayer}/{fileSize}/{node}/{ppn}")
                    # check for averaging directory by checking if <file_path><[0]> exists 
                    if(os.path.isdir(f"{file_dir}/{ioLayer}/{fileSize}/{node}/{ppn}/0") == True):
                        iter_list = next(os.walk(f"{file_dir}/{ioLayer}/{fileSize}/{node}/{ppn}"))[1]

                        avg_data = {} 

                        for jobnum in iter_list: 
                            computeCSV = f"{file_dir}/{ioLayer}/{fileSize}/{node}/{ppn}/{jobnum}/compserver_output.csv"
                            if(os.path.isfile(computeCSV)):
                                avg_data[jobnum] = csv2json(computeCSV)

                            # if ioserver exists, then update the write and window time for each kernel 
                            ioCSV = f"{file_dir}/{ioLayer}/{fileSize}/{node}/{ppn}/{jobnum}/ioserver_output.csv"
                            if(os.path.isfile(ioCSV)):
                                # append I/O data to the compute data dictionary 
                                avg_data = ioserver2json(ioCSV, avg_data, jobnum)

                        # average out all data parameters per job number 
                        mydata = average_data(avg_data, iter_list)

                    else:
                        computeCSV = f"{file_dir}/{ioLayer}/{fileSize}/{node}/{ppn}/compserver_output.csv"

                        if(os.path.isfile(computeCSV)):
                            mydata = csv2json(computeCSV)
                        
                        # if ioserver exists, then update the write and window time for each kernel 
                        ioCSV = f"{file_dir}/{ioLayer}/{fileSize}/{node}/{ppn}/ioserver_output.csv"
                        if(os.path.isfile(ioCSV)):
                            # append I/O data to the compute data dictionary 
                            avg_data = ioserver2json(ioCSV, avg_data, 0)

                    data_numprocs[num_procs] = mydata

            data_fileSize[fileSize] = data_numprocs
        
        data_io[ioLayer] = data_fileSize

    json_data = json.dumps(data_io)

    return(json_data)

"""
Function takes in json data and outputs averaged json data. 
"""
def average_data(avg_data, iter_list ):

    # initialise output dictionary with stream kernels
    output_data = dict.fromkeys(STREAM_KERNELS, None) 

    # initialise wall time 
    output_data["wallTime"] = 0.0 

    num_avg_jobs = len(iter_list)

    # initialise different variables 
    wallTime = [] 
    # iterate over slurm job array numbers  
    for jobnum in iter_list:

        # average wall times 
        # output_data["wallTime"]  += avg_data[jobnum]["wallTime"]/num_avg_jobs
        wallTime.append(avg_data[jobnum]["wallTime"]/num_avg_jobs)

        log(jobnum)
        walltime_job = avg_data[jobnum]["wallTime"]
        log(f"walltime = {walltime_job}")
        walltime_avg = output_data["wallTime"] 
        log(f"avg walltime = {walltime_avg}")

    # find std and averaging using np functions on wallTime list object   
    output_data["wallTime"]  = np.mean(wallTime)
    output_data["wallTime_std"] = np.std(wallTime)

    # iterate over kernels to get the per kernel data values  
    for kernels in STREAM_KERNELS:

        log(kernels)
        # initialise kernel data using parameters 
        kernel_data = dict.fromkeys(avg_data["0"][kernels].keys(), 0.0)

        # iterate over slurm job array numbers  
        for jobnum in iter_list: 

            log(jobnum)
            # find out the parameters such as Min BW, Max BW etc. 
            for parameters in avg_data[jobnum][kernels].keys():

                val = avg_data[jobnum][kernels][parameters]
                kernel_data[parameters] += val/num_avg_jobs

                log(f"Job value for {parameters} = {val}")
                log(f"Average value for {parameters} = {kernel_data[parameters]}")
    
        output_data[kernels] = kernel_data 

    return(output_data)

def STREAM_iocomp_timers(parentDir):

    """
    Iterate through parent dir to get list of cores 
    """
    dir_list = next(os.walk(parentDir))[1]
    data = {} # main dictionary initialised   

    """
    Iterate over core sizes
    """
    for coreSize in dir_list:
        """
        Calculate num of processors based on core and node according to node_core format 
        """
        core = coreSize.split("_",1)[1] 
        node = coreSize.split("_",1)[0] 
        numProcs = int(core)*int(node) 

        """ 
        Iterate over array sizes 
        """
        data_array = {} 
        array_list = next(os.walk(f"{parentDir}/{coreSize}"))[1]
        for arraySize in array_list: 

            layer_list = next(os.walk(f"{parentDir}/{coreSize}/{arraySize}"))[1]

            """
            Iterate over I/O layers 
            """
            data_io = {} # dictionary for I/O stuff 
            for ioLayer in layer_list: 

                """
                Iterate over slurm mappings 
                """
                data_mapping = {} 
                slurm_list = next(os.walk(f"{parentDir}/{coreSize}/{arraySize}/{ioLayer}"))[1]
                for slurmMapping in slurm_list: 

                    mydata = {} 
                    if os.path.isfile(f"{parentDir}/{coreSize}/{arraySize}/{ioLayer}/{slurmMapping}/compute_write_time.csv") == True: # i.e. it does not follow 0,1,2 after slurm mapping convention
                        filename =  f"{parentDir}/{coreSize}/{arraySize}/{ioLayer}/{slurmMapping}/compute_write_time.csv"
                        """
                        read info from text file 
                        """
                        mydata["0"] = readData(filename)
                    else:  # if it does have average jobs etc., for averaging read values from each iteration and then store as arrayJobNum: <data>
                        avg_job_list = next(os.walk(f"{parentDir}/{coreSize}/{arraySize}/{ioLayer}/{slurmMapping}"))[1]
                        job_index = 0 # index for list such as 0, 1, 2   
                        for jobIter in avg_job_list: 

                            path = f"{parentDir}/{coreSize}/{arraySize}/{ioLayer}/{slurmMapping}/{jobIter}"
                            
                            filename = f"{path}/compute_write_time.csv"
                            if(os.path.isfile(filename)):
                                """
                                read info from text file 
                                """
                                mydata[jobIter] = readData(filename)
                    
                    """
                    Average over job array
                    add data per slurm mapping
                    """
                    data_mapping[slurmMapping] = average_jobs_STREAM(mydata)

                """
                add data per I/O layer
                """
                data_io[ioLayer] = data_mapping
            
            """
            add data per array size 
            """
            data_array[arraySize] = data_io

        """
        add data per core size 
        """
        data[numProcs] = data_array
    
    return(data)

"""
Function takes in ioServer filename and outputs the parameters as a dictionary
used if iobw is passed in makefile flags 
"""
def ioserver2json(filename, data, jobnum):
    mydata = pd.read_csv(filename,index_col=0,skiprows=0,header=0)

    # ordering of I/O server     
    # a, c, b : 0, 1, 2 
    # 2 : SCALE b  
    # 1: ADD c 
    # 0: TRIAD a  
    KERNELS_WINDOW = {
        "TRIAD" : 0,   
        "ADD"   : 1, 
        "SCALE" : 2 
    }

    # data = dict.fromkeys(KERNELS_WINDOW, None)

    for kernels, windowNum  in KERNELS_WINDOW.items():

        data_per_kernel = {} 

        # get new data frame from previous dataframe when window num == kernel iteration 
        df = mydata.loc[mydata['Window_Number']==windowNum]

        # average out the window time and write time per number of iterations 
        data_per_kernel["Window_Time(s)"] = df['Window_Time(s)'].mean()
        data_per_kernel["Write_Time(s)"] = df['Write_Time(s)'].mean()

        # add window and write time ker STREAM kernel  
        data[jobnum][kernels].update(data_per_kernel)
    
    return(data)

def csv2json(filename):
    mydata = pd.read_csv(filename,index_col=0,skiprows=0,header=0)

    data = {}
    for kernels in STREAM_KERNELS:
        data[kernels] = mydata.loc[ kernels, "Best_Rate(GB/s)":"Max_time(s)" ].to_dict()
    data["wallTime"] = mydata.loc["WALL","Max_Walltime(s)"]

    json_obj = json.dumps(data) 
    json_data = json.loads(json_obj)
    return(json_data)

"""
function inserts compute, total, wait and wall times 
into a data dictionary. 
"""
def readData(filename):
    """
    Read data
    """
    mydata2 = pd.read_csv(filename, index_col=False, skiprows=0)
    data = {}

    computeTime = {}
    sendTime = {} 
    waitTime = {}

    """
    Data analysis, ignore the first element
    """
    xAxis = mydata2.columns.values[1:]  # header file
    computeTime2 = mydata2.iloc[0].values[1:5]
    sendTime2 = mydata2.iloc[1].values[1:5]
    waitTime2 = mydata2.iloc[2].values[1:5]
    wallTime = mydata2.iloc[3].values[5] # bug in the iocomp output file, walltimer should have one more comma, it should be 5 value. 

    """
    add elements to the dictionary by their element 
    """
    for iter in range(0,4):
        computeTime[xAxis[iter]] = computeTime2[iter]
        sendTime[xAxis[iter]] = sendTime2[iter]
        waitTime[xAxis[iter]] = waitTime2[iter]

    data["computeTime"] = computeTime
    data["sendTime"] = sendTime
    data["waitTime"] = waitTime
    data["wallTime"] = wallTime

    return data

"""
average_jobs averages the average loop, wait and comp time over the array of jobs
"""
def average_jobs_STREAM(avg): # mydata[0], mydata[1] etc. 


    avg_job = {} 
    avg_job["wallTime"] = 0 # initialise wall time seperately as it doesnt have stream timers 
    
    # hard coded values! 
    iocompTimerList = [ "computeTime", "waitTime", "sendTime", "wallTime" ] 
    streamKernels = ["Copy(s)", "Scalar(s)", "Add(s)", "Triad(s)"] 

    for iocompTimer in iocompTimerList: 
        values = [] # store iocomp timers to get averages etc. 

        if(iocompTimer == "wallTime"): # as wallTimer doesnt have values for stream kernels 
            for key, value in avg.items(): 
                values.append(avg[key][iocompTimer])
            avg_job[iocompTimer] = np.mean(values)
            avg_job[f"{iocompTimer}_std"] = np.std(values)

        else: # for compute, wait or send times which have stream kernel values: 
            dict1 = {} 
            dict1 = dict.fromkeys(streamKernels,0) # initialise with 0s 
            for stream in streamKernels: 
                for key, value in avg.items(): # key will be 0,1,2 etc job array submissions
                    values.append(avg[key][iocompTimer][stream]) # append values for the different jobs 

                # append values to dictionary  
                dict1[stream] = np.mean(values) # average values for the different stream categories   
                dict1[f"{stream}_std"] = np.std(values) # average values for the different stream categories   
        
            avg_job[iocompTimer] = dict1  

    return(avg_job)



"""
log function prints messages if DEBUG flag is set to true 
"""
def log(s): 
    if DEBUG:
        print(s)

