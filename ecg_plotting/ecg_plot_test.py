from scipy.io import loadmat
import ecg_plot
import csv
import numpy as np

with open('ECG.csv', 'r') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    test_ecg = list(csv_reader)
    a = [*zip(*test_ecg)]

def load_ecg_from_mat(file_path):
    mat = loadmat(file_path)
    data = mat["data"]
    feature = data[0:12]
    return(feature)

test_ecg = load_ecg_from_mat('example_ecg.mat')

print(type(test_ecg[0][0]))
print(type(a))
npa = np.asarray(a, dtype=np.float64)

ecg_plot.plot_12(npa, sample_rate = 500, columns = 2, title = '12-lead ECG')
ecg_plot.save_as_svg('example_ecg','')
