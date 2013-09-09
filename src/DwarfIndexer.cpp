/*****
 *
 * DwarfIndexer.cpp
 *
 * Author: Reinier M. Hartog
 *
 *****/

#include "DwarfIndexer.h"

#include <dwarf.h>
#include <iostream>
#include <sstream>
using namespace std;

Dwarf_Half DwarfIndexer::getDwarfTag(Dwarf_Die die, Dwarf_Error dwarf_error) {
	int		res = 0;
	Dwarf_Half	die_tag;

	if ((res = dwarf_tag(die, &die_tag, &dwarf_error)) != DW_DLV_OK) {
		return DW_TAG_hi_user;
	} else {
		return die_tag;
	}
}

string DwarfIndexer::getDwarfName(Dwarf_Die die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	int	res = 0;
	char	*dw_name;
	string	name;

	res = dwarf_diename(die, &dw_name, &dwarf_error);
	if (res == DW_DLV_OK) {
		name = dw_name;
		dwarf_dealloc(dwarf_handle, dw_name, DW_DLA_STRING);
	} else if (res == DW_DLV_NO_ENTRY) {
		name = "<unnamed>";
	} else {
		name = "";
	}	
	return name;
}

Dwarf_Unsigned DwarfIndexer::getDwarfUnsigned(Dwarf_Die die, Dwarf_Half attr, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	int		res = 0;
	Dwarf_Attribute	dw_attr;	
	Dwarf_Unsigned	udata;

	if ((res = dwarf_attr(die, attr, &dw_attr, &dwarf_error)) == DW_DLV_OK) {
		if ((res = dwarf_formudata(dw_attr, &udata, &dwarf_error)) == DW_DLV_OK) {
			return udata;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

Dwarf_Off DwarfIndexer::getDwarfOffset(Dwarf_Die die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	int		res = 0;
	Dwarf_Off	die_off;	

	if ((res = dwarf_dieoffset(die, &die_off, &dwarf_error)) == DW_DLV_OK) {
		return die_off;
	} else {
		return -1;
	}
}

Dwarf_Off DwarfIndexer::getDwarfRefOffset(Dwarf_Die die, Dwarf_Half attr, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	int		res = 0;
	Dwarf_Attribute	dw_attr;	
	Dwarf_Off	off;

	if ((res = dwarf_attr(die, attr, &dw_attr, &dwarf_error)) == DW_DLV_OK) {
		if ((res = dwarf_global_formref(dw_attr, &off, &dwarf_error)) == DW_DLV_OK) {
			return off;
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

unsigned int DwarfIndexer::getDwarfPC(Dwarf_Die die, Dwarf_Addr* p_lopc, Dwarf_Addr* p_hipc, Dwarf_Error dwarf_error) {
	Dwarf_Addr	lowpc;
	Dwarf_Addr	hipc;

	if (dwarf_lowpc(die, &lowpc, &dwarf_error) != DW_DLV_OK) {
		return 1;
	}
	if (dwarf_highpc(die, &hipc, &dwarf_error) != DW_DLV_OK) {
		return 2;
	}

	if (p_lopc != 0) {
		*p_lopc = lowpc;
	}
	if (p_hipc != 0) {
		*p_hipc = hipc;
	}

	return 0;
}

unsigned int DwarfIndexer::getDwarfScriptList(Dwarf_Die die, Dwarf_Half attr, DwarfScriptList &scriptlist, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error, unsigned long long offset = 0) {
	int		res = 0;
	Dwarf_Attribute	dw_attr;
	Dwarf_Signed	lcnt;
	Dwarf_Locdesc	**llbuf;
	DwarfScriptList	sl;
	
	if ((res = dwarf_attr(die, attr, &dw_attr, &dwarf_error)) == DW_DLV_OK) {
		if ((res = dwarf_loclist_n(dw_attr, &llbuf, &lcnt, &dwarf_error)) == DW_DLV_OK) {
			for (int i = 0; i < lcnt; i++) {
				DwarfLocationScript dls;

				if (llbuf[i]->ld_lopc == 0 && llbuf[i]->ld_hipc == (Dwarf_Unsigned) (-1LL)) {
					dls.lowpc = 0;
					dls.hipc = (Dwarf_Unsigned) (-1LL);
				} else {
					dls.lowpc = llbuf[i]->ld_lopc + offset;
					dls.hipc = llbuf[i]->ld_hipc + offset;
				}
				for (int j = 0; j < llbuf[i]->ld_cents; j++) {
					DwarfOperation op;

					op.opcode = llbuf[i]->ld_s[j].lr_atom;
					op.operand1 = llbuf[i]->ld_s[j].lr_number;
					op.operand2 = llbuf[i]->ld_s[j].lr_number2;
					op.offset = llbuf[i]->ld_s[j].lr_offset;

					dls.script.push_back(op);
				}

				sl.push_back(dls);

				dwarf_dealloc(dwarf_handle, llbuf[i]->ld_s, DW_DLA_LOC_BLOCK);
				dwarf_dealloc(dwarf_handle, llbuf[i], DW_DLA_LOCDESC);
			}

			dwarf_dealloc(dwarf_handle, llbuf, DW_DLA_LIST);
			scriptlist = sl;
			return 0;
		} else {
			return 2;
		}
	} else {
		return 1;
	}
}

unsigned int DwarfIndexer::getChildren(Dwarf_Die parent_die, list<Dwarf_Die>& children, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	int 		res = 0;
	Dwarf_Die	kid_die;	
	list<Dwarf_Die> local;

	if ((res = dwarf_child(parent_die, &kid_die, &dwarf_error)) == DW_DLV_OK) {
		do
		{
			local.push_back(kid_die);
		}
		while ((res = dwarf_siblingof(dwarf_handle, kid_die, &kid_die, &dwarf_error)) == DW_DLV_OK);
	}
	if (res != DW_DLV_NO_ENTRY) {
		return 1;
	}

	children = local;

	return 0;
}

void DwarfIndexer::fixIndirectType(struct TypeEntry &te) {
	if (te.basetype_off != 0 && te.basetype_type == 0) {
		te.basetype_type = new TypeEntry;
		if (getType(te.basetype_off, te.basetype_type) != 0) {
			delete te.basetype_type;
			te.basetype_type = 0;
		} else {
			fixIndirectType(*te.basetype_type);
			if (te.type == TT_Typedef) {
				te.size = te.basetype_type->size;
			}
			else if (te.type == TT_Array) {
				te.name = te.basetype_type->name + "[]";
			}
			else if (te.type == TT_BoundedArray) {
				stringstream name_ss;
				name_ss << te.basetype_type->name << "[" << (te.upper_bound + 1) << "]";
				name_ss >> te.name;
				te.size = te.basetype_type->size * (te.upper_bound + 1);
			}
			else if (te.type == TT_Pointer) {
				te.name = te.basetype_type->name + "*";
			}
			else if (te.type == TT_Const) {
				te.name = te.basetype_type->name + " const";
				te.size = te.basetype_type->size;
			}
		}
	}
}

void DwarfIndexer::fixIndirectTypes() {
	map<Dwarf_Off, TypeEntry*>::iterator ti;
	for (ti = types.begin(); ti != types.end(); ti++) {
		TypeEntry *te = ti->second;

		fixIndirectType(*te);
	}
}

void DwarfIndexer::fixIndirectVariableTypes() {
	std::map<Dwarf_Off, VarEntry*>::iterator git;
	std::map<Dwarf_Off, struct FunctionEntry*>::iterator fit;
	
	for (git = global_variables.begin(); git != global_variables.end(); git++) {
		fixIndirectVariableType(*git->second);
	}

	for (fit = functions.begin(); fit != functions.end(); fit++) {
		fixIndirectLocalVariableTypes(*fit->second);
	}
}

void DwarfIndexer::fixIndirectLocalVariableTypes(const struct FunctionEntry &fe) {
	std::map<Dwarf_Off, VarEntry*>::const_iterator vit;

	for (vit = fe.variables.begin(); vit != fe.variables.end(); vit++) {
		fixIndirectVariableType(*vit->second);
	}	
}

void DwarfIndexer::fixIndirectVariableType(struct VarEntry &ve) {
	if (ve.type_off != 0) {
		getType(ve.type_off, &ve.type);
	}
}

DwarfIndexer::DwarfIndexer() : CU_lopc(0), CU_hipc(0), current_basetype(0) {
}

DwarfIndexer::~DwarfIndexer() {
}

unsigned int DwarfIndexer::accept(Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	int		res = 0;
	Dwarf_Die 	cu_die = NULL;
	Dwarf_Unsigned 	header_length, abbrev_offset, next_cu_header;
	Dwarf_Half	version_stamp, address_size;
	DwarfIndex	global_index = { types, global_variables, functions };


	cerr << "Indexing dwarf..." << endl;

	current_function_id = "";
	while ((res = dwarf_next_cu_header(dwarf_handle, &header_length,
					&version_stamp, &abbrev_offset,
					&address_size, &next_cu_header, &dwarf_error)) == DW_DLV_OK) {
		if ((res = dwarf_siblingof(dwarf_handle, 0, &cu_die, &dwarf_error)) == DW_DLV_OK) {
			if (getDwarfTag(cu_die, dwarf_error) == DW_TAG_compile_unit) {
				if (visitCU(global_index, cu_die, dwarf_handle, dwarf_error) != 0) {
					return 2;
				}
			}
		} else {
			return 3;
		}
	}

	fixIndirectTypes();
	fixIndirectVariableTypes();

	map<Dwarf_Off, TypeEntry*>::iterator ti;
	cerr << "Types: " << endl;
	for (ti = types.begin(); ti != types.end(); ti++) {
		cerr << " - " << ti->second->name << endl;	
	}

	map<Dwarf_Off, VarEntry*>::iterator vi;
	cerr << "Globals: " << endl;
	for (vi = global_variables.begin(); vi != global_variables.end(); vi++) {
		TypeEntry &type = vi->second->type;

		cerr << " - [";
		cerr << type.name;
		cerr << "] " << vi->second->name << endl;

		DwarfScriptList::iterator slit;
		for (slit = vi->second->location.begin(); slit != vi->second->location.end(); slit++) {
			cerr << "   <" << (void*) slit->lowpc << " - " << (void*) slit->hipc << ">:" << endl;
			
			DwarfScript::iterator si;
			for (si = slit->script.begin(); si != slit->script.end(); si++) {
				cerr << "      #" << (void*) si->offset << " - " << (void*) si->opcode << " (" << (void*) si->operand1 << ", " << (void*) si->operand2 << ")" << endl;
			}
		}
	}

	map<Dwarf_Off, FunctionEntry*>::iterator fi;
	cerr << "Functions: " << endl;
	for (fi = functions.begin(); fi != functions.end(); fi++) {
		ti = types.find(fi->second->return_type);

		cerr << " - [";
		if (fi->second->return_type == -1) {
			cerr << "void";
		} else if (ti != types.end()) {
			cerr << ti->second->name;
		} else {
			cerr << fi->second->return_type;
		}
		cerr << "] " << fi->second->name << " (";
		// print parameters
		cerr << ")" << endl;

		cerr << "   Frame Base: " << endl;
		DwarfScriptList::iterator slit;
		for (slit = fi->second->frame_base.begin(); slit != fi->second->frame_base.end(); slit++) {
			cerr << "   <" << (void*) slit->lowpc << " - " << (void*) slit->hipc << ">:" << endl;
			
			DwarfScript::iterator si;
			for (si = slit->script.begin(); si != slit->script.end(); si++) {
				cerr << "      #" << (void*) si->offset << " - " << (void*) si->opcode << " (" << si->operand1 << ", " << si->operand2 << ")" << endl;
			}
		}

		if (!fi->second->variables.empty()) {
			cerr << "   Local Variables: " << endl;
			for (vi = fi->second->variables.begin(); vi != fi->second->variables.end(); vi++) {
				TypeEntry &type = vi->second->type;
		
				cerr << "    - [";
				cerr << type.name;
				cerr << "] " << vi->second->name << endl;
			
				DwarfScriptList::iterator slit;
				for (slit = vi->second->location.begin(); slit != vi->second->location.end(); slit++) {
					cerr << "      <" << (void*) slit->lowpc << " - " << (void*) slit->hipc << ">:" << endl;
					
					DwarfScript::iterator si;
					for (si = slit->script.begin(); si != slit->script.end(); si++) {
						cerr << "         #" << (void*) si->offset << " - " << (void*) si->opcode << " (" << si->operand1 << ", " << si->operand2 << ")" << endl;
					}
				}
			}
		}
	}

	return 0;
}

unsigned int DwarfIndexer::accept(DwarfIndex &index, Dwarf_Die die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	Dwarf_Half tag;
	switch(tag = getDwarfTag(die, dwarf_error)) {
	case DW_TAG_compile_unit:
		return visitCU(index, die, dwarf_handle, dwarf_error);	
	case DW_TAG_base_type:
		return visitBaseType(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_typedef:
		return visitTypedefType(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_array_type:
		return visitArrayType(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_subrange_type:
		return visitSubrangeType(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_pointer_type:
		return visitPointerType(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_const_type:
		return visitConstType(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_variable:
		return visitVariable(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_formal_parameter:
		// might need a seperate function
		return visitVariable(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_subprogram:
		return visitSubProgram(index, die, dwarf_handle, dwarf_error);
	default:
		cerr << "Unsupported tag number " << tag << ". Has to be added." << endl;
		return 255;
	}
}

unsigned int DwarfIndexer::acceptChildren(DwarfIndex &index, Dwarf_Die die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	unsigned int	res;
	list<Dwarf_Die> children;
	list<Dwarf_Die>::iterator it;

	if (getChildren(die, children, dwarf_handle, dwarf_error) != 0) {
		return 1; 	
	}

	for (it = children.begin(); it != children.end(); it++) {
		res = accept(index, *it, dwarf_handle, dwarf_error);
		if (res != 0 && res != 255) {
			return 2;
		}
	}
	return 0;
}

unsigned int DwarfIndexer::visitCU(DwarfIndex &index, Dwarf_Die cu_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	Dwarf_Addr		lopc, hipc;
	
	if (getDwarfPC(cu_die, &lopc, &hipc, dwarf_error) != 0) {
		cerr << "Warning: could not read lowpc or hipc of CU" << endl;
		lopc = hipc = 0;
	}
	CU_lopc = lopc;
	CU_hipc = hipc;

	return acceptChildren(index, cu_die, dwarf_handle, dwarf_error);
}

unsigned int DwarfIndexer::visitBaseType(DwarfIndex &index, Dwarf_Die type_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	string 		name;
	Dwarf_Off	offset;
	Dwarf_Unsigned	size;
	TypeEntry	*te;

	name = getDwarfName(type_die, dwarf_handle, dwarf_error);
	offset = getDwarfOffset(type_die, dwarf_handle, dwarf_error);
	size = getDwarfUnsigned(type_die, DW_AT_byte_size, dwarf_handle, dwarf_error);

	te = new TypeEntry;
	te->type = TT_Base;
	te->name = name;
	te->size = size;

	index.types[offset] = te;

	cerr << name.c_str() << endl;

	return 0;
}

unsigned int DwarfIndexer::visitTypedefType(DwarfIndex &index, Dwarf_Die type_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	string 		name;
	Dwarf_Off	offset;
	Dwarf_Off	realtype;
	TypeEntry	*te;

	name = getDwarfName(type_die, dwarf_handle, dwarf_error);
	offset = getDwarfOffset(type_die, dwarf_handle, dwarf_error);
	realtype = getDwarfRefOffset(type_die, DW_AT_type, dwarf_handle, dwarf_error);

	te = new TypeEntry;
	te->type = TT_Typedef;
	te->name = name;
	te->basetype_off = realtype;
	te->basetype_type = 0;
	te->size = 0;

	index.types[offset] = te;

	return 0;
}

unsigned int DwarfIndexer::visitArrayType(DwarfIndex &index, Dwarf_Die type_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	string 		name;
	Dwarf_Off	offset;
	Dwarf_Off	element;
	TypeEntry	*te;

	offset = getDwarfOffset(type_die, dwarf_handle, dwarf_error);
	element = getDwarfRefOffset(type_die, DW_AT_type, dwarf_handle, dwarf_error);

	te = new TypeEntry;
	te->type = TT_Array;
	te->name = "";
	te->basetype_off = element;
	te->basetype_type = 0;
	te->size = 0;

	index.types[offset] = te;

	TypeEntry* prev_basetype = current_basetype;
	current_basetype = te;

	acceptChildren(index, type_die, dwarf_handle, dwarf_error);

	current_basetype = prev_basetype;

	return 0;
}

unsigned int DwarfIndexer::visitSubrangeType(DwarfIndex &index, Dwarf_Die type_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	Dwarf_Unsigned	upper_bound;

	upper_bound = getDwarfUnsigned(type_die, DW_AT_upper_bound, dwarf_handle, dwarf_error);

	if (current_basetype != 0) {
		if (current_basetype->type == TT_Array) {
			current_basetype->type = TT_BoundedArray;
			current_basetype->upper_bound = upper_bound;
		}
	}	

	return 0;
}

unsigned int DwarfIndexer::visitPointerType(DwarfIndex &index, Dwarf_Die type_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	string 		name;
	Dwarf_Off	offset;
	Dwarf_Off	pointee;
	Dwarf_Unsigned	size;
	TypeEntry	*te;

	offset = getDwarfOffset(type_die, dwarf_handle, dwarf_error);
	pointee = getDwarfRefOffset(type_die, DW_AT_type, dwarf_handle, dwarf_error);
	size = getDwarfUnsigned(type_die, DW_AT_byte_size, dwarf_handle, dwarf_error);

	te = new TypeEntry;
	te->type = TT_Pointer;
	te->name = "";
	te->basetype_off = pointee;
	te->basetype_type = 0;
	te->size = size;

	index.types[offset] = te;

	return 0;
}

unsigned int DwarfIndexer::visitConstType(DwarfIndex &index, Dwarf_Die type_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	string 		name;
	Dwarf_Off	offset;
	Dwarf_Off	basetype;
	Dwarf_Unsigned	size;
	TypeEntry	*te;

	offset = getDwarfOffset(type_die, dwarf_handle, dwarf_error);
	basetype = getDwarfRefOffset(type_die, DW_AT_type, dwarf_handle, dwarf_error);

	te = new TypeEntry;
	te->type = TT_Const;
	te->name = "";
	te->basetype_off = basetype;
	te->basetype_type = 0;

	index.types[offset] = te;

	return 0;
}

unsigned int DwarfIndexer::visitVariable(DwarfIndex &index, Dwarf_Die variable_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	string 		name;
	Dwarf_Off	offset;
	Dwarf_Off	type;
	VarEntry	*ve;

	name = getDwarfName(variable_die, dwarf_handle, dwarf_error);
	offset = getDwarfOffset(variable_die, dwarf_handle, dwarf_error);
	type = getDwarfRefOffset(variable_die, DW_AT_type, dwarf_handle, dwarf_error);

	ve = new VarEntry;
	ve->name = name;
	ve->function_id = current_function_id;
	ve->type_off = type;
	if (getDwarfScriptList(variable_die, DW_AT_location, ve->location, dwarf_handle, dwarf_error, CU_lopc) != 0) {
		cerr << "Warning: variable " << ve->name << " has no location." << endl;
	}

	index.variables[offset] = ve;

	cerr << name.c_str() << endl;

	return 0;
}

unsigned int DwarfIndexer::visitSubProgram(DwarfIndex &index, Dwarf_Die sp_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	string 			name;
	Dwarf_Off		offset;
	Dwarf_Off		rettype;
	Dwarf_Addr		lopc, hipc;
	FunctionEntry		*fe;

	name = getDwarfName(sp_die, dwarf_handle, dwarf_error);
	offset = getDwarfOffset(sp_die, dwarf_handle, dwarf_error);
	rettype = getDwarfRefOffset(sp_die, DW_AT_type, dwarf_handle, dwarf_error);
	cerr << name.c_str() << endl;

	if (getDwarfPC(sp_die, &lopc, &hipc, dwarf_error) != 0) {
		cerr << "Warning: could not read lowpc or hipc" << endl;
		lopc = hipc = 0;
	}

	fe = new FunctionEntry;
	fe->name = name;
	fe->return_type = rettype;
	fe->lopc = lopc;
	fe->hipc = hipc;
	stringstream unique_name_ss;
	unique_name_ss << fe->name << "_" << fe->lopc;
	unique_name_ss >> fe->unique_id;

	if (getDwarfScriptList(sp_die, DW_AT_frame_base, fe->frame_base, dwarf_handle, dwarf_error, CU_lopc) != 0) {
		cerr << "Warning: function " << fe->name << " has no frame_base." << endl;
	}

	DwarfIndex local_index = { index.types, fe->variables, index.functions };

	string prev_func = current_function_id;
	current_function_id = fe->unique_id;
	if (acceptChildren(local_index, sp_die, dwarf_handle, dwarf_error) == 0) {
		index.functions[offset] = fe;
	} else {
		delete fe;
		fe = 0;
	}
	current_function_id = prev_func;

	if (fe != 0) {
		return 0;
	} else {	
		return 1;
	}
}

unsigned int DwarfIndexer::getType(Dwarf_Off off, TypeEntry* te) const {
	map<Dwarf_Off, TypeEntry*>::const_iterator tit;

	if (te == 0) {
		return 1;
	}

	tit = types.find(off);
	if (tit == types.end()) {
		return 2;
	} else {
		*te = *tit->second;
		return 0;
	}
}

list<VarEntry> DwarfIndexer::getVariables() const {
	list<VarEntry> vars;
	map<Dwarf_Off, VarEntry*>::const_iterator vit;

	for (vit = global_variables.begin(); vit != global_variables.end(); vit++) {
		vars.push_back(*vit->second);
	}

	return vars;
}

list<VarEntry> DwarfIndexer::getVariables(const FunctionEntry &fe) {
	list<VarEntry> vars;
	map<Dwarf_Off, VarEntry*>::const_iterator vit;

	for (vit = fe.variables.begin(); vit != fe.variables.end(); vit++) {
		vars.push_back(*vit->second);
	}

	return vars;
}

list<FunctionEntry> DwarfIndexer::getFunctions() const {
	list<FunctionEntry> funcs;
	map<Dwarf_Off, FunctionEntry*>::const_iterator fit;

	for (fit = functions.begin(); fit != functions.end(); fit++) {
		funcs.push_back(*fit->second);
	}

	return funcs;
}
