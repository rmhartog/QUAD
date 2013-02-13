/*****
 *
 * DwarfMachine.h
 *
 * Author: Reinier M. Hartog
 *
 *****/

#ifndef DWARFMACHINE_H
#define DWARFMACHINE_H

#include "dwarf.h"
#include <cstddef>
#include <list>
#include <stack>

struct DwarfOperation {
	unsigned char		opcode;
	unsigned long long 	operand1;
	unsigned long long 	operand2;
	unsigned long long	offset;
};

typedef std::list<DwarfOperation>		DwarfScript;

struct DwarfLocationScript {
	unsigned long long 	lowpc;
	unsigned long long 	hipc;
	DwarfScript		script;
};

typedef std::list<DwarfLocationScript>		DwarfScriptList;

enum eMachineState
{
	MS_Executing,
	MS_Failed,
	MS_Done
};

enum eTypeEncoding {
	TE_Signed,
	TE_Unsigned,
	TE_Address
};

struct StackType
{
	enum eTypeEncoding	encoding;
	size_t			size;
};

struct StackValue
{
	StackType		type;
	union
	{
		void*		address;
		char		_placeholder[16];
	};
};

class DwarfMachine
{
private:
	const class ExecutionContext	&context;
	const DwarfScript		&script;
	enum eMachineState		state;
	unsigned long long		instruction;
	std::stack<StackValue>		stack;

	DwarfMachine(const class ExecutionContext&, const DwarfScript&);
	~DwarfMachine();

	bool isInState(enum eMachineState);
	void gotoState(enum eMachineState);

	void 		push(StackValue);
	void		push(void*);
	StackValue	pop();

	void step();
	void op(unsigned char, unsigned long long, unsigned long long);

	void op_default(unsigned char);
	void op_addr(unsigned long long, unsigned long long);
	void op_breg4(unsigned long long, unsigned long long);
	void op_fb_reg(unsigned long long, unsigned long long);
public:
	static unsigned int evaluateLocation(const class ExecutionContext&, const DwarfScriptList&, void**);
	static unsigned int evaluate(const class ExecutionContext&, const DwarfScriptList&, StackValue*);
	static unsigned int evaluate(const class ExecutionContext&, const DwarfScript&, StackValue*);
};

#endif // DWARMACHINE_H
