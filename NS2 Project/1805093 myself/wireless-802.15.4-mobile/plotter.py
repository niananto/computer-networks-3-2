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

for line in out_file:
    split_list = line.split(",")
    x_param.append(split_list[0])
    for i in range(1, len(split_list)):
        y_param[metrics[i]].append(float(split_list[i]))

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