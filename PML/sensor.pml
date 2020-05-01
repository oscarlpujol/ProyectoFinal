ltl IDLE {
[] (((state == 0) && starts) -> <> (state == 1))
}

ltl w8_1 {
[] (((state == 1) && maq) -> <> (state == 2))
}
ltl w8_2 {
[] (((state == 1) && I2C) -> <> (state == 0))
}
ltl w8_3 {
[] (((state == 1) && IAQ) -> <> (state == 0))
}

ltl maqq {
[] (((state == 2) && starts) -> <> (state == 3))
}

ltl msg_1 {
[] (((state == 3) && flagXCK) -> <> (state == 4))
}
ltl msg_2 {
[] (((state == 3) && flagACK) -> <> (state == 3))
}
ltl msg_3 {
[] (((state == 3) && CO2) -> <> (state == 3))
}

byte state;
bit starts;
bit maq;
bit I2C;
bit IAQ;
bit flagXCK;
bit flagACK;
bit CO2;

active proctype sensor_fsm () {
state = 0;
do
    :: (state == 0)-> atomic {
  		if
  		:: starts -> state = 1; starts = 0;
  		fi
  	}
    :: (state == 1)-> atomic {
  		if
  		:: (maq && (!I2C) && (!IAQ)) -> state = 2; maq = 0;
      :: (I2C && (!maq) && (!IAQ)) -> state = 0; I2C = 0;
      :: (IAQ && (!maq) && (!I2C)) -> state = 0; IAQ = 0;
  		fi
  	}
    :: (state == 2)-> atomic {
  		if
  		:: starts -> state = 3; starts = 0;
  		fi
  	}
    :: (state == 3)-> atomic {
  		if
  	  :: (flagXCK && (!flagACK) && (!CO2)) -> state = 0; flagXCK = 0;
      :: ((!flagXCK) && (flagACK) && (!CO2)) -> state = 3; flagACK = 0;
      :: ((!flagXCK) && (!flagACK) && (CO2)) -> state = 3; CO2 = 0;
  		fi
  	}
od
}

active proctype entorno() {
	do
	::  (state == 0) ->
		if
		:: starts = 1
		fi
	::  (state == 1) ->
		if
		:: maq = 1
		:: I2C = 1
		:: IAQ = 1
		fi
  :: (state == 2) ->
    if
    :: starts = 1
    fi
  :: (state == 3) ->
    if
    :: flagXCK
    :: flagACK
    :: CO2
    fi;
		printf("%d, %d, %d, %d, %d, %d, %d, %d", state, starts, maq, I2C, IAQ, flagXCK, flagACK, CO2)
	od
}
