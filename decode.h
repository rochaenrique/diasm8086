#ifndef _H_DECODE_
#define _H_DECODE_

#include <string.h>

#include "mem.h"
#include "diasm8086.h"

#define W_FLAG_DATA 0
#define W_FLAG_WORD 1

#define D_FLAG_REG_SOURCE 0
#define D_FLAG_REG_DEST   1

#define S_FLAG_NO_SIGN_EXT        0
#define S_FLAG_SIGN_EXT_16_IF_W   1

#define MOD_MEMORY_MODE_NO_DISP 0x0
#define MOD_MEMORY_MODE_8_DISP  0x1
#define MOD_MEMORY_MODE_16_DISP 0x2
#define MOD_REGISTER_MODE       0x3

#define REG_CODES_REG_W (const char *[][2]){	\
	[0x0] = { "al",  "ax" },			        \
	[0x1] = { "cl",  "cx" },			        \
	[0x2] = { "dl",  "dx" },			        \
	[0x3] = { "bl",  "bx" },			        \
	[0x4] = { "ah",  "sp" },			        \
	[0x5] = { "ch",  "bp" },			        \
	[0x6] = { "dh",  "si" },			        \
	[0x7] = { "bh",  "di" }      	            \
}

#define EFF_ADDR_NO_DISP_R_M (const char *[]) {	\
    [0x0] = "bx + si",						\
    [0x1] = "bx + di",						\
    [0x2] = "bp + si",						\
    [0x3] = "bp + di",						\
    [0x4] = "si",							\
    [0x5] = "di",							\
	[0x6] = "bp",							\
    [0x7] = "bx"							\
}
// in no displacement mode at 110, this is direct address
#define EFF_ADDR_NO_DISP_DIR_ACC_R_M 0x6

static Instr Opcode_Instr_4bit[] = {
	[0xB] = MOV_Imm_to_Reg,
};

static Instr Opcode_Instr_6bit[] = {
	[0x22] = MOV_Reg_Mem_to_from_Reg,
	
	[0x20] = ARITHM_Imm_and_Reg_Mem, // Check instruction comments
	[0x00] = ADD_Reg_Mem_with_Reg_to_Reg_Mem,
	[0x0A] = SUB_Reg_Mem_and_Reg_to_Reg_Mem,
	[0x0E] = CMP_Reg_Mem_and_Reg,
};

static Instr Opcode_Instr_7bit[] = {
	[0x63] = MOV_Imm_to_Reg_Mem,
	[0x50] = MOV_Mem_to_Acc,
	[0x51] = MOV_Acc_to_Mem,
	
	[0x02] = ADD_Imm_to_Acc,
	[0x16] = SUB_Imm_from_Acc,
	[0x1E] = CMP_Imm_with_Acc,
};

static Instr Opcode_Instr_8bit[] = {
	[0x74] = JE_Jmp_Eq_Zr,			
	[0x7C] = JL_Jmp_Less,			
	[0x7E] = JLE_Jmp_Less_or_Eq,		
	[0x72] = JB_Jmp_Below,			
	[0x76] = JBE_Jmp_Below_or_Eq,	
	[0x7A] = JP_Jmp_Parity,			
	[0x70] = JO_Jmp_Overflow,		
	[0x78] = JS_Jmp_Sign,			
	[0x75] = JNE_Jump_Not_Eq,		
	[0x7D] = JNL_Jmp_Not_Less,		
	[0x7F] = JG_Jmp_Gt,				
	[0x73] = JNB_Jmp_Not_Below,		
	[0x77] = JA_Jmp_Above,			
	[0x7B] = JNP_Not_Par,			
	[0x71] = JNO_Not_OverFlow,		
	[0x79] = JNS_Not_Sign,			
	[0xE2] = LOOP_CX_Times,			
	[0xE1] = LOOPZ_While_Zero,		
	[0xE0] = LOOPNZ_While_Not_Zero,	
	[0xE3] = JCXZ_Jmp_CX_Zero, 
};


#define DECODE_FUNC_PARAMS (u8, u8, char[], char[])

// generic formats
void decode_Reg_and_Disp_DW        DECODE_FUNC_PARAMS; // decode register and displacement with D and W flags
void decode_Imm_and_Reg_Mem_SW     DECODE_FUNC_PARAMS; // decode immediate and register/memory with D and S flags
                                                       // * decode all arithmetic instructions that involve immediate and register/memory
													   // figures out which arithmetic and uses Imm_and_Reg_Mem_SW

