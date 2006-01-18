/*! @file 	bot-2-sim.c 
 * @brief 	Verbindung c't-Bot zu c't-Sim
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	26.12.05
*/

#include "ct-Bot.h"

#ifdef PC

#include "bot-2-sim.h"
#include "tcp.h"
#include "command.h"
#include "display.h"
#include "sensor.h"
#include "bot-logik.h"
#include "motor.h"
#include "command.h"

#include <stdio.h>      // for printf() and fprintf()
#include <stdlib.h>     // for atoi() and exit()
#include <sys/time.h>
#include <pthread.h>
#include <time.h>

/* Linux with glibc:
 *   _REENTRANT to grab thread-safe libraries
 *   _POSIX_SOURCE to get POSIX semantics
 */
#ifdef __linux__
#  define _REENTRANT
//#  define _POSIX_SOURCE
#endif

/* Hack for LinuxThreads */
#ifdef __linux__
#  define _P __P
#endif

#define low_init tcp_init	///< Low-Function zum initialisieren

pthread_t simThread;			///< Simuliert den Bot
pthread_t bot_2_sim_Thread;		///< Thread sammelt Sensor Daten, überträgt Motor-Daten

pthread_cond_t      command_cond  = PTHREAD_COND_INITIALIZER;	///< Schüztt das Commando
pthread_mutex_t     command_cond_mutex = PTHREAD_MUTEX_INITIALIZER;	///< Schüztt das Commando

void signal_command_available(void);
int wait_for_command(int timeout_s);

#ifdef WIN32
	 /* These are winbase.h definitions, but to avoid including
	tons of Windows related stuff, it is reprinted here */
	
	typedef struct _FILETIME {
		unsigned long dwLowDateTime;
		unsigned long dwHighDateTime;
	} FILETIME;
	
	void __stdcall GetSystemTimeAsFileTime(FILETIME*);	
	void gettimeofday(struct timeval* p, void* tz /* IGNORED */);
	
	void gettimeofday(struct timeval* p, void* tz /* IGNORED */){
		union {
			long long ns100; // time since 1 Jan 1601 in 100ns units 
			FILETIME ft;
		} _now;
	
		GetSystemTimeAsFileTime( &(_now.ft) );
		p->tv_usec=(long)((_now.ns100 / 10LL) % 1000000LL );
		p->tv_sec= (long)((_now.ns100-(116444736000000000LL))/10000000LL);
		return;
	}
#endif

/*! 
 * Dieser Thread nimmt die Daten vom PC entgegen
 */
void *bot_2_sim_rcv_isr(void * arg){
	display_cursor(11,1);
	printf("bot_2_sim_rcv_isr() comming up\n");
	for (;;){
		// only write if noone reads command
		if (command_read()!=0)
			printf("Error reading command\n");			// read a command
		else {		
//			command_display(&received_command);	// show it
			if (command_evaluate() ==0)			// use data transfered
				signal_command_available();		// tell anyone waiting
		}
	}
	return 0;
}

/*!
 * Ein wenig Initilisierung kann nicht schaden 
 */
void bot_2_sim_init(void){
	low_init();
		
	if (pthread_create(&bot_2_sim_Thread,  // thread struct
		NULL,		      // default thread attributes
		bot_2_sim_rcv_isr,	      // start routine
		NULL)) {              // arg to routine
			printf("Thread Creation failed");
			exit(1);
	}
}


int count=1;	///< Zähler für Packet-Sequenznummer
int not_answered_error=1;


/*!
 *  Tell simulator data -- dont wait for answer!
 */
void bot_2_sim_tell(uint8 command, uint8 subcommand, int16* data_l,int16* data_r){
	command_t cmd;
	
	cmd.startCode=CMD_STARTCODE;
	cmd.request.direction=DIR_REQUEST;		// Anfrage
	cmd.request.command= command;
	cmd.request.subcommand= subcommand;
	
	cmd.payload=0x00;
	cmd.data_l=*data_l;
	cmd.data_r=*data_r;
	cmd.seq=count++;
	cmd.CRC=CMD_STOPCODE;
	
	tcp_write((char *)&cmd,sizeof(command_t));
}



/*! 
 * Wartet auf die Antwort des PCS
 * @param timeout_s Wartezeit in Sekunden
 * @return 0 Wenn Ok
 */
int wait_for_command(int timeout_s){
	struct timespec   ts;
	struct timeval    tp;
	int result=0;
	
	pthread_mutex_lock(&command_cond_mutex);
	
	gettimeofday(&tp, NULL);
	// Convert from timeval to timespec
	ts.tv_sec  = tp.tv_sec;
	ts.tv_nsec = tp.tv_usec * 1000;
	ts.tv_sec += timeout_s;

	result= pthread_cond_timedwait(&command_cond, &command_cond_mutex, &ts);
	
	pthread_mutex_unlock(&command_cond_mutex);
	
	return result;
}

/*!
 * Benachrichtigt wartende Threads über eingetroffene Connands
 */
void signal_command_available(void){
	pthread_mutex_lock(&command_cond_mutex);
	pthread_cond_signal(&command_cond);
	pthread_mutex_unlock(&command_cond_mutex);
}

/*!
 * Schickt einen Thread in die Warteposition
 * @param timeout_us Wartezeit in µs
 */
void wait_for_time(long timeout_us){
	pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
	struct timespec   ts;
	struct timeval    tp;
	
	pthread_mutex_lock(&mutex);
	gettimeofday(&tp, NULL);
	// Convert from timeval to timespec
	
	tp.tv_usec += (timeout_us % 1000000);
	tp.tv_sec += (timeout_us / 1000000);
	
	ts.tv_sec  = tp.tv_sec+ (tp.tv_usec/1000000);
	ts.tv_nsec = (tp.tv_usec % 1000000)* 1000;
	
	pthread_cond_timedwait(&cond, &mutex, &ts);
	pthread_mutex_unlock(&mutex);
}
#endif
