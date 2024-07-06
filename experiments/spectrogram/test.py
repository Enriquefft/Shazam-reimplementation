import numpy as np


import math


from random import randint

from librosa import stft

arr1 = np.ones((5, 1))
mat1 = np.ones((5, 2)) * randint(10, 20)

print(arr1)
print(mat1)

print(arr1 * mat1)

matrx = np.array((100, 100))

np.fft.rfft(matrx, axis=-2)

np.transpose(np.fft.rfft(np.transpose(matrx)))


# Define a 2D array and a 1D array
# matrix1 = np.array([[1, 2, 3, 4], [4, 3, 2, 1]])
# matrix1 = np.array([[0, 0, 0, 0], [0, 0, 0, 0]])
# matrix1 = np.array([[1, -1, 1, -1]])
# matrix1 = np.array([[1], [-1], [1], [-1]])
#
# result1 = np.fft.rfft(matrix1, axis=-2rr
# print(result1)
#
#
# mat2 = np.transpose(matrix1)
# result2 = np.zeros((1, 3), dtype="complex128")
# print("shape", mat2.shape, mat2.size)
# for idx, row in enumerate(mat2):
#     new = np.fft.rfft(row)
#     print("row", new)
#     result2[idx] = new
#
# print(np.transpose(result2))
#
#
# print(np.transpose(np.fft.rfft(np.transpose(matrix1))))

# Result 1:
