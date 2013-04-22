set xlabel "Number of states of automaton"
set ylabel "Time"
set terminal png
set output "plot_hkc.png"
#set zlabel "Time"
set autoscale
set grid
plot "evals_hkc.data" using 1:2 title "OCaml", "evals_hkc.data" using 1:3 title "VATA congr"
