import pickle
from sktime.datasets import load_from_tsfile_to_dataframe
import argparse

def get_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-m', '--model', type=str)
    parser.add_argument('-i', '--input', type=str)
    return parser

if __name__ == '__main__':
    
    parser = get_parser()
    args = parser.parse_args()
    
    model = args.model
    predict_file = args.input
    with open(model, 'rb') as f:
        clf = pickle.load(f)
    
    data_x = load_from_tsfile_to_dataframe(predict_file)
    value = clf.predict(data_x)[0]
    print(value)

    if value == '0':
        print("姿勢錯誤-手腕有轉")
    else: 
        print("姿勢正確-手腕沒轉")

