import numpy as np
from librosa import load, stft as lib_stft, display, amplitude_to_db

from stft import stft

import matplotlib.pyplot as plt


def plot(s, n):
    s = np.abs(s)
    fig, ax = plt.subplots()
    img = display.specshow(
        amplitude_to_db(s, ref=np.max), y_axis="log", x_axis="time", ax=ax
    )
    ax.set_title("Power spectrogram")
    fig.colorbar(img, ax=ax, format="%+2.0f dB")
    fig.savefig(str(n))

=======
filename = "../../assets/3. You & Me - Good & Evil.wav"


y, sr = load(filename, sr=None, dtype="float64")
s = stft(y)
s_default = lib_stft(y)

# plot(s, 1)
# plot(s_default, 2)

print("STILL_EQUAL??", (s == s_default).all())
