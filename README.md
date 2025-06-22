# diasm8086
My own implementation of the computer enhanced series intel 8086 dissassembler. 

## Usage
```bash
./diasm8086 <assembled_8086_asm_file>
```

## Test
Create a listings directory and place the listings you wish to test. The `test.sh` file accepts a 4 digit 0-padded number and will dissasemble then assemble with `nasm` and compare with the original.

## Resources
- [Computer Enhanced](https://www.computerenhance.com/p/table-of-contents)
- [Intel 8086 Family Manual](https://edge.edx.org/c4x/BITSPilani/EEE231/asset/8086_family_Users_Manual_1_.pdf)
- [Net-wide Assembler (NASM)](https://www.nasm.us/)
