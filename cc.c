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

#define IDA_MAX_COMMENT_SIZE 1023

// big enough to hold all instructions till I_MAX_INS
static bool ins_reported_as_unhandled[512]={ false };

unsigned what_to_notice (process *p, Da *da, strbuf *comments, CONTEXT *ctx, MemoryCache *mc)
{
    L ("%s() begin\n", __func__);
    unsigned rt=0;

    address PC=CONTEXT_get_PC(ctx);

    if (Da_ins_is_FPU(da))
        return 0; // do nothing (yet)

    bool op_present[3];

    for (int i=0; i<3; i++)
        op_present[i]=da->_op[i]==NULL ? false : true;

    unsigned ops=0;

    if (op_present[0] && !op_present[1] && !op_present[2]) // 1 op
        ops=1;
    else
        if (op_present[0] && op_present[1] && !op_present[2]) // 2 op
            ops=2;
        else
            if (op_present[0] && op_present[1] && op_present[2]) // 3 op
                ops=3;

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
            if (da->_op[0]->type==DA_OP_TYPE_REGISTER || da->_op[0]->type==DA_OP_TYPE_VALUE_IN_MEMORY)
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
            if (da->_op[0]->type==DA_OP_TYPE_REGISTER || da->_op[0]->type==DA_OP_TYPE_VALUE_IN_MEMORY) // add symbol to comment
            {
                address adr;
                s_Value val;

                strbuf_addstr (comments, "op1=");
                if (Da_op_get_value_of_op (da->_op[0], &adr, ctx, mc, __FILE__, __LINE__, &val))
                    process_get_sym (p, get_as_REG (&val), true, comments);
                else
                    strbuf_addstr (comments, "<can't get value of op1 here>");
                strbuf_addstr (comments, " ");
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
            break;

        case I_MUL:
            SET_BIT (rt, NOTICE_OP1);
            SET_BIT (rt, NOTICE_AX);
            break;

        case I_IMUL:
            switch (ops)
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

        default:
            {
                if (ins_reported_as_unhandled[da->ins_code]==false)
                {
                    strbuf sb=STRBUF_INIT;
                    process_get_sym (p, PC, true, &sb);
                    L ("(cc) WARNING: instruction %s (at least at %s) not handled\n", Da_ins_code_ToString(da), sb.buf);
                    strbuf_deinit(&sb);
                    ins_reported_as_unhandled[da->ins_code]=true;
                };

                strbuf_addstr (comments, "WARNING: this instruction wasn't handled");
            };
            break;
    };

    if (ops==2)
    {
        // if two first operands are the same...
        if (Da_op_equals (da->_op[0], da->_op[1]))
            REMOVE_BIT(rt, NOTICE_OP2); // do not report second

        // XOR with same operands - we do not interesting in them
        if (da->ins_code==I_XOR || da->ins_code==I_PXOR)
            REMOVE_BIT(rt, NOTICE_OP1);
    };

    for (int i=0; i<3; i++)
        if (op_present[i])
        {
            if (Da_op_is_reg (da->_op[i], R_EBP) ||       // we do not interesting in operands with EBP values
                    (Da_op_is_reg (da->_op[i], R_ESP)) || // neither in operands with ESP values
                    (da->_op[i]->type==DA_OP_TYPE_VALUE)) // neither in operands with values
                REMOVE_BIT(rt, 1<<i); // these are always NOTICE_OP1, _OP2, _OP3 at lower bits
        };

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
            Da_op_ToString(da->_op[i], out);
            break;

        case WORKOUT_OP2:

            if (da->ins_code==I_LEA && da->_op[i]->type==DA_OP_TYPE_VALUE_IN_MEMORY) // LEA case, op2 -> memory
                strbuf_addf (out, "op%d", 2);
            else
                Da_op_ToString(da->_op[i], out);
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

        default:
            assert(0);
    };
    //L ("%s() end\n", __func__);
};

static void cc_free_op(op_info *op)
{
    rbtree_foreach(op->values, NULL, NULL, NULL);
    rbtree_deinit(op->values);
    if (op->ptr_to_string_set)
    {
        rbtree_foreach(op->ptr_to_string_set, NULL, dfree, NULL);
        rbtree_deinit(op->ptr_to_string_set);
    };

    DFREE (op);
};

