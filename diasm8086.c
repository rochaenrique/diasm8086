#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

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

#define MAX_LABELS 64

typedef struct Labels {
	int instr_pos[MAX_LABELS];
	size_t len;
} Labels;

typedef enum Data_Type {
	Mem,
	Imm,
	Explicit_Imm,
} Data_Type;

typedef enum Instr {
	None,
	ARITHM_Imm_and_Reg_Mem, // Generic arithmetic instruction, used by: ADD_Imm_to_Reg_Mem, SUB_Imm_from_Reg_Mem
	
	MOV_Reg_Mem_to_from_Reg, // Move register/memory to register/memory
	MOV_Imm_to_Reg_Mem,      // Move immediate to register/memory
	MOV_Imm_to_Reg,          // Move immediate to register
	MOV_Mem_to_Acc,          // Move memory value to accumulator
	MOV_Acc_to_Mem,          // Move accumulator value to memory

	ADD_Reg_Mem_with_Reg_to_Reg_Mem, // Add register/memory with a register into register/memory
	ADD_Imm_to_Reg_Mem,              // * Add immediate to register/memory *(decoded with ARITHM_Imm_and_Reg_Mem)
	ADD_Imm_to_Acc,                  // Add immediate to the accumulator
	
	SUB_Reg_Mem_and_Reg_to_Reg_Mem, // Subtract register/memory and register into register/memory
	SUB_Imm_from_Reg_Mem,           // * Subtract immediate from register/memory  *(decoded with ARITHM_Imm_and_Reg_Mem)
	SUB_Imm_from_Acc,               // Subtract immediate from accumulator
	
	CMP_Reg_Mem_and_Reg,  // Compare register/memory and register
	CMP_Imm_with_Reg_Mem, // * Compare immediate with register/memory  *(decoded with ARITHM_Imm_and_Reg_Mem)
	CMP_Imm_with_Acc,     // Compare immediate with accumulator

	JE_Jmp_Eq_Zr,			// JE/JZ Jump on equal/zero
	JL_Jmp_Less,			// JL/JNGE Jump on less/not greater or equal
	JLE_Jmp_Less_or_Eq,		// JLE/JNG Jump on less or equal/not greater
	JB_Jmp_Below,			// JB/JNAE Jumpo on below/not above or equal
	JBE_Jmp_Below_or_Eq,	// JBE/JNA Jump on below or equal/not above
	JP_Jmp_Parity,			// JP/JMPE Jump on partity/parity even
	JO_Jmp_Overflow,		// Jump on overflow
	JS_Jmp_Sign,			// Jump on sign
	JNE_Jump_Not_Eq,		// JNE/JNZ Jump on not equal/not zero
	JNL_Jmp_Not_Less,		// JNL/JGE Jump on not less/greater or equal
	JG_Jmp_Gt,				// JNLE/JG Jump on not less or equal/greater
	JNB_Jmp_Not_Below,		// JNB/JAE Jump on not below/above or equal
	JA_Jmp_Above,			// JNBE/JA Jump on not below or equal/above
	JNP_Not_Par,			// JNP/JPO Jump on not par/par odd
	JNO_Not_OverFlow,		// Jump on not overflow
	JNS_Not_Sign,			// Jump on not sign
	LOOP_CX_Times,			// Loop CX Times
	LOOPZ_While_Zero,		// LOOPZ/LOOPE Loop while zero/equal
	LOOPNZ_While_Not_Zero,	// LOOPNZ/LOOPNE Loop while not zero/equal
	JCXZ_Jmp_CX_Zero,		// JCXZ Jump on CX zero
} Instr;

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

