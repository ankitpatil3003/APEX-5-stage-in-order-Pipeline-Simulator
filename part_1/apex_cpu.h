/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"
/*struct flagCheck
{
  int flagIsUsed;
  int flagIsNotUsed;
  int isRegValEmpty;
} flagCheck;*/

enum
{
  Fetch,
  DRF,
  EX,
  MEM,
  WB,
  NUM_STAGES
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int imm;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int memory_address;
    int has_no_insn;
    int is_interrupted;
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction* code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int zero_flag_valid;
    int previous_ins_pc;
    int fetch_from_next_cycle;
    int regChecking[REG_FILE_SIZE];

    /* Array of 5 CPU_stage */
    CPU_Stage stage[5];

    struct {
        int Z;  // Zero flag
        int N;  // Negative flag
        int P;  // Positive flag
    } cc_flags;

    // /* Pipeline stages */
    // CPU_Stage fetch;
    // CPU_Stage decode;
    // CPU_Stage execute;
    // CPU_Stage memory;
    // CPU_Stage writeback;
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename, const char* function, const int cycles);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
int fetch(APEX_CPU* cpu);
int decode(APEX_CPU* cpu);
int execute(APEX_CPU* cpu);
int memory(APEX_CPU* cpu);
int writeback(APEX_CPU* cpu);
#endif
