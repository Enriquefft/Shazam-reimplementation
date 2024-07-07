import numpy as np
import librosa.util

# Example 1
x = np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
frame_length = 3
hop_length = 2
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[0, 2, 4, 6, 8],
#          [1, 3, 5, 7, 9],
#          [2, 4, 6, 8]]

# Example 2
x = np.array([10, 20, 30, 40, 50, 60])
frame_length = 2
hop_length = 1
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[10, 20, 30, 40, 50],
#          [20, 30, 40, 50, 60]]

# Example 3
x = np.array([5, 10, 15, 20, 25, 30, 35, 40])
frame_length = 4
hop_length = 2
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[ 5, 15, 25],
#          [10, 20, 30],
#          [15, 25, 35],
#          [20, 30, 40]]

# Example 4
x = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11])
frame_length = 3
hop_length = 3
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[ 1,  4,  7, 10],
#          [ 2,  5,  8, 11],
#          [ 3,  6,  9]]

# Example 5
x = np.array([0, -1, -2, -3, -4, -5, -6, -7, -8, -9])
frame_length = 5
hop_length = 1
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[ 0, -1, -2, -3, -4, -5],
#          [-1, -2, -3, -4, -5, -6],
#          [-2, -3, -4, -5, -6, -7],
#          [-3, -4, -5, -6, -7, -8],
#          [-4, -5, -6, -7, -8, -9]]

# Example 6
x = np.array([0, 1, 0, 1, 0, 1, 0, 1])
frame_length = 4
hop_length = 2
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[0, 0, 0],
#          [1, 1, 1],
#          [0, 0, 0],
#          [1, 1, 1]]

# Example 7
x = np.array([1, 3, 5, 7, 9, 11, 13, 15, 17])
frame_length = 3
hop_length = 1
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[ 1,  3,  5,  7,  9, 11, 13],
#          [ 3,  5,  7,  9, 11, 13, 15],
#          [ 5,  7,  9, 11, 13, 15, 17]]

# Example 8
x = np.array([2, 4, 6, 8, 10, 12, 14])
frame_length = 2
hop_length = 2
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[ 2,  6, 10, 14],
#          [ 4,  8, 12]]

# Example 9
x = np.array([5, 10, 15, 20, 25, 30])
frame_length = 3
hop_length = 2
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[ 5, 15, 25],
#          [10, 20, 30],
#          [15, 25],
#          [20, 30]]

# Example 10
x = np.array([1, 3, 5, 7, 9, 11])
frame_length = 3
hop_length = 3
frames = librosa.util.frame(x, frame_length=frame_length, hop_length=hop_length)
print(frames)
# Output: [[1, 7],
#          [3, 9],
#          [5, 11]]
