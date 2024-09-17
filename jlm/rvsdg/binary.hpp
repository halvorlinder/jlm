/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JLM_RVSDG_BINARY_HPP
#define JLM_RVSDG_BINARY_HPP

#include <jlm/rvsdg/graph.hpp>
#include <jlm/rvsdg/operation.hpp>
#include <jlm/rvsdg/simple-normal-form.hpp>
#include <jlm/util/common.hpp>

namespace jlm::rvsdg
{

typedef size_t binop_reduction_path_t;

class binary_op;

class binary_normal_form final : public simple_normal_form
{
public:
  virtual ~binary_normal_form() noexcept;

  binary_normal_form(
      const std::type_info & operator_class,
      jlm::rvsdg::node_normal_form * parent,
      jlm::rvsdg::graph * graph);

  virtual bool
  normalize_node(jlm::rvsdg::node * node) const override;

  virtual std::vector<jlm::rvsdg::output *>
  normalized_create(
      rvsdg::Region * region,
      const jlm::rvsdg::simple_op & op,
      const std::vector<jlm::rvsdg::output *> & arguments) const override;

  virtual void
  set_reducible(bool enable);

  inline bool
  get_reducible() const noexcept
  {
    return enable_reducible_;
  }

  virtual void
  set_flatten(bool enable);

  inline bool
  get_flatten() const noexcept
  {
    return enable_flatten_;
  }

  virtual void
  set_reorder(bool enable);

  inline bool
  get_reorder() const noexcept
  {
    return enable_reorder_;
  }

  virtual void
  set_distribute(bool enable);

  inline bool
  get_distribute() const noexcept
  {
    return enable_distribute_;
  }

  virtual void
  set_factorize(bool enable);

  inline bool
  get_factorize() const noexcept
  {
    return enable_factorize_;
  }

private:
  bool
  normalize_node(jlm::rvsdg::node * node, const binary_op & op) const;

  bool enable_reducible_;
  bool enable_reorder_;
  bool enable_flatten_;
  bool enable_distribute_;
  bool enable_factorize_;

  friend class flattened_binary_normal_form;
};

class flattened_binary_normal_form final : public simple_normal_form
{
public:
  virtual ~flattened_binary_normal_form() noexcept;

  flattened_binary_normal_form(
      const std::type_info & operator_class,
      jlm::rvsdg::node_normal_form * parent,
      jlm::rvsdg::graph * graph);

  virtual bool
  normalize_node(jlm::rvsdg::node * node) const override;

  virtual std::vector<jlm::rvsdg::output *>
  normalized_create(
      rvsdg::Region * region,
      const jlm::rvsdg::simple_op & op,
      const std::vector<jlm::rvsdg::output *> & arguments) const override;
};

/**
  \brief Binary operator

  Operator taking two arguments (with well-defined reduction for more
  operands if operator is associative).
*/
class binary_op : public simple_op
{
public:
  enum class flags
  {
    none = 0,
    associative = 1,
    commutative = 2
  };

  virtual ~binary_op() noexcept;

  inline binary_op(
      const std::vector<std::shared_ptr<const jlm::rvsdg::type>> operands,
      std::shared_ptr<const jlm::rvsdg::type> result)
      : simple_op(std::move(operands), { std::move(result) })
  {}

  virtual binop_reduction_path_t
  can_reduce_operand_pair(const jlm::rvsdg::output * op1, const jlm::rvsdg::output * op2)
      const noexcept = 0;

  virtual jlm::rvsdg::output *
  reduce_operand_pair(
      binop_reduction_path_t path,
      jlm::rvsdg::output * op1,
      jlm::rvsdg::output * op2) const = 0;

  virtual jlm::rvsdg::binary_op::flags
  flags() const noexcept;

  inline bool
  is_associative() const noexcept;

  inline bool
  is_commutative() const noexcept;

  static jlm::rvsdg::binary_normal_form *
  normal_form(jlm::rvsdg::graph * graph) noexcept
  {
    return static_cast<jlm::rvsdg::binary_normal_form *>(
        graph->node_normal_form(typeid(binary_op)));
  }
};

class flattened_binary_op final : public simple_op
{
public:
  enum class reduction
  {
    linear,
    parallel
  };

  virtual ~flattened_binary_op() noexcept;

  inline flattened_binary_op(std::unique_ptr<binary_op> op, size_t narguments) noexcept
      : simple_op({ narguments, op->argument(0) }, { op->result(0) }),
        op_(std::move(op))
  {
    JLM_ASSERT(op_->is_associative());
  }

  inline flattened_binary_op(const binary_op & op, size_t narguments)
      : simple_op({ narguments, op.argument(0) }, { op.result(0) }),
        op_(std::unique_ptr<binary_op>(static_cast<binary_op *>(op.copy().release())))
  {
    JLM_ASSERT(op_->is_associative());
  }

  virtual bool
  operator==(const operation & other) const noexcept override;

  virtual std::string
  debug_string() const override;

  virtual std::unique_ptr<jlm::rvsdg::operation>
  copy() const override;

  inline const binary_op &
  bin_operation() const noexcept
  {
    return *op_;
  }

  static jlm::rvsdg::flattened_binary_normal_form *
  normal_form(jlm::rvsdg::graph * graph) noexcept
  {
    return static_cast<flattened_binary_normal_form *>(
        graph->node_normal_form(typeid(flattened_binary_op)));
  }

  jlm::rvsdg::output *
  reduce(
      const flattened_binary_op::reduction & reduction,
      const std::vector<jlm::rvsdg::output *> & operands) const;

  static void
  reduce(rvsdg::Region * region, const flattened_binary_op::reduction & reduction);

  static inline void
  reduce(jlm::rvsdg::graph * graph, const flattened_binary_op::reduction & reduction)
  {
    reduce(graph->root(), reduction);
  }

private:
  std::unique_ptr<binary_op> op_;
};

/* binary flags operators */

static inline constexpr enum binary_op::flags
operator|(enum binary_op::flags a, enum binary_op::flags b)
{
  return static_cast<enum binary_op::flags>(static_cast<int>(a) | static_cast<int>(b));
}

static inline constexpr enum binary_op::flags
operator&(enum binary_op::flags a, enum binary_op::flags b)
{
  return static_cast<enum binary_op::flags>(static_cast<int>(a) & static_cast<int>(b));
}

/* binary methods */

inline bool
binary_op::is_associative() const noexcept
{
  return static_cast<int>(flags() & binary_op::flags::associative);
}

inline bool
binary_op::is_commutative() const noexcept
{
  return static_cast<int>(flags() & binary_op::flags::commutative);
}

static const binop_reduction_path_t binop_reduction_none = 0;
/* both operands are constants */
static const binop_reduction_path_t binop_reduction_constants = 1;
/* can merge both operands into single (using some "simpler" operator) */
static const binop_reduction_path_t binop_reduction_merge = 2;
/* part of left operand can be folded into right */
static const binop_reduction_path_t binop_reduction_lfold = 3;
/* part of right operand can be folded into left */
static const binop_reduction_path_t binop_reduction_rfold = 4;
/* left operand is neutral element */
static const binop_reduction_path_t binop_reduction_lneutral = 5;
/* right operand is neutral element */
static const binop_reduction_path_t binop_reduction_rneutral = 6;
/* both operands have common form which can be factored over op */
static const binop_reduction_path_t binop_reduction_factor = 7;

}

#endif