#define COUNT_OF_INT_ARR(arr) (int)(sizeof(arr)/4)
#define INSTR_OPCODE_NBITS(byte, bits) \
	byte >> (8 - bits) < COUNT_OF_INT_ARR(Opcode_Instr_## bits ## bit) \
	 ? (Opcode_Instr_## bits ## bit)[byte >> (8 - bits)] : None

// NOTE: This will not work with the ARITHM_Imm_and_Reg_Mem Instr, since it is a generic instruction for many,
// use table below
static const char* Instr_Nmonic[] = {
	[MOV_Reg_Mem_to_from_Reg			... MOV_Acc_to_Mem]		= "mov",
	[ADD_Reg_Mem_with_Reg_to_Reg_Mem	... ADD_Imm_to_Acc]		= "add",
	[SUB_Reg_Mem_and_Reg_to_Reg_Mem		... SUB_Imm_from_Acc]	= "sub",
	[CMP_Reg_Mem_and_Reg				... CMP_Imm_with_Acc]	= "cmp",
	[JE_Jmp_Eq_Zr]		    = "je",			
	[JL_Jmp_Less]		    = "jl",			
	[JLE_Jmp_Less_or_Eq]    = "jle",		
	[JB_Jmp_Below]		    = "jb",			
	[JBE_Jmp_Below_or_Eq]   = "jbe",	
	[JP_Jmp_Parity]		    = "jp",			
	[JO_Jmp_Overflow]	    = "jo",		
	[JS_Jmp_Sign]		    = "js",			
	[JNE_Jump_Not_Eq]	    = "jne",		
	[JNL_Jmp_Not_Less]	    = "jnl",		
	[JG_Jmp_Gt]			    = "jg",				
	[JNB_Jmp_Not_Below]	    = "jnb",		
	[JA_Jmp_Above]		    = "ja",			
	[JNP_Not_Par]		    = "jnp",			
	[JNO_Not_OverFlow]	    = "jno",		
	[JNS_Not_Sign]		    = "jns",			
	[LOOP_CX_Times]		    = "loop",			
	[LOOPZ_While_Zero]	    = "loopz",		
	[LOOPNZ_While_Not_Zero] = "loopnz",	
	[JCXZ_Jmp_CX_Zero]	    = "jcxz", 	
};

static const char* ARITHM_Prefix_to_Nmonic[] = {
	[0x0] = "add", // 000
	[0x5] = "sub", // 101
	[0x7] = "cmp", // 111
};

#define DECODE_FUNC_PARAMS (uint8_t, uint8_t, char[], char[])

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

typedef void decode_func(uint8_t b1, uint8_t b2, char src[], char dest[]);
static decode_func* Instr_Decode_Func[] = {
	[ARITHM_Imm_and_Reg_Mem]			= decode_Imm_and_Reg_Mem_SW,
	
	[MOV_Reg_Mem_to_from_Reg]			= decode_Reg_and_Disp_DW,
	[MOV_Imm_to_Reg_Mem]				= decode_MOV_Imm_to_Reg_Mem,
	[MOV_Imm_to_Reg]					= decode_MOV_Imm_to_Reg,
	[MOV_Mem_to_Acc]					= decode_MOV_Mem_to_Acc,	   
	[MOV_Acc_to_Mem]					= decode_MOV_Acc_to_Mem,
									 
	[ADD_Reg_Mem_with_Reg_to_Reg_Mem]	= decode_Reg_and_Disp_DW, 
	[ADD_Imm_to_Acc]					= decode_Imm_to_Acc,
									 
	[SUB_Reg_Mem_and_Reg_to_Reg_Mem]	= decode_Reg_and_Disp_DW, 
	[SUB_Imm_from_Acc]					= decode_Imm_to_Acc,

	[CMP_Reg_Mem_and_Reg]				= decode_Reg_and_Disp_DW, 
	[CMP_Imm_with_Acc]					= decode_Imm_to_Acc,

	[JE_Jmp_Eq_Zr ... JCXZ_Jmp_CX_Zero] = decode_Jmp_8bit_Inc_IP,
};



void print_bits8(int v);
void print_bits16(int v);
void write_signed_eff_addr(char *buf, uint8_t r_m, int16_t data);
void decode_reg_mem(char *buf, uint8_t mod, uint8_t r_m, bool w_flag);
void write_fmt_data_w(char *buf, int8_t data_low, bool w_flag, Data_Type type);
void init_labels();
size_t append_label_by_inc(int instr_ptr_inc);
int curr_label();

// utility macros
#define CONCAT_BYTES(high, low) ((int16_t)(high) << 8) + (uint8_t)(low)
#define UNIMPLEMENTED() do { printf("%s:%d: Unimplmented: in %s()\n", __FILE__, __LINE__, __func__); abort(); } while(0)
#if 0
#define HERE() printf("; %s()\n", __func__)
#else
#define HERE()
#endif

static FILE *fp = 0;
static int instr_ptr = 0;
static Labels labels = {0};

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Error: Expected input files\n");
		return 1;
	}

	fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "Error: Unable to open '%s'\n", argv[1]);
		return 1;
	}

	init_labels();
	
	printf("; %s dissasembly\n", argv[1]);
	printf("bits 16\n\n");

	int b1, b2;
	while ((b1 = fgetc(fp)) != EOF) {
		instr_ptr++;
		if ((b2 = fgetc(fp)) == EOF) break;
		instr_ptr++;

		// decoding opcode into instruction
		Instr instr = None;
		if (!instr) instr = INSTR_OPCODE_NBITS(b1, 4);
		if (!instr) instr = INSTR_OPCODE_NBITS(b1, 6);
		if (!instr) instr = INSTR_OPCODE_NBITS(b1, 7);
		if (!instr) instr = INSTR_OPCODE_NBITS(b1, 8);
		if (!instr) {
			fprintf(stderr, "Error: Unknown OPCODE:, 0x%x, ", b1);
			print_bits8(b1);
			return 1;
		}
		
		if (instr >= sizeof Instr_Decode_Func) {
			fprintf(stderr, "Error: Missing decode function to instr code: %d\n", (int)instr);
			return 1;
		}
		
		char src[16], dest[16];
		Instr_Decode_Func[instr](b1, b2, src, dest);

		const char *nmonic;
		if (instr == ARITHM_Imm_and_Reg_Mem) {
			uint8_t arithm_prefix = (b2 >> 3) & 0x7;
			nmonic = ARITHM_Prefix_to_Nmonic[arithm_prefix];
		} else {
			nmonic = Instr_Nmonic[instr];
		}

		if (!nmonic) {
			fprintf(stderr, "Error: unknown instruction nmonic: instr code: %d\n", (int)instr);
			return 1;
		}

		int label = curr_label();
		if (label != -1) // if the current instruction pointer has a label
			printf("label_%d:\n", label);

		if (strlen(src) > 0) // if there is no src, just print the destination (instructions like jmp)
			printf("%s %.16s, %.16s\n", nmonic, dest, src);
		else
			printf("%s %.16s\n", nmonic, dest);
	}
}

