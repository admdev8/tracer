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

#include <assert.h>
#include <stdbool.h>

#include "CONTEXT_utils.h"
#include "bitfields.h"
#include "disas_utils.h"
#include "cc.h"
#include "process.h"
#include "module.h"
#include "utils.h"
#include "stuff.h"
#include "X86_register_helpers.h"
#include "set.h"
#include "oassert.h"
#include "x86.h"

#define IDA_MAX_COMMENT_SIZE 1023
#define STRING_LEN 4

// big enough to hold all instructions till I_MAX_INS
// FIXME: add same thing in emulator!
static bool ins_reported_as_unhandled[512]={ false };

static int compare_doubles(void* leftp, void* rightp)
{
    double left = *(double*)leftp, right = *(double*)rightp;
    //printf ("%s() left=%f right=%f\n", __func__, left, right);
    
    if (left==right)
        return 0;
    
    if (left < right)
        return -1;
    
    if (left > right)
        return 1;

    // NaNs case
    return memcmp (leftp, rightp, sizeof(double)); // it's not correct, but what can I do
};

unsigned what_to_notice (process *p, Da *da, strbuf *comments, CONTEXT *ctx, MemoryCache *mc)
{
    if (cc_c_debug)
        L ("%s() begin\n", __func__);
    unsigned rt=0;

    address PC=CONTEXT_get_PC(ctx);

    if (da==NULL)
        return 0; // wasn't disassembled: do nothing

    //if (Da_ins_is_FPU(da))
    //    return 0; // do nothing (yet)

    bool op_present[3]={ false, false, false };

    if (da->ops_total>=1)
        op_present[0]=true;
    if (da->ops_total>=2)
        op_present[1]=true;
    if (da->ops_total==3)
        op_present[2]=true;

    switch (da->ins_code)
    {
        // single operand
        case I_PUSH:
        case I_INC:
        case I_DEC:
        case I_NOT:
        case I_NEG:
        case I_BSWAP:
        case I_LDMXCSR:
            SET_BIT (rt, NOTICE_OP1);
            break;

        case I_JMP:
            if (da->op[0].type==DA_OP_TYPE_REGISTER || da->op[0].type==DA_OP_TYPE_VALUE_IN_MEMORY)
                SET_BIT (rt, NOTICE_OP1);
            break;

        case I_STOSB:
        case I_STOSW:
        case I_STOSD:
        case I_REP_STOSB:
        case I_REP_STOSW:
        case I_REP_STOSD:
            SET_BIT (rt, NOTICE_AX);
            SET_BIT (rt, NOTICE_CX);
            break;

        case I_LODSB:
            //case I_LODSW:
        case I_LODSD:
            // TODO: ESI?
            break;

        case I_REP_MOVSB:
        case I_REP_MOVSW:
        case I_REP_MOVSD:
            SET_BIT (rt, NOTICE_CX);
            break;

            // FIXME: unfinished here
        case I_REPNE_SCASB:
        case I_REPNE_SCASW:
        case I_REPNE_SCASD:
            SET_BIT (rt, NOTICE_CX);
            break;

            // FIXME: unfinished here
        case I_REPE_SCASB:
        case I_REPE_SCASW:
        case I_REPE_SCASD:
            SET_BIT (rt, NOTICE_CX);
            break;

            // FIXME: unfinished here
        case I_REP_CMPSB:
        case I_REP_CMPSW:
        case I_REP_CMPSD:
            SET_BIT (rt, NOTICE_CX);
            break;

        case I_LOOP:
            SET_BIT (rt, NOTICE_CX);
            break;

            // single operand, AX, DX
        case I_DIV:
        case I_IDIV:
            SET_BIT (rt, NOTICE_OP1);
            SET_BIT (rt, NOTICE_AX);
            SET_BIT (rt, NOTICE_DX);
            break;

        case I_CALL:
            if (da->op[0].type==DA_OP_TYPE_REGISTER || da->op[0].type==DA_OP_TYPE_VALUE_IN_MEMORY) // add symbol to comment
            {
                address adr;
                obj val;

                strbuf_addstr (comments, "op1=");
                if (Da_op_get_value_of_op (&da->op[0], &adr, ctx, mc, __FILE__, __LINE__, &val))
                    process_get_sym (p, obj_get_as_REG (&val), true, true, comments);
                else
                    strbuf_addstr (comments, "<can't get value of op1 here>");
                //strbuf_addstr (comments, " ");
            }
            else
                SET_BIT (rt, NOTICE_OP1);
            break;

        case I_ADC:
        case I_SBB:
            SET_BIT (rt, NOTICE_OP1);
            SET_BIT (rt, NOTICE_OP2);
            SET_BIT (rt, NOTICE_CF);

        case I_CMC:
            SET_BIT (rt, NOTICE_CF);
            break;

            // both operands and EAX
        case I_CMPXCHG:
            SET_BIT (rt, NOTICE_OP1);
            SET_BIT (rt, NOTICE_OP2);
            SET_BIT (rt, NOTICE_AX);
            break;

            // both operands
        case I_BTR:
        case I_ADD:
        case I_SUB:
        case I_CMP:
        case I_TEST:
        case I_SHL:
        case I_SHR:
        case I_SAR:
        case I_ROR:
        case I_ROL:
        case I_RCR:
        case I_RCL:
        case I_OR:
        case I_AND:
        case I_XOR:
        case I_PXOR:
        case I_XADD:
        case I_XCHG:
        case I_BSR:
        case I_BSF:
        case I_PADDB:
        case I_PCMPEQD:
            SET_BIT (rt, NOTICE_OP1);
            SET_BIT (rt, NOTICE_OP2);
            break;

            // second only
        case I_MOV:
        case I_MOVD:
        case I_MOVZX:
        case I_MOVSX:
        case I_MOVSXD:
        case I_MOVDQA:
        case I_MOVDQU:
        case I_LEA:
            SET_BIT (rt, NOTICE_OP2);
            break;

            // second and third
        case I_PSHUFD:
            SET_BIT (rt, NOTICE_OP2);
            SET_BIT (rt, NOTICE_OP3);
            break;

            // EAX
        case I_CDQ:
        case I_RETN:
        case I_CPUID:
            SET_BIT (rt, NOTICE_AX);
            SET_BIT (rt, NOTICE_ST0);
            break;

        case I_MUL:
            SET_BIT (rt, NOTICE_OP1);
            SET_BIT (rt, NOTICE_AX);
            break;

        case I_IMUL:
            switch (da->ops_total)
            {
                case 1:
                    SET_BIT (rt, NOTICE_OP1);
                    SET_BIT (rt, NOTICE_AX);
                    break;
                case 2:
                    SET_BIT (rt, NOTICE_OP1);
                    SET_BIT (rt, NOTICE_OP2);
                    break;
                case 3:
                    SET_BIT (rt, NOTICE_OP2);
                    SET_BIT (rt, NOTICE_OP3);
                    break;
                default:
                    assert (!"unknown IMUL instruction format");
            };

            break;

            // xCX only
        case I_JECXZ:
        case I_XGETBV:
            SET_BIT (rt, NOTICE_CX);
            break;

        case I_JA:  
        case I_SETA:  
        case I_JBE:   
        case I_SETBE: 
            SET_BIT (rt, NOTICE_ZF);
            SET_BIT (rt, NOTICE_CF);
            break;

        case I_JB:	
        case I_SETB:  
        case I_JNB:   
        case I_SETNB: 
            SET_BIT (rt, NOTICE_CF);
            break;

        case I_JG:	
        case I_SETG:  
        case I_JLE:   
        case I_SETLE: 
            SET_BIT (rt, NOTICE_ZF);
            SET_BIT (rt, NOTICE_SF);
            SET_BIT (rt, NOTICE_OF);
            break;

        case I_JGE:	
        case I_SETGE: 
        case I_JL:    
        case I_SETL:  
            SET_BIT (rt, NOTICE_SF);
            SET_BIT (rt, NOTICE_OF);
            break;

        case I_JO:	
        case I_JNO:	  
        case I_SETO:  
        case I_SETNO: 
            SET_BIT (rt, NOTICE_OF);
            break;

        case I_JNP:	
        case I_JP: 	  
        case I_SETP:  
        case I_SETNP: 
            SET_BIT (rt, NOTICE_PF);
            break;

        case I_JNS:	
        case I_JS:	  
        case I_SETNS: 
        case I_SETS:  
            SET_BIT (rt, NOTICE_SF);
            break;

        case I_JZ:	
        case I_JNZ:	  
        case I_SETE:  
        case I_SETNE: 
            SET_BIT (rt, NOTICE_ZF);
            break;

        case I_CMOVNZ: 
        case I_CMOVZ:  
            SET_BIT (rt, NOTICE_OP2);
            SET_BIT (rt, NOTICE_ZF);
            break;

        case I_CMOVNS: 
        case I_CMOVS:  
            SET_BIT (rt, NOTICE_OP2);
            SET_BIT (rt, NOTICE_SF);
            break;

        case I_CMOVA:  
            SET_BIT (rt, NOTICE_OP2);
            SET_BIT (rt, NOTICE_CF);
            SET_BIT (rt, NOTICE_ZF);
            break;

        case I_CMOVB:  
        case I_CMOVNB: 
            SET_BIT (rt, NOTICE_OP2);
            SET_BIT (rt, NOTICE_CF);
            break;

        case I_CMOVGE: 
        case I_CMOVL:  
            SET_BIT (rt, NOTICE_OP2);
            SET_BIT (rt, NOTICE_SF);
            SET_BIT (rt, NOTICE_OF);
            break;

            // nothing
        case I_SYSENTER:
        case I_NOP:
        case I_CLD: 
        case I_STD:
        case I_POP:
        case I_PUSHFD: 
        case I_POPFD:
        case I_LEAVE:
        case I_MOVSB: 
        case I_MOVSW: 
        case I_MOVSD:
        case I_WAIT:
        case I_STMXCSR:
        case I_STC: 
        case I_CLC:
        case I_PUSHA: 
        case I_POPA:
            break;

        case I_FLD:
        case I_FILD:
            // do nothing - yet
            //SET_BIT (rt, NOTICE_OP1);
            break;

        case I_FSTP:
            SET_BIT (rt, NOTICE_ST0);
            break;

        case I_FMULP:
            SET_BIT (rt, NOTICE_OP1);
            SET_BIT (rt, NOTICE_OP2);
            break;

        default:
            if (ins_reported_as_unhandled[da->ins_code]==false)
            {
                strbuf sb=STRBUF_INIT;
                process_get_sym (p, PC, true, true, &sb);
                L ("(cc) WARNING: instruction %s (at least at %s) not handled\n", Da_ins_code_ToString(da), sb.buf);
                strbuf_deinit(&sb);
                ins_reported_as_unhandled[da->ins_code]=true;
            };

            strbuf_addstr (comments, "WARNING: this instruction wasn't handled");
            goto exit;
    };

    if (da->ops_total==2)
    {
        // if two first operands are the same...
        if (Da_op_equals (&da->op[0], &da->op[1]))
            REMOVE_BIT(rt, NOTICE_OP2); // do not report second

        // XOR with same operands - we do not interesting in them
        if (da->ins_code==I_XOR || da->ins_code==I_PXOR)
            REMOVE_BIT(rt, NOTICE_OP1);
    };

    for (int i=0; i<3; i++)
        if (op_present[i])
        {
            if (Da_op_is_reg (&da->op[i], R_EBP) ||       // we do not interesting in operands with EBP values
                    (Da_op_is_reg (&da->op[i], R_ESP)) || // neither in operands with ESP values
                    (da->op[i].type==DA_OP_TYPE_VALUE)) // neither in operands with values
                REMOVE_BIT(rt, 1<<i); // these are always NOTICE_OP1, _OP2, _OP3 at lower bits

            if (IS_SET(da->prefix_codes, PREFIX_FS) || 
                    IS_SET(da->prefix_codes, PREFIX_SS) || 
                    IS_SET(da->prefix_codes, PREFIX_GS))
                REMOVE_BIT(rt, 1<<i);
        };

exit:
    if (cc_c_debug)
        L ("%s() end\n", __func__);
    return rt;
};

