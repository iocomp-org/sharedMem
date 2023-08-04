#!/bin/bash --login
#SBATCH --job-name=stream
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=00:05:00
#SBATCH --account=e609
#SBATCH --partition=standard 
#SBATCH --qos=standard 

# GNU 
module swap PrgEnv-cray PrgEnv-gnu
module use /work/y07/shared/archer2-lmod/dev
#module swap craype-network-ofi craype-network-ucx 
#module swap cray-mpich cray-mpich-ucx 

# EXE 
export EXE=/mnt/lustre/a2fs-work3/work/e609/e609/shr203/sharedmem/sharedmem

# Setup environment
export PPN=${SLURM_NTASKS_PER_NODE}
export OMP_NUM_THREADS=1

# for command line arguments 
N=10**6
io=1
FILESIZE=$((${N}*8/10**6))
IOLIB=("MPIIO" "HDF5") 

# <path>/ioLibrary/Filesize(MB) 
export RUNDIR=/mnt/lustre/a2fs-work3/work/e609/e609/shr203/sharedmem/OUTPUT/${IOLIB[${io}]}/${FILESIZE} 

rm -rf ${RUNDIR} 
mkdir -p ${RUNDIR} 
lfs setstripe -c -1 ${RUNDIR}
cd ${RUNDIR} 
export SRUN_CPUS_PER_TASK=$SLURM_CPUS_PER_TASK

echo "Job started " $(date +"%T") # start time 

srun ${EXE} --N ${N} --io ${io}

wait 

echo $(module list) 

echo "Job ended " $(date +"%T") # end time 
