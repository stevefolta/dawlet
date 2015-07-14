#ifndef AudioFile_h
#define AudioFile_h

#include <string>


class AudioFile {
	public:
		AudioFile(std::string path_in, int id_in = 0)
			: path(path_in), id(id_in) {}
		virtual ~AudioFile();

		struct Info {
			int	sample_rate;
			int	num_channels;
			uint64_t	length_in_samples;
			};

		virtual Info	read_info() = 0;

	protected:
		string	path;
		int	id;
		Info	info;
	};


#endif	// !AudioFile_h

