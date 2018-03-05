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

#include "rbtree.h"
#include "opts_aux.h"

struct module
{
    struct process *parent_process;
    char *filename;
    char *filename_without_ext;
    char *path;
    address base;
    byte saved_OEP_byte; // in case of 'main' executable
    rbtree *symbols; // -> address, symbol
    bool skip_all_symbols_in_module_on_trace;
    
    // cc
    rbtree *PC_infos; // address, PC_info
    
    // PE info
    address OEP;
    SIZE_T size;
    DWORD PE_timestamp;
    address original_base;
    char *internal_name; // may be NULL
    IMAGE_SECTION_HEADER *sections; // ptr to array of sections. allocated via DMALLOC. deep copy here.
    unsigned sections_total;

    // for one-time INT3 breakpoints
    rbtree* INT3_BP_bytes; // address, byte

    // if dump_seh is true
    bool security_cookie_adr_known;
    address security_cookie_adr;
};

struct module* add_module (struct process *p, address img_base, HANDLE file_hdl, struct MemoryCache *mc);
void unload_module_and_free(struct module *m);
void remove_module (struct process *p, address img_base);
bool address_in_module (struct module *m, address a);
void module_get_sym (struct module *m, address a, bool add_module_name, bool add_offset, strbuf *out);
struct symbol* module_sym_exist_at (struct module *m, address a);
char *get_module_name (struct module *m);
bool module_adr_in_executable_section (struct module *m, address a);
address get_module_end(struct module *m);
address module_get_next_sym_address_after (struct module *m, address a);

/* vim: set expandtab ts=4 sw=4 : */
