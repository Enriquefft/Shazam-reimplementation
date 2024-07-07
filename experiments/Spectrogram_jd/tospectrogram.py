
import librosa
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.colors import LinearSegmentedColormap

def generate_and_save_mono_spectrogram(audio_path, save_path_png, save_path_csv, n_fft=2048, hop_length=256):
    # Load audio file
    y, sr = librosa.load(audio_path)

    # Compute spectrogram
    D = np.abs(librosa.stft(y, n_fft=n_fft, hop_length=hop_length))

    # Convert to dB scale
    DB = librosa.amplitude_to_db(D, ref=np.max)

    # Create a grayscale colormap
    cmap = LinearSegmentedColormap.from_list("my_cmap", [(0, 'black'), (1, 'white')])

    # Determine the dimensions of the spectrogram
    num_frames = DB.shape[1]  # Number of time frames
    freq_bins = DB.shape[0]    # Number of frequency bins

    # Plot spectrogram without axis and color bar
    plt.figure(figsize=(num_frames / 100, freq_bins / 100), dpi=100)
    plt.imshow(DB, aspect='auto', origin='lower', cmap=cmap)
    plt.axis('off')  # Remove axis
    plt.tight_layout()

    # Save spectrogram as a PNG image file
    plt.savefig(save_path_png, bbox_inches='tight', pad_inches=0, dpi=100, transparent=True)

    # Convert spectrogram to DataFrame for CSV export
    df = pd.DataFrame(data=DB)

    # Save spectrogram as a CSV file
    df.to_csv(save_path_csv, index=False, header=False)

    # Close plot to free memory
    plt.close()

# Example usage:
audio_path = 'Inputs/1mb.wav'
save_path_png = 'Outputs/1mb_spec_librosa.png'
save_path_csv = 'Outputs/1mb_spec_librosa.csv'
generate_and_save_mono_spectrogram(audio_path, save_path_png, save_path_csv, n_fft=2048, hop_length=256)
