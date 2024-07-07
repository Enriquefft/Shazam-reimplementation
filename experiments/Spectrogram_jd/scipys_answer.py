import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.signal import spectrogram
from skimage.feature import peak_local_max
import numpy as np

def main(audio_file):
    # Load audio data
    sample_rate, audio_data = wavfile.read(audio_file)

    # Compute spectrogram
    f, t, Sxx = spectrogram(audio_data, fs=sample_rate)

    # Find local peaks in the spectrogram
    peaks = peak_local_max(Sxx, min_distance=10, threshold_abs=20, exclude_border=False)

    # Plot the spectrogram in monochrome
    plt.figure(figsize=(10, 6))
    plt.imshow(Sxx, aspect='auto', origin='lower', cmap='gray')
    plt.title('Monochrome Spectrogram with Local Peaks')
    plt.xlabel('Time [s]')
    plt.ylabel('Frequency [Hz]')
    plt.colorbar(label='Intensity [dB]')

    # Plot peaks on the spectrogram
    plt.scatter(peaks[:, 1], peaks[:, 0], color='red', marker='o', label='Local Peaks')

    plt.legend()
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    audio_file = 'the_bidding.wav'  # Replace with your audio file path
    main(audio_file)