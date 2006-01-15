/*! @file 	rc5.c
 * @brief 	RC5-Fernbedienung
 * Eventuell muessen die Codes an die jeweilige 
 * Fernbedienung angepasst werden
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	20.12.05
*/
#include "ct-Bot.h"
#include "ir.h"
#include "bot-mot.h"
#include "bot-logik.h"

#ifdef RC5_AVAILABLE

/*!
 * Fernbedienung mit Jog-Dial-Rad, 
 * Achtung dann muessen auch die Adress-Bits auf die FB angepasst werden
 */
//#define JOG_DIAL

#define RC5_TOGGLE		0x0800		///< Das RC5-Toggle-Bit
#define RC5_ADDRESS		0x07C0		///< Der Adress-bereich
#define RC5_COMMAND		0x103F		///< Der Kommandobereich

#define RC5_MASK (RC5_COMMAND)		///< Welcher Teil des Kommandos wird ausgewertet?


#define RC5_CODE_0	(0x3940 & RC5_MASK)		///< Taste 0
#define RC5_CODE_1	(0x3941 & RC5_MASK)		///< Taste 1
#define RC5_CODE_2	(0x3942 & RC5_MASK)		///< Taste 2
#define RC5_CODE_3	(0x3943 & RC5_MASK)		///< Taste 3
#define RC5_CODE_4	(0x3944 & RC5_MASK)		///< Taste 4
#define RC5_CODE_5	(0x3945 & RC5_MASK)		///< Taste 5
#define RC5_CODE_6	(0x3946 & RC5_MASK)		///< Taste 6
#define RC5_CODE_7	(0x3947 & RC5_MASK)		///< Taste 7
#define RC5_CODE_8	(0x3948 & RC5_MASK)		///< Taste 8
#define RC5_CODE_9	(0x3949 & RC5_MASK)		///< Taste 9

#define RC5_CODE_UP		(0x2950 & RC5_MASK)		///< Taste Hoch
#define RC5_CODE_DOWN	(0x2951 & RC5_MASK)	///< Taste Runter
#define RC5_CODE_LEFT	(0x2955 & RC5_MASK)	///< Taste Links
#define RC5_CODE_RIGHT	(0x2956 & RC5_MASK)	///< Taste Rechts

#define RC5_CODE_PWR	(0x394C & RC5_MASK)	///< Taste An/Aus

#ifdef JOG_DIAL		
	// Jogdial geht nur inkl. Adresscode
	#undef RC5_MASK
	#define RC5_MASK (RC5_COMMAND |RC5_ADDRESS )

	#define RC5_CODE_JOG_MID	(0x3969 & RC5_MASK)	///< Taste Jog-Dial Mitte
	#define RC5_CODE_JOG_L1		(0x3962 & RC5_MASK)	///< Taste Jog-Dial Links 1
	#define RC5_CODE_JOG_L2		(0x396F & RC5_MASK)	///< Taste Jog-Dial Links 2
	#define RC5_CODE_JOG_L3		(0x395F & RC5_MASK)	///< Taste Jog-Dial Links 3
	#define RC5_CODE_JOG_L4		(0x3A6C & RC5_MASK)	///< Taste Jog-Dial Links 4
	#define RC5_CODE_JOG_L5		(0x3A6B & RC5_MASK)	///< Taste Jog-Dial Links 5
	#define RC5_CODE_JOG_L6		(0x396C & RC5_MASK)	///< Taste Jog-Dial Links 6
	#define RC5_CODE_JOG_L7		(0x3A6A & RC5_MASK)	///< Taste Jog-Dial Links 7
	
	#define RC5_CODE_JOG_R1		(0x3968 & RC5_MASK)	///< Taste Jog-Dial Rechts 1
	#define RC5_CODE_JOG_R2		(0x3975 & RC5_MASK)	///< Taste Jog-Dial Rechts 2
	#define RC5_CODE_JOG_R3		(0x396A & RC5_MASK)	///< Taste Jog-Dial Rechts 3
	#define RC5_CODE_JOG_R4		(0x3A6D & RC5_MASK)	///< Taste Jog-Dial Rechts 4
	#define RC5_CODE_JOG_R5		(0x3A6E & RC5_MASK)	///< Taste Jog-Dial Rechts 5
	#define RC5_CODE_JOG_R6		(0x396E & RC5_MASK)	///< Taste Jog-Dial Rechts 6
	#define RC5_CODE_JOG_R7		(0x3A6F & RC5_MASK)	///< Taste Jog-Dial Rechts 7
