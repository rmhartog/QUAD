/*****
 *
 * PinExecutionContext.cpp
 *
 * Author: Reinier M. Hartog
 * Date:   19 November 2012
 *
 * This is the implementation of an ExecutionContext specific for pin.
 *
 *****/

#include "PinExecutionContext.h"

PinExecutionContext::PinExecutionContext(CONTEXT* pc) : pin_context(pc) {
}

PinExecutionContext::~PinExecutionContext() {
}

unsigned int PinExecutionContext::mapRegisterToPin(enum eRegister reg, REG *pin_reg) {
	switch (reg) {
	case EREG_EAX: *pin_reg = REG_EAX; return 0;
	case EREG_ECX: *pin_reg = REG_ECX; return 0;
	case EREG_EDX: *pin_reg = REG_EDX; return 0;
	case EREG_EBX: *pin_reg = REG_EBX; return 0;
	case EREG_STACK_POINTER:
		*pin_reg = REG_STACK_PTR;
		return 0;
	case EREG_BASE_POINTER:
		*pin_reg = REG_EBP;
		return 0;
	case EREG_INST_POINTER:
		*pin_reg = REG_INST_PTR;
		return 0;
	default:
		return 1;
	}
}

unsigned int PinExecutionContext::getRegisterValue(enum eRegister reg, unsigned long *value) const {
	REG 		pin_register;
	ADDRINT 	content;
	unsigned int 	result;

	result = mapRegisterToPin(reg, &pin_register);
	if (result == 0) {
		content = PIN_GetContextReg(pin_context, pin_register);
		*value = (unsigned long) content;
		return 0;
	} else {
		return result;
	}
}

unsigned int PinExecutionContext::setRegisterValue(enum eRegister reg, unsigned long value) {
	REG 		pin_register;
	unsigned int 	result;

	result = mapRegisterToPin(reg, &pin_register);
	if (result == 0) {
		PIN_SetContextReg(pin_context, pin_register, (ADDRINT) value);
		return 0;
	} else {
		return result;
	}
}

unsigned int PinExecutionContext::getInstructionPointer(void **value) const {
	return getRegisterValue(EREG_INST_POINTER, (unsigned long *) value);
}

unsigned int PinExecutionContext::setInstructionPointer(void *value) {
	return setRegisterValue(EREG_INST_POINTER, (unsigned long) value);
}

unsigned int PinExecutionContext::getMemory(const char *addr, size_t size, char *buffer) const {
	return (unsigned int) PIN_SafeCopy(buffer, addr, size);
}

unsigned int PinExecutionContext::setMemory(char *addr, size_t size, char *buffer) {
	return (unsigned int) PIN_SafeCopy(addr, buffer, size);
}
