
/*****
 *
 * PinExecutionContext.h
 *
 * Author: Reinier M. Hartog
 * Date:   19 November 2012
 *
 * This is the implementation of an ExecutionContext specific for pin.
 *
 *****/

#ifndef PINEXECUTIONCONTEXT_H
#define PINEXECUTIONCONTEXT_H

#include "pin.H"
#include "ExecutionContext.h"

class PinExecutionContext : public ExecutionContext
{
private:
     CONTEXT* 			pin_context;
public:
     				PinExecutionContext(CONTEXT*);
     virtual 			~PinExecutionContext();

     static unsigned int	mapRegisterToPin(enum eRegister reg, REG *pin_reg);

     virtual unsigned int	getRegisterValue(enum eRegister reg, unsigned long *value) 	const;
     virtual unsigned int	setRegisterValue(enum eRegister reg, unsigned long value);
     virtual unsigned int	getInstructionPointer(void **value)				const;
     virtual unsigned int	setInstructionPointer(void *value);
     virtual unsigned int	getMemory(const char *addr, size_t size, char *buffer)		const;
     virtual unsigned int	setMemory(char *addr, size_t size, char *buffer);
};

#endif // PINEXECUTIONCONTEXT_H
