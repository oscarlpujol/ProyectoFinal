/*ltl spec on {
	[] (((state == 0) && starts) -> <> (state == 1))
}
*/
/*ltl spec off {
	[] (((state == 1) && starts) -> <> (state == 0))
}*/
/*
ltl spec rec {
	[] (((state == 1) && received) -> <> (state == 1))
}
*/

/*ltl spec tie {
	[] (((state == 1) && times) -> <> (state == 0))
}
*/

ltl spec /*band*/ {
	[] (((state == 1) && times) -> <> (state == 2))
}

byte state;
bit starts;
bit received;
bit times;
bit ack_or_xck;

active proctype sensor_fsm () {
	state = 0;
	do
	:: (state == 0) -> atomic {
		if
		:: starts -> state = 1; starts = 0;
		fi
	}
	:: (state == 1) -> atomic {
		if
		:: (starts && (!ack_or_xck) && (!times) && (!received)) -> state = 0; starts = 0
		:: (received && (!ack_or_xck) && (!times) && (!starts)) -> state = 1; received = 0
		:: (times && (!ack_or_xck) && (!received) && (!starts)) -> state = 2;  times = 0
		:: (ack_or_xck && (!times) && (!received) && (!starts)) -> state = 1; ack_or_xck = 0
		fi
	}
  :: (state == 2) -> atomic {
    if
    :: starts -> state = 0; starts = 0;
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
	:: state ==1 ->
		if
		:: starts = 1
		:: ack_or_xck = 1
		:: times = 1
		:: received = 1
		fi
  :: else ->
    if
    :: starts = 1
    fi
	od
}
