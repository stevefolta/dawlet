#ifndef AudioTypes_h
#define AudioTypes_h

// We might want to use "double" for samples, but LV2 only supports "float"s.
typedef float AudioSample;
#define AUDIO_SAMPLE_FLOAT

typedef double ProjectPosition;


#endif 	// !AudioTypes_h

