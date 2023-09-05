import pandas as pd
import os
import matplotlib.pyplot as plt
import json 
from datetime import datetime
import shutil 
from datacollect import*

plt.style.use(f"{os.getcwd()}/style.mplstyle")

DEBUG = False # debug command 
SLURM_MAPPING="Consecutive" # setting as consecutive by default. 

IOLAYERLIST = [
    "MPIIO",
    "HDF5", 
    "ADIOS2_HDF5",
    "ADIOS2_BP4"
]

fileSize = 128

STREAM_KERNELS = [ 
    # "COPY", 
    "SCALE", 
    "ADD", 
    "TRIAD"
]

num_procs_list = [
    128, 
    256, 
    512,
    1024, 
    2048,
    # 4096
]

stream_list = [
    # "Copy(s)",
    "Scalar(s)",
    "Add(s)",
    "Triad(s)"
]

"""
Create output directory = <function/plots/year/month/date>
"""
def get_output_directory(): 
    a = datetime.now()
    year = int(a.strftime('%Y')) 
    month = int(a.strftime('%m')) 
    day = int(a.strftime('%d')) 
    OUTPUT_DIR = f"../output/{year}/{month}/{day}"
    return(OUTPUT_DIR)

"""
Plot wall time and STREAM kernels 
"""
def plot_compare_walltime_with_iocomp(json_data, iocomp_data, args):

    json_object = json.loads(json_data)


    # Position of bars on x-axis
    ind = np.arange(len(num_procs_list))

    # Width of a bar 
    width = 0.3       

    ioLayer_count = len(IOLAYERLIST)
    # if more than 1 iolayer, create a 2x2 plot of all i/o layers together 
    if(ioLayer_count > 1):
        fig1, ax1 = plt.subplots(2, 2,figsize=(10,10),sharey=True)
        ioLayer_iter = 0 
    else:
        fig1 = plt.figure()

    for IO_key in  IOLAYERLIST:

        wallTime_iocomp = {}  
        wallTime_std_iocomp = {}  
        wallTime_shared = {}  
        wallTime_std_shared = {}  
        for procs in num_procs_list:
            wallTime_shared[procs] = json_object[IO_key][f"{fileSize}MiB"][str(procs)]["wallTime"]
            wallTime_std_shared[procs] = json_object[IO_key][f"{fileSize}MiB"][str(procs)]["wallTime_std"]
            wallTime_iocomp[procs] = iocomp_data[procs][f"{fileSize}MiB"][IO_key][SLURM_MAPPING]["wallTime"]
            wallTime_std_iocomp[procs] = iocomp_data[procs][f"{fileSize}MiB"][IO_key][SLURM_MAPPING]["wallTime_std"]
    
        """
        bar plot 
        """ 
        # convert list of cores to strings and divide by 2 to get compute processes. Then append the string
        # value of that number for simpler plotting. 
        procs = list(wallTime_shared.keys())
        procs_str_list = [str(n) for n in procs] 
        comp_procs_list = [] 
        for x in procs_str_list:
            comp_procs_list.append(str(int(int(x)/2)))

        walltime_shared_list = list(wallTime_shared.values())
        walltime_std_shared_list = list(wallTime_std_shared.values())
        walltime_iocomp_list = list(wallTime_iocomp.values())
        walltime_std_iocomp_list = list(wallTime_std_iocomp.values())

        if(ioLayer_count>1):

            i=int(ioLayer_iter/2)
            j=int(ioLayer_iter%2)

            # bar plot to plot wall time against number of processes
            ax1[i,j].bar(ind, walltime_shared_list, yerr=walltime_std_shared_list, capsize=5, width=width, align='center', color = "blue", label="sharedMem")
            ax1[i,j].bar(ind+width, walltime_iocomp_list, width, yerr=walltime_std_iocomp_list, capsize=5, align='center', color = "orange", label="iocomp")
            ax1[i,j].title.set_text(IO_key)

            # set ticks and labels for each subplot
            ax1[i,j].set_xticks(ind) 
            ax1[i,j].set_xticklabels(comp_procs_list)

            # increment I/O layer counter that controls assignment of subplots in the plot  
            ioLayer_iter += 1 
            fig1.supxlabel(f'Number of compute processes')
            fig1.supylabel(f'Wall time(s)')
            # fig1.tight_layout()

        else:
            plt.bar(ind, walltime_shared_list, width, align='center', color = "blue", label="sharedMem")
            plt.bar(ind+width, walltime_iocomp_list, width, align='center', color = "orange", label="iocomp")
            plt.xlabel('Number of compute processes')
            plt.ylabel('Wall time(s)')
            plt.title(f"Local {fileSize}MiB for {IO_key}")


    # xticks()
    # First argument - A list of positions at which ticks should be placed
    # Second argument -  A list of labels to place at the given locations
    plt.xticks(ind + width / 2, comp_procs_list)
    fig1.suptitle("Wall time vs number of processes")
    fig1.tight_layout()
    plt.legend()
    plt_save_or_show(json_data, plt,args,f"wallTime_comparison")

