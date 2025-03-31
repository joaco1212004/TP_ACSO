.text

// Test ZeroExtend function with edge cases
movz X0, 0xFFFF             // Use a valid 16-bit immediate value
lsl X0, X0, 0               // Ensure no shift
adds X1, X0, XZR            // Test ZeroExtend indirectly

// Test check_flags with edge cases
subs X2, X0, X0             // Result should be zero, FLAG_Z = 1
subs X3, X0, 1              // Result should be negative, FLAG_N = 1

// Test adds_extended_register with edge cases
adds X4, XZR, XZR           // Add zero registers
adds X5, X1, XZR            // Add a register with zero

// Test subs_extended_register with edge cases
subs X6, XZR, XZR           // Subtract zero registers
subs X7, X1, XZR            // Subtract zero from a register

// Test adds_shifted_register with edge cases
lsl X12, X1, 63             // Shift register to maximum
adds X13, X1, X12           // Add shifted register

// Test mul with edge cases
movz X14, 0xFFFF            // Use a valid 16-bit immediate value
mul X15, X14, X14           // Multiply maximum values

// Test eor_immediate with edge cases
movz X16, 0xFFF             // Load immediate value into a register
eor X16, XZR, X16           // XOR zero with maximum immediate
movz X17, 0xFFF             // Load immediate value into another register
eor X17, X1, X17            // XOR register with maximum immediate

// Test orr_shift_reg with edge cases
orr X18, XZR, XZR           // OR zero registers
orr X19, X1, XZR            // OR register with zero

// Test b_target with edge cases
b foo                       // Branch to label
nop                         // Placeholder for skipped instruction

foo:
adds X21, X1, XZR           // Should execute

// Test cbz and cbnz with edge cases
cbz XZR, bar                // Branch if zero
nop                         // Placeholder for skipped instruction

bar:
cbnz X1, baz                // Branch if not zero
nop                         // Placeholder for skipped instruction

baz:
hlt 0                       // End of program
