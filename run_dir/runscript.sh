## SBATCH parameters 
# time array parameter 
TIMES=("3:00:00" "03:00:00" "04:00:00" "05:00:00" "05:00:00") 

# qos
QOS="standard"

# job array parameter for averaging 
ARRAY="0-2"

## Program specific parameters 
# io start and end, for initialising the loop going over all the I/O back-ends 
IOSTART=0
IOEND=3

# size defined that initialises the N variable
LOCAL_SIZE=$((2**26))

# directory 
DIR="OUTPUT"

# iterate over increasing number of nodes 
for j in $(seq 0 2)
do

  NUM_NODES=$((2**$j))
  # TIME_DEF="01:00:00"
  TIME_DEF=${TIMES[${j}]} 
  sbatch --export=ALL,IOSTART=${IOSTART},IOEND=${IOEND},N=${LOCAL_SIZE},DIR=${DIR} --nodes=${NUM_NODES} --time=${TIME_DEF} --qos=${QOS} --array=${ARRAY} archer2.slurm
  echo "NODES ${NUM_NODES} LOCAL SIZE ${LOCAL_SIZE} TIME ${TIME_DEF} IO ${IOSTART} to ${IOEND}"

done 
