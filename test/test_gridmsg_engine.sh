#!/bin/bash

if [ `pwd` != "/home/raygan/Cosms/Dboy/Snafu" ]
then
	echo "must run from /home/raygan/Cosms/Dboy/Snafu directory"
	exit
fi

CMD=./out/test_gridmsg_engine
$CMD "griddb:rootstr get snafu://bangfish.local"
$CMD "griddb:dcel new"
$CMD "griddb:dcel source 12 file://bangfish.local"

$CMD "griddb:axpa new"
$CMD "griddb:axpa set 101 dcel 12"
$CMD "griddb:rootstr set snafu://bangfish.local 101"
$CMD "griduser:axpa lookup 101 home"
$CMD "griduser:axpa lookup 102 redboy"
$CMD "griduser:axpa lookup 103 project"
$CMD "griddb:axpa get 103 dcel"
$CMD "griddb:dcel get 14 server"
$CMD "griduser:server init preserve_local_files 14 server %{svr}"

$CMD "griduser:server restore preserve_local_files 14"
$CMD "griddb:dcel set 14 server 100"
$CMD "griduser:axpa lookup 103 music" 
