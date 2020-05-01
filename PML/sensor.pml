ltl spec on {
	[] (((state == 0) && starts) -> <> (state == 1))
}

ltl spec off {
	[] (((state == 1) && starts) -> <> (state == 0))
}

ltl spec rec {
	[] (((state == 1) && received) -> <> (state == 1))
}

ltl spec tie {
	[] (((state == 1) && times) -> <> (state == 0))
}

ltl spec band {
	[] (((state == 1) && flag) -> <> (state == 1))
}

byte state;
bit starts;
bit received;
bit times;
bit flag;

active proctype sensor_fsm () {
	state = 0;
	do
	:: (state == 0) -> atomic {
		if
		:: starts -> state = 1; starts = 0; received = 0; times = 0; flag = 0
		fi
	}
	:: (state == 1) -> atomic {
		if
		:: (starts && (!flag) && (!times) && (!received)) -> state = 0; starts = 0
		:: (received && (!flag) && (!times) && (!starts)) -> state = 1; received = 0
		:: (times && (!flag) && (!received) && (!starts)) -> state = 0;  times = 0
		:: (flag && (!times) && (!received) && (!starts)) -> state = 1; flag = 0
		fi
	}
	od
}

active proctype entorno() {
	do
	::  state == 0 ->
		if
		:: starts = 1
		fi
	:: else ->
		if
		:: starts = 1
		:: flag = 1
		:: times = 1
		:: received = 1
		fi;
		printf("E: %d, S: %d, F: %d, T: %d, R: %d \n", state, starts, flag, times, received)
	od
}
