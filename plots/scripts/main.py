import argparse
from datacollect import* 
from plots import*

parser = argparse.ArgumentParser(
                    prog = 'ProgramName',
                    description = 'What the program does',
                    epilog = 'Text at the bottom of help')

parser.add_argument('--save', action='store_true', help='Flag to save the plot, pass name flag for custom name for saved plot')  # if save used then fig is saved, otherwise plt.show
parser.add_argument('--iocompdir',  help = 'Directory of outputs for iocomp')  # name of directory where to search 
parser.add_argument('--artifactdir',  help = 'Directory of outputs for artifact')  # name of directory where to search 

args = parser.parse_args()

"""
Get data from sharedMem
"""
if(args.artifactdir != None): 
    json_data = trawl_files(f'{args.artifactdir}')
else:
    print("Please input the artifact directory. Exiting.")
    exit() 

"""
Get data from iocomp 
"""
if(args.iocompdir != None): 
    iocomp_data = STREAM_iocomp_timers(f'{args.iocompdir}')

"""
Compare wall time with icomp if iocompdir is specified. otherwise print out wall time for just artifact.
"""
if(args.iocompdir != None): 
    plot_compare_walltime_with_iocomp(json_data, iocomp_data, args)
else:
    plot_walltime(json_data, args)


"""
Plot compute time for artifact data and compare with iocomp if directory is given. 
Otherwise plot compute time for only the artifact data
"""
if(args.iocompdir!=None): 
    plot_comptime(json_data, args, iocomp_data)
else:
    plot_comptime(json_data, args)


