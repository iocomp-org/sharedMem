TIMERS=("00:02:00" "00:05:00" "00:10:00" "00:15:00" "00:20:00" "00:30:00")
for i in $(seq 20 24) 
do 
  N_SIZE=$((2**${i})) 
  sbatch --export=ALL,N=${N_SIZE} --time=${TIMERS[${i}-20]} archer2.slurm
done 
