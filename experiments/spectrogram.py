from librosa import load, stft, amplitude_to_db, display
import numpy as np
import matplotlib.pyplot as plt

filename = "../assets/3. You & Me - Good & Evil.wav"

y, sr = load(filename, sr=None, dtype="float64")
s = stft(y)

print("target:\n")
print(s.shape)
print(s.dtype)
print(np.sum(s))
print()
print()

s = np.abs(s)
print("target 2:\n")
print(s.shape)
print(s.dtype)
print(np.sum(s))
