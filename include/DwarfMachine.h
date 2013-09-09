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
#include "ExecutionContext.h"
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
	TE_Address,
	TE_Register
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
		signed long long	signedNumber;
		unsigned long long	unsignedNumber;
		void*			address;
		enum eRegister		registerNumber;
		char			_placeholder[16];
	};
};

class DwarfMachine
{
private:
	const class ExecutionContext	&context;
	const DwarfScript		&script;
	const struct FunctionEntry	*function;
	enum eMachineState		state;
	unsigned long long		instruction;
	std::stack<StackValue>		stack;

	DwarfMachine(const class ExecutionContext&, const DwarfScript&, const struct FunctionEntry*);
	~DwarfMachine();

	bool isInState(enum eMachineState);
	void gotoState(enum eMachineState);

	void 		push(StackValue);
	void		push(signed long long);
	void		push(unsigned long long);
	void		push(void*);
	void		push(enum eRegister);
	bool		hasResult()		const;
	StackValue	pop();

	void step();
	void op(unsigned char, unsigned long long, unsigned long long);

	void op_default(unsigned char);
	void op_litn(unsigned long long, unsigned long long, unsigned long long);
	void op_addr(unsigned long long, unsigned long long);
	void op_regn(unsigned long long, unsigned long long, unsigned long long);
	void op_bregn(unsigned long long, unsigned long long, unsigned long long);
	void op_fb_reg(unsigned long long, unsigned long long);

	void op_dup();
	void op_drop();
	void op_deref();

	void op_plus();
public:
	static unsigned int evaluateLocation(const class ExecutionContext&, const DwarfScriptList&, void**, enum eRegister*, const struct FunctionEntry*);
	static unsigned int evaluateLocation(const class ExecutionContext&, const DwarfScriptList&, void**, enum eRegister*);
	static unsigned int evaluate(const class ExecutionContext&, const DwarfScriptList&, const struct FunctionEntry*, StackValue*);
	static unsigned int evaluate(const class ExecutionContext&, const DwarfScript&, const struct FunctionEntry*, StackValue*);
};

#endif // DWARMACHINE_H