// ============================================================
// Decode functions
// ============================================================

void decode_Reg_and_Disp_DW(uint8_t b1, uint8_t b2, char src[], char dest[]) {
	HERE();
	uint8_t d_flag = b1 & 0x2;
	bool w_flag = b1 & 0x1;
	uint8_t mod	= b2 >> 6;
	uint8_t reg	= (b2 >> 3) & 0x7;
	uint8_t r_m = b2 & 0x7;

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

void decode_Imm_and_Reg_Mem_SW(uint8_t b1, uint8_t b2, char src[], char dest[]) {
	HERE();

	bool s_flag = b1 & 0x2;
	bool w_flag = b1 & 0x1;
	uint8_t mod	   = b2 >> 6;
	uint8_t r_m    = b2 & 0x7;

	decode_reg_mem(dest, mod, r_m, w_flag);
	
	uint8_t data_low = fgetc(fp);
	
	write_fmt_data_w(src, data_low, !s_flag && w_flag, Imm);
}

void decode_Imm_to_Acc(uint8_t b1, uint8_t addr_low, char src[], char dest[]) {
	HERE();
	bool w_flag = b1 & 0x1;

	if (w_flag == W_FLAG_DATA)
		sprintf(dest, "al");
	else // W_FLAG_WORD
		sprintf(dest, "ax");
	
	write_fmt_data_w(src, addr_low, w_flag, Imm);
}

void decode_Jmp_8bit_Inc_IP(uint8_t _, uint8_t ip_8bit_inc, char src[], char dest[]) {
	HERE();
	
	size_t label = append_label_by_inc(ip_8bit_inc);
	sprintf(dest, "label_%zu", label); // convert to signed
	strcpy(src, "");
}

void decode_MOV_Imm_to_Reg_Mem(uint8_t b1, uint8_t b2, char src[], char dest[]) {
	HERE();
	bool w_flag = b1 & 0x1;
	uint8_t mod    = b2 >> 6;
	uint8_t r_m    = b2 & 0x7;

	decode_reg_mem(dest, mod, r_m, w_flag);

	uint8_t data_low = fgetc(fp);
	write_fmt_data_w(src, data_low, w_flag, Explicit_Imm);
}

void decode_MOV_Imm_to_Reg(uint8_t b1, uint8_t data_low, char src[], char dest[]) {
	HERE();
	bool w_flag = b1 & 0x8;
	uint8_t reg = (b1 >> 3) & 0x7;
	

	strcpy(dest, REG_CODES_REG_W[reg][w_flag]);
	write_fmt_data_w(src, data_low, w_flag, Imm);
}

void decode_MOV_Mem_to_Acc(uint8_t b1, uint8_t addr_low, char src[], char dest[]) {
	HERE();
	bool w_flag = b1 & 0x1;
	
	sprintf(dest, "ax");
	write_fmt_data_w(src, addr_low, w_flag, Mem);
}

void decode_MOV_Acc_to_Mem(uint8_t b1, uint8_t addr_low, char src[], char dest[]) {
	HERE();
	bool w_flag = b1 & 0x1;
	
	write_fmt_data_w(dest, addr_low, w_flag, Mem);
	sprintf(src, "ax");
}

// ============================================================
// Uitility functions
// ============================================================

void print_bits8(int v) {
	for (int i = 7; i >= 0; i--) printf("%d", (v >> i) & 1);
	printf("\n");
}

void print_bits16(int v) {
	for (int i = 15; i >= 0; i--) printf("%d", (v >> i) & 1);
	printf("\n");
}

void write_signed_eff_addr(char *buf, uint8_t r_m, int16_t data) {
	if (!data) {
		sprintf(buf, "[%s]", EFF_ADDR_NO_DISP_R_M[r_m]);
	} else {
		char sign = data > 0 ? '+' : '-';
		sprintf(buf, "[%s %c %d]", EFF_ADDR_NO_DISP_R_M[r_m], sign, abs(data));
	}
}

void decode_reg_mem(char *buf, uint8_t mod, uint8_t r_m, bool w_flag) {
	switch (mod) {
	case MOD_MEMORY_MODE_NO_DISP: {
		if (r_m == EFF_ADDR_NO_DISP_DIR_ACC_R_M) {
			
			int8_t disp_low = fgetc(fp);
			int8_t disp_high = fgetc(fp);
			sprintf(buf, "[%d]", CONCAT_BYTES(disp_high, disp_low));
			
		} else {
			sprintf(buf, "[%s]", EFF_ADDR_NO_DISP_R_M[r_m]);
		}
			
		break;
	}
	case MOD_MEMORY_MODE_8_DISP: {
		int8_t data = fgetc(fp);
		write_signed_eff_addr(buf, r_m, data);
		break;
	}
	case MOD_MEMORY_MODE_16_DISP: {
		int8_t disp_low = fgetc(fp);
		int8_t disp_high = fgetc(fp);
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

void write_fmt_data_w(char *buf, int8_t data_low, bool w_flag, Data_Type type) {
	HERE();
	int16_t data = data_low;

	if (w_flag == W_FLAG_WORD) {
		int8_t data_high = fgetc(fp);
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

void init_labels() {
	for (size_t i = 0; i < MAX_LABELS; i++)
		labels.instr_pos[i] = -1;
}

size_t append_label_by_inc(int instr_ptr_inc) {
	if (labels.len >= MAX_LABELS) {
		fprintf(stderr, "Error: Too many labels!\n");
		exit(1);
	}

	int new_label = instr_ptr + instr_ptr_inc;

	size_t i = 0;
	while (i < labels.len && new_label < labels.instr_pos[i])
		i++;

	if (new_label != labels.instr_pos[i]) {
		for (size_t j = i + 1; j <= labels.len; j++)
			labels.instr_pos[j] = labels.instr_pos[j - 1];

		labels.instr_pos[i] = new_label; // new label at the end
		labels.len++;
	}

	return i;
}

int curr_label() {
	int i = labels.len;
	while (i != 0 && labels.instr_pos[i] != instr_ptr) 
		i--;

	return i != 0 ? i : -1;
}
