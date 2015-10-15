#ifndef Stats_h
#define Stats_h


struct Stats {
	int	playback_xruns, capture_xruns;
	int	missing_file_reads, read_slot_overflows, exhausted_reads;
	int	exhausted_buffers, exhausted_record_buffers;

	void	reset();
	};


#endif	// !Stats_h

