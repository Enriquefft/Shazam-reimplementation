import numpy as np
from librosa import load, stft as lib_stft, display, amplitude_to_db

from stft import stft
from pathlib import Path

import utils

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


assets_path = Path("../../assets")
sounds: list[Path] = [
    # Path(assets_path / "1mb.wav"),
    # Path(assets_path / "5mb.wav"),
    # Path(assets_path / "10mb.wav"),
    Path(assets_path / "the_bidding.wav"),
    # Path(assets_path / "long_wav.wav"),
]

for soundPath in sounds:
    print(soundPath)
    y, sr = load(soundPath, sr=None, dtype="float64")

    s = stft(y)

    utils.print_info(np.abs(s), "My STFT")

    utils.print_info(amplitude_to_db(np.abs(s)), "My STFT")

    s_default = lib_stft(y)

    print("STILL_EQUAL??", (s == s_default).all())
    print()
    print()
