#!/bin/bash

agents=("reno" "westwood" "newreno" "areno" "westwoodnr")
resultdir="results-merged"
plotdir="plots-merged"

mkdir -p $resultdir
mkdir -p $plotdir

nodes=120
flows=80
packets=500
speed=15

y_a="congestion-window-(packets)"
y_b="ssthreshold-(packets)"

for agent in "${agents[@]}"; do
  echo "Running for $agent"
  echo -e "time,$y_a" > "${resultdir}/varying-cwnd_${agent}.csv"
  ns sim.tcl $nodes $flows $packets $speed $agent
  python3 traceparser2.py $resultdir $agent "cwnd"
done

for agent in "${agents[@]}"; do
  echo "Running for $agent"
  echo -e "time,$y_b" > "${resultdir}/varying-ssthresh_${agent}.csv"
  ns sim.tcl $nodes $flows $packets $speed $agent
  python3 traceparser2.py $resultdir $agent "ssthresh"
done

python3 plotter2.py $resultdir $plotdir
