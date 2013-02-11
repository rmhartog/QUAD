/*****
 *
 * DwarfMachine.cpp
 *
 * Author: Reinier M. Hartog
 *
 *****/

#include "DwarfMachine.h"
#include "ExecutionContext.h"

DwarfMachine::DwarfMachine(const ExecutionContext &econtext, const DwarfScript &s) : context(econtext), script(s), state(MS_Executing), instruction(0) {
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

void DwarfMachine::push(void* p) {
	StackType t;
	t.encoding = TE_Address;
	t.size = sizeof(void*);
	StackValue v;
	v.type = t;
	v.address = p;

	push(v);
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
	case DW_OP_addr: op_addr(operand1, operand2); break;
	default:
		op_default(opcode);
	break;
	}
}

unsigned int DwarfMachine::evaluateLocation(const ExecutionContext &context, const DwarfScriptList &sl, void **addr) {
	StackValue result;

	if (addr == 0) {
		return 1;
	}

	if (evaluate(context, sl, &result) == 0) {
		if (result.type.encoding == TE_Address && result.type.size <= sizeof(void*)) {
			*addr = result.address;
			return 0;
		} else {
			return 3;
		}
	} else {
		return 2;
	}
}

unsigned int DwarfMachine::evaluate(const ExecutionContext &context, const DwarfScriptList &sl, StackValue *result) {
	DwarfScriptList::const_iterator slit;
	void* ip;

	if (context.getInstructionPointer(&ip) != 0) {
		cerr << "Failed to obtain instruction pointer." << endl;
		return 1;
	}

	for (slit = sl.begin(); slit != sl.end(); slit++) {
		if ((slit->lowpc == 0 && slit->hipc == 0) ||
		    ((void*) slit->lowpc <= ip && (void*) slit->hipc > ip)) {
			if (evaluate(context, slit->script, result) == 0) {
				return 0;
			} else {
				return 3;
			}
		}
	} 
	cerr << "No suitable location list entry found." << endl;
	return 2;
}

unsigned int DwarfMachine::evaluate(const ExecutionContext &context, const DwarfScript &script, StackValue *result) {
	DwarfMachine 	machine(context, script);

	while (machine.isInState(MS_Executing)) {
		machine.step();
	}

	if (!machine.isInState(MS_Done)) {
		return 1;
	}
	*result = machine.pop();
	return 0;
}

void DwarfMachine::op_default(unsigned char opcode) {
	cerr << "Unhandled opcode " << (unsigned int) opcode << endl;
	gotoState(MS_Failed);
}

void DwarfMachine::op_addr(unsigned long long operand1, unsigned long long operand2) {
	push((void*) operand1);
}
