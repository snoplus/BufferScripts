#!/bin/bash

# This script handles pushing files to the nfs server on nlug.
# It watches several directories and scp's new files as they
# become available.  The code is based on the L2 client.

# Specifying where things are located--------------
## directories holding the data to ship
burstdir=/raid/data/burst
l1dir=/raid/data/l1
l2dir=/raid/data/l2
testdir=/raid/data/test
zdabdir=/raid/data/zdab
l2donedir=/home/trigger/l2done
## filelist files
LSl1=/home/trigger/BufferScripts/nlug/lsl1.txt
LSl2=/home/trigger/BufferScripts/nlug/lsl2.txt
LSburst=/home/trigger/BufferScripts/nlug/lsburst.txt
LStest=/home/trigger/BufferScripts/nlug/lstest.txt
FLl1=/home/trigger/BufferScripts/nlug/fll1.txt
FLl2=/home/trigger/BufferScripts/nlug/fll2.txt
FLburst=/home/trigger/BufferScripts/nlug/flburst.txt
FLtest=/home/trigger/BufferScripts/nlug/fltest.txt
## nlug stuff
NLUG=192.168.80.138
## builder directory
BUILDERDATA=/raid/data/l1
#--------------------------------------------------

# This function updates the text files holding the lists of files
# available on the buffer, and the lists of files we have already
# shipped to nlug.
updatefilelists(){
# Get the list of files available to ship
## L1
for i in $(find /raid/data/l1 -name *.zdab)
do
  if [ -f $i'_closed' ]
  then
    echo $i >> nlugtemp.txt
  fi
done
sort nlugtemp.txt > $LSl1
rm nlugtemp.txt
## L2
find $l2dir | grep .zdab > nlugtemp.txt
sort nlugtemp.txt > $LSl2
rm nlugtemp.txt
## Bursts
find $burstdir | grep .zdab > nlugtemp.txt
sort nlugtemp.txt > $LSburst
rm nlugtemp.txt
## Test
find $testdir | grep .zdab > nlugtemp.txt
sort nlugtemp.txt > $LStest
rm nlugtemp.txt

# If there is no $FL file, assume we want to ship everything available
if [ ! -f $FLl1 ]
then
  cp $LSl1 $FLl1
fi
if [ ! -f $FLl2 ]
then
  cp $LSl2 $FLl2
fi
if [ ! -f $FLburst ]
then
  cp $LSburst $FLburst
fi
if [ ! -f $FLtest ]
then
  cp $LStest $FLtest
fi

# Drop files from $FL no longer in $LS
comm -12 $LSl1 $FLl1 > nlugtemp.txt
sort nlugtemp.txt > $FLl1
rm nlugtemp.txt

comm -12 $LSl2 $FLl2 > nlugtemp.txt
sort nlugtemp.txt > $FLl2
rm nlugtemp.txt

comm -12 $LSburst $FLburst > nlugtemp.txt
sort nlugtemp.txt > $FLburst
rm nlugtemp.txt

comm -12 $LStest $FLtest > nlugtemp.txt
sort nlugtemp.txt > $FLtest
rm nlugtemp.txt
}

# This function checks whether all the subfiles of a run have been transferred
# and if so, pushes a file named by the runnumber to nlug.  This is for the 
# use of the nearline master program.
fullrun(){
  # First identify last file shipped
  LASTFILE=$(cat $FLl2 | tail -n 1)
  LASTRUN=$( echo $LASTFILE | awk 'BEGIN {FS="_"}{print $2}' )
  LASTSUBRUN=$( echo $LASTFILE | awk 'BEGIN {FS="_"}{print $3}' )
  LASTSUBRUN=$( echo $LASTSUBRUN | cut -d '.' -f1)
  GLASTSUBRUN=\'$LASTSUBRUN\.\'
  # Next, check whether builder is done
  BUILDERFILE=$(ls $BUILDERDATA | grep closed | grep $LASTRUN | grep $GLASTSUBRUN)
  if [ $(cat $BUILDERDATA/'SNOP_'$LASTRUN'_'$LASTSUBRUN'.zdab_closed' | grep END_RUN | wc -l) -ne 0 ]
  then
    # Builder is done 
    # Check whether other subfiles have been shipped
    let "LASTSUBRUN +=1"
    if [ $(cat $FLl2 | grep $LASTRUN | wc -l) -eq $LASTSUBRUN ]
    then
      # All files shipped - tell nlug
      ssh $NLUG touch $l2donedir/$LASTRUN
    else
      # Remember this for future
      echo $LASTSUBRUN > $l2donedir/$LASTRUN
    fi
  else
    # Not last file, check whether this completes a finished run anyway
    if [ -f $l2donedir/$LASTRUN ]
    then
      SUBFILES=$(echo $l2donedir/$LASTRUN)
      if [ $(cat FLl2 | grep $LASTRUN | wc -l) -eq $SUBFILES -1 ]
      then
        ssh $NLUG touch $l2donedir/$LASTRUN
      fi
    fi   
  fi
}

# MAIN
while true
do
  updatefilelists
  # L1
  if [ $(comm -23 $LSl1 $FLl1 | wc -l) -gt 0 ]
  then
    FILE=$(ls -t $(comm -23 $LSl1 $FLl1) | tail -n 1)
    scp $FILE $NLUG:$FILE 
    echo $FILE >> $FLl1
    sort $FLl1 > nlugtemp.txt
    mv nlugtemp.txt $FLl1
    ZCOUNT=$(ssh nlug "ls /raid/data/zdab" | wc -l)
    if [ $ZCOUNT -eq 10 ]
    then
      ZFILE=$(ssh nlug "ls -tr /raid/data/zdab" | tail -n 1)
      ssh nlug "rm /raid/data/zdab/$ZFILE"
    fi
    ZFILE=$(ssh nlug 'ls -tr /raid/data/l1' | tail -n 1)
    ssh nlug "cp /raid/data/l1/$ZFILE /raid/data/zdab/$ZFILE"
  fi
  # L2
  if [ $(comm -23 $LSl2 $FLl2 | wc -l) -gt 0 ]
  then
    FILE=$(ls -t $(comm -23 $LSl2 $FLl2) | tail -n 1)
    scp $FILE $NLUG:$FILE
    echo $FILE >> $FLl2
    sort $FLl2 > nlugtemp.txt
    mv nlugtemp.txt $FLl2
    fullrun
  fi
  # bursts
  if [ $(comm -23 $LSburst $FLburst | wc -l) -gt 0 ]
  then
    FILE=$(ls -t $(comm -23 $LSburst $FLburst) | tail -n 1)
    scp $FILE $NLUG:$FILE
    echo $FILE >> $FLburst
    sort $FLburst > nlugtemp.txt
    mv nlugtemp.txt $FLburst
  fi
  # test
  if [ $(comm -23 $LStest $FLtest | wc -l) -gt 0 ]
  then
    FILE=$(ls -t $(comm -23 $LStest $FLtest) | tail -n 1)
    scp $FILE $NLUG:$FILE
    echo $FILE >> $FLtest
    sort $FLtest > nlugtemp.txt
    mv nlugtemp.txt $FLtest
  fi
  # Don't sleep for too long to reduce latency in getting burst
  # files to nlug
  sleep 1
done
