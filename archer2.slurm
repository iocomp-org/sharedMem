#!/bin/bash --login
#SBATCH --job-name=stream
#SBATCH --nodes=1
#SBATCH --tasks-per-node=128
#SBATCH --cpus-per-task=1
#SBATCH --time=00:05:00
#SBATCH --account=e609
#SBATCH --partition=standard 
#SBATCH --qos=standard 

# GNU 
module swap PrgEnv-cray PrgEnv-gnu
module use /work/y07/shared/archer2-lmod/dev
module load valgrind4hpc 
#module swap craype-network-ofi craype-network-ucx 
#module swap cray-mpich cray-mpich-ucx 

# EXE 
export EXE=/mnt/lustre/a2fs-work3/work/e609/e609/shr203/sharedmem/sharedmem

# Setup environment
export PPN=${SLURM_NTASKS_PER_NODE}
export OMP_NUM_THREADS=1

# for command line arguments 
if [[ -z ${N} ]]; 
then 
  N=$((2**22))
  echo "N is not set, set to be ${N}"
fi 

if [[ -z ${io} ]]; 
then 
  io=0
  echo "I/O is not set, set to be ${io}"
fi 

FILESIZE=$((${N}*8/2**20))
IOLIB=("MPIIO" "HDF5") 

# <path>/ioLibrary/Filesize(MB) 
export RUNDIR=/mnt/lustre/a2fs-work3/work/e609/e609/shr203/sharedmem/TEST/${IOLIB[${io}]}/${FILESIZE}MB/${SLURM_NNODES}/${SLURM_NTASKS_PER_NODE}
echo "DIR $RUNDIR"

rm -rf ${RUNDIR} 
mkdir -p ${RUNDIR} 
lfs setstripe -c -1 ${RUNDIR}
cd ${RUNDIR} 
export SRUN_CPUS_PER_TASK=$SLURM_CPUS_PER_TASK

echo "Job started " $(date +"%T") # start time 
echo "N" ${N} "IO" ${io} "FILESIZE" ${FILESIZE}MiB

srun --hint=nomultithread  --distribution=block:block ${EXE} --N ${N} --io ${io} > test.out 

wait 

module list  2>&1 | tee -a test.out
echo "JOB ID"  $SLURM_JOBID >> test.out
echo "JOB NAME" ${SLURM_JOB_NAME} >> test.out

echo "Job ended " $(date +"%T") # end time 
