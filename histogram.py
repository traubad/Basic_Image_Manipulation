from sys import argv
from statistics import stdev,variance
import matplotlib.pyplot as plt

values = [int(e) for e in argv[-1].split(",") if e.isdigit()]
commands = argv[-2].replace("g","")
if commands.endswith(".py") or not commands:
    title_string = "Untouched Image Histogram"
else:
    title_string = f"Image Histogram after {commands}"
actuals = [i for i,v in enumerate(values) for j in range(v)]
mean = round(sum(actuals)/len(actuals))
title_string += f"\nVariance: {round(variance(actuals))}         Standard Deviation: {round(stdev(actuals))}"
plt.title(title_string)
plt.xlabel("Color Value")
plt.ylabel("Frequency")
plt.plot(range(256), values, "b")
plt.plot(mean,values[mean], "ro")
plt.legend(["Grayscale Values",f"Mean ({mean})"])
plt.show()
