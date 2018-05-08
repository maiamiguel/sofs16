#!/bin/bash
#version 1.0

createDisk()
{	
	./createDisk /tmp/zzz $*
}

formatDisk_bin()
{
	./mksofs_bin /tmp/zzz $*
}

formatDisk()
{	
	./mksofs /tmp/zzz $*
}

testtool()
{
	./testtool /tmp/zzz $*
}

sofsmount()
{
	command mkdir /tmp/mnt
	./sofsmount $* /tmp/zzz /tmp/mnt
}

showBlock()
{
	./showblock /tmp/zzz $*
}

allocInode()
{	
	for i in $(seq 1 $1)
	do
		 echo -e "2\n$2\n0\n" | ./testtool /tmp/zzz -q 1
	done
}

allocCluster()
{
	for i in $(seq 1 $1)
	do
		 echo -e "4\n0\n" | ./testtool /tmp/zzz -q 1
	done	
}

freeInode()
{
	for i in $*
	do
		echo -e "3\n$i\n0\n" | ./testtool /tmp/zzz -q 1
	done
}

freeCluster()
{
	for i in $*
	do
		echo -e "5\n$i\n0\n" | ./testtool /tmp/zzz -q 1
	done
}

replenish()
{
	echo -e "6\n0\n" | ./testtool /tmp/zzz -q 1
}

deplete()
{
	echo -e "7\n0\n" | ./testtool /tmp/zzz -q 1
}

allocFilecluster()
{
	for i in $(seq 0 $2)
	do
		echo -e "9\n$1\n$i\n0\n" | ./testtool /tmp/zzz -q 1
	done
}

freeFilecluster()
{
	echo -e "10\n$1\n$2\n0\n" | ./testtool /tmp/zzz -q 1
}

addDirentry()
{	
	allocInode $2 2
	for i in $(seq 2 $2)
	do
		echo -e "15\n$1\nA$i\n$i\n0\n" | ./testtool /tmp/zzz -q 1 
	done
}

delDirentry()
{
	for i in $(seq 2 $2)
	do
		echo -e "17\n$1\nA$i\n0\n" | ./testtool /tmp/zzz -q 1
	done
}
