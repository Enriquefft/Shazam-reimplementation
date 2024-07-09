import numpy as np


data = np.array([[1], [-1], [1], [-1]])


def row_fft(x):
    t = np.transpose(x)
    fft = np.fft.rfft(t)
    response = np.transpose(fft)
    return response


print(row_fft(data))
