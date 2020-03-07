from scipy.io import loadmat
import ecg_plot

def load_ecg_from_mat(file_path):
    mat = loadmat(file_path)
    data = mat["data"]
    feature = data[0:12]
    return(feature)

test_ecg = load_ecg_from_mat('example_ecg.mat')


ecg_plot.plot_12(test_ecg, sample_rate = 500, columns = 2, title = '12-lead ECG')
ecg_plot.save_as_svg('example_ecg','')
