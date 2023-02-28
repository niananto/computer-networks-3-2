import sys

if len(sys.argv) != 4:
    print("Usage: python3 traceparser.py <varying-param> <value> <input-dir>")
    sys.exit(1)

# reading event entries from trace.tr
trace_file = open("trace.tr", "r")

recevied_bytes = 0
sent_packets = 0
received_packets = 0
dropped_packets = 0

total_delay = 0
start_time = None
end_time = 0

header_bytes = 20

energy_per_node = 12 # must be same as in tcl
number_of_nodes = 500 # this is the max
energy_list = [energy_per_node]*number_of_nodes

sent_time = dict()

for line in trace_file:
    split_list = line.split()
    event_type = split_list[0]
    event_start_time = None

    if event_type == "N":
        event_start_time = float(split_list[2])
        node_id = int(split_list[4])
        energy = float(split_list[6])

        if start_time is None: start_time = event_start_time
        if end_time < event_start_time: end_time = event_start_time

        energy_list[node_id] = energy

    elif event_type == "r" or event_type == "s" or event_type == "D":
        event_start_time = float(split_list[1])
        layer_type = split_list[3]
        packet_id = split_list[5]
        packet_type = split_list[6]
        packet_size = int(split_list[7])

        if start_time is None: start_time = event_start_time
        if end_time < event_start_time: end_time = event_start_time

        if layer_type=="AGT" and (packet_type=="tcp" or packet_type=="exp" or packet_type=="cbr"):
            if event_type == "s":
                sent_time[packet_id] = event_start_time
                sent_packets += 1
            elif event_type == "r":
                delay = event_start_time - float(sent_time[packet_id])
                if delay < 0:
                    print("error : negative delay")
                else:
                  total_delay += delay
                  recevied_bytes += packet_size-header_bytes
                  received_packets += 1

        if (packet_type=="tcp" or packet_type=="exp" or packet_type=="cbr") and event_type == "D":
            dropped_packets += 1
        
trace_file.close()

sim_time = end_time - start_time
if sim_time == 0:
    print("error : simulation time is zero")
    sys.exit(1)
if sent_packets == 0:
    print("error : sent packets is zero")
    sys.exit(1)
if received_packets == 0:
    print("error : received packets is zero")
    sys.exit(1)

total_consumed_energy = energy_per_node*number_of_nodes - sum(energy_list)

# writing parsed results
fout = open(sys.argv[3] + "/varying-"+sys.argv[1]+".csv", "a")

fout.write(sys.argv[2]
          + "," + str((recevied_bytes*8)/(sim_time*1000))
          + "," + str(total_delay/received_packets)
          + "," + str((received_packets*1.0)/sent_packets*100)
          + "," + str((dropped_packets*1.0)/sent_packets*100)
          + "," + str(total_consumed_energy) + "\n")

fout.close()
