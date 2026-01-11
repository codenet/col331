\# Errata — Lecture Notes 2



\## Page 14: Conditional Jump Instructions



\### 1. Typographical Error

The mnemonic `J\[N]Z` appears on the slide.  

This is not a valid x86 instruction.



\*\*Correction:\*\*  

`JNZ` (Jump if Not Zero)



---



\### 2. Incorrect Description of `JP`



The slide describes `JP` as "Jump if positive".



According to the Intel Software Developer’s Manual:

\- `JP` corresponds to the \*\*Parity Flag (PF = 1)\*\*

\- It does not represent positivity or negativity



\*\*Reference:\*\*

\- Intel SDM, Table 7.4 (Conditional Jumps)

\- Intel SDM, Section 3.4.3.1 (Status Flags)