void decode_Imm_to_Acc             DECODE_FUNC_PARAMS; // decode and immidiate value and low or not accumulator with W flag
void decode_Jmp_8bit_Inc_IP        DECODE_FUNC_PARAMS; // decode a jump instruction with a 8 bit instruction pointer increment

// specific decode formats
void decode_MOV_Imm_to_Reg_Mem     DECODE_FUNC_PARAMS;
void decode_MOV_Imm_to_Reg         DECODE_FUNC_PARAMS;
void decode_MOV_Mem_to_Acc         DECODE_FUNC_PARAMS;
void decode_MOV_Acc_to_Mem         DECODE_FUNC_PARAMS;

typedef void decode_func(u8 b1, u8 b2, char src[], char dest[]);
#define SPECIFIC_DECO(x) [x] = decode_##x
#define ARITHM_DECO(reg_mem, imm_acc) 	[reg_mem]	= decode_Reg_and_Disp_DW, [imm_acc] = decode_Imm_to_Acc



static decode_func* Instr_Decode_Func[] = {
	[ARITHM_Imm_and_Reg_Mem]			= decode_Imm_and_Reg_Mem_SW,

	[MOV_Reg_Mem_to_from_Reg]			= decode_Reg_and_Disp_DW,
	
	SPECIFIC_DECO(MOV_Imm_to_Reg_Mem),
	SPECIFIC_DECO(MOV_Imm_to_Reg),
	SPECIFIC_DECO(MOV_Mem_to_Acc),
	SPECIFIC_DECO(MOV_Acc_to_Mem),

	ARITHM_DECO(ADD_Reg_Mem_with_Reg_to_Reg_Mem, ADD_Imm_to_Acc),
	ARITHM_DECO(SUB_Reg_Mem_and_Reg_to_Reg_Mem, SUB_Imm_from_Acc),
	ARITHM_DECO(CMP_Reg_Mem_and_Reg, CMP_Imm_with_Acc),

	[JE_Jmp_Eq_Zr ... JCXZ_Jmp_CX_Zero] = decode_Jmp_8bit_Inc_IP,
};

void write_signed_eff_addr(char *buf, u8 r_m, s16 data);
void decode_reg_mem(char *buf, u8 mod, u8 r_m, bool w_flag);
void write_fmt_data_w(char *buf, s8 data_low, bool w_flag, Data_Type type);

// ============================================================
// Decode functions
// ============================================================

void decode_Reg_and_Disp_DW(u8 b1, u8 b2, char src[], char dest[]) {
	HERE();
	/* bool d_flag = b1 & 0x2; */
	bool d_flag = FLAG(b1, 2);
	bool w_flag = FLAG(b1, 1);

	u8 mod = FIELD(b2, 6, 3);
	u8 reg = FIELD(b2, 3, 3);
	u8 r_m = FIELD(b2, 0, 3);

	char *tmp;

	if (d_flag == D_FLAG_REG_SOURCE) {
		strcpy(src, REG_CODES_REG_W[reg][w_flag]);
		tmp = dest;
	} else { // D_FLAG_REG_DEST
		strcpy(dest, REG_CODES_REG_W[reg][w_flag]);
		tmp = src;
	}

	decode_reg_mem(tmp, mod, r_m, w_flag);
}

void decode_Imm_and_Reg_Mem_SW(u8 b1, u8 b2, char src[], char dest[]) {
	HERE();

	bool s_flag = FLAG(b1, 2);
	bool w_flag = FLAG(b1, 1);
	u8 mod = FIELD(b2, 6, 2);
	u8 r_m = FIELD(b2, 0, 3);

	decode_reg_mem(dest, mod, r_m, w_flag);
	
	u8 data_low = get_byte();
	write_fmt_data_w(src, data_low, !s_flag && w_flag, Imm);
}

void decode_Imm_to_Acc(u8 b1, u8 addr_low, char src[], char dest[]) {
	HERE();
	bool w_flag = FLAG(b1, 0x1);

	if (w_flag == W_FLAG_DATA)
		sprintf(dest, "al");
	else // W_FLAG_WORD
		sprintf(dest, "ax");
	
	write_fmt_data_w(src, addr_low, w_flag, Imm);
}

