/*! @file 	rc5.h
 * @brief 	RC5-Fernbedienung
 * @author 	Benjamin Benz (bbe@heise.de)
 * @date 	26.12.05
*/

#ifndef rc5_H_
#define rc5_H_


extern uint16 RC5_Code;        ///< Letzter empfangener RC5-Code

/*!
 * Liest einen RC5-Codeword und wertet ihn aus
 */
void rc5_control(void);
#endif
