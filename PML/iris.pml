/*
ltl spec IDLE {
[] (((state == 0) && starts) -> <> (state == 1))
}
//CHECKED



*/
ltl spec {
[] (((state == 0) && button_on) -> <> (state == 1))
}




byte state;
bit button_on;
bit button_off;
bit time_on;
bit flag_ACK_and_msg_IAQ_left;
bit flag_ACK_and_msg_IAQ;
bit initialized_and_maq_now;
bit flag_ACK_and_msg_MAQ_left;
bit flag_ACK_and_msg_MAQ;
bit bits_received_not_all_msg;
bit all_msg;
bit msg_checked;
bit all_received_and_checked;
bit initialized_and_mrs_now;
bit flag_ACK_and_msg_MRS_left;
bit flag_ACK_and_msg_MRS;


active proctype sensor_fsm () {
    state = 0;
    do
    :: (state == 0) -> atomic {
      if
      :: button_on -> state = 1; button_on = 0; button_off = 0; initialized_and_maq_now = 0; time_on = 0; initialized_and_mrs_now = 0;
      fi
    }
    :: (state == 1) -> atomic {
      if
      :: button_off -> state = 0; button_off = 0; button_on = 0;
      :: time_on -> state = 2; flag_ACK_and_msg_IAQ_left = 0; flag_ACK_and_msg_IAQ = 0;
      :: initialized_and_maq_now -> state = 3; flag_ACK_and_msg_MAQ_left = 0; flag_ACK_and_msg_MAQ = 0;
      :: initialized_and_mrs_now -> state = 6; flag_ACK_and_msg_MRS_left = 0; flag_ACK_and_msg_MRS = 0;
      fi
    }
    :: (state == 2) -> atomic {
      if
      :: flag_ACK_and_msg_IAQ_left -> state = 2; flag_ACK_and_msg_IAQ_left = 0;
      :: flag_ACK_and_msg_IAQ -> state = 1; flag_ACK_and_msg_IAQ = 0; button_off = 0; initialized_and_maq_now = 0; time_on = 0; initialized_and_mrs_now = 0;
      fi
    }
    :: (state == 3) -> atomic {
      if
      :: flag_ACK_and_msg_MAQ_left -> state = 3; flag_ACK_and_msg_MAQ_left = 0;
      :: flag_ACK_and_msg_MAQ -> state = 4; flag_ACK_and_msg_MAQ = 0; bits_received_not_all_msg = 0; all_msg = 0; all_received_and_checked = 0;
      fi
    }
    :: (state == 4) -> atomic {
      if
      :: bits_received_not_all_msg -> state = 4; bits_received_not_all_msg = 0;
      :: all_msg -> state = 5; all_msg = 0; msg_checked = 0;
      :: all_received_and_checked -> state = 0; all_received_and_checked = 0; button_off = 0; initialized_and_maq_now = 0; time_on = 0; initialized_and_mrs_now = 0;
      fi
    }
    :: (state == 5) -> atomic {
      if
      :: msg_checked -> state = 4; msg_checked = 0; bits_received_not_all_msg = 0; all_msg = 0; all_received_and_checked = 0;
      fi
    }
    :: (state == 6) -> atomic {
      if
      :: flag_ACK_and_msg_MRS_left -> state = 6; flag_ACK_and_msg_MRS_left = 0;
      :: flag_ACK_and_msg_MRS -> state = 7; flag_ACK_and_msg_MRS = 0; bits_received_not_all_msg = 0; all_msg = 0; all_received_and_checked = 0;
      fi
    }
    :: (state == 7) -> atomic {
      if
      :: bits_received_not_all_msg -> state = 7; bits_received_not_all_msg = 0;
      :: all_msg -> state = 8; all_msg = 0; msg_checked = 0;
      :: all_received_and_checked -> state = 0; all_received_and_checked = 0; button_off = 0; initialized_and_maq_now = 0; time_on = 0; initialized_and_mrs_now = 0;
      fi
    }
    :: (state == 8) -> atomic {
      if
      :: msg_checked -> state = 7; msg_checked = 0; bits_received_not_all_msg = 0; all_msg = 0; all_received_and_checked = 0;
      fi
    }
    od
}

active proctype entorno() {
	do
	:: if
		 :: button_on = 1
		 :: button_off = 1
     :: time_on = 1
		 :: flag_ACK_and_msg_IAQ_left = 1
     :: flag_ACK_and_msg_IAQ = 1
     :: initialized_and_maq_now = 1
     :: flag_ACK_and_msg_MAQ_left = 1
     :: flag_ACK_and_msg_MAQ = 1
     :: bits_received_not_all_msg = 1
     :: all_msg = 1
     :: msg_checked = 1
     :: all_received_and_checked = 1
     :: initialized_and_mrs_now = 1
     :: flag_ACK_and_msg_MAQ_left = 1
     :: flag_ACK_and_msg_MAQ = 1
     :: skip
     fi;
		 printf("STATE = %d, Button_on = %d, Button_off = %d, Time_on = %d, Flag_ACK_and_msg_IAQ_left = %d, Flag_ACK_and_msg_IAQ = %d", state, button_on, button_off, time_on, flag_ACK_and_msg_IAQ_left, flag_ACK_and_msg_IAQ)
	od
}
