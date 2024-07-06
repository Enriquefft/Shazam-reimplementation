import numpy as np
from pathlib import Path

from librosa import stft, amplitude_to_db, load

assets_path = Path("../../assets")
sounds: list[Path] = [
    Path(assets_path / "the_bidding.wav"),
    # Path(assets_path / "1mb.wav"),
    # Path(assets_path / "5mb.wav"),
    # Path(assets_path / "10mb.wav"),
    # Path(assets_path / "long_wav.wav"),
]

for sound in sounds:
    # get relevant matrices
    audiodata, sr = load(sound, sr=None, dtype="float64")

    print(audiodata[:10])

    complex_spectrogram = stft(audiodata)
    spectrogram = np.abs(complex_spectrogram)

    normalized_spectrogram = amplitude_to_db(spectrogram)

    print(sound)

    print(f"audiodata: shape:\t{audiodata.shape}\tsum: {audiodata.sum()}")
    print(
        f"complex spectrogram:\tshape: {complex_spectrogram.shape}\tsum: {complex_spectrogram.sum()}"
    )
    print(f"spectrogram: shape:\t{spectrogram.shape}\tsum: {spectrogram.sum()}")
    print(
        f"normalized spectrogram:\tshape: {normalized_spectrogram.shape}\tsum: {normalized_spectrogram.sum()}"
    )

    print()

    # write matrices
    # with sound.open(mode='wb') as sound_file:
    #     for line in y:
    #         np.savetxt(sound_file, line, fmt='%.2f')
