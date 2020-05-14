/*ltl spec IDLE {
[] (((state == 0) && starts) -> <> (state == 1))
}

ltl spec w8_1 {
[] (((state == 1) && maq && ((!I2C) && (!IAQ)) U (state != 1)) -> <> (state == 2))
}

ltl spec w8_2 {
[] (((state == 1) && I2C && ((!maq) && (!IAQ)) U (state != 1)) -> <> (state == 0))
}

ltl spec w8_3 {
[] (((state == 1) && IAQ && ((!I2C) && (!maq)) U (state != 1)) -> <> (state == 0))
}

ltl spec maqq {
[] (((state == 2) && starts) -> <> (state == 3))
}

ltl spec msg_1 {
[] (((state == 3) && flagXCK  && ((!CO2) && (!flagACK)) U (state != 3)) -> <> (state == 0))
}


ltl spec msg_2 {
[] (((state == 3) && flagACK) -> <> (state == 3))
}

ltl spec msg_3 {
[] (((state == 3) && CO2) -> <> (state == 3))
}
*/


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
    :: (state == 0) -> atomic {
  		if
  		:: starts -> state = 1; starts = 0; maq = 0; I2C = 0; IAQ = 0;
  		fi
  	}
    :: (state == 1) -> atomic {
  		if
  		:: maq -> state = 2; maq = 0; starts = 0;
      :: I2C -> state = 0; I2C = 0; starts = 0;
      :: IAQ -> state = 0; IAQ = 0; starts = 0;
  		fi
  	}
    :: (state == 2)-> atomic {
  		if
  		:: starts -> state = 3; starts = 0; flagACK = 0; flagXCK = 0; CO2 = 0;
  		fi
  	}
    :: (state == 3)-> atomic {
  		if
  	  :: flagXCK  -> state = 0; flagXCK = 0; starts = 0;
      :: flagACK -> state = 3; flagACK = 0;
      :: CO2 -> state = 3; CO2 = 0;
  		fi
  	}
    od
}

active proctype entorno() {
	do
	:: if
		 :: starts = 1
		 :: maq = 1
		 :: I2C = 1
		 :: IAQ = 1
     :: flagXCK = 1
     :: flagACK = 1
     :: CO2 = 1
     :: skip
     fi;
		 printf("%d, %d, %d, %d, %d, %d, %d, %d\n", state, starts, maq, I2C, IAQ, flagXCK, flagACK, CO2)
	od
}
