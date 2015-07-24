#ifndef ProjectReader_h
#define ProjectReader_h

#include "JSONParser.h"
#include <string>
#include <map>

class Project;
class Track;
class AudioFile;


class ProjectReader {
	public:
		ProjectReader(std::string text, Project* project_in);
		~ProjectReader();

		JSONParser*	parser;

		AudioFile*	file_for_id(int id);

		// Helpers that pass thru to the parser.
		void	start_object() { parser->start_object(); }
		std::string	next_field() { return parser->next_field(); }
		void	start_array() { parser->start_array(); }
		bool	array_is_done() { return parser->array_is_done(); }
		void	ignore_value() { parser->ignore_value(); }
		int	next_int() { return parser->next_int(); }
		double	next_double() { return parser->next_double(); }
		std::string	next_string() { return parser->next_string(); }
		bool	next_bool() { return parser->next_bool(); }

		static int	string_to_id(std::string str);

		Project*	get_project() { return project; }

	protected:
		Project*	project;
		std::map<int, AudioFile*>	files;
	};



#endif	// !ProjectReader_h

