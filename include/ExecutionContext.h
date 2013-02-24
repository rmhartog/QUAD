/*****
 *
 * ExecutionContext.h
 *
 * Author: Reinier M. Hartog
 * Date:   19 November 2012
 *
 * This header dictates the basic interface for a class that represents a target process state.
 * The actual implementation depends on the debugging/instrumentation framework and the target processor architecture.
 *
 *****/

#ifndef EXECUTIONCONTEXT_H
#define EXECUTIONCONTEXT_H

#include <unistd.h>

enum eRegister
{
	EREG_INST_POINTER = 0,
#ifdef TARGET_IA32
	// register definitions for IA32
#endif
#ifdef TARGET_IA64
	// register definitions for A64
#endif
	EREG_MAX
};

class ExecutionContext
{
public:
     // this method retrieves the value of a register from the execution context into *value.
     // the register index is architecture-specific.
     // returns zero on success, non-zero on failure.
     // NOTE: implementations of this method should not modify the execution context in any way,
     //       as indicated by the const-keyword.
     virtual unsigned int	getRegisterValue(enum eRegister reg, unsigned long *value) 	const 	= 0;
     // this method modifies the value of a register from the execution context to value 'value'.
     // the register index is architecture-specific.
     // returns zero on success, non-zero on failure.
     virtual unsigned int	setRegisterValue(enum eRegister reg, unsigned long value)		= 0;

     // this method retrieves the instruction pointer from the execution context into *value.
     // returns zero on success, non-zero on failure.
     // NOTE: implementations of this method should not modify the execution context in any way,
     //       as indicated by the const-keyword.
     virtual unsigned int	getInstructionPointer(void **value)				const 	= 0;
     // this method modifies the instruction pointer from the execution context to address 'value'.
     // returns zero on success, non-zero on failure.
     virtual unsigned int	setInstructionPointer(void *value)					= 0;

     // this method copies a block of 'size' bytes from the target address 'addr' into 'buffer'.
     // this method returns the number of successfully copied bytes.
     // NOTE: implementations of this method should not modify the execution context in any way,
     //       as indicated by the const-keyword.
     virtual unsigned int	getMemory(const char *addr, size_t size, char *buffer)		const	= 0;
     // this method copies a block of 'size' bytes from 'buffer' to the target address 'addr'.
     // this method returns the number of successfully copied bytes.
     virtual unsigned int	setMemory(char *addr, size_t size, char *buffer)			= 0;
};

#endif // EXECUTIONCONTEXT_H
