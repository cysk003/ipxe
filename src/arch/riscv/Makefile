# Assembler section type character
#
ASM_TCHAR	:= @
ASM_TCHAR_OPS	:= @

# Include RISCV-specific headers
#
INCDIRS		:= arch/$(ARCH)/include arch/riscv/include $(INCDIRS)

# RISCV-specific directories containing source files
#
SRCDIRS		+= arch/riscv/core

# RISCV-specific flags
#
CFLAGS		+= -mno-strict-align -mno-plt

# EFI requires -fshort-wchar, and nothing else currently uses wchar_t
#
CFLAGS          += -fshort-wchar
