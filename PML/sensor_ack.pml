ltl IDLE  {
	[] (((state == 0) && starts) -> <> (state == 1))
}

ltl ACK_1  {
	[] (((state == 1) && starts && (((!received) && (!ack_or_xck)) U (state != 1))) -> <> (state == 0))
}

ltl spec {
	[] (((state == 1) && received && (((!ack_or_xck) && (!starts)) U (state != 1))) -> <> (state == 1))
}

ltl spec {
	[] (((state == 1) && ack_or_xck && (((!received) && (!starts)) U (state != 1))) -> <> (state == 1))
}

byte state;
bit starts;
bit received;
bit ack_or_xck;

active proctype sensor_fsm () {
	state = 0;
	do
	:: (state == 0) -> atomic {
		if
		:: starts -> state = 1; starts = 0; received = 0; ack_or_xck = 0;
		fi
	}
	:: (state == 1) -> atomic {
		if
		:: (starts && (!ack_or_xck) && (!times) && (!received)) -> state = 0; starts = 0
		:: (received && (!ack_or_xck) && (!times) && (!starts)) -> state = 1; received = 0
		:: (ack_or_xck && (!times) && (!received) && (!starts)) -> state = 1; ack_or_xck = 0
		fi
	}
	od
}

active proctype entorno() {
	do
	:: if
		:: starts = 1
		:: ack_or_xck = 1
		:: times = 1
		:: received = 1
		fi;
		printf("E: %d, S: %d, F: %d, R: %d \n", state, starts, ack_or_xck, received)
	od
}
