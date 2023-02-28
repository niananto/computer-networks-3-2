import sys

if len(sys.argv) != 4:
    print("Usage: python3 traceparser2.py <input-dir> <agent> <tracevar>")
    sys.exit(1)

# reading event entries from trace.tr
trace_file = open("trace2.tr", "r")

trace = dict()

i=0
for line in trace_file:
    i+=1
    # if i%5 != 0:
    #     continue

    split_list = line.split()
    
    if len(split_list) != 7:
        continue
    
    if split_list[5].__contains__(sys.argv[3]):
        # t = round(float(split_list[0]), 1)
        t = int(float(split_list[0]))
        trace[t] = split_list[6]

trace_file.close()


# writing parsed results
file_name = sys.argv[1] + "/varying-" + sys.argv[3] + "_" + sys.argv[2] + ".csv"
# print(file_name)
fout = open(file_name, "a")

for t, d in trace.items():
    # print(t, d)
    fout.write(str(t) + "," + str(d) + "\n")

fout.close()