static void cc_dump_op_name (Da *da, unsigned i, strbuf *out)
{
    //L ("%s() begin i=%d\n", __func__, i);
    switch (i)
    {
        case WORKOUT_OP1:
        case WORKOUT_OP3:
            Da_op_ToString(&da->op[i], out);
            break;

        case WORKOUT_OP2:
#if 0
            if (da->ins_code==I_LEA && da->op[i]->type==DA_OP_TYPE_VALUE_IN_MEMORY) // LEA case, op2 -> memory
                strbuf_addf (out, "op%d", 2);
            else
#endif
                Da_op_ToString(&da->op[i], out);
            break;

        case WORKOUT_AX:
            strbuf_addstr (out, get_AX_register_name());
            break;

        case WORKOUT_CX:
            strbuf_addstr (out, get_CX_register_name());
            break;

        case WORKOUT_DX:
            strbuf_addstr (out, get_DX_register_name());
            break;
        
        case WORKOUT_ST0:
            if (dump_fpu)
                strbuf_addstr (out, "ST0");
            break;

        default:
            assert(0);
    };
    //L ("%s() end\n", __func__);
};

static void cc_free_op(op_info *op, unsigned tmp_i, address tmp_a)
{
    rbtree_foreach(op->values, NULL, NULL, NULL);
    rbtree_deinit(op->values);
    rbtree_foreach(op->FPU_values, NULL, dfree, NULL);
    rbtree_deinit(op->FPU_values);
    if (op->ptr_to_string_set)
    {
        rbtree_foreach(op->ptr_to_string_set, NULL, dfree, NULL);
        rbtree_deinit(op->ptr_to_string_set);
    };

    DFREE (op);
};

