index_executable="../../executables/SHAZAM_CPP_APP_INDEXING"
search_executable="../../executables/SHAZAM_CPP_APP"

path_to_songs="../../Dataset/songs"
path_to_samples="../../Dataset/samples"
echo "Hashing audios"
"$index_executable" "$path_to_songs" ./hashes --config settings.ini --dump ./data

for folder in "$path_to_samples"/*; do
	folder_name=$(basename "$folder")
	echo "Doing samples on '$folder'" 
	"$search_executable" ./hashes --sampleset "$folder" --config settings.ini --dump "./data/${folder_name}_searches.csv"
done
