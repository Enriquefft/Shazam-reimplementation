import numpy as np


x = 4  # Replace with desired value for x
a = 1  # Replace with desired value for a
b = 2  # Replace with desired value for b

# Generate random arrays
mat1 = np.random.rand(x, a)
mat2 = np.random.rand(x, b)

# Print the arrays
print("mat1:\n", mat1, mat1.shape)
print("mat2:\n", mat2, mat2.shape)

print("mat1 * mat2:\n", mat1 * mat2, (mat1 * mat2).shape)
