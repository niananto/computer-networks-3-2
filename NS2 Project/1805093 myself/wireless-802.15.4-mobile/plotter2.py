import matplotlib.pyplot as plt 
import sys

if len(sys.argv) != 3:
    print("Usage: python3 plotter.py <input-dir> <output-dir>")
    sys.exit(1)

x_param = {}
y_param = {}
metrics = ["cwnd", "ssthresh"]
agents = ["reno", "westwood", "newreno", "areno", "westwoodnr"]
for agent in agents:
  x_param[agent] = {}
  y_param[agent] = {}
  for metric in metrics:
    # reading statistics from output.csv
    out_file = open(sys.argv[1] + "/varying-" + metric + "_" + agent + ".csv", "r")

    out_file.readline().split(",")
    x_param[agent][metric] = []
    y_param[agent][metric] = []

    for line in out_file:
        split_list = line.split(",")
        x_param[agent][metric].append(float(split_list[0]))
        y_param[agent][metric].append(float(split_list[1]))

    out_file.close()

# plotting graphs
varying_parameter = "time"
# markers = ["^", "v", "<", ">", "o"]
colors = ["b", "r", "m", "g", "y", "c", "k"]

# plot merged graph for one metric
for metric in metrics:
    # get the minimum length of x_param
    min_len = min([len(x_param[agent][metric]) for agent in agents])
    for j in range(0, len(agents)):
        plt.plot(x_param[agents[j]][metric][:min_len], y_param[agents[j]][metric][:min_len], label=agents[j], color=colors[j])

    plt.ylabel(metric.replace("-", " "))
    plt.xlabel(varying_parameter)
    # omit x axis grid
    plt.grid(axis='y')
    # omit x axis ticks
    plt.xticks([])
    plt.legend(loc='upper right', shadow=False)
    title = metric + "_vs_" + varying_parameter
    plt.savefig(sys.argv[2] + "/" + title + ".jpg")
    plt.clf()

# plot merged graph for one agent
for agent in agents:
    # get the minimum length of x_param
    min_len = min([len(x_param[agent][m]) for m in metrics])
    for j in range(0, len(metrics)):
        plt.plot(x_param[agent][metrics[j]][:min_len], y_param[agent][metrics[j]][:min_len], label=metrics[j], color=colors[j])
      
    plt.ylabel("")
    plt.xlabel(varying_parameter)
    plt.grid(axis='y')
    plt.xticks([])
    plt.legend(loc='upper right', shadow=False)
    title = agent + "-merged"
    plt.savefig(sys.argv[2] + "/" + title + ".jpg")
    plt.clf()