set xlabel "Number of states of automaton"
set ylabel "Number of states of automaton"
#set zlabel "Time"
set autoscale
set yrange[0:0.04]
set grid
plot "hknt-1.0/tempPlot.data" using 1:2 title "pokus", "hknt-1.0/tempPlot.data" using 1:3 title "hokus"
