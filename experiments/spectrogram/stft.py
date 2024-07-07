import numpy as np
from scipy.signal import get_window
import sys

from librosa import stft

from typing import Optional, Union, Any, Callable, Literal
from numpy.typing import DTypeLike

import utils

ModuleType = type(sys)
MAX_MEM_BLOCK = 2**8 * 2**10

filename = "../assets/3. You & Me - Good & Evil.wav"

# Padding modes for head/tail padding
# These rule out padding modes that depend on the entire array
_STFTPad = Literal[
    "constant",
    "edge",
    "linear_ramp",
    "reflect",
    "symmetric",
    "empty",
]
_PadModeSTFT = Union[_STFTPad, Callable[..., Any]]


def stft(
    y: np.ndarray,
    *,
    n_fft: int = 2048,
    hop_length: Optional[int] = None,
    win_length: Optional[int] = None,
    window: str = "hann",
    center: bool = True,
    dtype: Optional[DTypeLike] = None,
    pad_mode: _PadModeSTFT = "constant",
    out: Optional[np.ndarray] = None,
) -> np.ndarray:
    """Short-time Fourier transform (STFT).

    The STFT represents a signal in the time-frequency domain by
    computing discrete Fourier transforms (DFT) over short overlapping
    windows.

    This function returns a complex-valued matrix D such that

    - ``np.abs(D[..., f, t])`` is the magnitude of frequency bin ``f``
      at frame ``t``, and

    - ``np.angle(D[..., f, t])`` is the phase of frequency bin ``f``
      at frame ``t``.

    The integers ``t`` and ``f`` can be converted to physical units by means
    of the utility functions `frames_to_samples` and `fft_frequencies`.

    Parameters
    ----------
    y : np.ndarray [shape=(..., n)], real-valued
        input signal. Multi-channel is supported.

    n_fft : int > 0 [scalar]
        length of the windowed signal after padding with zeros.
        The number of rows in the STFT matrix ``D`` is ``(1 + n_fft/2)``.
        The default value, ``n_fft=2048`` samples, corresponds to a physical
        duration of 93 milliseconds at a sample rate of 22050 Hz, i.e. the
        default sample rate in librosa. This value is well adapted for music
        signals. However, in speech processing, the recommended value is 512,
        corresponding to 23 milliseconds at a sample rate of 22050 Hz.
        In any case, we recommend setting ``n_fft`` to a power of two for
        optimizing the speed of the fast Fourier transform (FFT) algorithm.

    hop_length : int > 0 [scalar]
        number of audio samples between adjacent STFT columns.

        Smaller values increase the number of columns in ``D`` without
        affecting the frequency resolution of the STFT.

        If unspecified, defaults to ``win_length // 4`` (see below).

    win_length : int <= n_fft [scalar]
        Each frame of audio is windowed by ``window`` of length ``win_length``
        and then padded with zeros to match ``n_fft``.  Padding is added on
        both the left- and the right-side of the window so that the window
        is centered within the frame.

        Smaller values improve the temporal resolution of the STFT (i.e. the
        ability to discriminate impulses that are closely spaced in time)
        at the expense of frequency resolution (i.e. the ability to discriminate
        pure tones that are closely spaced in frequency). This effect is known
        as the time-frequency localization trade-off and needs to be adjusted
        according to the properties of the input signal ``y``.

        If unspecified, defaults to ``win_length = n_fft``.

    window : string, tuple, number, function, or np.ndarray [shape=(n_fft,)]
        Either:

        - a window specification (string, tuple, or number);
          see `scipy.signal.get_window`
        - a window function, such as `scipy.signal.windows.hann`
        - a vector or array of length ``n_fft``

        Defaults to a raised cosine window (`'hann'`), which is adequate for
        most applications in audio signal processing.

        .. see also:: `filters.get_window`

    center : boolean
        If ``True``, the signal ``y`` is padded so that frame
        ``D[:, t]`` is centered at ``y[t * hop_length]``.

        If ``False``, then ``D[:, t]`` begins at ``y[t * hop_length]``.

        Defaults to ``True``,  which simplifies the alignment of ``D`` onto a
        time grid by means of `librosa.frames_to_samples`.
        Note, however, that ``center`` must be set to `False` when analyzing
        signals with `librosa.stream`.

        .. see also:: `librosa.stream`

    dtype : np.dtype, optional
        Complex numeric type for ``D``.  Default is inferred to match the
        precision of the input signal.

    pad_mode : string or function
        If ``center=True``, this argument is passed to `np.pad` for padding
        the edges of the signal ``y``. By default (``pad_mode="constant"``),
        ``y`` is padded on both sides with zeros.

        .. note:: Not all padding modes supported by `numpy.pad` are supported here.
            `wrap`, `mean`, `maximum`, `median`, and `minimum` are not supported.

            Other modes that depend at most on input values at the edges of the
            signal (e.g., `constant`, `edge`, `linear_ramp`) are supported.

        If ``center=False``,  this argument is ignored.

        .. see also:: `numpy.pad`

    out : np.ndarray or None
        A pre-allocated, complex-valued array to store the STFT results.
        This must be of compatible shape and dtype for the given input parameters.

        If `out` is larger than necessary for the provided input signal, then only
        a prefix slice of `out` will be used.

        If not provided, a new array is allocated and returned.

    Returns
    -------
    D : np.ndarray [shape=(..., 1 + n_fft/2, n_frames), dtype=dtype]
        Complex-valued matrix of short-term Fourier transform
        coefficients.

        If a pre-allocated `out` array is provided, then `D` will be
        a reference to `out`.

        If `out` is larger than necessary, then `D` will be a sliced
        view: `D = out[..., :n_frames]`.

    See Also
    --------
    istft : Inverse STFT
    reassigned_spectrogram : Time-frequency reassigned spectrogram

    Notes
    -----
    This function caches at level 20.

    Examples
    --------
    >>> y, sr = librosa.load(librosa.ex('trumpet'))
    >>> S = np.abs(librosa.stft(y))
    >>> S
    array([[5.395e-03, 3.332e-03, ..., 9.862e-07, 1.201e-05],
           [3.244e-03, 2.690e-03, ..., 9.536e-07, 1.201e-05],
           ...,
           [7.523e-05, 3.722e-05, ..., 1.188e-04, 1.031e-03],
           [7.640e-05, 3.944e-05, ..., 5.180e-04, 1.346e-03]],
          dtype=float32)

    Use left-aligned frames, instead of centered frames

    >>> S_left = librosa.stft(y, center=False)

    Use a shorter hop length

    >>> D_short = librosa.stft(y, hop_length=64)

    Display a spectrogram

    >>> import matplotlib.pyplot as plt
    >>> fig, ax = plt.subplots()
    >>> img = librosa.display.specshow(librosa.amplitude_to_db(S,
    ...                                                        ref=np.max),
    ...                                y_axis='log', x_axis='time', ax=ax)
    >>> ax.set_title('Power spectrogram')
    >>> fig.colorbar(img, ax=ax, format="%+2.0f dB")
    """
    # By default, use the entire frame
    if win_length is None:
        win_length = n_fft

    # Set the default hop, if it's not already specified
    if hop_length is None:
        hop_length = int(win_length // 4)

    # fft_window = get_window(window, win_length, fftbins=True)
    fft_window = utils.hann(win_length)

    # Pad the window out to n_fft size
    fft_window = utils.pad_center(fft_window, size=n_fft)

    # Reshape so that the window can be broadcast
    fft_window = utils.expand_to(fft_window, ndim=1 + y.ndim)

    if n_fft > y.shape[-1]:
        print("WARNING")
        print(f"n_fft={n_fft} is too large for input signal of length={y.shape[-1]}")

    # Set up the padding array to be empty, and we'll fix the target dimension later
    padding = [(0, 0) for _ in range(y.ndim)]

    # How many frames depend on left padding?
    start_k = int(np.ceil(n_fft // 2 / hop_length))

    # What's the first frame that depends on extra right-padding?

    tail_k = (y.shape[-1] + n_fft // 2 - n_fft) // hop_length + 1

    if tail_k <= start_k:
        # If tail and head overlap, then just copy-pad the signal and carry on
        start = 0
        extra = 0
        padding[-1] = (n_fft // 2, n_fft // 2)
        y = np.pad(y, padding, mode=pad_mode)
    else:
        # If tail and head do not overlap, then we can implement padding on each part separately
        # and avoid a full copy-pad

        # "Middle" of the signal starts here, and does not depend on head padding
        start = start_k * hop_length - n_fft // 2
        padding[-1] = (n_fft // 2, 0)

        # +1 here is to ensure enough samples to fill the window
        # fixes bug #1567
        y_pre = np.pad(
            y[..., : (start_k - 1) * hop_length - n_fft // 2 + n_fft + 1],
            padding,
            mode=pad_mode,
        )

        y_frames_pre = utils.frame2(y_pre, frame_length=n_fft, hop_length=hop_length)

        # Trim this down to the exact number of frames we should have
        y_frames_pre = y_frames_pre[..., :start_k]

        # How many extra frames do we have from the head?
        extra = y_frames_pre.shape[-1]

        # Determine if we have any frames that will fit inside the tail pad
        if tail_k * hop_length - n_fft // 2 + n_fft <= y.shape[-1] + n_fft // 2:
            padding[-1] = (0, n_fft // 2)

            y_post_idx = (tail_k) * hop_length - n_fft // 2

            y_post_trimmed = y[..., y_post_idx:]

            y_post = np.pad(y_post_trimmed, padding, mode=pad_mode)

            y_frames_post = utils.frame2(
                y_post, frame_length=n_fft, hop_length=hop_length, info=True
            )

            # How many extra frames do we have from the tail?
            extra += y_frames_post.shape[-1]
        else:
            # In this event, the first frame that touches tail padding would run off
            # the end of the padded array
            # We'll circumvent this by allocating an empty frame buffer for the tail
            # this keeps the subsequent logic simple
            post_shape = list(y_frames_pre.shape)
            post_shape[-1] = 0
            y_frames_post = np.empty_like(y_frames_pre, shape=post_shape)

    if dtype is None:
        dtype = utils.dtype_r2c(y.dtype)

    # Window the time series.
    y_frames = utils.frame2(y[..., start:], frame_length=n_fft, hop_length=hop_length)

    # Pre-allocate the STFT matrix
    shape = list(y_frames.shape)

    # This is our frequency dimension
    shape[-2] = 1 + n_fft // 2

    # If there's padding, there will be extra head and tail frames
    shape[-1] += extra

    stft_matrix = np.zeros(shape, dtype=dtype, order="F")

    utils.print_info(fft_window, "fft_window")
    utils.print_info(y_frames, "y_frames")
    utils.print_info(y_frames_pre, "y_frames_pre")
    utils.print_info(y_frames_post, "y_frames_post")
    utils.print_info(stft_matrix, "stft_matrix")
    print("extra", extra)

    # Fill in the warm-upw
    fft_pre = np.fft.rfft(fft_window * y_frames_pre, axis=-2)
    fft_post = np.fft.rfft(fft_window * y_frames_post, axis=-2)

    if center and extra > 0:
        off_start = y_frames_pre.shape[-1]
        stft_matrix[..., :off_start] = fft_pre

        off_end = y_frames_post.shape[-1]
        if off_end > 0:
            stft_matrix[..., -off_end:] = fft_post
    else:
        off_start = 0

    n_columns = int(MAX_MEM_BLOCK // (np.prod(y_frames.shape[:-1]) * y_frames.itemsize))
    n_columns = max(n_columns, 1)

    for bl_s in range(0, y_frames.shape[-1], n_columns):
        bl_t = min(bl_s + n_columns, y_frames.shape[-1])

        stft_matrix[..., bl_s + off_start : bl_t + off_start] = np.fft.rfft(
            fft_window * y_frames[..., bl_s:bl_t], axis=-2
        )
    return stft_matrix
