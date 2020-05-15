/*
ltl spec IDLE {
[] (((state == 0) && starts) -> <> (state == 1))
}
// CHECKED

ltl spec w8_1_1 {
[] (((state == 1) && correct && (((!wrong) && (!received)) U (state != 1))) -> <> (state == 2))
}
//CHECKED

ltl spec w8_1_2 {
[] (((state == 1) && wrong && (((!correct) && (!received)) U (state != 1))) -> <> (state == 0))
}
//CHECKED

ltl spec w8_1_3 {
[] (((state == 1) && received) -> <> (state == 1))
}
// CHECKED

ltl spec w8_2_1 {
[] (((state == 2) && maq && (((!received) && (!mrs) && (!IAQ) && (!incorrect)) U (state != 2))) -> <> (state == 3))
}
//CHECKED

ltl spec w8_2_2 {
[] (((state == 2) && mrs && (((!received) && (!maq) && (!IAQ) && (!incorrect)) U (state != 2))) -> <> (state == 5))
}
//CHECKED

ltl spec w8_2_3 {
[] (((state == 2) && received) -> <> (state == 2))
}
//CHECKED

ltl spec w8_2_4 {
[] (((state == 2) && IAQ && (((!received) && (!mrs) && (!maq) && (!incorrect)) U (state != 2))) -> <> (state == 0))
}
//CHECKED

ltl spec w8_2_5 {
[] (((state == 2) && incorrect && (((!received) && (!mrs) && (!IAQ) && (!maq)) U (state != 2))) -> <> (state == 0))
}
//CHECKED

ltl spec state_MAQ {
[] (((state == 3) && starts) -> <> (state == 4))
}
//CHECKED

ltl spec state_MSG_MAQ_1 {
[] (((state == 4) && maq_left  && (((!flagXCK)) U (state != 4))) -> <> (state == 0))
}
//CHECKED

ltl spec state_MSG_MAQ_2 {
[] (((state == 4) && flagXCK) -> <> (state == 4))
}
//CHECKED

ltl spec state_MRS {
[] (((state == 5) && starts) -> <> (state == 6))
}
//CHECKED

ltl spec state_MSG_MRS_1 {
[] (((state == 6) && mrs_left  && (((!flagXCK)) U (state != 6))) -> <> (state == 0))
}
//CHECKED

ltl spec state_MSG_MRS_2 {
[] (((state == 6) && flagXCK) -> <> (state == 6))
}
//CHECKED
*/

byte state;
bit starts;
bit wrong;
bit received;
bit correct;
bit incorrect;
bit maq;
bit IAQ;
bit maq_left;
bit flagXCK;
bit mrs;
bit mrs_left;


active proctype sensor_fsm () {
    state = 0;
    do
    :: (state == 0) -> atomic {
  		if
  		:: starts -> state = 1; starts = 0; wrong = 0; received = 0; correct = 0;
  		fi
  	}
    :: (state == 1) -> atomic {
  		if
  		:: correct -> state = 2; correct = 0; maq = 0;
      :: received -> state = 1; received = 0;
      :: wrong -> state = 0; wrong = 0; starts = 0;
  		fi
  	}
    :: (state == 2)-> atomic {
  		if
  		:: maq -> state = 3; maq = 0; starts = 0;
      :: received -> state = 2; received = 0;
      :: IAQ -> state = 0; IAQ = 0; starts = 0;
      :: incorrect -> state = 0; incorrect = 0; starts = 0;
      :: mrs -> state = 5; mrs = 0; starts = 0;
  		fi
  	}
    :: (state == 3) -> atomic{
      if
      :: starts -> state = 4; starts = 0; received = 0; flagXCK = 0;
      fi
    }

    :: (state == 4)-> atomic {
  		if
  	  :: flagXCK  -> state = 0; flagXCK = 0; starts = 0;
      :: maq_left -> state = 4; received = 0;
  		fi
  	}
    :: (state == 5) -> atomic{
      if
      :: starts -> state = 6; starts = 0;
      fi
    }
    :: (state == 6) -> atomic{
      if
      :: flagXCK -> state = 0; flagXCK = 0; starts = 0;
      :: mrs_left -> state = 6; mrs_left = 0;
      fi
    }
    od
}

active proctype entorno() {
	do
	:: if
		 :: starts = 1
		 :: wrong = 1
     :: received = 1
		 :: correct = 1
     :: incorrect = 1
     :: maq = 1
     :: IAQ = 1
     :: maq_left = 1
     :: flagXCK = 1
     :: mrs = 1
     :: mrs_left = 1
     :: skip
     fi;
		 printf("E=%d, S=%d, W=%d, R=%d, C=%d, I=%d, MAQ=%d, IAQ=%d, MAQL=%d, F=%d, MRS=%d, MRSL=%d\n", state, starts, wrong, received, correct, incorrect, maq, IAQ, maq_left,flagXCK, mrs, mrs_left)
	od
}
