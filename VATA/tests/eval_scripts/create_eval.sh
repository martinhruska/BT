#! /bin/bash
EVALSCRIPTS=$1
EDITED="$3_edited"

sed -e"s/-/$4/g" $3 > $EDITED;
python $EVALSCRIPTS/transformResult.py $2> hkc_transformed;
python $EVALSCRIPTS/toGnuPlot.py -a hkc_transformed $EDITED > evals_ac.data;
python $EVALSCRIPTS/toGnuPlot.py -c hkc_transformed $EDITED > evals_ca.data;
python $EVALSCRIPTS/toGnuPlot.py -o hkc_transformed $EDITED > evals_hkc.data;
python $EVALSCRIPTS/winnerStats.py evals_ac.data > winner_ac;
python $EVALSCRIPTS/winnerStats.py evals_ca.data > winner_ca;
python $EVALSCRIPTS/winnerStats.py evals_hkc.data > winner_hkc;
cp $EVALSCRIPTS/plot_ac.plt ./;
cp $EVALSCRIPTS/plot_ca.plt ./;
cp $EVALSCRIPTS/plot_hkc.plt ./;
gnuplot *.plt;
