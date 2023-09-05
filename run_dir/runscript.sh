## SBATCH parameters 
# time array parameter 
TIMES=("3:00:00" "03:00:00" "04:00:00" "05:00:00" "05:00:00" "05:00:00" "06:00:00" "06:00:00") 
#TIME_CUSTOM="00:10:00"

# qos
QOS="standard"

# job array parameter for averaging 
ARRAY="0-2"

## Program specific parameters 
# io start and end, for initialising the loop going over all the I/O back-ends 
IOSTART=0
IOEND=3

# size defined that initialises the N variable
LOCAL_SIZE=$((2**24))

# directory 
DIR="OUTPUT"

# Node start and end integer as power of 2s
NODE_START = 0 
NODE_END = 3

# iterate over increasing number of nodes 
for j in $(seq ${NODE_START} ${NODE_END})
do
  # set time to custom time if defined, otherwise set it to array 
  if [[ -n ${TIME_CUSTOM} ]];
  then 
    TIME_DEF=${TIME_CUSTOM}
  else 
    TIME_DEF=${TIMES[${j}]} 
  fi 

  NUM_NODES=$((2**$j))
  sbatch --export=ALL,IOSTART=${IOSTART},IOEND=${IOEND},N=${LOCAL_SIZE},DIR=${DIR} --nodes=${NUM_NODES} --time=${TIME_DEF} --qos=${QOS} --array=${ARRAY} archer2.slurm
  echo "NODES ${NUM_NODES} LOCAL SIZE ${LOCAL_SIZE} TIME ${TIME_DEF} IO ${IOSTART} to ${IOEND}"

done 
