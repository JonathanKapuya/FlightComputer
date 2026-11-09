import sys
import csv
import matplotlib.pyplot as plt

def main():

    filename = sys.argv[1] #call python3 plot.py log.csv means log.csv is argv[1]

    times = []
    alts = []
    with open (filename) as f:
        reader = csv.DictReader(f)
        for row in reader:
            times.append((float(row["time_ms"])/1000))
            alts.append(float(row["altitude_m"]))

    start = times[0]
    for i in range(len(times)):
        times[i] -= start

    plt.plot(times, alts)
    plt.xlabel("Time (s)")
    plt.ylabel("Altitude (m)")
    plt.grid(True)
    plt.savefig("Altitude.png", dpi=150)
    plt.show()




if __name__ == "__main__":
    main()