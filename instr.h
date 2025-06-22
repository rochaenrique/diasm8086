#ifndef _H_INSTR_
#define _H_INSTR_

typedef enum Fields {
	REG,
	R_M,
	MOD,
	Fields_Size,
} Fields;

typedef enum Flags {
	W,
	D,
	S,
	Flags_Size,
} Flags;

typedef struct Instr_Data {
	u8 fields[Fields_Size];
	u8 flags_bitset;
} Instr_Data;

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

#define ARR_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))
#define INSTR_OPCODE_NBITS(byte, bits) \
	byte >> (8 - bits) < (int)ARR_COUNT(Opcode_Instr_## bits ## bit)	\
	 ? (Opcode_Instr_## bits ## bit)[byte >> (8 - bits)] : None

#endif // _H_INSTR_