static bool cc_dump_op_and_free (Da *da, PC_info* info, unsigned i, strbuf *out)
{
    //L ("%s() begin\n", __func__);
    op_info *op=info->op[i];
    enum value_t op_t=info->op_t[i];

    if (op==NULL)
    {
        assert (op_t==V_INVALID);
        return false;
    };

    // get op name
    cc_dump_op_name(da, i, out);
    strbuf_addc(out, '=');

    switch (op_t)
    {
        case V_BYTE:
        case V_WORD:
        case V_DWORD:
        case V_QWORD: 
            {
                rbtree_node *max=rbtree_maximum(op->values);
                for (rbtree_node *j=rbtree_minimum(op->values); j; j=rbtree_succ(j))
                {
                    strbuf_addf (out, "0x" PRI_REG_HEX, (REG)j->key);
                    if (j!=max)
                        strbuf_addstr (out, ", ");
                };
                if (rbtree_count(op->ptr_to_string_set)>0)
                {
                    strbuf_addstr (out, ", ");
                    rbtree_node *max=rbtree_maximum(op->ptr_to_string_set);
                    for (rbtree_node *j=rbtree_minimum(op->ptr_to_string_set); j; j=rbtree_succ(j))
                    {
                        strbuf_addstr (out, (char*)j->key);
                        if (j!=max)
                            strbuf_addstr (out, ", ");
                    };
                };
            };
            break;

            //case V_FPU: ?
        default:
            assert(0);
    };

    cc_free_op (op);

    //L ("%s() end\n", __func__);
    return true;
};

static void dump_one_PC_and_free(address a, PC_info *info, process *p, MemoryCache *mc, 
        FILE *f_txt, FILE *f_idc, FILE* f_clear_idc)
{
    Da* da=MC_disas(a, mc);

    // there shouldn't be entries for instructions we can't disassemble.
    // (these entries shouldn't be added at all)
    assert (da); 

    strbuf sb_txt=STRBUF_INIT, sb_common=STRBUF_INIT, sb_sym=STRBUF_INIT;
    process_get_sym(p, a, false, &sb_sym);
    strbuf_addf (&sb_txt, "0x" PRI_ADR_HEX " (%s), e=%8I64d [", a, sb_sym.buf, info->executed);
    strbuf_deinit(&sb_sym);
    Da_ToString(da, &sb_txt); // add disasmed
    strbuf_addstr (&sb_txt, "] ");
    if (info->comment)
        strbuf_addf (&sb_common, "%s ", info->comment);
    // add all info
    for (unsigned j=0; j<6; j++)
    {
        if (cc_dump_op_and_free (da, info, j, &sb_common))
            strbuf_addc (&sb_common, ' ');
    };
    // TODO: flags?
    fputs (sb_txt.buf, f_txt);
    fputs (sb_common.buf, f_txt);
    fputs ("\n", f_txt);

    fprintf (f_idc, "\tSetColor (0x" PRI_ADR_HEX ", CIC_ITEM, 0xffdfdf);\n", a);
    strbuf tmp=STRBUF_INIT;
    strbuf_cvt_to_C_string(&sb_common, &tmp, false);
    fprintf (f_idc, "\tMakeComm (0x" PRI_ADR_HEX ", \"%s\");\n", a, tmp.buf);
    strbuf_deinit(&tmp);
    fprintf (f_clear_idc, "\tSetColor (0x" PRI_ADR_HEX ", CIC_ITEM, 0xffffff);\n", a);
    fprintf (f_clear_idc, "\tMakeComm (0x" PRI_ADR_HEX ", \"\");\n", a);

    Da_free(da);
    strbuf_deinit(&sb_txt);
    strbuf_deinit(&sb_common);
    DFREE (info->comment);
    DFREE (info);
};

