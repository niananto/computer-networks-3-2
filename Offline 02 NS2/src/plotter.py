import matplotlib.pyplot as plt 

# reading statistics from output.csv
out_file = open("output.csv", "r")
metrics = out_file.readline().split(",")

parameters = []
network_throughputs = []
end_to_end_avg_delays = []
packet_delivery_ratios = []
packet_drop_ratios = []

for line in out_file:
    split_list = line.split(",")
    parameters.append(int(split_list[0]))
    network_throughputs.append(float(split_list[1]))
    end_to_end_avg_delays.append(float(split_list[2]))
    packet_delivery_ratios.append(float(split_list[3]))
    packet_drop_ratios.append(float(split_list[4]))

out_file.close()

# plotting graphs
varying_parameter = metrics[0]

plt.plot(parameters, network_throughputs, marker="^", color="b")
plt.ylabel("Network Throughput (kbps)")
plt.xlabel(varying_parameter.replace("-", " "))
plt.grid()
title = "network-throughput_vs_" + varying_parameter
plt.savefig("../graph/" + title + ".jpg")
plt.clf()

plt.plot(parameters, end_to_end_avg_delays, marker="v", color="g")
plt.ylabel("End-to-End Avg Delay (ms)")
plt.xlabel(varying_parameter.replace("-", " "))
plt.grid()
title = "end-to-end-avg-delay_vs_" + varying_parameter
plt.savefig("../graph/" + title + ".jpg")
plt.clf()

plt.plot(parameters, packet_delivery_ratios, marker="<", color="r")
plt.ylabel("Packet Delivery Ratio")
plt.xlabel(varying_parameter.replace("-", " "))
plt.grid()
title = "packet-delivery-ratio_vs_" + varying_parameter
plt.savefig("../graph/" + title + ".jpg")
plt.clf()

plt.plot(parameters, packet_drop_ratios, marker=">", color="y")
plt.ylabel("Packet Drop Ratio")
plt.xlabel(varying_parameter.replace("-", " "))
plt.grid()
title = "packet-drop-ratio_vs_" + varying_parameter
plt.savefig("../graph/" + title + ".jpg")
plt.clf()