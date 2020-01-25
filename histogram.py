from sys import argv
import matplotlib.pyplot as plt

values = [int(e) for e in argv[-1].split(",") if e.isdigit()]
commands = argv[-2].replace("g","")
if commands.endswith(".py") or not commands:
    title_string = "Untouched Image Histogram"
else:
    title_string = f"Image Histogram after {commands}"
title_string += f"\nMean: {round(sum([i*v for i,v in enumerate(values)])/sum(values))}"
plt.title(title_string)
plt.plot(range(256), values)
plt.show()
