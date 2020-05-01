ltl spec {
	[] (((state == apagado) && startstop) -> <> (state == encendido)) && 
	[] (((state == encendido) && startstop) -> <> (state == apagado)) 

}

mtype = { encendido, apagado };
mtype state = apagado;
bit startstop;

active proctype sensor_fsm () {
	state = apagado;
	do
	:: (state == apagado) -> atomic {
		if
		:: startstop -> 
			state = encendido; 
			startstop = 0;
			printf("Estoy encendido\n")
		fi
	}
	:: (state == encendido) -> atomic {
		if
		:: startstop -> 
			state = apagado; 
			startstop = 0;
			printf("Estoy apagado\n")
		fi
	}
	od
}

active proctype entorno() {
	do
	::  if
		:: startstop = 1
		fi;
		printf("E: %e, S: %d \n", state, startstop)
	od
}