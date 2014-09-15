/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <llvm/IR/LLVMContext.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>

#include <iostream>

int main (int argc, char ** argv)
{
	if (argc < 2) {
		std::cerr << "Expected LLVM IR file as input\n";
		exit(1);
	}

	llvm::LLVMContext & context = llvm::getGlobalContext();

	llvm::SMDiagnostic err;
	llvm::Module * module = llvm::ParseIRFile(argv[1], err, context);

	if (!module) {
		err.print(argv[0], llvm::errs());
		return 1;
	}

	return 0;
}
