import glob;
import serial;
from datetime import datetime;
import csv;


def main():

    port = glob.glob("/dev/tty.usbmodem*")[0] #glob finds files in a pattern based way, since rasp pi is the only usbmodem plugged in, it will find it first. [0]
    ser = serial.Serial(port, 115200, timeout=1) #Serial reader, read 115200 bits/s, timeout=1 means if line doesnt come for 1s max return empty and don't freeze

    headers = ["time_ms", "altitude_m", "pressure_pa", "temp_c"]
    filename = "log_" + datetime.now().strftime("%Y-%m-%d_%H-%M") + ".csv"

    #with means close after the indented code.
    with open(filename, "w", newline="", encoding="utf-8") as file: #create and write to file
        writer = csv.writer(file)
        writer.writerow(headers)
        try:
            while(True):
                line = ser.readline().decode(errors="ignore").strip() #decode turns bytes read into letters strip so no \n or spaces, errors (garbled characters) ignored.
                if line.count(",") == 3: #no empty data
                    line = line.split(",") #line is now an array of strings
                    writer.writerow(line)
                    file.flush() #save the file each line
                    print(line)
        except KeyboardInterrupt:
            print("Saved") #ctrl c saves


    

if __name__ == "__main__":
    main()