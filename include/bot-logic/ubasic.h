/*
 * Copyright (c) 2006, Adam Dunkels
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * ------------------------------------------------------
 * Source modified by Uwe Berger (bergeruw@gmx.net); 2010
 * ------------------------------------------------------
 */

/**
 * \file 	ubasic.h
 * \brief 	uBasic-Interpreter
 * \author	Adam Dunkels
 * \author 	Uwe Berger (bergeruw@gmx.net)
 * \date 	10.06.2010
 */

#ifndef __UBASIC_H__
#define __UBASIC_H__

#ifdef BEHAVIOUR_UBASIC_AVAILABLE

// Error-Nummern
#define SYNTAX_ERROR			1
#define UNKNOWN_ADC_CHANNEL		2
#define UNKNOWN_IO_PORT			3
#define FOR_WITHOUT_TO			4
#define UNKNOWN_STATEMENT		5
#define UNKNOWN_CALL_FUNCT		6
#define UNKNOWN_CALL_FUNCT_TYP	7
#define UNKNOWN_CVAR_NAME		8

int current_linenum;

void ubasic_init(const char * program);
void ubasic_run(void);
int ubasic_finished(void);

int ubasic_get_variable(int varnum);
void ubasic_set_variable(int varum, int value);

void accept(int token);
int expr(void);
void ubasic_break(void);
void line_statement(void);

#endif // BEHAVIOUR_UBASIC_AVAILABLE
#endif // __UBASIC_H__
