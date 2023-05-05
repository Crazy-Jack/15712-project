import matplotlib.pyplot as plt
import numpy as np

ypoints = np.array([10, 20, 30, 40])
ypoints2 = np.array([30, 30, 30, 30])

plt.xlabel("Kappa")
plt.ylabel("Time (s)")
plt.plot(ypoints, color = 'r', label="OSMR")
plt.plot(ypoints2, color = 'b', label="PBFT")
plt.savefig("expected.png")