static bool cc_dump_op_and_free (Da *da, PC_info* info, unsigned i, strbuf *out, address tmp_a)
{
    //L ("%s() begin\n", __func__);
    assert(da);
    op_info *op=info->op[i];
    enum obj_type op_t=info->op_t[i];

    if (op==NULL)
    {
        oassert (op_t==OBJ_NONE);
        return false;
    };

    // get op name
    cc_dump_op_name(da, i, out);
    strbuf_addc(out, '=');

    switch (op_t)
    {
        case OBJ_BYTE:
        case OBJ_WYDE:
        case OBJ_TETRABYTE:
        case OBJ_OCTABYTE: 
            {
                set_of_REG_to_string (op->values, out, 10);

                if (rbtree_count(op->ptr_to_string_set)>0)
                {
                    strbuf_addstr (out, ", ");
                    set_of_string_to_string (op->ptr_to_string_set, out, 5);
                };
            };
            break;

        case OBJ_DOUBLE:
            if (dump_fpu)
                set_of_doubles_to_string (op->FPU_values, out, 10);
            break;
        
        case OBJ_NONE:
            oassert(!"op_t==OBJ_NONE");
            break;

        default:
            assert(0);
            break;
    };

    cc_free_op (op, i, tmp_a);

    //L ("%s() end\n", __func__);
    return true;
};

