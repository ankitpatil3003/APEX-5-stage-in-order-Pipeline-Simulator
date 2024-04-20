/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */

/* Flag to enable to display register and memory values */
int ENABLE_DISPLAY;

/* Flag to enable counting code_memory_size */
int ENABLE_COUNTING;

struct flagCheck
{
    int flagIsUsed;
    int flagIsNotUsed;
    int isRegValEmpty;
}flagCheck;

// int emptyMemory = 0;
// int emptyExecute = 0;
// int flagIsUsed = 1;
// int flagIsNotUsed = 0;
// int isRegValEmpty = 0;

static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d", stage->opcode_str, stage->rd, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        case OPCODE_LOADP:
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_JALR:
        {
            printf("%s,R%d,R%d,#%d", stage->opcode_str, stage->rd, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STOREP:
        {
            printf("%s,R%d,R%d,#%d", stage->opcode_str, stage->rs1, stage->rs2, stage->imm);
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BP:
        case OPCODE_BNP:
        case OPCODE_BN:
        case OPCODE_BNN:
        {
            printf("%s,#%d", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }

        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_JUMP:
        case OPCODE_CML:
        {
            printf("%s,R%d,#%d", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void 
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    CPU_Stage* stage = &cpu->stage[Fetch];
    if (!stage->has_no_insn && !stage->is_interrupted) {
        APEX_Instruction *current_ins;
    /*if (cpu->fetch.has_insn == TRUE)
    {
        /* This fetches new branch target instruction from next cycle
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle
            return;
        }*/

        /* Store current PC in fetch latch */
        stage->pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
        * into fetch latch  */
        
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(stage->opcode, current_ins->opcode_str);
        stage->rd = current_ins->rd;
        stage->rs1 = current_ins->rs1;
        stage->rs2 = current_ins->rs2;
        stage->imm = current_ins->imm;

        /*if (cpu->decode.is_interrupted == flagIsNotUsed)
        {
            /* Update PC for next instruction */
        cpu->pc += 4;
        /* Copy data from fetch latch to decode latch*/
        if (!cpu->stage[DRF].is_interrupted) {
            cpu->stage[DRF] = cpu->stage[Fetch];
        }
        else {
            stage->is_interrupted = 1;
        }

        if (ENABLE_DEBUG_MESSAGES) {
            print_stage_content("Fetch", stage);
        }
    }
    else {
        /* If HALT reached into WB stage, then finish program in this stage*/
        if (strcmp(cpu->stage[WB].opcode, "HALT") == 0) {
            cpu->code_memory_size = cpu->clock + 1;
        }

        /* If current stage contains branch instructions, then stop stalling the stage and show the content of fetch stage
        */
        if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0 || strcmp(stage->opcode, "BP") == 0 || 
            strcmp(stage->opcode, "BNP") == 0 || strcmp(stage->opcode, "BN") == 0 || strcmp(stage->opcode, "BNN") == 0) {
                stage->is_interrupted = 0;
            if (ENABLE_DEBUG_MESSAGES) {
                print_stage_content("Fetch", stage);
            }
        }

        /* If DRF stage is not interrupted, and does not contain branch intructions, then
        * stop stalling fetch stage, and copy data into the next stage.
        */
        if (!cpu->stage[DRF].is_interrupted && (strcmp(cpu->stage[DRF].opcode, "BZ") != 0 || strcmp(cpu->stage[DRF].opcode, "BNZ") != 0 || 
                                                strcmp(cpu->stage[DRF].opcode, "BP") != 0 || strcmp(cpu->stage[DRF].opcode, "BNP") != 0 || 
                                                strcmp(cpu->stage[DRF].opcode, "BN") != 0 || strcmp(cpu->stage[DRF].opcode, "BNN") != 0)) {
            stage->is_interrupted = 0;
            cpu->stage[DRF] = cpu->stage[Fetch];
            if (ENABLE_DEBUG_MESSAGES) {
                print_stage_content("Fetch", stage);
            }
        }

        /* Show if next stage is not HALT */
        if (cpu->stage[DRF].is_interrupted && strcmp(cpu->stage[DRF].opcode, "HALT") != 0) {
            if (ENABLE_DEBUG_MESSAGES) {
                print_stage_content("Fetch", stage);
            }
        }

        if (ENABLE_DEBUG_MESSAGES) {
            print_stage_content("Fetch: EMPTY\n", stage);
        }
    }
    return 0;
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    CPU_Stage* stage = &cpu->stage[DRF];
    if (!stage->has_no_insn && !stage->is_interrupted)
    {
        /* Read operands from register file based on the instruction type */
        switch (stage->opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_DIV:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {
                if (cpu->regChecking[stage->rs1] && cpu->regChecking[stage->rs2])
                {
                    cpu->regChecking[stage->rd] = 0;
                    stage->rs1_value = cpu->regs[stage->rs1];
                    stage->rs2_value = cpu->regs[stage->rs2];
                }
                else
                {
                    stage->is_interrupted = 1;   
                }
                break;
            }
            
            case OPCODE_MOVC:
            {
                cpu->regChecking[stage->rd] = 0;
                /* MOVC doesn't have register operands */
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_LOADP:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_JALR:
            {
                if (cpu->regChecking[stage->rs1])
                {
                    cpu->regChecking[stage->rd] = 1;
                    stage->memory_address = stage->rs1_value + stage->imm;
                    cpu->regs[stage->rd] = stage->pc + 4;
                }
                else
                {
                    stage->is_interrupted = 1;
                }
                break;
            }


            case OPCODE_STORE:
            case OPCODE_STOREP:
            {
                if (cpu->regChecking[stage->rs1] && cpu->regChecking[stage->rs2]) {
                    cpu->regChecking[stage->rd] = 0;
                    stage->rs1_value = cpu->regs[stage->rs1];
                    stage->rs2_value = cpu->regs[stage->rs2];
                }
                else {
                    stage->is_interrupted = 1;
                }
                break;
            }

            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_BN:
            case OPCODE_BNN:
            {
                if (!cpu->zero_flag_valid) {
                    stage->is_interrupted = 1;
                }
                break;
            }

            case OPCODE_HALT:
            {
                cpu->stage[Fetch].is_interrupted = 1;
                stage->is_interrupted = 1;
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_CMP:
            {
                if (cpu->regChecking[stage->rs1] && cpu->regChecking[stage->rs2])
                {
                    cpu->regChecking[stage->rd] = 0;
                    stage->rs1_value = cpu->regs[stage->rs1];
                    stage->rs2_value = cpu->regs[stage->rs2];
                }
                else
                {
                    stage->is_interrupted = 1;
                }
                break;
            }

            case OPCODE_CML:
            {
                if (cpu->regChecking[stage->rs1])
                {
                    cpu->regChecking[stage->rd] = 0;
                    stage->rs1_value = cpu->regs[stage->rs1];

                    /*int result = stage->rs1_value - stage->imm;

                    cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                    cpu->cc_flags.N = (result < 0) ? 1 : 0;
                    cpu->cc_flags.P = (result > 0) ? 1 : 0;*/
                }
                else
                {
                    stage->is_interrupted = 1;
                }
                break;
            }

            case OPCODE_JUMP:
            {
                if (cpu->regChecking[stage->rs1])
                {
                    cpu->regChecking[stage->rd] = 0;
                    stage->rs1_value = cpu->regs[stage->rs1];
                    stage->memory_address = stage->rs1_value + stage->imm;
                }
                else
                {
                    stage->is_interrupted = 1;
                }
                break;
            }

            default:
            {
                break;
            }
        }
        
        /* Copy data from decode stage to execute stage
        if decode stage is not stalled
        else transfer bubble*/
        if (!stage->is_interrupted) {
            cpu->stage[EX] = cpu->stage[DRF];
        }
        else {
            cpu->stage[EX].pc = 0000;
        }

        if (ENABLE_DEBUG_MESSAGES) {
            print_stage_content("Decode/RF", stage);
        }

        if (strcmp(stage->opcode, "HALT") == 0) {
            cpu->stage[Fetch].is_interrupted = 1;
            stage->is_interrupted = 1;
        }

    }
    else
    {
        /* If current stage does not contain HALT, and next stage is not interrupted, then
        * current stage is interrupted becuase of dependency between source and destination registers
        */
        if (stage->is_interrupted && strcmp(stage->opcode, "HALT") != 0 && !cpu->stage[EX].is_interrupted)
        {
            if (ENABLE_COUNTING)
            {
                cpu->code_memory_size++;
            }

            if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "AND") == 0 ||
                strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0 || strcmp(stage->opcode, "MUL") == 0 ||
                strcmp(stage->opcode, "DIV") == 0 || strcmp(stage->opcode, "STORE") == 0 ||  strcmp(stage->opcode, "STOREP") == 0)
            {

                if (cpu->regChecking[stage->rs1] && cpu->regChecking[stage->rs2])
                {

                    /* If it is STORE or STOREP, do not make rd invalid */
                    if (strcmp(stage->opcode, "STORE") != 0 || strcmp(stage->opcode, "STOREP") != 0) {
                        cpu->regChecking[stage->rd] = 0;
                    }
                    stage->rs1_value = cpu->regs[stage->rs1];
                    stage->rs2_value = cpu->regs[stage->rs2];
                    stage->is_interrupted = 0;
                    cpu->stage[EX] = cpu->stage[DRF];
                }
            }

            if (strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "LOADP") == 0 || strcmp(stage->opcode, "ADDL") == 0 ||
                strcmp(stage->opcode, "SUBL") == 0 || strcmp(stage->opcode, "JALR") == 0)
            {
                if (cpu->regChecking[stage->rs1])
                {
                    cpu->regChecking[stage->rd] = 0;
                    stage->rs1_value = cpu->regs[stage->rs1];
                    stage->is_interrupted = 0;
                    cpu->stage[EX] = cpu->stage[DRF];
                }
            }

            if (strcmp(stage->opcode, "JUMP") == 0)
            {
                if (cpu->regChecking[stage->rs1])
                {
                    stage->rs1_value = cpu->regs[stage->rs1];
                    stage->is_interrupted = 0;
                    cpu->stage[EX] = cpu->stage[DRF];
                }
            }

            if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0 ||  strcmp(stage->opcode, "BP") == 0 ||
                strcmp(stage->opcode, "BNP") == 0 || strcmp(stage->opcode, "BN") == 0 || strcmp(stage->opcode, "BNN") == 0)
            {
                if (cpu->zero_flag_valid)
                {
                    stage->is_interrupted = 0;
                    cpu->stage[EX] = cpu->stage[DRF];
                }
            }

            if (strcmp(stage->opcode, "NOP") == 0)
            {
                cpu->regChecking[stage->rd] = 0;
                cpu->stage[EX] = cpu->stage[DRF];
            }

            if (ENABLE_DEBUG_MESSAGES) {
                print_stage_content("Decode/RF", stage);
            }

            if (strcmp(stage->opcode, "MOVC") == 0)
            {
                if (cpu->regChecking[stage->rd])
                {
                    cpu->regChecking[stage->rd] = 0;
                    cpu->regs[stage->rd] = stage->imm;
                    stage->is_interrupted = 0;
                    cpu->stage[EX] = cpu->stage[DRF];
                }
            }
        }

        if (cpu->stage[EX].is_interrupted && strcmp(cpu->stage[EX].opcode, "HALT") != 0)
        {
            if (ENABLE_DEBUG_MESSAGES) {
                print_stage_content("Decode/RF: EMPTY\n", stage);
            }
        }
    }
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    CPU_Stage* stage = &cpu->stage[EX];
    if (!stage->has_no_insn && !stage->is_interrupted)
    {
        switch (stage->opcode)
        {
            case OPCODE_ADD:
            {
                int result = stage->result_buffer = stage->rs1_value + stage->rs2_value;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            case OPCODE_SUB:
            {
                int result = stage->result_buffer = stage->rs1_value - stage->rs2_value;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;

                break;
            }

            case OPCODE_MUL:
            {
                int result = stage->result_buffer = stage->rs1_value * stage->rs2_value;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            case OPCODE_DIV:
            {
                int result = stage->result_buffer = stage->rs1_value / stage->rs2_value;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            case OPCODE_AND:
            {
                int result = stage->result_buffer = stage->rs1_value & stage->rs2_value;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            case OPCODE_OR:
            {
                int result = stage->result_buffer = stage->rs1_value | stage->rs2_value;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            case OPCODE_XOR:
            {
                int result = stage->result_buffer = stage->rs1_value ^ stage->rs2_value;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            case OPCODE_MOVC: 
            {
                int result = stage->result_buffer = stage->imm;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            case OPCODE_LOAD:
            {
                stage->memory_address = stage->rs1_value + stage->imm;
                break;
            }

            case OPCODE_LOADP:
            {
                stage->memory_address = stage->rs1_value + stage->imm;
                stage->rs1_value = stage->rs1_value + 4;
                break;
            }

            case OPCODE_ADDL:
            {
                int result = stage->result_buffer = stage->rs1_value + stage->imm;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            case OPCODE_SUBL:
            {
                int result = stage->result_buffer = stage->rs1_value - stage->imm;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;
                
                break;
            }

            // case OPCODE_JALR:
            // {
            //     int target_address = stage->rs1_value + stage->imm;
            //     stage->result_buffer = stage->pc + 4;
            //     cpu->fetch.pc = target_address;
            //     break;
            // }

            case OPCODE_JALR:
            {
                int target_address = cpu->stage[EX].rs1_value + cpu->stage[EX].imm;

                target_address = target_address & 0xFFFFFFFC;

                cpu->regs[cpu->stage[EX].rd] = cpu->stage[EX].pc + 4;

                cpu->stage[Fetch].pc = target_address;
                cpu->stage[EX].is_interrupted = 1;
                cpu->stage[MEM].is_interrupted = 1;
                cpu->stage[WB].is_interrupted = 1;

                break;
            }

            case OPCODE_STORE:
            {
                stage->memory_address = stage->rs2_value + stage->imm;
                break;
            }
            
            case OPCODE_STOREP:
            {
                stage->result_buffer = stage->rs1_value +4;
                stage->memory_address = stage->rs2_value + stage->imm;
                break;
            }

            case OPCODE_BZ:
            {
                if (cpu->cc_flags.Z) {
                    stage->result_buffer = stage->pc + stage->imm;
                    control_flow(cpu);
                }
                break;
            }

            case OPCODE_BNZ:
            {
                if (!cpu->cc_flags.Z) {
                    stage->result_buffer = stage->pc + stage->imm;
                    control_flow(cpu);
                }
                break;
            }

            case OPCODE_BP:
            {
                if (cpu->cc_flags.P) {
                    stage->result_buffer = stage->pc + stage->imm;
                    control_flow(cpu);
                }
                break;
            }

            case OPCODE_BNP:
            {
                if (!cpu->cc_flags.P) {
                    stage->result_buffer = stage->pc + stage->imm;
                    control_flow(cpu);
                }
                break;
            }

            case OPCODE_BN:
            {
                if (cpu->cc_flags.N) {
                    stage->result_buffer = stage->pc + stage->imm;
                    control_flow(cpu);
                }
                break;
            }

            case OPCODE_BNN:
            {
                if (!cpu->cc_flags.N) {
                    stage->result_buffer = stage->pc + stage->imm;
                    control_flow(cpu);
                }
                break;
            }

            case OPCODE_HALT:
            {
                stage->is_interrupted = 1;
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_CMP:
            {
                int result = stage->rs1_value - stage->rs2_value;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;

                break;
            }

            case OPCODE_JUMP:
            {
                int target_address = (stage->rs1_value + stage->imm) & (~0x3);
                stage->pc = target_address;
                break;
            }

            case OPCODE_CML:
            {
                int result = stage->rs1_value - stage->imm;

                cpu->cc_flags.Z = (result == 0) ? 1 : 0;
                cpu->cc_flags.N = (result < 0) ? 1 : 0;
                cpu->cc_flags.P = (result > 0) ? 1 : 0;

                break;
            }
            default:
            {
                break;
            }
        }

        /* Copy data from execute latch to memory latch*/
        if (!stage->is_interrupted) {
            cpu->stage[MEM] = cpu->stage[EX];
        }
        else {
            cpu->stage[DRF].is_interrupted = 1;
            cpu->stage[MEM].pc = 0000;
        }

        if (ENABLE_DEBUG_MESSAGES) {
            print_stage_content("Execute", stage);
        }

        if (strcmp(stage->opcode, "HALT") == 0) {
            stage->is_interrupted = 1;
        }
    }
    else
    {
        /* If current stage is interrupted then it is because of MUL and DIV
        * stop stalling and copy data into the next stage
        */
        if (stage->is_interrupted && strcmp(stage->opcode, "MUL") == 0 && strcmp(stage->opcode, "DIV") == 0) {
            if (ENABLE_COUNTING) {
                cpu->code_memory_size++;
            }
            stage->is_interrupted = 0;
            cpu->stage[DRF].is_interrupted = 0;
            cpu->stage[MEM] = cpu->stage[EX];

            if (ENABLE_DEBUG_MESSAGES) {
                print_stage_content("Execute", stage);
            }
        }
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    CPU_Stage* stage = &cpu->stage[MEM];
    if (!stage->has_no_insn && !stage->is_interrupted)
    {
        switch (stage->opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MOVC:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_JALR:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_BN:
            case OPCODE_BNN:
            case OPCODE_HALT:
            case OPCODE_NOP:
            case OPCODE_CMP:
            case OPCODE_JUMP:
            case OPCODE_CML:
            {
                /* No work */
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_LOADP:
            {
                stage->result_buffer = cpu->data_memory[stage->memory_address];
                break;
            }

            case OPCODE_STORE:
            case OPCODE_STOREP:
            {
                cpu->data_memory[stage->memory_address] = stage->rs1_value;
                break;
            }
            default:
            {
                break;
            }
        }

        /* Copy data from memory latch to writeback latch*/
        cpu->stage[WB] = cpu->stage[MEM];

        if (ENABLE_DEBUG_MESSAGES) {
            print_stage_content("Memory", stage);
        }

        if (strcmp(stage->opcode, "HALT") == 0) {
            stage->is_interrupted = 1;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("Memory: Empty\n");
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    CPU_Stage* stage = &cpu->stage[WB];
    if (!stage->has_no_insn && !stage->is_interrupted)
    {
        /* Write result to register file based on instruction type */
        switch (stage->opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MOVC:
            case OPCODE_LOAD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_JUMP:
            {
                cpu->regs[stage->rd] = stage->result_buffer;
                break;
            }

            case OPCODE_LOADP:
            {
                cpu->regs[stage->rd] = stage->result_buffer;
                cpu->stage->rs1_value = stage->result_buffer;
                break;
            }

            case OPCODE_JALR:
            {
                cpu->regs[stage->rd] = stage->result_buffer;
                cpu->regs[stage->pc] = stage->result_buffer;
                break;
            }

            case OPCODE_STORE:
            case OPCODE_STOREP:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_BN:
            case OPCODE_BNN:
            case OPCODE_HALT:
            case OPCODE_NOP:
            case OPCODE_CMP:
            {
                break;
            }

            case OPCODE_CML:
            {
                int src1_value = stage->rs1_value;
                int literal = stage->imm;
            }
        }
        
        if ((stage->rd != cpu->stage[MEM].rd || strcmp(cpu->stage[MEM].opcode, "STORE") == 0 || strcmp(cpu->stage[MEM].opcode, "BZ") == 0 ||
                strcmp(cpu->stage[MEM].opcode, "BNZ") == 0 || strcmp(cpu->stage[MEM].opcode, "JUMP") == 0 || strcmp(cpu->stage[MEM].opcode, "BUBBLE") == 0) &&
                (stage->rd != cpu->stage[EX].rd || strcmp(cpu->stage[EX].opcode, "STORE") == 0 || strcmp(cpu->stage[EX].opcode, "BZ") == 0 ||
                strcmp(cpu->stage[EX].opcode, "BNZ") == 0 || strcmp(cpu->stage[EX].opcode, "JUMP") == 0 || strcmp(cpu->stage[EX].opcode, "BUBBLE") == 0)) {
            cpu->regChecking[stage->rd] = 1;
        }

        /* Update Z-flag */
        if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0 ||
            strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0 || strcmp(stage->opcode, "DIV") == 0 ||
            strcmp(stage->opcode, "CMP") == 0 || strcmp(stage->opcode, "CML") == 0)
        {
            /*  Check whether there is an instruction (ADD or SUB or MUL or DIV or ADDL or SUBL or CMP or CML) in EX or MEM stage
            *  If so, do NOT make zero_flag valid
            */
            if (strcmp(cpu->stage[MEM].opcode, "ADD") != 0 && strcmp(cpu->stage[MEM].opcode, "SUB") != 0 && strcmp(cpu->stage[MEM].opcode, "MUL") != 0 &&
                strcmp(cpu->stage[MEM].opcode, "ADDL") != 0 && strcmp(cpu->stage[MEM].opcode, "SUBL") != 0 && strcmp(cpu->stage[MEM].opcode, "DIV") != 0 &&
                strcmp(cpu->stage[MEM].opcode, "CMP") != 0 && strcmp(cpu->stage[MEM].opcode, "CML") != 0 &&
                strcmp(cpu->stage[EX].opcode, "ADD") != 0 && strcmp(cpu->stage[EX].opcode, "SUB") != 0 && strcmp(cpu->stage[EX].opcode, "MUL") != 0 &&
                strcmp(cpu->stage[EX].opcode, "ADDL") != 0 && strcmp(cpu->stage[EX].opcode, "SUBL") != 0 && strcmp(cpu->stage[EX].opcode, "DIV") != 0 &&
                strcmp(cpu->stage[EX].opcode, "CMP") != 0 && strcmp(cpu->stage[EX].opcode, "CML") != 0) {

                cpu->zero_flag_valid = 1;
                if (stage->result_buffer == 0) {
                    cpu->cc_flags.Z = 1;
                }
                else {
                    cpu->cc_flags.Z = 0;
                }
            }
        }

        cpu->insn_completed++;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", stage);
        }

        if (stage->opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }

        if (stage->opcode == OPCODE_NOP)
        {
            cpu->stage[Fetch] = cpu->stage[WB];
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("Writeback: EMPTY\n");
        
        }
    }
    return 0;
}

int print_state_of_architectural_register_file(APEX_CPU *cpu)
{
    printf("\n |============= STATE OF ARCHITECTURAL REGISTER FILE =============|\n");

    for (int i = 0; i < ((sizeof(cpu->regs) / sizeof(cpu->regs[0]))); ++i)
    {
        char status[10];
        if (cpu->regChecking[i])
        {
            strcpy(status, "NOT VALID");
        }
        else
        {
            strcpy(status, "VALID");
        }
        printf("| \t REG[%d] \t | \t Value = %d \t | \t Status = %s \t \n", i, cpu->regs[i], status);
    }
    return 0;
}

int print_state_of_data_memory(APEX_CPU *cpu)
{
    printf("\n |================ STATE OF DATA MEMORY ================|\n");
    for (int i = 0; i < 250; i++)
    {
        printf("| \t MEM[%d] \t | \t Data Value = %d \t \n", i, cpu->data_memory[i]);
    }
    return 0;
}

int APEX_cpu_display_simulate_show_mem(APEX_CPU *cpu, int cycEntred, const char *functionType)
{
    if (strcmp(functionType, "display") == 0)
    {
        printf("\n display   ####################################################################   display\n");
        for (int i = 0; i < cycEntred; cycEntred--)
        {
            if (ENABLE_DEBUG_MESSAGES)
            {
                printf("--------------------------------------------\n");
                int clockCycle = cpu->clock + 1;
                printf("Clock Cycle #: %d\n", clockCycle);
                printf("--------------------------------------------\n");
            }
            if (APEX_writeback(cpu))
            {
                int clockCycle = cpu->clock + 1;
                printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", clockCycle, cpu->insn_completed);
                break;
            }
            APEX_memory(cpu);
            APEX_execute(cpu);
            APEX_decode(cpu);
            APEX_fetch(cpu);
            print_reg_file(cpu);
            cpu->clock++;
        }
        print_state_of_architectural_register_file(cpu);
        print_state_of_data_memory(cpu);
    }

    else if (strcmp(functionType, "simulate") == 0)
    {
        char user_prompt_val;
        printf("\nsimulate   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    simulate\n");
        for (int i = 0; i < cycEntred; --cycEntred)
        {
            if (ENABLE_DEBUG_MESSAGES)
            {
                printf("--------------------------------------------\n");
                int clockCycle = cpu->clock + 1;
                printf("Clock Cycle #: %d\n", clockCycle);
                printf("--------------------------------------------\n");
            }
            if (APEX_writeback(cpu))
            {
                int clockCycle = cpu->clock + 1;
                printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", clockCycle, cpu->insn_completed);
                break;
            }
            APEX_memory(cpu);
            APEX_execute(cpu);
            APEX_decode(cpu);
            APEX_fetch(cpu);
            print_reg_file(cpu);
            cpu->clock++;
        }

        while (TRUE)
        {
            if (ENABLE_DEBUG_MESSAGES)
            {
                printf("--------------------------------------------\n");
                int clockCycle = cpu->clock + 1;
                printf("Clock Cycle #: %d\n", clockCycle);
                printf("--------------------------------------------\n");
            }

            if (APEX_writeback(cpu))
            {
                int clockCycle = cpu->clock + 1;
                printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", clockCycle, cpu->insn_completed);
                break;
            }

            APEX_memory(cpu);
            APEX_execute(cpu);
            APEX_decode(cpu);
            APEX_fetch(cpu);

            print_reg_file(cpu);

            if (cpu->single_step)
            {
                printf("Press any key to advance CPU Clock or <q> to quit:\n");
                scanf("%c", &user_prompt_val);

                if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
                {
                    int clockCycle = cpu->clock + 1;
                    printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", clockCycle, cpu->insn_completed);
                    break;
                }
            }
            cpu->clock++;

        }
        print_state_of_architectural_register_file(cpu);
        print_state_of_data_memory(cpu);
    }

    else if (strcmp(functionType, "show_mem") == 0)
    {
        for (int i = 0; i < cycEntred; cycEntred--)
        {
            if (APEX_writeback(cpu))
            {
                int clockCycle = cpu->clock + 1;
                printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", clockCycle, cpu->insn_completed);
            }
            APEX_memory(cpu);
            APEX_execute(cpu);
            APEX_decode(cpu);
            APEX_fetch(cpu);
            cpu->clock++;
        }
        print_state_of_architectural_register_file(cpu);
        print_state_of_data_memory(cpu);
    }
    return 0;
}

int
control_flow(APEX_CPU* cpu)
{
    CPU_Stage* stage = &cpu->stage[EX];
    int diff = stage->result_buffer - stage->pc;

    if (strcmp(cpu->stage[DRF].opcode, "BZ") == 0 || strcmp(cpu->stage[DRF].opcode, "BNZ") == 0 || strcmp(cpu->stage[DRF].opcode, "BP") == 0 ||
        strcmp(cpu->stage[DRF].opcode, "BNP") == 0 || strcmp(cpu->stage[DRF].opcode, "BN") == 0 || strcmp(cpu->stage[DRF].opcode, "BNN") == 0) {
        cpu->stage[DRF].pc = 0000;
    }

    if (strcmp(cpu->stage[Fetch].opcode, "BZ") == 0 || strcmp(cpu->stage[Fetch].opcode, "BNZ") == 0 || strcmp(cpu->stage[Fetch].opcode, "BP") == 0 ||
        strcmp(cpu->stage[Fetch].opcode, "BNP") == 0 || strcmp(cpu->stage[Fetch].opcode, "BN") == 0 || strcmp(cpu->stage[Fetch].opcode, "BNN") == 0) {
        cpu->stage[Fetch].pc = 0000;
        cpu->stage[Fetch].is_interrupted = 1;
    }

  cpu->pc = stage->result_buffer;

  if (ENABLE_COUNTING) {
    if (diff == 4) {
      cpu->code_memory_size += 2;
    }

    if (diff == 8) {
      cpu->code_memory_size++;
    }

    if (diff > 12) {
      int temp = (diff - 12) / 4;
      cpu->code_memory_size -= temp;
    }

    if (stage->imm < 0) {
      diff = abs(diff);
      int temp = diff / 4;
      cpu->code_memory_size = cpu->code_memory_size + temp + 3;
    }
  }
  return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename, const char* function, const int cycles)
{
    int i;
    APEX_CPU *cpu;

    if (!filename && !function && !cycles)
    {
        return NULL;
    }

    cpu = malloc(sizeof(*cpu));

    if (!cpu)
    {
        return NULL;
    }

    if (strcmp(function, "simulate") == 0) {
        ENABLE_DISPLAY = 1;
    }
    else {
        ENABLE_DISPLAY = 1;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;
    // cpu->fetch.is_interrupted = 0;
    // cpu->decode.is_interrupted = 0;
    // cpu->execute.is_interrupted = 0;
    // cpu->memory.is_interrupted = 0;
    // cpu->writeback.is_interrupted = 0;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

    /* Making Z flag invalid for the first branch instruction */
    cpu->zero_flag_valid = 1;

    cpu->clock = 1;

    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

        for (int i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* Make all stages busy except Fetch stage, initally to start the pipeline */
    for (int i = 1; i < NUM_STAGES; ++i) {
        cpu->stage[i].has_no_insn = 1;
    }

    ENABLE_COUNTING = 0;
    cpu->code_memory_size = cycles;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    while (cpu->clock <= cpu->code_memory_size)
    {
        if (ENABLE_COUNTING)
        {
            /* All the instructions committed, so exit */
            if (cpu->insn_completed == cpu->code_memory_size) {
                printf("(apex) >> Simulation Complete\n");
                break;
            }
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("Positive Flag: %d\nNegative Flag: %d\nZero Flag: %d\n", cpu->cc_flags.P, cpu->cc_flags.N, cpu->cc_flags.Z);
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        /*print_reg_file(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("Positive Flag: %d\nNegative Flag: %d\nZero Flag: %d\n", cpu->positive_flag, cpu->negative_flag, cpu->zero_flag);
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }*/

        cpu->clock++;
    }

    if (ENABLE_DISPLAY) {
        print_state_of_architectural_register_file(cpu);
        print_state_of_data_memory(cpu);
    }
    return 0;
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}