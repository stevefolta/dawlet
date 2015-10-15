#ifndef AudioFile_h
#define AudioFile_h

#include <string>

class ProjectReader;
class Project;
class OpenAudioFile;
class AudioBuffer;
class IndentedOStream;


class AudioFile {
	public:
		AudioFile(Project* project_in, int id_in)
			: project(project_in), id(id_in), open_file(nullptr), num_opens(0)
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
		void	write_to_file(IndentedOStream& stream);
		void	load_info();

		OpenAudioFile*	open();
		void	close();
		OpenAudioFile*	get_open_file() { return open_file; }

		std::string	path;
		int	id;

		void	play_into_buffers(
			AudioBuffer** buffers_out, int num_out_channels, int start_out_frame,
			char* data, int start_in_frame, int num_frames);

	protected:
		Project*	project;
		OpenAudioFile*	open_file;
		int	num_opens;

		void	init();
	};


#endif	// !AudioFile_h