void cc_dump_and_free(module *m) // for module m
{
    //L ("%s() begin for module %s\n", __func__, get_module_name(m));

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

    for (rbtree_node *i=rbtree_minimum(m->PC_infos); i; i=rbtree_succ(i))
    {
        //L ("%s() loop begin\n", __func__);
        address a=(address)i->key;
        PC_info *info=i->value;

        dump_one_PC_and_free(a, info, p, mc, f_txt, f_idc, f_clear_idc);
    };

    rbtree_deinit(m->PC_infos);
    
    const char *idc_footer="}\n";
    fputs (idc_footer, f_idc);
    fputs (idc_footer, f_clear_idc);

    fclose (f_txt);
    fclose (f_idc);
    fclose (f_clear_idc);
    strbuf_deinit (&sb_filename_txt);
    strbuf_deinit (&sb_filename_idc);
    strbuf_deinit (&sb_filename_clear_idc);
    MC_Flush (mc);
    MC_MemoryCache_dtor (mc, true);
    //L ("%s() end\n", __func__);
};

static void save_info_about_op (address PC, unsigned i, s_Value *val, MemoryCache *mc, PC_info *info)
{
    REG v;
    // FIXME: XMM? FPU?
    switch (val->t)
    {
        case V_BYTE: 
            v=(REG)get_as_8(val);
            break;
        case V_WORD: 
            v=(REG)get_as_16(val);
            break;
        case V_DWORD: 
            v=(REG)get_as_32(val);
            break;
#ifdef _WIN64
        case V_QWORD: 
            v=(REG)get_as_64(val);
            break;
#endif
        default:
            assert(!"unknown val->t type");
            break;
    };

    if (info->op[i]==NULL)
    {
        info->op[i]=DCALLOC(op_info, 1, "op_info");
        info->op[i]->values=rbtree_create(true, "op_info:values", compare_size_t);
        info->op[i]->ptr_to_string_set=rbtree_create(true, "op_info:ptr_to_string_set", (int(*)(void*,void*))strcmp);
    };

    op_info *op=info->op[i];

    // save v
    rbtree_insert(op->values, (void*)v, NULL);

    info->op_t[i]=val->t;

#ifdef _WIN64
    if (val->t==V_QWORD)
#else
        if (val->t==V_DWORD)
#endif
        {
            strbuf sb=STRBUF_INIT;
            if (MC_get_any_string (mc, get_as_REG(val), &sb)) // unicode string too
                rbtree_insert(op->ptr_to_string_set, (void*)strbuf_detach(&sb, NULL), NULL);
        };
};

static void save_info_about_PC (module *m, strbuf *comment, unsigned to_notice, Da* da, CONTEXT *ctx, MemoryCache *mc)
{
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

    if (comment->strlen>0)
        info->comment=DSTRDUP(comment->buf, "char*");
    info->executed++;

    // TODO: add flags

    for (unsigned i=0; i<6; i++)
        if (IS_SET(to_notice, 1<<i)) // NOTICE_OP1, OP2, OP3, AX, CX, DX
        {
            if (i<=WORKOUT_OP3)
            {
                address adr;
                s_Value val;
                if (Da_op_get_value_of_op (da->_op[i], &adr, ctx, mc, __FILE__, __LINE__, &val))
                    save_info_about_op (PC, i, &val, mc, info);
            }
            else
            {
                s_Value val;
                switch (i)
                {
                    case WORKOUT_AX:
                        X86_register_get_value (R_EAX, ctx, &val);
                        break;
                    case WORKOUT_CX:
                        X86_register_get_value (R_ECX, ctx, &val);
                        break;
                    case WORKOUT_DX:
                        X86_register_get_value (R_EDX, ctx, &val);
                        break;
                    default:
                        assert(0);
                };
                save_info_about_op (PC, i, &val, mc, info);
            };
        };
    L ("%s() end\n", __func__);
};

void handle_cc(Da* da, process *p, thread *t, CONTEXT *ctx, MemoryCache *mc)
{
    //printf ("sizeof(ins_reported_as_unhandled)/sizeof(bool)=%d\n", sizeof(ins_reported_as_unhandled)/sizeof(bool));
    //printf ("I_MAX_INS=%d\n", I_MAX_INS);
    L ("%s() begin\n", __func__);
    assert (sizeof(ins_reported_as_unhandled)/sizeof(bool) > I_MAX_INS);

    strbuf comment=STRBUF_INIT;
    address PC=CONTEXT_get_PC(ctx);

    unsigned to_notice=what_to_notice(p, da, &comment, ctx, mc);

    module *m=find_module_for_address (p, PC);
    save_info_about_PC(m, &comment, to_notice, da, ctx, mc);

    strbuf_deinit(&comment);
    L ("%s() end\n", __func__);
};