void construct_common_string(strbuf *out, address a, PC_info *info)
{
    if (info->comment)
        strbuf_addf (out, "%s ", info->comment);
    // add all info
    if (info->da)
        for (unsigned j=0; j<7; j++)
        {
            if (cc_dump_op_and_free (info->da, info, j, out, a))
                strbuf_addc (out, ' ');
        };

    if (IS_SET(info->flags, FLAG_PF_CAN_BE_FALSE) && IS_SET(info->flags, FLAG_PF_CAN_BE_TRUE))
        strbuf_addstr (out, "PF=false,true ");
    else if (IS_SET(info->flags, FLAG_PF_CAN_BE_FALSE))
        strbuf_addstr (out, "PF=false ");
    else if (IS_SET(info->flags, FLAG_PF_CAN_BE_TRUE))
        strbuf_addstr (out, "PF=true ");

    if (IS_SET(info->flags, FLAG_SF_CAN_BE_FALSE) && IS_SET(info->flags, FLAG_SF_CAN_BE_TRUE))
        strbuf_addstr (out, "SF=false,true ");
    else if (IS_SET(info->flags, FLAG_SF_CAN_BE_FALSE))
        strbuf_addstr (out, "SF=false ");
    else if (IS_SET(info->flags, FLAG_SF_CAN_BE_TRUE))
        strbuf_addstr (out, "SF=true ");

    if (IS_SET(info->flags, FLAG_AF_CAN_BE_FALSE) && IS_SET(info->flags, FLAG_AF_CAN_BE_TRUE))
        strbuf_addstr (out, "AF=false,true ");
    else if (IS_SET(info->flags, FLAG_AF_CAN_BE_FALSE))
        strbuf_addstr (out, "AF=false ");
    else if (IS_SET(info->flags, FLAG_AF_CAN_BE_TRUE))
        strbuf_addstr (out, "AF=true ");

    if (IS_SET(info->flags, FLAG_ZF_CAN_BE_FALSE) && IS_SET(info->flags, FLAG_ZF_CAN_BE_TRUE))
        strbuf_addstr (out, "ZF=false,true ");
    else if (IS_SET(info->flags, FLAG_ZF_CAN_BE_FALSE))
        strbuf_addstr (out, "ZF=false ");
    else if (IS_SET(info->flags, FLAG_ZF_CAN_BE_TRUE))
        strbuf_addstr (out, "ZF=true ");

    if (IS_SET(info->flags, FLAG_OF_CAN_BE_FALSE) && IS_SET(info->flags, FLAG_OF_CAN_BE_TRUE))
        strbuf_addstr (out, "OF=false,true ");
    else if (IS_SET(info->flags, FLAG_OF_CAN_BE_FALSE))
        strbuf_addstr (out, "OF=false ");
    else if (IS_SET(info->flags, FLAG_OF_CAN_BE_TRUE))
        strbuf_addstr (out, "OF=true ");

    if (IS_SET(info->flags, FLAG_CF_CAN_BE_FALSE) && IS_SET(info->flags, FLAG_CF_CAN_BE_TRUE))
        strbuf_addstr (out, "CF=false,true ");
    else if (IS_SET(info->flags, FLAG_CF_CAN_BE_FALSE))
        strbuf_addstr (out, "CF=false ");
    else if (IS_SET(info->flags, FLAG_CF_CAN_BE_TRUE))
        strbuf_addstr (out, "CF=true ");

    if (out->strlen>0)
        strbuf_trim_last_char (out);
};

