This folder contains the tests and results for the project and all tools for it. 
Its structure is as follows
measurements
 \-> README.md
 \-> executables
 |	 	\-> index
 |      \-> search
 \-> interpret.py (takes a runs directory that has been ran and outputs readable data!)
 \-> Dataset
 |			\-> songs 
 |			|		\-> song1.wav, song2.wav,...
 |			\ -> samples
 |					  \-> NoiseLevel1
 |					  |				\-> song1_sample_1.wav, song1_sample_2.wav,...
 |					  \-> NoiseLevel2
 |					  |				\-> song1_sample_1.wav, song1_sample_2.wav,...
 |					  ...
 \-> Runs
		\-> run1
			\-> config.ini
			\-> run.sh
			\-> hashes
				... (internal usage by shazam reimplement)
			\-> grouped_data.txt
			\-> indexing.csv
			\-> search.csv

Config file looks like this:
confing.ini:

; Size of window for STFT computation
FFT_WINDOW=4096

; Size of maxfilter on x direction
MINLIST_SIZEX=60

; Size of maxfilter on y direction
MINLIST_SIZEY=150

; 'Radius' of the neighbours a point must be grater to to be a local maxima
GTN_SIZE=5

; percentage of mean loudness a point must have to qualify as a local max
GTN_THRESHOLD=1.2

; one of MINLIST (maxfilter then check for equality), 
; GTN(check if point is grater that neighbours), MINLISTGTN (maxfilter then use gtn to 
; refine)

PEAK_ALGORITHM=MINLISTGTN

; size in x of the box to generate combinatorial hashes
HASH_BOXX=150
;size in y of the box to generate combinatorial hashes
HASH_BOXY=200
; how much in front is the box to generate combinatorial hashes drawn?
HASH_BOX_DISPLACEMENT=5

the index command is meant to be run in a 