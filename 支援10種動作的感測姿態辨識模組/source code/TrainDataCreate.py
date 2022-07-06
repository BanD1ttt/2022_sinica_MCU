import os
import argparse

def get_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--inputDir', type=str)
    parser.add_argument('-pn', '--problemName', type=str)
    parser.add_argument('-o', '--output', type=str)
    return parser

if __name__ == '__main__':
    parser = get_parser()
    args = parser.parse_args()
    
    dir_path = args.inputDir
    problemName = args.problemName
    timeStamps = "false"
    missing="false"
    univariate="false"
    dimensions="6"
    equalLength="false"
    seriesLength="100"
    classLabel="true 0 1"

    output = args.output
    outputWriter = open(output, "w")

    outputWriter.write("@problemName "+problemName+"\n")
    outputWriter.write("@timeStamps "+timeStamps+"\n")
    outputWriter.write("@missing "+missing+"\n")
    outputWriter.write("@univariate "+univariate+"\n")
    outputWriter.write("@dimensions "+dimensions+"\n")
    outputWriter.write("@equalLength "+equalLength+"\n")
    outputWriter.write("@seriesLength "+seriesLength+"\n")
    outputWriter.write("@classLabel "+classLabel+"\n")
    outputWriter.write("@data\n")
    
    for filename in os.listdir(dir_path):
        ax_list = []
        ay_list = []
        az_list = []
        gx_list = []
        gy_list = []
        gz_list = []
        label = ''

        if filename.endswith(".csv"):
            with open(os.path.join(dir_path,filename),"r") as f:
                for line in f:
                    if "qw" not in line:           
                        line_list = line. rstrip("\n").split(",")
                        ax_list.append(line_list[5])
                        ay_list.append(line_list[6])
                        az_list.append(line_list[7])
                        gx_list.append(line_list[8])
                        gy_list.append(line_list[9])
                        gz_list.append(line_list[10])
        
            label = filename.split("_")[2].split(".")[0]
            ax = ','.join(ax_list)
            ay = ','.join(ay_list)
            az = ','.join(az_list)
            gx = ','.join(gx_list)
            gy = ','.join(gy_list)
            gz = ','.join(gz_list)

            all = ':'.join([ax,ay,az,gx,gy,gz,label])
            outputWriter.write(all+"\n")
                
    outputWriter.close()