void free_PC_info (PC_info *i)
{
    DFREE (i->da);
    DFREE (i->comment);
    DFREE (i);
};

static void dump_one_PC_and_free(address a, PC_info *info, process *p, MemoryCache *mc, 
        FILE *f_txt, FILE *f_idc, FILE* f_clear_idc)
{
    strbuf sb_txt=STRBUF_INIT, sb_common=STRBUF_INIT, sb_sym=STRBUF_INIT;
    process_get_sym(p, a, false, true, &sb_sym);

    module *m=find_module_for_address (p, a);
    address adr_in_PE_file=a - m->base + m->original_base;
    
    strbuf_addf (&sb_txt, "0x" PRI_ADR_HEX " (%s), e=%8I64d [", adr_in_PE_file, sb_sym.buf, info->executed);
    strbuf_deinit(&sb_sym);

    if (info->da)
        Da_ToString(info->da, &sb_txt); // add disasmed
    else
        strbuf_addstr (&sb_txt, "not disasmed");

    strbuf_addstr (&sb_txt, "] ");

    construct_common_string(&sb_common, a, info);

    fputs (sb_txt.buf, f_txt);
    fputs (sb_common.buf, f_txt);
    fputs ("\n", f_txt);

    fprintf (f_idc, "\tSetColor (0x" PRI_ADR_HEX ", CIC_ITEM, 0xffdfdf);\n", adr_in_PE_file);
    strbuf tmp=STRBUF_INIT;
    strbuf_cvt_to_C_string(sb_common.buf, &tmp, false);
    strbuf_trim_string_with_comment (&tmp, IDA_MAX_COMMENT_SIZE, " <too long part of comment skipped>");
    fprintf (f_idc, "\tMakeComm (0x" PRI_ADR_HEX ", \"%s\");\n", adr_in_PE_file, tmp.buf);
    strbuf_deinit(&tmp);
    fprintf (f_clear_idc, "\tSetColor (0x" PRI_ADR_HEX ", CIC_ITEM, 0xffffff);\n", adr_in_PE_file);
    fprintf (f_clear_idc, "\tMakeComm (0x" PRI_ADR_HEX ", \"\");\n", adr_in_PE_file);

    strbuf_deinit(&sb_txt);
    strbuf_deinit(&sb_common);
    free_PC_info (info);
};

