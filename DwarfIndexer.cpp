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

unsigned int DwarfIndexer::getDwarfScriptList(Dwarf_Die die, Dwarf_Half attr, DwarfScriptList &scriptlist, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	int		res = 0;
	Dwarf_Attribute	dw_attr;
	Dwarf_Signed	lcnt;
	Dwarf_Locdesc	*llbuf;
	DwarfScriptList	sl;
	
	if ((res = dwarf_attr(die, attr, &dw_attr, &dwarf_error)) == DW_DLV_OK) {
		if ((res = dwarf_loclist(dw_attr, &llbuf, &lcnt, &dwarf_error)) == DW_DLV_OK) {
			for (int i = 0; i < lcnt; i++) {
				DwarfLocationScript dls;

				dls.lowpc = llbuf[i].ld_lopc;
				dls.hipc = llbuf[i].ld_hipc;
				for (int j = 0; j < llbuf[i].ld_cents; j++) {
					DwarfOperation op;

					op.opcode = llbuf[i].ld_s[j].lr_atom;
					op.operand1 = llbuf[i].ld_s[j].lr_number;
					op.operand2 = llbuf[i].ld_s[j].lr_number2;
					op.offset = llbuf[i].ld_s[j].lr_offset;

					dls.script.push_back(op);
				}

				sl.push_back(dls);

				dwarf_dealloc(dwarf_handle, llbuf[i].ld_s, DW_DLA_LOC_BLOCK);
			}

			dwarf_dealloc(dwarf_handle, llbuf, DW_DLA_LOCDESC);

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

DwarfIndexer::DwarfIndexer() {
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

	map<Dwarf_Off, TypeEntry*>::iterator ti;
	cerr << "Types: " << endl;
	for (ti = types.begin(); ti != types.end(); ti++) {
		cerr << " - " << ti->second->name << endl;	
	}

	map<Dwarf_Off, VarEntry*>::iterator vi;
	cerr << "Globals: " << endl;
	for (vi = global_variables.begin(); vi != global_variables.end(); vi++) {
		ti = types.find(vi->second->type);

		cerr << " - [";
		if (ti != types.end()) {
			cerr << ti->second->name;
		} else {
			cerr << vi->second->type;
		}
		cerr << "] " << vi->second->name << endl;

		DwarfScriptList::iterator slit;
		for (slit = vi->second->location.begin(); slit != vi->second->location.end(); slit++) {
			cerr << "   <" << (void*) slit->lowpc << " - " << (void*) slit->hipc << ">:" << endl;
			
			DwarfScript::iterator si;
			for (si = slit->script.begin(); si != slit->script.end(); si++) {
				cerr << "      #" << (void*) si->offset << " - " << (void*) si->opcode << " (" << si->operand1 << ", " << si->operand2 << ")" << endl;
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

		if (!fi->second->variables.empty()) {
			cerr << "   Local Variables: " << endl;
			for (vi = fi->second->variables.begin(); vi != fi->second->variables.end(); vi++) {
				ti = types.find(vi->second->type);
		
				cerr << "    - [";
				if (ti != types.end()) {
					cerr << ti->second->name;
				} else {
					cerr << vi->second->type;
				}
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
	switch(getDwarfTag(die, dwarf_error)) {
	case DW_TAG_compile_unit:
		return visitCU(index, die, dwarf_handle, dwarf_error);	
	case DW_TAG_base_type:
		return visitBaseType(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_variable:
		return visitVariable(index, die, dwarf_handle, dwarf_error);
	case DW_TAG_subprogram:
		return visitSubProgram(index, die, dwarf_handle, dwarf_error);
	default:
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
	return acceptChildren(index, cu_die, dwarf_handle, dwarf_error);
}

unsigned int DwarfIndexer::visitBaseType(DwarfIndex &index, Dwarf_Die type_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	string 		name;
	Dwarf_Off	offset;
	TypeEntry	*te;

	name = getDwarfName(type_die, dwarf_handle, dwarf_error);
	offset = getDwarfOffset(type_die, dwarf_handle, dwarf_error);

	te = new TypeEntry;
	te->name = name;

	index.types[offset] = te;

	cerr << name.c_str() << endl;

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
	ve->type = type;
	if (getDwarfScriptList(variable_die, DW_AT_location, ve->location, dwarf_handle, dwarf_error) != 0) {
		delete ve;
		return 1;
	}

	index.variables[offset] = ve;

	cerr << name.c_str() << endl;

	return 0;
}

unsigned int DwarfIndexer::visitSubProgram(DwarfIndex &index, Dwarf_Die sp_die, Dwarf_Debug dwarf_handle, Dwarf_Error dwarf_error) {
	cerr << "subprogram" << endl;
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

	DwarfIndex local_index = { index.types, fe->variables, index.functions };

	if (acceptChildren(local_index, sp_die, dwarf_handle, dwarf_error) == 0) {
		index.functions[offset] = fe;
	} else {
		delete fe;
		return 1;
	}
	
	return 0;
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
