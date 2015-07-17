#ifndef AudioFile_h
#define AudioFile_h

#include <string>

class ProjectReader;
class Project;
class OpenAudioFile;


class AudioFile {
	public:
		AudioFile(int id_in)
			: id(id_in), open_file(nullptr), num_opens(0)
			{ init(); }
		virtual ~AudioFile();

		struct Info {
			int	sample_rate;
			int	num_channels;
			int	bits_per_sample;
			uint64_t	length_in_frames;
			};
		Info	info;

		void	read_json(ProjectReader* reader);
		void	load_info();

		OpenAudioFile*	open();
		void	close();

		std::string	path;
		int	id;

	protected:
		Project*	project;
		OpenAudioFile*	open_file;
		int	num_opens;

		void	init();
	};


#endif	// !AudioFile_h

