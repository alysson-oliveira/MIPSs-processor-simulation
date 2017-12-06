/* code-c - Student's code for mmcpu

   Copyright 2017  Monaco F. J.   <monaco@icmc.usp.br>

   This file is part of Muticlycle Mips CPU (MMCPU)

   MMCPU is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


//Alysson Oliveira - 9771355



/* After you have implemented your functions, you may safely remove these lines. */
#include <stdio.h>
#include <stdlib.h>
#include "mask.h"		/* Useful masks; customize at you please. */
#include "cpu.h"
#include "cpu.c"
char overflow;
char zero;

//Realiza operacoes de acordo com o alu_op
int alu(int a, int b, char alu_op, int *result_alu, char *zero, char *overflow){
    switch(alu_op) //Seleciona a operacao da ula
    {
      case ativa_soma:
        *result_alu = a + b;
        break;
      case ativa_subtracao:
        *result_alu = a - b;
        break;
      case ativa_or:
        *result_alu = a | b;
        break;
      case ativa_and:
        *result_alu = a & b;
        break;
      case ativa_slt:
        if(a<b) *result_alu = 1;
        else    *result_alu = 0;
        break;
    }
    if(*result_alu == 0) *zero = ativa_bit_zero;
    else *zero = desativa_bit_zero;
    return 0;
}

//Passa alu_op para a ula
char alu_control(int IR, int sc){
    switch(((sc & separa_ALUOp0) | (sc & separa_ALUOp1)) >> 5)
    {
        //LW e SW
        case 0x0:
            return 0b0010;
            break;
        //Branch
        case 0x1:
            return 0b0110;
            break;
        //Tipo-R
        case 0x2:
            switch(IR & 0b1111)
            {
                //ADD
                case 0b0000:
                    return 0b0010;
                    break;

                //SUB
                case 0b0010:
                    return 0b0110;
                    break;
                //AND
                case 0b0100:
                    return 0b0000;
                    break;
                //OR
                case 0b0101:
                    return 0b0001;
                    break;
                //SLT
                case 0b1010:
                    return 0b0111;
                    break;
            }
            break;
        //Tipo-R
        case 0x3:
          switch(IR & 0b1111)
          {
              //SUB
              case 0b0010:
                return 0b0110;
                break;
              //SLT
              case 0b1010:
                return 0b0111;
                break;
          }
          break;
    }
}

void control_unit(int IR, short int *sc){
    if(IR == -1) *sc = 0b1001010000001000;
    else
    {
        char op = (IR & separa_cop) >> 26;
        if(*sc == ((short int) 0b1001010000001000)) *sc = 0b0000000000011000;
        else
        {
            switch(op)
            {
              case ((short int) 0x00): //tipo r;
                switch(*sc)
                {
                  case ((short int)0b0000000000011000):
                    *sc = 0b0000000001000100;
                    break;
                  case ((short int)0b0000000001000100):
                    *sc = 0b0000000000000011;
                    break;
                  case ((short int)0b0000000000000011):
                    *sc = 0b1001010000001000;
                    break;
                }
                break;
              case ((short int) 0x23)://lw
                switch(*sc)
                {
                  case ((short int) 0b0000000000011000):
                    *sc = 0b0000000000010100;
                    break;
                  case ((short int) 0b0000000000010100):
                    *sc = 0b0001100000000000;
                    break;
                  case ((short int) 0b0001100000000000):
                    *sc = 0b0100000000000010;
                    break;
                  case ((short int) 0b0100000000000010):
                    *sc = 0b1001010000001000;
                    break;
                }
                    break;
              case ((short int) 0x2b)://sw
                switch(*sc)
                {
                  case ((short int) 0b0000000000011000):
                    *sc = 0b0000000000010100;
                    break;
                  case ((short int) 0b0000000000010100):
                    *sc = 0b0000100000000010;
                    break;
                  case ((short int) 0b0000100000000010):
                    *sc = 0b1001010000001000;
                    break;
                }
                break;
              case ((short int) 0x04)://beq
                switch(*sc)
                {
                  case ((short int) 0b0000000000011000):
                    *sc = 0b0000001010100100;
                    break;
                  case ((short int) 0b0000001010100100):
                    *sc = 0b1001010000001000;
                    break;
                }
                break;
              case ((short int) 0x02)://j
                switch(*sc)
                {
                  case ((short int) 0b0000000000011000):
                    *sc = 0b0000010010000000;
                    break;
                  case ((short int) 0b0000010010000000):
                    *sc = 0b1001010000001000;
                    break;
                }
                break;
            }
        }
    }
}


void instruction_fetch( short int sc, int PC, int ALUOUT, int IR, int* PCnew, int* IRnew,int* MDRnew){
    if(sc == ((short int) 0b1001010000001000))
    {
        *IRnew = memory[PC];
        char alu_op = alu_control(IR, sc);
        alu(PC, 1, alu_op, &ALUOUT, &zero, &overflow);
        *MDRnew = memory[PC];
        *PCnew = ALUOUT;
        if(*IRnew == 0) loop = 0;
  }
}

void decode_register(short int sc, int IR, int PC, int A, int B, int *Anew, int *Bnew, int *ALUOUTnew){
    if(sc == ((short int) 0b0000000000011000))
    {
        *Anew = reg[(separa_rs & IR) >> 21];
        *Bnew = reg[(separa_rt & IR) >> 16];

        char alu_op = alu_control(IR, sc);
        alu(PC, (separa_imediato & IR), alu_op, ALUOUTnew, &zero, &overflow);
    }
}
void exec_calc_end_branch(short int sc, int A, int B, int IR, int PC, int ALUOUT, int *ALUOUTnew, int *PCnew){
    char alu_op = alu_control(IR, sc);
    if(sc == (short int) 0b0000000001000100)    alu(A, B, alu_op, ALUOUTnew, &zero, &overflow);
    if (sc == (short int) 0b0000000000010100)   alu(A, IR & separa_imediato, alu_op, ALUOUTnew, &zero, &overflow);
    if(sc == (short int) 0b0000001010100100){
        alu(A, B, alu_op, ALUOUTnew, &zero, &overflow);
        if(zero == 1)   *PCnew = ALUOUT;
    }
    if(sc == (short int) 0b0000010010000000)    alu((PC & separa_4bits_PC),(IR & separa_endereco_jump),ativa_or,PCnew,&zero,&overflow);
}

void write_r_access_memory( short int sc,int B, int IR, int ALUOUT, int PC, int *MDRnew, int *IRnew){
  if(sc == (short int) 0b0000000000000011)  reg[(IR & separa_rd) >> 11] = ALUOUT;
  if(sc == (short int) 0b0001100000000000)  *MDRnew = memory[ALUOUT];
  if (sc == (short int) 0b0000100000000010) memory[ALUOUT] = reg[(IR & separa_rt) >> 16];
}

void write_ref_mem(short int sc, int IR, int MDR, int ALUOUT){
    if(sc == ((short int) 0b0100000000000010))  reg[(IR & separa_rt) >> 16] = MDR;
}
