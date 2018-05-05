/*
 * Copyright 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-operation.hpp"
#include "test-registry.hpp"
#include "test-types.hpp"

#include <jive/rvsdg/control.h>
#include <jive/rvsdg/gamma.h>
#include <jive/types/function/fcttype.h>
#include <jive/view.h>

#include <jlm/ir/cfg-structure.hpp>
#include <jlm/ir/lambda.hpp>
#include <jlm/ir/module.hpp>
#include <jlm/ir/rvsdg.hpp>
#include <jlm/ir/view.hpp>
#include <jlm/rvsdg2jlm/rvsdg2jlm.hpp>

static int
test()
{
	jlm::valuetype vt;
	jive::bittype bt1(1);
	jive::fct::type ft({&bt1, &vt}, {&vt});

	jlm::rvsdg rvsdg("", "");

	jlm::lambda_builder lb;
	auto arguments = lb.begin_lambda(rvsdg.graph()->root(), ft);

	auto match = jive::match(1, {{0, 0}}, 1, 2, arguments[0]);
	auto gamma = jive::gamma_node::create(match, 2);
	auto ev = gamma->add_entryvar(arguments[1]);
	auto output = jlm::create_testop(gamma->subregion(1), {ev->argument(1)}, {&vt})[0];
	auto ex = gamma->add_exitvar({ev->argument(0), output});

	auto lambda = lb.end_lambda({ex});

	rvsdg.graph()->add_export(lambda->output(0), "");

	jive::view(*rvsdg.graph(), stdout);

	auto module = jlm::rvsdg2jlm::rvsdg2jlm(rvsdg);
	auto & clg = module->callgraph();
	assert(clg.nnodes() == 1);

	auto cfg = dynamic_cast<const jlm::function_node&>(*clg.begin()).cfg();
	jlm::view_ascii(*cfg, stdout);

	assert(!is_proper_structured(*cfg));
	assert(is_structured(*cfg));

	return 0;
}

JLM_UNIT_TEST_REGISTER("libjlm/r2j/test-partial-gamma", test)