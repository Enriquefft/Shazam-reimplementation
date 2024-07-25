import numpy as np
import librosa
import matplotlib.pyplot as plt

songs = [
    "the_bidding.wav",
    # "95_053. Avicii - Hey Brother.wav",
    # "13_030. Maroon 5 - Animals.wav",
    # "57_Numb.wav",
    # "81_013. Passenger  Let Her Go (Official Video).wav",
    # "./103_037. Calvin Harris, Rihanna - This Is What You Came For (Official Video) ft. Rihanna.wav",
    # librosa.ex("trumpet"),
]

print(librosa.util.list_examples())


for song in songs:
    y, sr = librosa.load(song)

    S = np.abs(librosa.stft(y))

    fig, ax = plt.subplots()
    img = librosa.display.specshow(
        librosa.amplitude_to_db(S, ref=np.max), y_axis="log", x_axis="time", ax=ax
    )
    ax.set_title("Power spectrogram")
    fig.colorbar(img, ax=ax, format="%+2.0f dB")
    fig.savefig(f"{song}.png")
