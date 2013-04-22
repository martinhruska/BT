set xlabel "Number of states of automaton"
set ylabel "Time"
#set zlabel "Time"
set terminal png
set output "plot_ac.png"
set autoscale
set grid
plot "evals_ac.data" using 1:2 title "VATA Congr", "evals_ac.data" using 1:3 title "VATA Congr"
