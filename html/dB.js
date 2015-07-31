var min_dB = -100;


function dB_to_gain(dB) {
	if (dB <= min_dB)
		return 0;
	return Math.pow(10.0, dB * 0.05);
	}

function gain_to_dB(gain) {
	if (gain <= 0)
		return min_dB;
	return Math.log10(gain) * 20;
	}

