import os
import argparse

def get_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--inputDir', type=str)
    return parser

if __name__ == '__main__':
    
    parser = get_parser()
    args = parser.parse_args()

    dir_path = args.inputDir

    opt_data_size = 0
    data_size_list = []
    last_data = []

    for filename in os.listdir(dir_path):

        if filename.endswith(".csv"):
            with open(os.path.join(dir_path,filename),"r") as f:
                data = f.readlines()
                data_size = len(data)
                
                data_size_list.append(data_size)
                last_data.append(data[data_size-1])

                if opt_data_size == 0 :
                    opt_data_size = data_size
                elif opt_data_size < data_size:
                    opt_data_size = data_size
                
    
    index = 0

    for filename in os.listdir(dir_path):
        
        if filename.endswith(".csv"):
            with open(os.path.join(dir_path,filename),"a") as f:
                data_size = data_size_list[index]

                if(data_size<opt_data_size):
                    write_times = opt_data_size - data_size
                    for i in range(write_times):
                        f.write("\n"+last_data[index])
                
                index += 1

