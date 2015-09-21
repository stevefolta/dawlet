#ifndef Stats_h
#define Stats_h


struct Stats {
	int	playback_xruns, capture_xruns;
	int	missing_file_reads, read_slot_overflows, exhausted_reads;

	void	reset();
	};


#endif	// !Stats_h

