import matplotlib.pyplot as plt
import numpy as np

attempts = [1, 2, 3, 4, 5]
network_time = [5, 10, 15, 20, 25]
cpu_exc = [0.011, 0.041, 0.050, 0.079, 0.093]
cpu_stat = [0.015, 0.021, 0.026, 0.042, 0.038]

width = 0.35
ind = np.arange(len(attempts))

plt.figure(figsize=(8,5))
p1 = plt.bar(ind-width/2, network_time, width, label='Network time', color='lightgray')
p2 = plt.bar(ind-width/2, cpu_exc, width, bottom=network_time, label='CPU time (Exception)', color='red')
p3 = plt.bar(ind+width/2, network_time, width, label='_Network time', color='lightgray')
p4 = plt.bar(ind+width/2, cpu_stat, width, bottom=network_time, label='CPU time (Status)', color='blue')

plt.ylabel('Milliseconds (ms)')
plt.xlabel('Attempts')
plt.title('Wall time composition: Exception vs Status')
plt.xticks(ind, attempts)
plt.legend()
plt.tight_layout()
plt.show()
