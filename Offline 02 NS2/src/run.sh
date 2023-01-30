#!/bin/bash

touch output.csv

default_size=500
default_nodes=40
default_flows=20

echo -e "size-(m^2),network-throughput-(kbps),end-to-end-avg-delay-(sec),packet-delivery-ratio-(%),packet-drop-ratio-(%)" > output.csv

for((i=0; i<5; i++)); do
    size=`expr 250 + $i \* 250`
    echo -n "$size," >> output.csv

    echo -e "1805093.tcl: running with $size $default_nodes $default_flows\n"
    ns 1805093.tcl $size $default_nodes $default_flows
    echo -e "\nparser_1.py: running\n"
    python3 parser_1.py
done

echo -e "plotter.py: running\n"
python3 plotter.py


echo -e "nodes,network-throughput-(kbps),end-to-end-avg-delay-(sec),packet-delivery-ratio-(%),packet-drop-ratio-(%)" > output.csv

for((i=0; i<5; i++)); do
    nodes=`expr 20 + $i \* 20`
    echo -n "$nodes," >> output.csv

    echo -e "1805093.tcl: running with $default_size $nodes $default_flows\n"
    ns 1805093.tcl $default_size $nodes $default_flows
    echo -e "\nparser_1.py: running\n"
    python3 parser_1.py
done

echo -e "plotter.py: running\n"
python3 plotter.py


echo -e "flows,network-throughput-(kbps),end-to-end-avg-delay-(sec),packet-delivery-ratio-(%),packet-drop-ratio-(%)" > output.csv

for((i=0; i<5; i++)); do
    flows=`expr 10 + $i \* 10`
    echo -n "$flows," >> output.csv

    echo -e "1805093.tcl: running with $default_size $default_nodes $flows\n"
    ns 1805093.tcl $default_size $default_nodes $flows
    echo -e "\nparser_1.py: running\n"
    python3 parser_1.py
done

echo -e "plotter.py: running\n"
python3 plotter.py


rm animation.nam output.csv trace.tr