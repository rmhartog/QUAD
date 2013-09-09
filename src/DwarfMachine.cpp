/*****
 *
 * DwarfMachine.cpp
 *
 * Author: Reinier M. Hartog
 *
 *****/

#include "DwarfMachine.h"
#include "DwarfIndexer.h"
#include "ExecutionContext.h"

DwarfMachine::DwarfMachine(const ExecutionContext &econtext, const DwarfScript &s, const FunctionEntry *fe) : context(econtext), script(s),function(fe), state(MS_Executing), instruction(0) {
}

DwarfMachine::~DwarfMachine() {
}

#include <iostream>
using namespace std;

bool DwarfMachine::isInState(enum eMachineState s) {
	return state == s;
}

void DwarfMachine::gotoState(enum eMachineState s) {
	state = s;
}

void DwarfMachine::push(StackValue v) {
	stack.push(v);
}

void DwarfMachine::push(signed long long n) {
	StackType t;
	t.encoding = TE_Signed;
	t.size = sizeof(signed long long);
	StackValue v;
	v.type = t;
	v.signedNumber = n;

	push(v);
}

void DwarfMachine::push(unsigned long long n) {
	StackType t;
	t.encoding = TE_Unsigned;
	t.size = sizeof(unsigned long long);
	StackValue v;
	v.type = t;
	v.unsignedNumber = n;

	push(v);
}

void DwarfMachine::push(void* p) {
	StackType t;
	t.encoding = TE_Address;
	t.size = sizeof(void*);
	StackValue v;
	v.type = t;
	v.address = p;

	push(v);
}


void DwarfMachine::push(enum eRegister r) {
	StackType t;
	t.encoding = TE_Register;
	t.size = sizeof(enum eRegister);
	StackValue v;
	v.type = t;
	v.registerNumber = r;

	push(v);
}

bool DwarfMachine::hasResult() const {
	return !stack.empty();
}

StackValue DwarfMachine::pop() {
	StackValue v = stack.top();
	stack.pop();
	return v;
}

void DwarfMachine::step() {
	DwarfScript::const_iterator sit;

	instruction++;
	for (sit = script.begin(); sit != script.end(); sit++) {
		if (sit->offset == instruction) {
			op(sit->opcode, sit->operand1, sit->operand2);
			break;
		}
	}

	if (sit == script.end()) {
		gotoState(MS_Done);
	}
}

void DwarfMachine::op(unsigned char opcode, unsigned long long operand1, unsigned long long operand2) {
	switch(opcode) {
	// WARNING, this assumes that these opcodes are numbered sequentially.
	case DW_OP_lit0: case DW_OP_lit1: case DW_OP_lit2: case DW_OP_lit3: case DW_OP_lit4:
	case DW_OP_lit5: case DW_OP_lit6: case DW_OP_lit7: case DW_OP_lit8: case DW_OP_lit9:
	case DW_OP_lit10: case DW_OP_lit11: case DW_OP_lit12: case DW_OP_lit13: case DW_OP_lit14:
	case DW_OP_lit15: case DW_OP_lit16: case DW_OP_lit17: case DW_OP_lit18: case DW_OP_lit19:
	case DW_OP_lit20: case DW_OP_lit21: case DW_OP_lit22: case DW_OP_lit23: case DW_OP_lit24:
	case DW_OP_lit25: case DW_OP_lit26: case DW_OP_lit27: case DW_OP_lit28: case DW_OP_lit29:
	case DW_OP_lit30: case DW_OP_lit31: {
		op_litn(operand1, operand2, (opcode - DW_OP_lit0));
		break;
	}
	case DW_OP_addr: {
		op_addr(operand1, operand2);
		break;
	}
	// WARNING, this assumes that these opcodes are numbered sequentially.
	case DW_OP_reg0: case DW_OP_reg1: case DW_OP_reg2: case DW_OP_reg3: case DW_OP_reg4:
	case DW_OP_reg5: case DW_OP_reg6: case DW_OP_reg7: case DW_OP_reg8: case DW_OP_reg9:
	case DW_OP_reg10: case DW_OP_reg11: case DW_OP_reg12: case DW_OP_reg13: case DW_OP_reg14:
	case DW_OP_reg15: case DW_OP_reg16: case DW_OP_reg17: case DW_OP_reg18: case DW_OP_reg19:
	case DW_OP_reg20: case DW_OP_reg21: case DW_OP_reg22: case DW_OP_reg23: case DW_OP_reg24:
	case DW_OP_reg25: case DW_OP_reg26: case DW_OP_reg27: case DW_OP_reg28: case DW_OP_reg29:
	case DW_OP_reg30: case DW_OP_reg31: {
		op_regn(operand1, operand2, (opcode - DW_OP_reg0));
		break;
	}
	// WARNING, this assumes that these opcodes are numbered sequentially.
	case DW_OP_breg0: case DW_OP_breg1: case DW_OP_breg2: case DW_OP_breg3: case DW_OP_breg4:
	case DW_OP_breg5: case DW_OP_breg6: case DW_OP_breg7: case DW_OP_breg8: case DW_OP_breg9:
	case DW_OP_breg10: case DW_OP_breg11: case DW_OP_breg12: case DW_OP_breg13: case DW_OP_breg14:
	case DW_OP_breg15: case DW_OP_breg16: case DW_OP_breg17: case DW_OP_breg18: case DW_OP_breg19:
	case DW_OP_breg20: case DW_OP_breg21: case DW_OP_breg22: case DW_OP_breg23: case DW_OP_breg24:
	case DW_OP_breg25: case DW_OP_breg26: case DW_OP_breg27: case DW_OP_breg28: case DW_OP_breg29:
	case DW_OP_breg30: case DW_OP_breg31: {
		op_bregn(operand1, operand2, (opcode - DW_OP_breg0));
		break;
	}
	case DW_OP_fbreg: {
		op_fb_reg(operand1, operand2);
		break;
	}
	case DW_OP_dup: {
		op_dup();
		break;
	}
	case DW_OP_drop: {
		op_drop();
		break;
	}
	case DW_OP_deref: {
		op_deref();
		break;
	}
	case DW_OP_plus: {
		op_plus();
		break;
	}
	default:
		op_default(opcode);
	break;
	}
}

