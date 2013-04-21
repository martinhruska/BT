set xlabel "Number of states of automaton"
set ylabel "Number of states of automaton"
set zlabel "Time"
set autoscale
splot "tempPlot.data" using 1:2:3 title "pokus", "tempPlot.data" using 1:2:4 title "hokus"