#endif

uint16 RC5_Code;        ///< Letzter empfangener RC5-Code

/*!
 * Liest einen RC5-Codeword und wertet ihn aus
 */
void rc5_control(void){
	uint16 rc5=ir_read();
	if (rc5 !=0) {
		RC5_Code= rc5 & RC5_MASK;		// Alle uninteressanten Bits ausblenden
		
		switch(RC5_Code){
			case RC5_CODE_PWR:	
					speed_l=BOT_SPEED_STOP;
					speed_r=BOT_SPEED_STOP;	
				break;		

			case RC5_CODE_UP:	
					speed_l+=1;
					speed_r+=1;
				break;
			case RC5_CODE_DOWN:	
					speed_l-=1;
					speed_r-=1;
				break;
			case RC5_CODE_LEFT:	
					speed_l-=1;
				break;
			case RC5_CODE_RIGHT:	
					speed_r-=1;
				break;
			
				
			case RC5_CODE_1:	
					speed_l=BOT_SPEED_SLOW;
					speed_r=BOT_SPEED_SLOW;	
				break;					
			case RC5_CODE_3:	
					speed_l=BOT_SPEED_MAX;
					speed_r=BOT_SPEED_MAX;	
				break;
			
			case RC5_CODE_5: bot_goto(0,0);	break;
			case RC5_CODE_6: bot_goto(20,-20);	break;
			case RC5_CODE_4: bot_goto(-20,20);	break;
			case RC5_CODE_2: bot_goto(100,100);	break;				
			case RC5_CODE_8: bot_goto(-100,-100);	break;				
			case RC5_CODE_7: bot_goto(-40,40);	break;
			case RC5_CODE_9: bot_goto(40,-40);	break;

			#ifdef JOGDIAL
				case RC5_CODE_JOG_MID:
						speed_l=BOT_SPEED_MAX;
						speed_r=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_L1:
						speed_l=BOT_SPEED_FAST;
						speed_r=BOT_SPEED_MAX;	
				case RC5_CODE_JOG_L2:
						speed_l=BOT_SPEED_NORMAL;
						speed_r=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_L3:
						speed_l=BOT_SPEED_SLOW;
						speed_r=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_L4:
						speed_l=BOT_SPEED_STOP;
						speed_r=BOT_SPEED_MAX;	
					break;
					
				case RC5_CODE_JOG_L5:
						speed_l=-BOT_SPEED_NORMAL;
						speed_r=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_L6:
						speed_l=-BOT_SPEED_FAST;
						speed_r=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_L7:
						speed_l=-BOT_SPEED_MAX;
						speed_r=BOT_SPEED_MAX;	
					break;
					
				case RC5_CODE_JOG_R1:
						speed_r=BOT_SPEED_FAST;
						speed_l=BOT_SPEED_MAX;	
				case RC5_CODE_JOG_R2:
						speed_r=BOT_SPEED_NORMAL;
						speed_l=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_R3:
						speed_r=BOT_SPEED_SLOW;
						speed_l=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_R4:
						speed_r=BOT_SPEED_STOP;
						speed_l=BOT_SPEED_MAX;	
					break;
					
				case RC5_CODE_JOG_R5:
						speed_r=-BOT_SPEED_NORMAL;
						speed_l=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_R6:
						speed_r=-BOT_SPEED_FAST;
						speed_l=BOT_SPEED_MAX;	
					break;
				case RC5_CODE_JOG_R7:
						speed_r=-BOT_SPEED_MAX;
						speed_l=BOT_SPEED_MAX;	
					break;			
			#endif
		}

	}
}
#endif
