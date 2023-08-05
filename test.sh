### TEST 
#for i in $(seq 1 7)
#do 
#  TASKS=$((2**${i}))
#  N_SIZE=$(( (2**8)/(${TASKS}/2) ))  # as size of dataset will grow by NTASKS/2 
#  sbatch --export=ALL,N=${N_SIZE} --time="00:05:00" --nodes=1 --ntasks-per-node=${TASKS} archer2.slurm
#  wait 
#done

# now verify 

export TEST=/mnt/lustre/a2fs-work3/work/e609/e609/shr203/sharedmem/TEST/1/2/HDF5/0
for i in $(seq 2 7)
do 
  
  TASKS=$((2**${i}))
  N_SIZE=$(( (2**8)/(${TASKS}/2) ))  # as size of dataset will grow by NTASKS/2 
  FILESIZE=$((${N_SIZE}*8/2**20))
  NODES=1
  export VERIFY=/mnt/lustre/a2fs-work3/work/e609/e609/shr203/sharedmem/TEST/${NODES}/${TASKS}/HDF5/${FILESIZE} 
  echo "h5diff between ${VERIFY}/0.dat and ${TEST}/0.dat" 
  h5diff ${VERIFY}/0.dat ${TEST}/0.dat 
  echo "h5diff between ${VERIFY}/1.dat and ${TEST}/1.dat" 
  h5diff ${VERIFY}/1.dat ${TEST}/1.dat 
  echo "h5diff between ${VERIFY}/2.dat and ${TEST}/2.dat" 
  h5diff ${VERIFY}/2.dat ${TEST}/2.dat 
  export TEST=${VERIFY} 
   
done


