import matplotlib.pyplot as plt 
import sys

if len(sys.argv) != 4:
    print("Usage: python3 plotter.py <varying-param> <input-dir> <output-dir>")
    sys.exit(1)

# reading statistics from output.csv
out_file = open(sys.argv[2] + "/varying-" + sys.argv[1] + ".csv", "r")
metrics = out_file.readline().split(",")

x_param = []
y_param = {}
for metric in metrics[1:]:
    y_param[metric] = []

# network_throughputs = []
# end_to_end_avg_delays = []
# packet_delivery_ratios = []
# packet_drop_ratios = []
# energy_consumptions = []

for line in out_file:
    split_list = line.split(",")
    x_param.append(int(split_list[0]))
    for i in range(1, len(split_list)):
        y_param[metrics[i]].append(float(split_list[i]))
    # network_throughputs.append(float(split_list[1]))
    # end_to_end_avg_delays.append(float(split_list[2]))
    # packet_delivery_ratios.append(float(split_list[3]))
    # packet_drop_ratios.append(float(split_list[4]))
    # energy_consumptions.append(float(split_list[5]))

out_file.close()

# plotting graphs
varying_parameter = metrics[0]
markers = ["^", "v", "<", ">", "o"]
colors = ["b", "g", "r", "y", "m"]

for i in range(1, len(metrics)):
    plt.plot(x_param, y_param[metrics[i]], marker=markers[i-1], color=colors[i-1])
    plt.ylabel(metrics[i].replace("-", " "))
    plt.xlabel(varying_parameter.replace("-", " "))
    plt.grid()
    title = metrics[i] + "_vs_" + varying_parameter
    plt.savefig(sys.argv[3] + "/" + title + ".jpg")
    plt.clf()


# plt.plot(x_param, network_throughputs, marker="^", color="b")
# plt.ylabel("Network Throughput (kbps)")
# plt.xlabel(varying_parameter.replace("-", " "))
# plt.grid()
# title = "network-throughput_vs_" + varying_parameter
# plt.savefig("plots/" + title + ".jpg")
# plt.clf()

# plt.plot(x_param, end_to_end_avg_delays, marker="v", color="g")
# plt.ylabel("End-to-End Avg Delay (ms)")
# plt.xlabel(varying_parameter.replace("-", " "))
# plt.grid()
# title = "end-to-end-avg-delay_vs_" + varying_parameter
# plt.savefig("plots/" + title + ".jpg")
# plt.clf()

# plt.plot(x_param, packet_delivery_ratios, marker="<", color="r")
# plt.ylabel("Packet Delivery Ratio")
# plt.xlabel(varying_parameter.replace("-", " "))
# plt.grid()
# title = "packet-delivery-ratio_vs_" + varying_parameter
# plt.savefig("plots/" + title + ".jpg")
# plt.clf()

# plt.plot(x_param, packet_drop_ratios, marker=">", color="y")
# plt.ylabel("Packet Drop Ratio")
# plt.xlabel(varying_parameter.replace("-", " "))
# plt.grid()
# title = "packet-drop-ratio_vs_" + varying_parameter
# plt.savefig("plots/" + title + ".jpg")
# plt.clf()

# plt.plot(x_param, energy_consumptions, marker="o", color="m")
# plt.ylabel("Energy Consumption (J)")
# plt.xlabel(varying_parameter.replace("-", " "))
# plt.grid()
# title = "energy-consumption_vs_" + varying_parameter
# plt.savefig("plots/" + title + ".jpg")
# plt.clf()