#!/bin/bash

# This script handles pushing files to the nfs server on nlug.
# It watches several directories and scp's new files as they
# become available.  The code is based on the L2 client.

# Specifying where things are located--------------
## directories holding the data to ship
PCAdir=/home/trigger/PCA
ECAdir=/home/trigger/ECA
burstdir=/raid/data/burst
zdabdir=/raid/data/l1
## filelist files
LSpca=/home/trigger/BufferScripts/lspca.txt
LSeca=/home/trigger/BufferScripts/lseca.txt
LSburst=/home/trigger/BufferScripts/lsburst.txt
LSzdab=/home/trigger/BufferScripts/lszdab.txt
FLpca=/home/trigger/BufferScripts/flpca.txt
FLeca=/home/trigger/BufferScripts/fleca.txt
FLburst=/home/trigger/BufferScripts/flburst.txt
FLzdab=/home/trigger/BufferScripts/flzdab.txt
## nlug stuff
NLUG=192.168.80.138
PCAdest=/raid/data/pca
ECAdest=/raid/data/eca
zdabdest=/raid/data/zdab
burstdest=/raid/data/burst
#--------------------------------------------------

# This function updates the text files holding the lists of files
# available on the buffer, and the lists of files we have already
# shipped to nlug.
updatefilelists(){
# Get the list of files available to ship
## PCA
ls $PCAdir/*.zdab > temp.txt
sort temp.txt > $LSpca
rm temp.txt
## ECA
ls $ECAdir/*.zdab > temp.txt
sort temp.txt > $LSeca
rm temp.txt
## Bursts
ls $burstdir/*.zdab > temp.txt
sort temp.txt > $LSburst
rm temp.txt
## zdabs
ls $zdabdir/*.zdab > temp.txt
sort temp.txt > $LSzdab
rm temp.txt

# If there is no $FL file, assume we want to ship everything available
if [ ! -f $FLpca ]
then
  cp $LSpca $FLpca
fi
if [ ! -f $FLeca ]
then
  cp $LSeca $FLeca
fi
if [ ! -f $FLburst ]
then
  cp $LSburst $FLburst
fi
if [ ! -f $FLzdab ]
then
  cp $LSzdab $FLzdab
fi

# Drop files from $FL no longer in $LS
comm -12 $LSpca $FLpca > temp.txt
sort temp.txt > $FLpca
rm temp.txt

comm -12 $LSeca $FLeca > temp.txt
sort temp.txt > $FLeca
rm temp.txt

comm -12 $LSburst $FLburst > temp.txt
sort temp.txt > $FLburst
rm temp.txt

comm -12 $LSzdab $FLzdab > temp.txt
sort temp.txt > $FLzdab
rm temp.txt
}

# MAIN
while true
do
  updatefilelists
  # PCA
  if [ $(comm -23 $LSpca $FLpca | wc -l) -gt 0 ]
  then
    FILE=$(ls -lt $(comm -23 $LSpca $FLpca) | head -n 1)
    scp $FILE $NLUG:$PCAdest/$FILE 
    # This is only a link so we can rm it once it is copied
    rm $FILE
  fi
  # ECA
  if [ $(comm -23 $LSeca $FLeca | wc -l) -gt 0 ]
  then
    FILE=$(ls -lt $(comm -23 $LSeca $FLeca) | head -n 1)
    scp $FILE $NLUG:$ECAdest/$FILE
    # This is only a link so we can rm it once it is copied
    rm $FILE
  fi
  # bursts
  if [ $(comm -23 $Lsburst $FLburst | wc -l) -gt 0 ]
  then
    FILE=$(ls -lt $(comm -23 $LSburst $FLburst) | head -n 1)
    scp $FILE $NLUG:$burstdest/$FILE
  fi
  # zdab
  if [ $(comm -23 $LSzdab $FLzdab | wc -l) -gt 0 ]
  then
    FILE=$(ls -lt $(comm -23 $LSzdab $FLzdab) | head -n 1)
    scp $FILE $NLUG:$zdabdest/$FILE
  fi
done
