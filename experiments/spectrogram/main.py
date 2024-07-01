import numpy as np
from librosa import load, stft as lib_stft

from stft import stft


filename = "../../assets/3. You & Me - Good & Evil.wav"


y, sr = load(filename, sr=None, dtype="float64")
s = stft(y)


s_default = lib_stft(y)