void decode_Jmp_8bit_Inc_IP(u8 _, u8 ip_8bit_inc, char src[], char dest[]) {
	HERE();

	// TODO: Add label parsing
	sprintf(dest, "label(%d)", ip_8bit_inc); // convert to signed
	strcpy(src, "");
}

void decode_MOV_Imm_to_Reg_Mem(u8 b1, u8 b2, char src[], char dest[]) {
	HERE();
	bool w_flag = FLAG(b1, 1);
	u8 mod = FIELD(b2, 6, 2);
	u8 r_m = FIELD(b2, 0, 3);

	decode_reg_mem(dest, mod, r_m, w_flag);

	u8 data_low = get_byte();
	write_fmt_data_w(src, data_low, w_flag, Explicit_Imm);
}

void decode_MOV_Imm_to_Reg(u8 b1, u8 data_low, char src[], char dest[]) {
	HERE();
	bool w_flag = FLAG(b1, 4);
	u8 reg = FIELD(b1, 3, 3);
	

	strcpy(dest, REG_CODES_REG_W[reg][w_flag]);
	write_fmt_data_w(src, data_low, w_flag, Imm);
}

void decode_MOV_Mem_to_Acc(u8 b1, u8 addr_low, char src[], char dest[]) {
	HERE();
	bool w_flag = FLAG(b1, 1);
	
	sprintf(dest, "ax");
	write_fmt_data_w(src, addr_low, w_flag, Mem);
}

void decode_MOV_Acc_to_Mem(u8 b1, u8 addr_low, char src[], char dest[]) {
	HERE();
	bool w_flag = FLAG(b1, 1);
	
	write_fmt_data_w(dest, addr_low, w_flag, Mem);
	sprintf(src, "ax");
}


// ============================================================
// Uitility functions
// ============================================================

void write_signed_eff_addr(char *buf, u8 r_m, s16 data) {
	if (!data) {
		sprintf(buf, "[%s]", EFF_ADDR_NO_DISP_R_M[r_m]);
	} else {
		char sign = data > 0 ? '+' : '-';
		sprintf(buf, "[%s %c %d]", EFF_ADDR_NO_DISP_R_M[r_m], sign, abs(data));
	}
}

void decode_reg_mem(char *buf, u8 mod, u8 r_m, bool w_flag) {
	switch (mod) {
	case MOD_MEMORY_MODE_NO_DISP: {
		if (r_m == EFF_ADDR_NO_DISP_DIR_ACC_R_M) {
			
			s8 disp_low = get_byte();
			s8 disp_high = get_byte();
			sprintf(buf, "[%d]", CONCAT_BYTES(disp_high, disp_low));
			
		} else {
			sprintf(buf, "[%s]", EFF_ADDR_NO_DISP_R_M[r_m]);
		}
			
		break;
	}
	case MOD_MEMORY_MODE_8_DISP: {
		s8 data = get_byte();
		write_signed_eff_addr(buf, r_m, data);
		break;
	}
	case MOD_MEMORY_MODE_16_DISP: {
		s8 disp_low = get_byte();
		s8 disp_high = get_byte();
		write_signed_eff_addr(buf, r_m, CONCAT_BYTES(disp_high, disp_low));
		break;
	}
	case MOD_REGISTER_MODE:
		sprintf(buf, "%s", REG_CODES_REG_W[r_m][w_flag]);
		break;
	default:
		printf("Error: Unkown MOD fied: 0x%x", mod);
		exit(1);
	}

}

void write_fmt_data_w(char *buf, s8 data_low, bool w_flag, Data_Type type) {
	HERE();
	s16 data = data_low;

	if (w_flag == W_FLAG_WORD) {
		s8 data_high = get_byte();
		data = CONCAT_BYTES(data_high, data_low);
	}
	
	char *fmt;
	switch (type) {
	case Mem: fmt = "[%d]"; break;
	case Imm: fmt = "%d";   break;
	case Explicit_Imm:
		fmt = w_flag ? "word %d" : "byte %d";
		break;
	default:
		fprintf(stderr, "Error: Unknown datatype: %d\n", type);
		exit(1);
	}

	sprintf(buf, fmt, data);
}

#endif // _H_DECODE_
