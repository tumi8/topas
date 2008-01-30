#!/bin/bash

# Creates a command file for gnuplot that plots metric-values and test-params information
# on the same graph. The following graphs will be generated:
# FOR EACH ENDPOINT-METRIC-COMBINATION:
#   FOR CUSUM:
#     g + N + alpha
#   FOR WKP:
#     metric-value + the three p-values


if [ $# != 1 ]
then
  echo "Usage: ./make-gnuplot-cmd-file.sh gnuplot_directory"
  echo "  gnuplot_directory: directory which contains the output files"
fi

cd $1
# if output file already exists, overwrite it
[ -f graphs.gnuplot ] && rm graphs.gnuplot

# entries for cusumparams @ metrics
echo set y2tics autofreq >> graphs.gnuplot
for datafile in *_metric.cusumparams.txt
do
  # set title of graph
  echo set title \'$datafile\' >> graphs.gnuplot
  # plot all graphs
  echo plot \'$datafile\' using 7:2 with lines title \'g\', \'$datafile\' using 7:3 with lines title \'N\',\
       \'$datafile\' using 7:6 with points title \'alarms\' axes x1y2 >> graphs.gnuplot
  # pause to look at plot (RETURN to resume)
  echo pause -1 >> graphs.gnuplot
done

# entries for wkpparams @ metrics
echo set y2tics autofreq >> graphs.gnuplot
for datafile in *_metric.wkpparams.txt
do
  # WMW
  # set title of graph
  echo set title \'$datafile\' >> graphs.gnuplot
  # plot p(wmw)
  echo plot \'$datafile\' using 9:2 title \'p\(wmw\)\' with lines, \'$datafile\' using 9:3 title \'alarms\(wmw\)\' axes x1y2 with points >> graphs.gnuplot
  # pause to look at plot (RETURN to resume)
  echo pause -1 >> graphs.gnuplot

  # KS
  echo set title \'$datafile\' >> graphs.gnuplot
  # plot p(ks)
  echo plot \'$datafile\' using 9:4 title \'p\(ks\)\' with lines, \'$datafile\' using 9:5 title \'alarms\(ks\)\' axes x1y2 with points >> graphs.gnuplot
  echo pause -1 >> graphs.gnuplot

  # PCS
  echo set title \'$datafile\' >> graphs.gnuplot
  # plot p(pcs)
  echo plot \'$datafile\' using 9:6 title \'p\(pcs\)\' with lines, \'$datafile\' using 9:7 title \'alarms\(pcs\)\' axes x1y2 with points >> graphs.gnuplot
  echo pause -1 >> graphs.gnuplot
done

