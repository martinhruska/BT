set xlabel "Number of states of automaton"
set ylabel "Time"
#set zlabel "Time"
set terminal png
set output "plot_ca.png"
set autoscale
set grid
plot "evals_ca.data" using 1:2 title "VATA AC", "evals_ca.data" using 1:3 title "VATA Congr"
