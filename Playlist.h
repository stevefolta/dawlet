#ifndef Playlist_h
#define Playlist_h

class AudioBuffer;


class Playlist {
	public:
		Playlist();
		~Playlist();

		void	run(AudioBuffer* buffer_out);

	protected:
	};


#endif 	// !Playlist_h

