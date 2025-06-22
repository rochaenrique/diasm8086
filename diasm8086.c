#include <stdio.h>

#include "diasm8086.h"
#include "instr.h"
#include "decode.h"
#include "mem.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Error: Expected input files\n");
		return 1;
	}

	IF_EXIT(!load_file_mem(argv[1]), "Error: Unable to open '%s'\n", argv[1]);
	
	printf("; %s dissasembly\n", argv[1]);
	printf("bits 16\n\n");

	int b1, b2;
	while ((b1 = fgetc(fp)) != EOF) {
		if ((b2 = fgetc(fp)) == EOF) break;

		// decoding opcode into instruction
		Instr instr = None;
		if (!instr) instr = INSTR_OPCODE_NBITS(b1, 4);
		if (!instr) instr = INSTR_OPCODE_NBITS(b1, 6);
		if (!instr) instr = INSTR_OPCODE_NBITS(b1, 7);
		if (!instr) instr = INSTR_OPCODE_NBITS(b1, 8);
		IF_EXIT(!instr, "Error: Unknown OPCODE:, 0x%x, ", b1);
		
		IF_EXIT(instr >= sizeof Instr_Decode_Func,
				"Error: Missing decode function to instr code: %d\n", (int)instr);
		
		char src[16], dest[16];
		Instr_Decode_Func[instr](b1, b2, src, dest);

		const char *nmonic;
		if (instr == ARITHM_Imm_and_Reg_Mem) {
			uint8_t arithm_prefix = (b2 >> 3) & 0x7;
			nmonic = ARITHM_Prefix_to_Nmonic[arithm_prefix];
		} else {
			nmonic = Instr_Nmonic[instr];
		}

		IF_EXIT(!nmonic, "Error: unknown instruction nmonic: instr code: %d\n", (int)instr);

		if (strlen(src) > 0) // if there is no src, just print the destination (instructions like jmp)
			printf("%s %.16s, %.16s\n", nmonic, dest, src);
		else
			printf("%s %.16s\n", nmonic, dest);
	}
}