"""
Plot wall time function, takes in json data, args, produces bar plot
and saves data in json format if save option is given.  
"""
def plot_walltime(json_data, args):


    json_object = json.loads(json_data)

    ioLayer_count = len(json_object)

    # if more than 1 iolayer, create a 2x2 plot of all i/o layers together 
    if(ioLayer_count > 1):
        fig1, ax1 = plt.subplots(2, 2,figsize=(10,10),sharey=True)
        ioLayer_iter = 0 
    else:
        fig1 = plt.figure()

    for IO_key, Node_list in json_object.items(): 

        wallTime = {}  
        for procs in num_procs_list:
            wallTime[procs] = json_object[IO_key][f"{fileSize}MiB"][str(procs)]["wallTime"] 
    
        """
        bar plot 
        """ 
        # convert list of cores to strings and divide by 2 to get compute processes. Then append the string
        # value of that number for simpler plotting. 
        procs = list(wallTime.keys())
        procs_str_list = [str(n) for n in procs] 
        comp_procs_list = [] 
        for x in procs_str_list:
            comp_procs_list.append(str(int(int(x)/2)))
        # obtain wall time 
        walltime_list = list(wallTime.values())
        
        if(ioLayer_count>1):

            i=int(ioLayer_iter/2)
            j=int(ioLayer_iter%2)

            # bar plot to plot wall time against number of processes
            ax1[i,j].bar(comp_procs_list, walltime_list, align='center')

            ax1[i,j].title.set_text(IO_key)
            # increment I/O layer counter that controls assignment of subplots in the plot  
            ioLayer_iter += 1 
            fig1.supxlabel(f'Number of compute processes')
            fig1.supylabel(f'Wall time(s)')

        else:
            plt.bar(procs_str_list, walltime_list, align='center')
            plt.xlabel('Number of compute processes')
            plt.ylabel('Wall time(s)')
            plt.title(f"Local {fileSize}MiB for {IO_key}")

    plt.title("Wall time vs number of processes")
    fig1.tight_layout()
    plt_save_or_show(json_data, plt,args,f"wallTime_overall")