unsigned int DwarfMachine::evaluateLocation(const ExecutionContext &context, const DwarfScriptList &sl, void **addr, enum eRegister *reg, const struct FunctionEntry *fe) {
	StackValue result;

	if (evaluate(context, sl, fe, &result) == 0) {
		if (result.type.encoding == TE_Address && result.type.size <= sizeof(void*)) {
			if (addr != 0) {
				*addr = result.address;
			}
			return 0;
		} else if (result.type.encoding == TE_Register && result.type.size == sizeof(enum eRegister)) {
			if (reg != 0) {
				*reg = result.registerNumber;
			}
			return 1;
		} else {
			return 4;
		}
	} else {
		return 2;
	}
}

unsigned int DwarfMachine::evaluateLocation(const ExecutionContext &context, const DwarfScriptList &sl, void **addr, enum eRegister *reg) {
	return evaluateLocation(context, sl, addr, reg, 0);
}

unsigned int DwarfMachine::evaluate(const ExecutionContext &context, const DwarfScriptList &sl, const FunctionEntry *fe, StackValue *result) {
	DwarfScriptList::const_iterator slit;
	void* ip;

	if (sl.empty()) {
		return 4;
	}

	if (context.getInstructionPointer(&ip) != 0) {
		cerr << "Failed to obtain instruction pointer." << endl;
		return 1;
	}

	for (slit = sl.begin(); slit != sl.end(); slit++) {
		if ((slit->lowpc == 0 && slit->hipc == 0) ||
		    ((void*) slit->lowpc <= ip && (void*) slit->hipc > ip)) {
			if (evaluate(context, slit->script, fe, result) == 0) {
				return 0;
			} else {
				return 3;
			}
		}
	} 
	cerr << "No suitable location list entry found. (" << ip << ")" << endl;
	return 2;
}

unsigned int DwarfMachine::evaluate(const ExecutionContext &context, const DwarfScript &script, const FunctionEntry *fe, StackValue *result) {
	DwarfMachine 	machine(context, script, fe);

	while (machine.isInState(MS_Executing)) {
		machine.step();
	}

	if (!machine.isInState(MS_Done)) {
		return 1;
	}
	if (!machine.hasResult()) {
		return 2;
	}
	*result = machine.pop();
	return 0;
}

void DwarfMachine::op_default(unsigned char opcode) {
	cerr << "Unhandled opcode " << (unsigned int) opcode << endl;
	gotoState(MS_Failed);
}

void DwarfMachine::op_litn(unsigned long long operand1, unsigned long long operand2, unsigned long long n) {
	push(n);
}

void DwarfMachine::op_addr(unsigned long long operand1, unsigned long long operand2) {
	push((void*) operand1);
}

void DwarfMachine::op_regn(unsigned long long operand1, unsigned long long operand2, unsigned long long n) {
	push((enum eRegister) n);
}

void DwarfMachine::op_bregn(unsigned long long operand1, unsigned long long operand2, unsigned long long n) {
	unsigned long value;
	if (context.getRegisterValue((enum eRegister) n, &value) == 0) {
		push((void*) (value + operand1));
	} else {
		cerr << "Failed to get register value." << endl;
		gotoState(MS_Failed);
	}
}

void DwarfMachine::op_fb_reg(unsigned long long operand1, unsigned long long operand2) {
	unsigned long frame_base;
	if (function != 0) {
		evaluateLocation(context, function->frame_base, (void **) &frame_base, 0, function);
		push((void*) (frame_base + operand1));	
	} else {
		cerr << "fbreg without a function." << endl;
		gotoState(MS_Failed);
	}	
}

void DwarfMachine::op_dup() {
	StackValue dup = pop();
	push(dup);
	push(dup);
}

void DwarfMachine::op_drop() {
	pop();
}

void DwarfMachine::op_deref() {
	StackValue addr = pop();

	if (addr.type.encoding == TE_Address) {
		void *address = addr.address;
		void *value = 0;
		if (context.getMemory((char*) address, sizeof(void*), (char*) &value) == sizeof(void*)) {
			push(value);
		} else {
			cerr << "Failed to dereference address" << endl;
			gotoState(MS_Failed);
		}
	} else {
		cerr << "Failed to convert value to address" << endl;
		gotoState(MS_Failed);
	}	
}

void DwarfMachine::op_plus() {
	StackValue val1 = pop();
	StackValue val2 = pop();

	if (val1.type.encoding == val2.type.encoding) {
		if (val1.type.encoding == TE_Unsigned) {
			push(val1.unsignedNumber + val2.unsignedNumber);
		} else if (val1.type.encoding = TE_Signed) {
			push(val1.signedNumber + val2.signedNumber);
		} else {
			cerr << "Cannot add values of this encoding." << endl;
			gotoState(MS_Failed);
		}
	} else {
		cerr << "Cannot add values of different types." << endl;
		gotoState(MS_Failed);
	}
}
