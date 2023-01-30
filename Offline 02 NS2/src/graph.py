import matplotlib.pyplot as plt 
  

def plotGraph(x, y, xlabel, ylabel, title):
    
    plt.scatter(x, y, color= "green",  marker= "o", s=30) 
    plt.plot(x, y) 

    plt.xlabel(xlabel) 
    plt.ylabel(ylabel) 
    plt.title(title)
    plt.grid()
    plt.savefig("../graph/" + title + ".jpg")

# Plots varying area
def plot1():
    x = [250, 500, 750, 1000, 1250]
    y = [4353.63, 5694.5, 8750.99, 5579.7, 9063.36]

    plotGraph(x=x, y=y, xlabel= "Area (Side of square-m)", ylabel="Throughput (bits/sec)", title="Area Vs Throughput")

def plot2():
    x = [250, 500, 750, 1000, 1250]
    y = [0.0376898, 0.0603136, 0.0316154, 0.0398074, 0.0605636]

    plotGraph(x=x, y=y, xlabel= "Area (Side of square-m)", ylabel="Average Delay (seconds)", title="Area Vs Average Delay")

def plot3():
    x = [250, 500, 750, 1000, 1250]
    y = [0.891026, 0.884176, 0.954809, 0.892206, 0.931943]

    plotGraph(x=x, y=y, xlabel= "Area (Side of square-m)", ylabel="Delivery Ratio", title="Area Vs Delivery Ratio")

def plot4():
    x = [250, 500, 750, 1000, 1250]
    y = [0.0128205, 0.0326264, 0.00115875, 0.013267, 0.0197585]

    plotGraph(x=x, y=y, xlabel= "Area (Side of square-m)", ylabel="Drop Ratio", title="Area Vs Drop Ratio")

# Plots varying number of nodes

def plot5():
    x = [20, 40, 60, 80, 100]
    y = [10889.5, 9451.18, 8857.81, 4281.5, 4273.06]

    plotGraph(x=x, y=y, xlabel= "Number of Nodes", ylabel="Throughput (bits/sec)", title="Number of Nodes Vs Throughput")

def plot6():
    x = [20, 40, 60, 80, 100]
    y = [0.0422982, 0.0340109, 0.0676806, 0.0484134, 0.0349433]

    plotGraph(x=x, y=y, xlabel= "Number of Nodes", ylabel="Average Delay (seconds)", title="Number of Nodes Vs Average Delay")

def plot7():
    x = [20, 40, 60, 80, 100]
    y = [0.95042, 0.959008, 0.939258, 0.861996, 0.90411]

    plotGraph(x=x, y=y, xlabel= "Number of Nodes", ylabel="Delivery Ratio", title="Number of Nodes Vs Delivery Ratio")

def plot8():
    x = [20, 40, 60, 80, 100]
    y = [0.012605, 0.00862999, 0.00224972, 0.0254777, 0.0251142]

    plotGraph(x=x, y=y, xlabel= "Number of Nodes", ylabel="Drop Ratio", title="Number of Nodes Vs Drop Ratio")

# Plots varying number of flows

def plot9():
    x = [10, 20, 30, 40, 50]
    y = [8972.53, 4320.89, 8969.47, 7739.62, 7781.74]

    plotGraph(x=x, y=y, xlabel= "Number of Flows", ylabel="Throughput (bits/sec)", title="Number of Flows Vs Throughput")

def plot10():
    x = [10, 20, 30, 40, 50]
    y = [0.0647032, 0.0289447, 0.0478332, 0.108463, 0.0611599]

    plotGraph(x=x, y=y, xlabel= "Number of Flows", ylabel="Average Delay (seconds)", title="Number of Flows Vs Average Delay")

def plot11():
    x = [10, 20, 30, 40, 50]
    y = [0.950282, 0.901099, 0.924092, 0.836689, 0.829689]

    plotGraph(x=x, y=y, xlabel= "Number of Flows", ylabel="Delivery Ratio", title="Number of Flows Vs Delivery Ratio")

def plot12():
    x = [10, 20, 30, 40, 50]
    y = [0.00451977, 0.00879121, 0.0253025, 0.0145414, 0.0402762]

    plotGraph(x=x, y=y, xlabel= "Number of Flows", ylabel="Drop Ratio", title="Number of Flows Vs Drop Ratio")


plot1()
plot2()
plot3()
plot4()
plot5()
plot6()
plot7()
plot8()
plot9()
plot10()
plot11()
plot12()