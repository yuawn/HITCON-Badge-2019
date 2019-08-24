/*******************************************************************
    Copyright (C) 2009 FreakLabs
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Originally written by Christopher Wang aka Akiba.
    Please post support questions to the FreakLabs forum.

*******************************************************************/
/*!
    \file Cmd.c

    This implements a simple command line interface for the Arduino so that
    its possible to execute individual functions within the sketch. 
*/
/**************************************************************************/

#include "Cmd.h"
#include "NuMicro.h"
#include "usbprintf.h"
#include "usbcom.h"
#include <string.h>
#include <stdlib.h>
// command line message buffer and pointer
static uint8_t msg[MAX_MSG_SIZE];
static uint8_t *msg_ptr;

// linked list for command table
//static cmd_t *cmd_tbl_list, *cmd_tbl;
static cmd_t cmd_tbl[0x20];

// text strings for command prompt (stored in flash)
const char cmd_banner[] = "\r\n  _  _ ___ _____ ___ ___  _  _   ___ __  _ ___ \r\n | || |_ _|_   _/ __/ _ \\| \\| | |_  )  \\/ / _ \\\r\n | __ || |  | || (_| (_) | .` |  / / () | \\_, /\r\n |_||_|___| |_| \\___\\___/|_|\\_| /___\\__/|_|/_/ \r\n                                               \r\n";
const char cmd_prompt[]  = "HitconBadge2019 >> ";
const char cmd_unrecog[]  = "HitconBadge2019: Command not found.";

/**************************************************************************/
/*!
    Generate the main command prompt
*/
/**************************************************************************/
void cmd_display()
{
    char buf[50];
    printfUSB("\r\n");
    strcpy(buf, cmd_prompt);
    printfUSB(buf);
}

/**************************************************************************/
/*!
    Parse the command line. This function tokenizes the command input, then
    searches for the command table entry associated with the commmand. Once found,
    it will jump to the corresponding function.
*/
/**************************************************************************/
void cmd_parse(char *cmd)
{
    uint8_t argc, i = 0;
    char *argv[30] = {NULL};
    char buf[50];
    //cmd_t *cmd_entry;

    USB_flush();

    // parse the command line statement and break it up into space-delimited
    // strings. the array of strings will be saved in the argv array.
    argv[i] = strtok(cmd, " ");
    if(argv[i] == NULL){
        //strcpy(buf, cmd_unrecog);
        //printfUSB(buf);
        //printfUSB("\r\n");

        cmd_display();
        return;
    }
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < 30) && (argv[i] != NULL));
    
    // save off the number of arguments for the particular command.
    argc = i;

    // parse the command table for valid command. used argv[0] which is the
    // actual command name typed in at the prompt

    for( int i = 0 ; i < 0x20 ; ++i ){
    	if( !strcmp( argv[0] , cmd_tbl[i].cmd ) ){
    		cmd_tbl[i].func( argc , argv );
    		cmd_display();
    		return;
    	}
    }

    /*for (cmd_entry = cmd_tbl; cmd_entry != NULL; cmd_entry = cmd_entry->next)
    {
        if (!strcmp(argv[0], cmd_entry->cmd))
        {
            cmd_entry->func(argc, argv);
            cmd_display();
            return;
        }
    }*/

    // command not recognized. print message and re-generate prompt.
    strcpy(buf, cmd_unrecog);
    printfUSB(buf);
    //printfUSB("\r\n");

    cmd_display();
}

/**************************************************************************/
/*!
    This function processes the individual characters typed into the command
    prompt. It saves them off into the message buffer unless its a "backspace"
    or "enter" key. 
*/
/**************************************************************************/
void cmd_handler()
{
    char c = USB_read();
    //if (c < 0x20 && c != '\r' && c != '\b')
    //{
    //    return;
    //}
    switch (c)
    {
    case '\r':
        // terminate the msg and reset the msg ptr. then send
        // it to the handler for processing.
        *msg_ptr = '\0';
        printfUSB("\r\n");
        cmd_parse((char *)msg);
        msg_ptr = msg;
        break;
    case '\b':
        // backspace 
        if (msg_ptr > msg)
        {
            USB_send(c);
            msg_ptr--;
        }
        break;
    case '\0':
        // do noting 
        break;
    default:
        // normal character entered. add it to the buffer
        if( msg_ptr - msg < MAX_MSG_SIZE ){
    		USB_send(c);
    		*msg_ptr++ = c;
    	}
        break;
    }
}

/**************************************************************************/
/*!
    This function should be set inside the main loop. It needs to be called
    constantly to check if there is any available input at the command prompt.
*/
/**************************************************************************/
void cmdPoll()
{
    while (USB_available()>0)
    {
        cmd_handler();
    }
}

/**************************************************************************/
/*!
    Initialize the command line interface. This sets the terminal speed and
    and initializes things. 
*/
/**************************************************************************/
void cmdInit()
{
    // init the msg ptr
    msg_ptr = msg;

    // init the command table
    //cmd_tbl_list = NULL;
    //printfUSB( "[Shell] cmd_tbl size: %d\r\n" , sizeof( cmd_tbl ) );
    memset( cmd_tbl , 0 , sizeof( cmd_tbl ) );

    // set the serial speed
    //Serial.begin(speed);
    printfUSB(cmd_banner);
    cmd_display();
}

/**************************************************************************/
/*!
    Add a command to the command table. The commands should be added in
    at the setup() portion of the sketch. 
*/
/**************************************************************************/
void cmdAdd(char *name, void (*func)(int argc, char **argv))
{

	for( int i = 0 ; i < 0x20 ; ++i ){
		if( cmd_tbl[i].func == NULL ){
            cmd_tbl[i].func = func;
			strcpy( cmd_tbl[i].cmd , name );
			break;
		}
	}

	/*
    // alloc memory for command struct
    cmd_tbl = (cmd_t *)malloc(sizeof(cmd_t));

    // alloc memory for command name
    char *cmd_name = (char *)malloc(strlen(name)+1);

    // copy command name
    strcpy(cmd_name, name);

    // terminate the command name
    cmd_name[strlen(name)] = '\0';

    // fill out structure
    cmd_tbl->cmd = cmd_name;
    cmd_tbl->func = func;
    cmd_tbl->next = cmd_tbl_list;
    cmd_tbl_list = cmd_tbl;
    */
}

int cmd_alias( char* old , char* new ){

	for( int i = 0 ; i < 0x20 ; ++i ){
		if( !strcmp( old , cmd_tbl[i].cmd ) ){
			printfUSB( "alias %s=%s" , new , old );
			cmdAdd( new , cmd_tbl[i].func );
			return 1;
		}
	}
	return 0;
}

/**************************************************************************/
/*!
    Convert a string to a number. The base must be specified, ie: "32" is a
    different value in base 10 (decimal) and base 16 (hexadecimal).
*/
/**************************************************************************/
uint32_t cmdStr2Num(char *str, uint8_t base)
{
    return strtol(str, NULL, base);
}
