#!/bin/bash

# Creates a command file for gnuplot that plots metric-values
# The following graphs will be generated:
# FOR EACH ENDPOINT-METRIC-COMBINATION:
#     metric-values


if [ $# != 1 ]
then
  echo "Usage: ./make-gnuplot-cmd-file-metrics.sh gnuplot_directory"
  echo "  gnuplot_directory: directory which contains the output files"
fi

cd $1
# if output file already exists, overwrite it
[ -f metrics.gnuplot ] && rm metrics.gnuplot

# print out metrics
for datafile in *_metrics.txt
do
  echo set title \'$datafile\' >> metrics.gnuplot
#   echo plot \'$datafile\' using 7:1 title \'packets_in\' axes x1y1 with lines >> metrics.gnuplot
#   echo pause -1 >> metrics.gnuplot
#   echo plot \'$datafile\' using 7:2 title \'packets_out\' with lines >> metrics.gnuplot
#   echo pause -1 >> metrics.gnuplot
#   echo plot \'$datafile\' using 7:3 title \'bytes_in\' with lines >> metrics.gnuplot
#   echo pause -1 >> metrics.gnuplot
#   echo plot  \'$datafile\' using 7:4 title \'bytes_out\' with lines >> metrics.gnuplot
#   echo pause -1 >> metrics.gnuplot
#   echo plot \'$datafile\' using 7:5 title \'records_in\' with lines >> metrics.gnuplot
#   echo pause -1 >> metrics.gnuplot
#   echo plot \'$datafile\' using 7:6 title \'records_out\' with lines >> metrics.gnuplot
#   echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:1 title \'bytes_in_per_packet_in\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:2 title \'bytes_out_per_packet_out\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:3 title \'packets_in/record_in\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:4 title \'packets_out/record_out\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:5 title \'bytes_in/record_in\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:6 title \'bytes_out/record_out\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:7 title \'packets_out-packets_in\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:8 title \'bytes_out-bytes_in\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:9 title \'records_out-records_in\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:10 title \'packets_in\(t\)-packets_in\(t-1\)\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:11 title \'packets_out\(t\)-packets_out\(t-1\)\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:12 title \'bytes_in\(t\)-bytes_in\(t-1\)\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:13 title \'bytes_out\(t\)-bytes_out\(t-1\)\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:14 title \'records_in\(t\)-records_in\(t-1\)\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
  echo plot \'$datafile\' using 16:15 title \'records_out\(t\)-records_out\(t-1\)\' with lines >> metrics.gnuplot
  echo pause -1 >> metrics.gnuplot
done
