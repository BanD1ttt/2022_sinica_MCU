import os
import argparse

# motion1 data length 42
# motion2 data length 77
# motion3 data length 91
# motion4 data length 151
# motion5 data length 107
# motion6 data length 77
# motion7 data length 132
# motion8 data length 103
# motion9 data length 149
# motion10 data length 80

def get_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=str)
    parser.add_argument('-o', '--output', type=str)
    parser.add_argument('-s', '--size', type=int)
    parser.add_argument('-pn','--problemName',type=str)
    return parser

if __name__ == '__main__':    
    parser = get_parser()
    args = parser.parse_args()

    input_f = args.input
    output_f = args.output

    problemName = args.problemName
    timeStamps = "false"
    missing="false"
    univariate="false"
    dimensions="6"
    equalLength="false"
    seriesLength="100"
    classLabel="false"

    output = open(output_f, "w")

    output.write("@problemName "+problemName+"\n")
    output.write("@timeStamps "+timeStamps+"\n")
    output.write("@missing "+missing+"\n")
    output.write("@univariate "+univariate+"\n")
    output.write("@dimensions "+dimensions+"\n")
    output.write("@equalLength "+equalLength+"\n")
    output.write("@seriesLength "+seriesLength+"\n")
    output.write("@classLabel "+classLabel+"\n")
    output.write("@data\n")
    
    ax_list = []
    ay_list = []
    az_list = []
    gx_list = []
    gy_list = []
    gz_list = []
    
    with open(input_f,"r") as f:
        for line in f:
            if "qw" not in line:           
                line_list = line. rstrip("\n").split(",")
                ax_list.append(line_list[5])
                ay_list.append(line_list[6])
                az_list.append(line_list[7])
                gx_list.append(line_list[8])
                gy_list.append(line_list[9])
                gz_list.append(line_list[10])

    size = len(ax_list)
    goal_size = args.size 

    if size>goal_size :
        ax_list = ax_list[0:goal_size]
        ay_list = ay_list[0:goal_size]
        az_list = az_list[0:goal_size]
        gx_list = gx_list[0:goal_size]
        gy_list = gy_list[0:goal_size]
        gz_list = gz_list[0:goal_size]
    else:
        while(size<goal_size):
            ax_list.append(ax_list[size-1])
            ay_list.append(ay_list[size-1])
            az_list.append(az_list[size-1])
            gx_list.append(gx_list[size-1])
            gy_list.append(gy_list[size-1])
            gz_list.append(gz_list[size-1])
            size = len(ax_list)

    
    ax = ','.join(ax_list)
    ay = ','.join(ay_list)
    az = ','.join(az_list)
    gx = ','.join(gx_list)
    gy = ','.join(gy_list)
    gz = ','.join(gz_list)

    all = ':'.join([ax,ay,az,gx,gy,gz])
    output.write(all+"\n")
                
    output.close()

