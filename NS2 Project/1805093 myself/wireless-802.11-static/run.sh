#!/bin/bash

# agent="reno"
agent="areno"
# agent="westwood"
resultdir="results-"$agent
plotdir="plots-"$agent

mkdir -p $resultdir
mkdir -p $plotdir

nodes=120
flows=80
packets=500
coverage=250

y_a="network-throughput-(kbps)"
y_b="end-to-end-avg-delay-(sec)"
y_c="packet-delivery-ratio-(%)"
y_d="packet-drop-ratio-(%)"
y_e="energy-consumption-(joule)"

echo -e "nodes, $y_a, $y_b, $y_c, $y_d, $y_e" > "$resultdir/varying-nodes.csv"
for i in {1..5}; do
  echo "Running for $(($i*20)) nodes"
  ns sim.tcl $(($i*20)) $flows $packets $coverage $agent
  python3 traceparser.py "nodes" $(($i*20)) $resultdir
  python3 plotter.py "nodes" $resultdir $plotdir
done

echo -e "flows, $y_a, $y_b, $y_c, $y_d, $y_e" > "$resultdir/varying-flows.csv"
for i in {1..5}; do
  echo "Running for $(($i*10)) flows"
  ns sim.tcl $nodes $(($i*10)) $packets $coverage $agent
  python3 traceparser.py "flows" $(($i*10)) $resultdir
  python3 plotter.py "flows" $resultdir $plotdir
done

echo -e "packets, $y_a, $y_b, $y_c, $y_d, $y_e" > "$resultdir/varying-packets.csv"
for i in {1..5}; do
  echo "Running for $(($i*100)) packets"
  ns sim.tcl $nodes $flows $(($i*100)) $coverage $agent
  python3 traceparser.py "packets" $(($i*100)) $resultdir
  python3 plotter.py "packets" $resultdir $plotdir
done

echo -e "coverage, $y_a, $y_b, $y_c, $y_d, $y_e" > "$resultdir/varying-coverage.csv"
for i in {1..5}; do
  echo "Running for $(($i*250)) coverage"
  ns sim.tcl $nodes $flows $packets $(($i*5)) $agent
  python3 traceparser.py "coverage" $(($i*5)) $resultdir
  python3 plotter.py "coverage" $resultdir $plotdir
done