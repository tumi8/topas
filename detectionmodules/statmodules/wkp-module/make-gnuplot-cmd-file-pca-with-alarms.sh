#!/bin/bash

# Creates a command file for gnuplot that plots pca-values and test-params information
# on the same graph. The following graphs will be generated:
# FOR EACH ENDPOINT-PCA_COMPONENT-COMBINATION:
#   FOR CUSUM:
#     g + N + alpha
#   FOR WKP:
#     pca-value + the three p-values


if [ $# != 1 ]
then
  echo "Usage: ./make-gnuplot-cmd-file-pca.sh gnuplot_directory"
  echo "  gnuplot_directory: directory which contains the output files"
fi

cd $1
# if output file already exists, overwrite it
[ -f graphs.gnuplot ] && rm graphs.gnuplot


echo set y2tics autofreq >> graphs.gnuplot

# entries for cusumparams @ pca
for datafile in *_pca_component.cusumparams.txt
do
  # set title of graph
  echo set title \'$datafile\' >> graphs.gnuplot
  # plot the graphs
  echo plot \'$datafile\' using 7:2 title \'g\' with lines, \'$datafile\' using 7:3 title \'N\' with lines,\
       \'$datafile\' using 7:1 title \'Value\' with lines, \'$datafile\' using 7:6 title \'alarms\' axes x1y2 with points >> graphs.gnuplot
  # pause to look at plot (RETURN to resume)
  echo pause -1 >> graphs.gnuplot
done

# entries for wkpparams @ pca
for datafile in *_pca_component.wkpparams.txt
do
  # WMW
  # set title of graph
  echo set title \'$datafile\' >> graphs.gnuplot
  # plot p(wmw)
  echo plot \'$datafile\' using 9:1 title \'Value\' with lines, \'$datafile\' using 9:2 title \'p\(wmw\)\' axes x1y2 with lines, \'$datafile\' using 9:8 title \'s-level\' axes x1y2 with lines >> graphs.gnuplot
  # pause to look at plot (RETURN to resume)
  echo pause -1 >> graphs.gnuplot
  # plot alarms(wmw)
  echo plot \'$datafile\' using 9:1 title \'Value\' with lines, \'$datafile\' using 9:3 title \'alarms\(wmw\)\' axes x1y2 with points >> graphs.gnuplot
  echo pause -1 >> graphs.gnuplot

  # KS
  echo set title \'$datafile\' >> graphs.gnuplot
  # plot p(ks)
  echo plot \'$datafile\' using 9:1 title \'Value\' with lines, \'$datafile\' using 9:4 title \'p\(ks\)\' axes x1y2 with lines, \'$datafile\' using 9:8 title \'s-level\' axes x1y2 with lines >> graphs.gnuplot
  echo pause -1 >> graphs.gnuplot
  # plot alarms(ks)
  echo plot \'$datafile\' using 9:1 title \'Value\' with lines, \'$datafile\' using 9:5 title \'alarms\(ks\)\' axes x1y2 with points >> graphs.gnuplot
  echo pause -1 >> graphs.gnuplot

  # PCS
  echo set title \'$datafile\' >> graphs.gnuplot
  # plot p(pcs)
  echo plot \'$datafile\' using 9:1 title \'Value\' with lines, \'$datafile\' using 9:6 title \'p\(pcs\)\' axes x1y2 with lines, \'$datafile\' using 9:8 title \'s-level\' axes x1y2 with lines >> graphs.gnuplot
  echo pause -1 >> graphs.gnuplot
  echo plot \'$datafile\' using 9:1 title \'Value\' with lines, \'$datafile\' using 9:7 title \'alarms\(pcs\)\' axes x1y2 with points >> graphs.gnuplot
  echo pause -1 >> graphs.gnuplot
done
