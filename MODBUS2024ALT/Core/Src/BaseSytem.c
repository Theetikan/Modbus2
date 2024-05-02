/*
 * BaseSytem.c
 *
 *  Created on: May 1, 2024
 *      Author: ponpon
 */
#include <BaseSystem.h>

State status;
ShelvePosition shelve;
GetValue Value;

void Heartbeat(){
	static uint64_t timestamp = 0;
		 	  if(HAL_GetTick() >= timestamp)
		 	  {
		 		  timestamp += 200;
				  //if (registerFrame[0].U16 == 18537) {
		 		 //registerFrame[0].U16 = 0x00;
		 		  registerFrame[0x00].U16 = 22881;
				  //}
		 	  }
}

void Routine(){
	static uint64_t timestamp2 = 0;
		if(HAL_GetTick() >= timestamp2) //ส่งค่าพวกนี้หลังจาก BaseSytem อ่าน Heartbeat ทุกๆ 200 ms
		{
		timestamp2 += 200;
		if (registerFrame[0x00].U16 == 18537){
			status.HeartFlag = 1 ; // ได้รับค่าจาก Heartbeat Protocal
		//BaseSytem Read

		//เช็คค่า Lead Switch ว่าอันไหนติด
		//if (lead switch1 == 1){
		registerFrame[0x04].U16 = 0b0000;
		registerFrame[0x04].U16 = 0b0010;
		//}
		//else if (lead switch2 == 1){
		registerFrame[0x04].U16 = 0b0000;
		registerFrame[0x04].U16 = 0b0001;//Gripper Movement Actual Status = 'Lead Switch 2 Status'
		//}
		registerFrame[0x10].U16 = status.Z_Status; //Z-axis Moving Status = Set Shelve
		registerFrame[0x11].U16 = 19*10; //Z-axis Actual Position = 19 ค่าจริง*10
		registerFrame[0x12].U16 = 20*10; //Z-axis Actual Speed = 20
		registerFrame[0x13].U16 = 21*10; //Z-axis Acceleration = 21
		registerFrame[0x40].U16 = 22*10; //X-axis Actual Position = 22
		}
}
}
void VacuumOn_Off(){ // อ่านค่า Vacuum จากการกดปุ่มที่ BaseSytem
	if(registerFrame[0x02].U16 == 0b0000){
		status.VacuumState = 0; //Off
	}
	if(registerFrame[0x02].U16 == 0b0001){
		status.VacuumState = 1; //On
		}
}

void Gripper_Movement(){ // อ่านค่า Gripper จากการกดปุ่มที่ BaseSytem
	if(registerFrame[0x03].U16 == 0b0000){
		status.GripperState = 0; //Backward
		}
	if(registerFrame[0x03].U16 == 0b0001){
		status.GripperState = 1; //Forward
			}
}

void Set_Shelves(){ //Setting Shelve Position
	if (registerFrame[0x01].U16 == 1){
		status.Z_Status = 1;
		registerFrame[0x10].U16 = status.Z_Status; // z-axis update z-xis moving status to "set shelves"

		//Joggig for set shelve

		registerFrame[0x23].U16 = shelve.ShelvePosition_1; // ค่า Shelve ที่ต้องส่งให้ BaseSytem
		registerFrame[0x24].U16 = shelve.ShelvePosition_2;
		registerFrame[0x25].U16 = shelve.ShelvePosition_3;
		registerFrame[0x26].U16 = shelve.ShelvePosition_4;
		registerFrame[0x27].U16 = shelve.ShelvePosition_5;

		// reset z-axis moving state after finish Jogging
		//if (Finish Jogging){
		if (status.reset == 1){ //if reset state == 1 จะทำการรีเซ็ตระบบให้กดทุกอย่างได้เหมือนเดิม
		status.reset = 0;
		registerFrame[0x01].U16 = 0;

		status.Z_Status = 0; // z-axis reset BaseSystem status
		registerFrame[0x10].U16 = status.Z_Status;
		}
		//}

	}
}
void GetGoalPoint(){
	if(registerFrame[0x01].U16 == 8){ // if run point mode
		Value.GoalPoint = (registerFrame[0x30].U16)/10 ; //Get Goal point from BaseSytem(Point Mode) that we pick/write After pressing Run Button
		 //ค่าที่ได้จาก BaseSytem จะได้ค่าที่เรากรอก*10 ดังนั้นต้องหาร10 ถึงจะได้ค่าจริงที่เรากรอก
	}
}

void RunPointMode(){
	if(registerFrame[0x01].U16 == 8){ //if run point mode
		status.Z_Status = 16;
		registerFrame[0x10].U16 = status.Z_Status; //update Z Status "Go Point"

		//going to point (use Goal point(0x30) for target z-axis position)

		//if (Gripper at GoalPoint){
		if (status.reset == 1){ //if reset state == 1 จะทำการรีเซ็ตระบบให้กดทุกอย่างได้เหมือนเดิม
		status.reset = 0;
		registerFrame[0x01].U16 = 0; //Reset BaseSystem Status
		status.Z_Status = 0;
		registerFrame[0x10].U16 = status.Z_Status; // reset z-axis moving state after finish jogging
		//}
		}
		}
	}

void SetHome(){
	if(registerFrame[0x01].U16 == 2){ //BaseSystem Status "Home"
		status.Z_Status = 2;
		registerFrame[0x10].U16 = status.Z_Status; // update Z-axis moving status "Home"

		//Homing

		//if (Gripper at HomePoint){
		if (status.reset == 1){ //if reset state == 1 จะทำการรีเซ็ตระบบให้กดทุกอย่างได้เหมือนเดิม
		status.reset = 0;
		registerFrame[0x01].U16 = 0; //reset Base System Status

		status.Z_Status = 0;
		registerFrame[0x10].U16 = status.Z_Status; // reset z-axis moving state after finish homing
		}
		//}
		}
}

void GetPick_PlaceOrder(){
	if(registerFrame[0x01].U16 == 4){ // after pressing run button on Jogmode (before running)
		Value.PickOder = registerFrame[0x21].U16 ; // ค่าชั้นที่ต้อง Pick from BaseSystem
		Value.PlaceOder = registerFrame[0x22].U16 ;// ค่าชั้นที่ต้อง Place from BaseSystem
		//ค่าที่ได้จะเรียงติดกัน ex.ถ้าเซ็ตค่าในUIชั้นแรกที่ต้อง Pick คือ ชั้น1-5 ตามลำดับ ค่าชั้นที่ต้องPick จะได้ 12345
	}
}

void RunJogMode(){
	if(registerFrame[0x01].U16 == 4){ //after pressing run button on Jogmode

		//Loop{
		//Pick
		status.Z_Status = 4;
		registerFrame[0x10].U16 = status.Z_Status; // go pick state

		//Going to Pick from Shelve 5 round(Use PickOder to do task)
		//When finish Pick from round(i) shelve --> Go Place

		//Place
		status.Z_Status = 8;
		registerFrame[0x10].U16 = status.Z_Status; // go place state

		//Going to Place from Shelve 5 round(Use PlaceOder to do task)
		//When finish Place from round(i) shelve --> Return Pick
		//When Finish Place round 5 --> Out of Loop
		//}

		//if(All Pick&Place == Finish){
		if (status.reset == 1){ //if reset state == 1 จะทำการรีเซ็ตระบบให้กดทุกอย่างได้เหมือนเดิม
		status.reset = 0;
		registerFrame[0x01].U16 = 0;

		status.Z_Status = 0;
		registerFrame[0x10].U16 = status.Z_Status; // after finish jogging
		//}
		}
	}
}

