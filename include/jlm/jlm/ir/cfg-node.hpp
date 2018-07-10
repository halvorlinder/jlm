/*
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JLM_IR_CFG_NODE_H
#define JLM_IR_CFG_NODE_H

#include <jlm/common.hpp>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>


namespace jlm {

class cfg;
class cfg_node;

class cfg_edge final {
public:
	~cfg_edge() noexcept {};

	cfg_edge(cfg_node * source, cfg_node * sink, size_t index) noexcept;

	void
	divert(cfg_node * new_sink);

	cfg_node *
	split();

	inline cfg_node * source() const noexcept { return source_; }
	inline cfg_node * sink() const noexcept { return sink_; }
	inline size_t index() const noexcept { return index_; }

	inline bool is_selfloop() const noexcept { return source_ == sink_; }

private:
	cfg_node * source_;
	cfg_node * sink_;
	size_t index_;

	friend cfg_node;
};

class cfg_node {
	typedef std::unordered_set<cfg_edge*>::const_iterator const_inedge_iterator;

	class const_outedge_iterator final {
	public:
		inline
		const_outedge_iterator(const std::vector<std::unique_ptr<cfg_edge>>::const_iterator & it)
		: it_(it)
		{}

		inline const const_outedge_iterator &
		operator++() noexcept
		{
			++it_;
			return *this;
		}

		inline const_outedge_iterator
		operator++(int) noexcept
		{
			auto tmp = *this;
			++*this;
			return tmp;
		}

		inline bool
		operator==(const const_outedge_iterator & other) const noexcept
		{
			return it_ == other.it_;
		}

		inline bool
		operator!=(const const_outedge_iterator & other) const noexcept
		{
			return !(other == *this);
		}

		inline cfg_edge *
		operator->() const noexcept
		{
			return edge();
		}

		inline cfg_edge *
		edge() const noexcept
		{
			return it_->get();
		}

	private:
		std::vector<std::unique_ptr<cfg_edge>>::const_iterator it_;
	};

public:
	virtual
	~cfg_node();

protected:
	inline
	cfg_node(jlm::cfg & cfg)
	: cfg_(cfg)
	{}

public:
	jlm::cfg &
	cfg() const noexcept
	{
		return cfg_;
	}

	cfg_edge *
	add_outedge(cfg_node * sink)
	{
		outedges_.push_back(std::make_unique<cfg_edge>(this, sink, noutedges()));
		sink->inedges_.insert(outedges_.back().get());
		return outedges_.back().get();
	}

	inline void
	remove_outedge(size_t n)
	{
		JLM_DEBUG_ASSERT(n < noutedges());
		auto edge = outedges_[n].get();

		edge->sink()->inedges_.erase(edge);
		for (size_t i = n+1; i < noutedges(); i++) {
			outedges_[i-1] = std::move(outedges_[i]);
			outedges_[i-1]->index_ = outedges_[i-1]->index_-1;
		}
		outedges_.resize(noutedges()-1);
	}

	inline void
	remove_outedges()
	{
		while (noutedges() != 0)
			remove_outedge(noutedges()-1);
	}

	inline cfg_edge *
	outedge(size_t n) const
	{
		JLM_DEBUG_ASSERT(n < noutedges());
		return outedges_[n].get();
	}

	size_t noutedges() const noexcept;

	inline const_outedge_iterator
	begin_outedges() const
	{
		return const_outedge_iterator(outedges_.begin());
	}

	inline const_outedge_iterator
	end_outedges() const
	{
		return const_outedge_iterator(outedges_.end());
	}

	inline const_inedge_iterator
	begin_inedges() const
	{
		return inedges_.begin();
	}

	inline const_inedge_iterator
	end_inedges() const
	{
		return inedges_.end();
	}

	inline void
	divert_inedges(jlm::cfg_node * new_successor)
	{
		while (ninedges())
			(*begin_inedges())->divert(new_successor);
	}

	void remove_inedges();

	size_t ninedges() const noexcept;

	bool no_predecessor() const noexcept;

	bool single_predecessor() const noexcept;

	bool no_successor() const noexcept;

	bool single_successor() const noexcept;

	inline bool is_branch() const noexcept { return noutedges() > 1; }

	bool has_selfloop_edge() const noexcept;

	static cfg_node *
	create(jlm::cfg & cfg);

private:
	jlm::cfg & cfg_;
	std::vector<std::unique_ptr<cfg_edge>> outedges_;
	std::unordered_set<cfg_edge*> inedges_;

	friend cfg_edge;
};

}

#endif
