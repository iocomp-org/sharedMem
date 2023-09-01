#!/bin/bash --login
#SBATCH --job-name=sharedmem
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
module load cray-hdf5-parallel

# adios2 dir 
export ADIOS2_DIR=/work/e609/e609/shr203/opt/gnu/8.0.0/ADIOS2
export LD_LIBRARY_PATH=${ADIOS2_DIR}/lib64:${LD_LIBRARY_PATH} 

# recommended flag for better io performance 
export FI_OFI_RXM_SAR_LIMIT=64K

# EXE 
export EXE=/mnt/lustre/a2fs-work3/work/e609/e609/shr203/sharedmem/src/sharedmem
export CONFIG=${SLURM_SUBMIT_DIR}/config.xml 

# Setup environment
export PPN=${SLURM_NTASKS_PER_NODE}
export OMP_NUM_THREADS=1

# for command line arguments, set default if not set. 
if [[ -z ${N} ]]; 
then 
  N=$((2**22))
  echo "N is not set, set to be ${N}"
fi 

if [[ -z ${IOSTART} ]]; 
then 
  IOSTART=0
  echo "I/O start is not set, set to be ${IOSTART}"
fi 

if [[ -z ${IOEND} ]]; 
then 
  IOEND=0
  echo "I/O start is not set, set to be ${IOEND}"
fi 

# averaging parameter using slurm variable, set by passing array parameter 
iter=${SLURM_ARRAY_TASK_ID}

# Ensure the cpus-per-task option is propagated to srun commands
export SRUN_CPUS_PER_TASK=$SLURM_CPUS_PER_TASK

FILESIZE=$((${N}*8/2**20))
IOLIB=("MPIIO" "HDF5" "ADIOS2_HDF5" "ADIOS2_BP4" "ADIOS2_BP5") 

# ARM map requirement 
# module load arm/forge
#
# Total number of procs for ARM 
TOTAL_PROCS=$((${SLURM_NNODES}*${SLURM_NTASKS_PER_NODE}))

for io in $(seq ${IOSTART} ${IOEND})
do 
  # <path>/ioLibrary/Filesize(MB) 
  export RUNDIR=${SLURM_SUBMIT_DIR}/${DIR}/${IOLIB[${io}]}/${FILESIZE}MiB/${SLURM_NNODES}/${SLURM_NTASKS_PER_NODE}/${iter}
  echo "DIR $RUNDIR"

  rm -rf ${RUNDIR} 
  mkdir -p ${RUNDIR} 
  cp ${CONFIG} ${RUNDIR}
  lfs setstripe -c -1 ${RUNDIR}
  cd ${RUNDIR} 

  echo "Job started " $(date +"%T") # start time 
  echo "N" ${N} "IO" ${io} "FILESIZE" ${FILESIZE}MiB

  srun --hint=nomultithread  --distribution=block:block ${EXE} --N ${N} --io ${io} > test.out 
  # map --mpi=slurm -n ${TOTAL_PROCS} --mpiargs="--hint=nomultithread --distribution=block:block" --profile ${EXE} --N ${N} --io ${io} 
  wait 
done 

module list  2>&1 | tee -a test.out
echo "JOB ID"  $SLURM_JOBID >> test.out
echo "JOB NAME" ${SLURM_JOB_NAME} >> test.out

echo "Job ended " $(date +"%T") # end time 
