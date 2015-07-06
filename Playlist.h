#ifndef Playlist_h
#define Playlist_h

class AudioBuffer;
class JSONParser;


class Playlist {
	public:
		Playlist();
		~Playlist();

		void	read_json(JSONParser* parser);

		void	run(AudioBuffer* buffer_out);

	protected:
	};


#endif 	// !Playlist_h