void cc_dump_and_free(module *m) // for module m
{
    if (cc_c_debug)
        L ("%s() begin for module %s\n", __func__, get_module_name(m));

    if (m->PC_infos==NULL)
    {
        //L ("%s() m->PC_infos==NULL, exiting\n", __func__);
        return; // no collected info for us
    };

    process *p=m->parent_process;
    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);
    strbuf sb_filename_txt=STRBUF_INIT, sb_filename_idc=STRBUF_INIT, sb_filename_clear_idc=STRBUF_INIT;
    const char *module_name=get_module_name(m);
    strbuf_addf (&sb_filename_txt, "%s.txt", module_name);
    strbuf_addf (&sb_filename_idc, "%s.idc", module_name);
    strbuf_addf (&sb_filename_clear_idc, "%s_clear.idc", module_name);
    FILE *f_txt=fopen_or_die (sb_filename_txt.buf, "w");
    FILE *f_idc=fopen_or_die (sb_filename_idc.buf, "w");
    FILE *f_clear_idc=fopen_or_die (sb_filename_clear_idc.buf, "w");

    const char *idc_header="#include <idc.idc>\nstatic main()\n{\n";
    fputs (idc_header, f_idc);
    fputs (idc_header, f_clear_idc);

    unsigned dumped=0;
    for (rbtree_node *i=rbtree_minimum(m->PC_infos); i; i=rbtree_succ(i))
    {
        //L ("%s() loop begin\n", __func__);
        address a=(address)i->key;
        PC_info *info=i->value;

        dump_one_PC_and_free(a, info, p, mc, f_txt, f_idc, f_clear_idc);
        dumped++;
    };

    rbtree_deinit(m->PC_infos);

    const char *idc_footer="}\n";
    fputs (idc_footer, f_idc);
    fputs (idc_footer, f_clear_idc);

    fclose (f_txt);
    fclose (f_idc);
    fclose (f_clear_idc);

    L ("(cc) saved to %s.(idc/txt/idc_clear): %d PCs\n", module_name, dumped);

    strbuf_deinit (&sb_filename_txt);
    strbuf_deinit (&sb_filename_idc);
    strbuf_deinit (&sb_filename_clear_idc);
    MC_Flush (mc);
    MC_MemoryCache_dtor (mc, true);
    if (cc_c_debug)
        L ("%s() end\n", __func__);
};

static op_info* allocate_op_n (PC_info *info, unsigned i)
{
    if (info->op[i])
        return; // already allocated

    info->op[i]=DCALLOC(op_info, 1, "op_info");
    info->op[i]->values=rbtree_create(true, "op_info:values", compare_size_t);
    info->op[i]->FPU_values=rbtree_create(true, "op_info:FPU_values", compare_doubles);
    info->op[i]->ptr_to_string_set=rbtree_create(true, "op_info:ptr_to_string_set", (int(*)(void*,void*))strcmp);
    return info->op[i];
};

