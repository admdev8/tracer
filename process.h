/*
 *  _______                      
 * |__   __|                     
 *    | |_ __ __ _  ___ ___ _ __ 
 *    | | '__/ _` |/ __/ _ \ '__|
 *    | | | | (_| | (_|  __/ |   
 *    |_|_|  \__,_|\___\___|_|   
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#pragma once

#include <windows.h>

#include "rbtree.h"
#include "dmalloc.h"
#include "address.h"
#include "strbuf.h"

typedef struct _module module;
typedef struct _symbol symbol;

typedef struct _process
{
    DWORD PID;
    HANDLE PHDL;
    address base_of_image;
    HANDLE file_handle;

    module *executable_module;

    rbtree *threads; // -> TID, thread
    rbtree *modules; // base_address (in process), module

    bool we_are_loading_and_OEP_was_executed;

    octabyte ins_emulated;
    octabyte ins_not_emulated;
} process;

process* process_init (DWORD PID, HANDLE PHDL, HANDLE file_handle, LPVOID base_of_image); 
void process_free (process *p);
process *find_process(DWORD PID);
module *find_module_for_address (process *p, address a);
symbol *process_sym_exist_at (process *p, address a);
void process_get_sym (process *p, address a, bool add_module_name, bool add_offset, strbuf *out);
bool adr_in_executable_section(process *p, address a);
address process_get_next_sym_address_after (process *p, address a);

/* vim: set expandtab ts=4 sw=4 : */