"""
Plot compute time function, takes in json data, args, produces bar plot
and saves data in json format if save option is given. 
Compute time is added per kernel. 
"""
def plot_comptime(json_data, args, iocompdata=None):

    # Position of bars on x-axis
    ind = np.arange(len(num_procs_list))

    # Width of a bar 
    width = 0.3       

    json_object = json.loads(json_data)

    ioLayer_count = len(json_object)

    # if more than 1 iolayer, create a 2x2 plot of all i/o layers together 
    if(ioLayer_count > 1):
        fig1, ax1 = plt.subplots(2, 2,figsize=(10,10),sharey=True)
        ioLayer_iter = 0 
    else:
        fig1 = plt.figure()

    for IO_key, Node_list in json_object.items(): 

        compTime = {}  

        for procs in num_procs_list:

            # initialise compTime per rank per I/O layer 
            compTime[procs] = 0

            # iterate over each kernel for each run and add up the compute time 
            for kernel in STREAM_KERNELS:
                compTime[procs] += json_object[IO_key][f"{fileSize}MiB"][str(procs)][kernel]["Avg_time(s)"] 

            # if iocomp data defined then iterate over stream list and obtain the total compute time  
            if(iocompdata!=None): 
                compTime_iocomp = {} 
                for stream_category in stream_list: 
                    compTime_iocomp[procs] = iocompdata[procs][f"{fileSize}MiB"][IO_key][SLURM_MAPPING]["computeTime"][stream_category]
        """
        bar plot 
        """ 
        # convert list of cores to strings and divide by 2 to get compute processes. Then append the string
        # value of that number for simpler plotting. 
        procs = list(compTime.keys())
        procs_str_list = [str(n) for n in procs] 
        comp_procs_list = [] 
        for x in procs_str_list:
            comp_procs_list.append(str(int(int(x)/2)))
        # obtain wall time 
        comptime_list = list(compTime.values())
        if(iocompdata!=None): 
            comptime_list_iocomp = list(compTime_iocomp.values())

        # plot iocomp data if data given 

        
        if(ioLayer_count>1):

            i=int(ioLayer_iter/2)
            j=int(ioLayer_iter%2)

            # bar plot to plot wall time against number of processes
            ax1[i,j].bar(ind, comptime_list, width, align='center', color="blue", label="sharedMem")

            # plot iocomp data if comparison needed  
            if(iocompdata!=None): 
                ax1[i,j].bar(ind+width, comptime_list_iocomp, width, align='center', color="orange", label="ioComp")

            ax1[i,j].title.set_text(IO_key)

            # set ticks and labels for each subplot
            ax1[i,j].set_xticks(ind) 
            ax1[i,j].set_xticklabels(comp_procs_list)

            # labels 
            fig1.supxlabel(f'Number of compute processes')
            fig1.supylabel(f'Total computational time(s)')

            # to not have the annoying bars 
            # ax1[i,j].set_ylim([0,350])
            # increment I/O layer counter that controls assignment of subplots in the plot  
            ioLayer_iter += 1 

        else:
            plt.bar(procs_str_list, comptime_list, align='center')
            plt.xlabel('Number of compute processes')
            plt.ylabel('Compute time(s)')
            plt.title(f"Local {fileSize}MiB for {IO_key}")
            plt.xticks(ind + width / 2, comp_procs_list)

    plt.legend()  
    fig1.suptitle("Computational time vs number of processes")
    fig1.tight_layout()
    plt_save_or_show(json_data, plt,args,f"compTime_overall")

def plt_save_or_show(json_data, plt, args, filename="output"):
    if(args.save):
        # append name of filename 
        FILE_DIR=f'{get_output_directory()}/{filename}' 
        # create a path to outdirectory if not already there 
        if(os.path.isdir(FILE_DIR)==False):
            os.makedirs(FILE_DIR)

        plt.savefig(f"{FILE_DIR}/{filename}.pdf")

        # save json file 
        with open(f'{FILE_DIR}/{filename}.json', 'w') as json_file:
            json_file.write(json_data)

        print("Output saved in...")
        print(f"{FILE_DIR}/{filename}.pdf")

    else:
        plt.show()

def csv2json(filename):
    mydata = pd.read_csv(filename,index_col=0,skiprows=0,header=0)

    data = {}
    for kernels in STREAM_KERNELS:
        data[kernels] = mydata.loc[ kernels, "Best_Rate(GB/s)":"Max_time(s)" ].to_dict()
    data["wallTime"] = mydata.loc["WALL","Max_Walltime(s)"]

    json_obj = json.dumps(data) 
    json_data = json.loads(json_obj)
    return(json_data)
