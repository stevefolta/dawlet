#ifndef RecordingClip_h
#define RecordingClip_h

class Track;
class Clip;

struct RecordingClip {
	Track*	track;
	Clip*	clip;

	RecordingClip(Track* track_in, Clip* clip_in)
		: track(track_in), clip(clip_in) {}
	};


#endif	// !RecordingClip_h