static void save_info_about_op (address PC, unsigned i, obj *val, MemoryCache *mc, PC_info *info)
{
    op_info *op=NULL;
    // TODO: add XMM?
    switch (val->t)
    {
        case OBJ_BYTE:
            op=allocate_op_n(info, i);
            info->op_t[i]=val->t;
            rbtree_insert(op->values, (void*)obj_get_as_byte(val), NULL);
            break;
        
        case OBJ_WYDE: 
            op=allocate_op_n(info, i);
            info->op_t[i]=val->t;
            rbtree_insert(op->values, (void*)obj_get_as_wyde(val), NULL);
            break;

        case OBJ_TETRABYTE: 
            op=allocate_op_n(info, i);
            info->op_t[i]=val->t;
            rbtree_insert(op->values, (void*)obj_get_as_tetrabyte(val), NULL);
            break;

#ifdef _WIN64
        case OBJ_OCTABYTE:
            op=allocate_op_n(info, i);
            info->op_t[i]=val->t;
            rbtree_insert(op->values, (void*)obj_get_as_octabyte(val), NULL);
            break;
#endif
        
        case OBJ_DOUBLE:
            if (dump_fpu)
            {
                op=allocate_op_n(info, i);
                info->op_t[i]=val->t;
                double d=obj_get_as_double(val);
                if (rbtree_is_key_present(op->FPU_values, (void*)&d)==false)
                    rbtree_insert(op->FPU_values, DMEMDUP(&d, sizeof(double), "double"), NULL);
            };
            break;
        default:
            // unknown val->t type
            // wont' save (yet!)
            obj_free_structures(val);
            return;
    };

    if (op==NULL) // still NULL, probably, unsupported type like XMM or OBJ_DOUBLE when dump_fpu=false
        return;

    if (val->t==
#ifdef _WIN64
            OBJ_OCTABYTE
#else
            OBJ_TETRABYTE
#endif
       )
    {
        strbuf sb=STRBUF_INIT;
        if (MC_get_any_string (mc, obj_get_as_REG(val), &sb)) // unicode string too
        {
            if (sb.strlen>1+1+STRING_LEN)
                set_add_string_or_free (op->ptr_to_string_set, strbuf_detach(&sb, NULL));
        };
        strbuf_deinit(&sb);
    };
};

