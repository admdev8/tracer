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
#include "strbuf.h"

struct process
{
    DWORD PID;
    HANDLE PHDL;
    address base_of_image;
    HANDLE file_handle;

    address PEB;

    struct module *executable_module;

    rbtree *threads; // -> TID, thread
    rbtree *modules; // base_address (in process), module

    bool we_are_loading_and_OEP_was_executed;

    octa ins_emulated;
    octa ins_not_emulated;

    bool INT3_DURING_FUNC_SKIP_used[4];
    address INT3_DURING_FUNC_SKIP_addresses[4];
    byte INT3_DURING_FUNC_SKIP_byte[4];
};

struct process* process_init (DWORD PID, HANDLE PHDL, HANDLE file_handle, LPVOID base_of_image); 
void process_free (struct process *p);
struct process *find_process(DWORD PID);
struct module *find_module_for_address (struct process *p, address a);
struct symbol *process_sym_exist_at (struct process *p, address a);
void process_get_sym (struct process *p, address a, bool add_module_name, bool add_offset, strbuf *out);
bool adr_in_executable_section(struct process *p, address a);
address process_get_next_sym_address_after (struct process *p, address a);

/* vim: set expandtab ts=4 sw=4 : */
