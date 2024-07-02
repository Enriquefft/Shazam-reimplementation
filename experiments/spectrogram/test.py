import numpy as np

# Define a 2D array and a 1D array
array_1d = np.array([[10], [20], [30]])

array_2d = np.array([[1, 2], [4, 5], [9, 8]])

# Multiply the 2D array with the 1D array
result = array_1d * array_2d
dim1 = [10, 20, 80, 100, 270, 240]
dim2 = [10, 80, 270]
dim3 = [10, 20]

matrix = np.zeros((2048, 2))
result = np.fft.rfft(matrix, axis=-2)