static void save_info_about_PC (module *m, strbuf *comment, unsigned to_notice, Da* da, CONTEXT *ctx, 
        MemoryCache *mc)
{
    if (cc_c_debug)
        L ("%s(comment=\"%s\") begin\n", __func__, comment->buf);
    address PC=CONTEXT_get_PC(ctx);

    // find entry in PC_infos
    if (m->PC_infos==NULL)
        m->PC_infos=rbtree_create (true, "PC_infos", compare_size_t);

    PC_info *info=rbtree_lookup(m->PC_infos, (void*)PC);
    if (info==NULL)
    {
        info=DCALLOC(PC_info, 1, "PC_info");
        //L ("%s() allocated info=0x%p\n", __func__, info);
        rbtree_insert(m->PC_infos, (void*)PC, info);
    };

    if (comment->strlen>0 && info->comment==NULL)
        info->comment=DSTRDUP(comment->buf, "char*");
    info->executed++;

    if (info->da==NULL && da)
        info->da=DMEMDUP (da, da->struct_size, "Da");

    // TODO: add flags

    for (unsigned i=0; i<13; i++)
        if (IS_SET(to_notice, 1<<i)) // NOTICE_OP1, OP2, OP3, AX, CX, DX, ST0
        {
            if (i<=WORKOUT_OP3)
            {
                address adr;
                obj val;
                if (Da_op_get_value_of_op (&da->op[i], &adr, ctx, mc, __FILE__, __LINE__, &val))
                {
                    save_info_about_op (PC, i, &val, mc, info);
                    //assert (info->op_t[i]!=V_INVALID);
                };
            }
            else
            {
                obj val;
                switch (i)
                {
                    case WORKOUT_AX:
                        X86_register_get_value (R_EAX, ctx, &val);
                        save_info_about_op (PC, i, &val, mc, info);
                        break;

                    case WORKOUT_CX:
                        X86_register_get_value (R_ECX, ctx, &val);
                        save_info_about_op (PC, i, &val, mc, info);

                        break;
                    case WORKOUT_DX:
                        X86_register_get_value (R_EDX, ctx, &val);
                        save_info_about_op (PC, i, &val, mc, info);

                        break;
                    case WORKOUT_ST0:
                        X86_register_get_value (R_ST0, ctx, &val);
                        save_info_about_op (PC, i, &val, mc, info);
                        break;

                    case WORKOUT_PF:
                        if (IS_SET(ctx->EFlags, FLAG_PF)) 
                            SET_BIT(info->flags, FLAG_PF_CAN_BE_TRUE);
                        else
                            SET_BIT(info->flags, FLAG_PF_CAN_BE_FALSE);
                        break;

                    case WORKOUT_SF:
                        if (IS_SET(ctx->EFlags, FLAG_SF)) 
                            SET_BIT(info->flags, FLAG_SF_CAN_BE_TRUE);
                        else
                            SET_BIT(info->flags, FLAG_SF_CAN_BE_FALSE);
                        break;

                    case WORKOUT_AF:
                        if (IS_SET(ctx->EFlags, FLAG_AF)) 
                            SET_BIT(info->flags, FLAG_AF_CAN_BE_TRUE);
                        else
                            SET_BIT(info->flags, FLAG_AF_CAN_BE_FALSE);
                        break;

                    case WORKOUT_ZF:
                        if (IS_SET(ctx->EFlags, FLAG_ZF)) 
                            SET_BIT(info->flags, FLAG_ZF_CAN_BE_TRUE);
                        else
                            SET_BIT(info->flags, FLAG_ZF_CAN_BE_FALSE);
                        break;

                    case WORKOUT_OF:
                        if (IS_SET(ctx->EFlags, FLAG_OF)) 
                            SET_BIT(info->flags, FLAG_OF_CAN_BE_TRUE);
                        else
                            SET_BIT(info->flags, FLAG_OF_CAN_BE_FALSE);
                        break;

                    case WORKOUT_CF:
                        if (IS_SET(ctx->EFlags, FLAG_CF)) 
                            SET_BIT(info->flags, FLAG_CF_CAN_BE_TRUE);
                        else
                            SET_BIT(info->flags, FLAG_CF_CAN_BE_FALSE);
                        break;

                    default:
                        assert(0);
                };
            };
        };
    if (cc_c_debug)
        L ("%s() end\n", __func__);
};

void handle_cc(Da* da, process *p, thread *t, CONTEXT *ctx, MemoryCache *mc, 
        bool CALL_to_be_skipped_due_to_module, bool CALL_to_be_skipped_due_to_trace_limit)
{
    //printf ("sizeof(ins_reported_as_unhandled)/sizeof(bool)=%d\n", sizeof(ins_reported_as_unhandled)/sizeof(bool));
    //printf ("I_MAX_INS=%d\n", I_MAX_INS);
    if (cc_c_debug)
        L ("%s() begin\n", __func__);

    assert (sizeof(ins_reported_as_unhandled)/sizeof(bool) > I_MAX_INS);

    strbuf comment=STRBUF_INIT;
    address PC=CONTEXT_get_PC(ctx);
    unsigned to_notice=0;

    if (da->ins_code!=I_INVALID)
        to_notice=what_to_notice(p, da, &comment, ctx, mc);
    else
        strbuf_addstr (&comment, "instruction wasn't disassembled");

    module *m=find_module_for_address (p, PC);
    oassert (m);

    if (CALL_to_be_skipped_due_to_trace_limit)
    {
        assert (da && da->ins_code==I_CALL);
        strbuf_addf (&comment, "tracing nested maximum level (%d) reached, skipping this CALL",
                limit_trace_nestedness);
    };
    if (CALL_to_be_skipped_due_to_module) // not used (so far).
    {
        assert (da && da->ins_code==I_CALL);
        strbuf_addf (&comment, "skipping this CALL: all functions in this module is to be skipped",
                limit_trace_nestedness);
    };

    save_info_about_PC(m, &comment, to_notice, da, ctx, mc);

    strbuf_deinit(&comment);
    if (cc_c_debug)
        L ("%s() end\n", __func__);
};

/* vim: set expandtab ts=4 sw=4 : */
