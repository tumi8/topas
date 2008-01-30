if [ $# != 1 ]
then
  echo "Usage: ./make-gnuplot-cmd-file.sh gnuplot_directory"
  echo "  gnuplot_directory: directory which contains the output files"
  exit
fi

cd $1
# if output file already exists, overwrite it
[ -f graphs.gnuplot ] && rm graphs.gnuplot

# entries for cusumparams @ metrics
echo set y2tics autofreq >> graphs.gnuplot
for METRICSFILE in *_metrics.txt
do
  echo "Processing $METRICSFILE ..."
  COLUMN=1
  TESTRUNCOLUMN=$(expr $(head -n 1 $METRICSFILE | wc -w) - 1)
  for METRIC in $(head -n 1 $METRICSFILE | sed 's/#//' | sed 's/Test-Run//')
  do
    CUSUMFILE=$(echo $METRICSFILE | sed "s/\_metrics\.txt/\.$METRIC\.cusumparams.txt/")
    WKPFILE=$(echo $CUSUMFILE | sed 's/cusum/wkp/')
    TITLE=$(echo $CUSUMFILE | sed 's/\.cusumparams.txt//')
    if [ -f $CUSUMFILE -a -f $WKPFILE ]; then
      echo "- $CUSUMFILE"
      echo "- $WKPFILE"
      echo set key outside >> graphs.gnuplot
      # plot all graphs
      echo set size 1, 1 >> graphs.gnuplot
      echo set origin 0, 0 >> graphs.gnuplot
      echo set title \'$TITLE\' >> graphs.gnuplot
      echo set xtics autofreq >> graphs.gnuplot
      echo set multiplot >> graphs.gnuplot
      echo set rmargin 30 >> graphs.gnuplot
      echo set lmargin 10 >> graphs.gnuplot
      # Value from METRICSFILE
      echo set origin 0, 0.64 >> graphs.gnuplot
      echo set size 1, 0.4 >> graphs.gnuplot
      echo set xrange [0:] writeback >> graphs.gnuplot
      echo set logscale y2 >> graphs.gnuplot
      echo "plot '$METRICSFILE' using $TESTRUNCOLUMN:$COLUMN title 'Value' with impulses, \
        '$METRICSFILE' using $TESTRUNCOLUMN:(\$$COLUMN>0 ? \$$COLUMN : (\$$COLUMN<0 ? -\$$COLUMN : 1)) title 'Value (log)' axes x1y2 with points" >> graphs.gnuplot
      # CUSUM parameters from CUSUMFILE
      #echo set title \'CUSUM parameters\' >> graphs.gnuplot
      echo unset logscale y2 >> graphs.gnuplot
      echo unset title >> graphs.gnuplot
      echo unset xtics >> graphs.gnuplot
      echo set tmargin -1 >> graphs.gnuplot
      echo set origin 0, 0.48 >> graphs.gnuplot
      echo set size 1, 0.17 >> graphs.gnuplot
      echo set xrange restore >> graphs.gnuplot
      echo plot \'$CUSUMFILE\' using 8:2 title \'g\' with lines, \
        \'$CUSUMFILE\' using 8:7 title \'alarms\' axes x1y2 with points, \
        \'$CUSUMFILE\' using 8:3 title \'N\' with lines >> graphs.gnuplot
      # WMW from WKPFILE
      #echo set title \'WMW parameters\' >> graphs.gnuplot
      echo set origin 0, 0.32 >> graphs.gnuplot
      echo set size 1, 0.17 >> graphs.gnuplot
      echo set xrange restore >> graphs.gnuplot
      echo plot \'$WKPFILE\' using 9:2 title \'p\(wmw\)\' with lines, \
        \'$WKPFILE\' using 9:3 title \'alarms\(wmw\)\' axes x1y2 with points >> graphs.gnuplot
      # KS from WKPFILE
      #echo set title \'KS parameters\' >> graphs.gnuplot
      echo set origin 0, 0.16 >> graphs.gnuplot
      echo set size 1, 0.17 >> graphs.gnuplot
      echo set xrange restore >> graphs.gnuplot
      echo plot \'$WKPFILE\' using 9:4 title \'p\(ks\)\' with lines, \
        \'$WKPFILE\' using 9:5 title \'alarms\(ks\)\' axes x1y2 with points >> graphs.gnuplot
      # PCS from WKPFILE
      #echo set title \'PCS parameters\' >> graphs.gnuplot
      echo set origin 0, 0 >> graphs.gnuplot
      echo set size 1, 0.17 >> graphs.gnuplot
      echo set xrange restore >> graphs.gnuplot
      echo plot \'$WKPFILE\' using 9:6 title \'p\(pcs\)\' with lines, \
        \'$WKPFILE\' using 9:7 title \'alarms\(pcs\)\' axes x1y2 with points >> graphs.gnuplot
      echo unset multiplot >> graphs.gnuplot
      # pause to look at plot (RETURN to resume)
      echo pause -1 >> graphs.gnuplot
    fi
    COLUMN=$(expr $COLUMN + 1)
  done
done
