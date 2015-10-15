#ifndef IndentedOStream_h
#define IndentedOStream_h

#include <ostream>

class IndentedOStream {
	public:
		IndentedOStream(std::ostream& stream_in)
			: stream(stream_in), level(0), at_bol(true) {}

		template <typename T>
			IndentedOStream& operator<<(const T& value) {
				if (at_bol) {
					write_indent();
					at_bol = false;
					}
				stream << value;
				return *this;
				}

		// ALWAYS use "stream << '\n'" to end a line.
		IndentedOStream& operator<<(char c) {
			if (c == '\n') {
				stream << '\n';
				at_bol = true;
				}
			else {
				if (at_bol) {
					write_indent();
					at_bol = false;
					}
				stream << c;
				}
			return *this;
			}

		void	indent() { level += 1; }
		void	unindent() { level -= 1; }

		class Indenter {
			public:
				Indenter(IndentedOStream& stream_in)
					: stream(stream_in)
				{
					stream.indent();
				}

				~Indenter()
				{
					stream.unindent();
				}

			protected:
				IndentedOStream& stream;
			};

	protected:
		std::ostream& stream;
		int level;
		bool at_bol;

		void	write_indent();
	};


#endif	// !IndentedOStream_h

