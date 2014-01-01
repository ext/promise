#ifndef PROMISE_HPP
#define PROMISE_HPP

#include <vector>
#include <functional>
#include <memory>
#include <pthread.h>

class PromisePimpl;
class Promise {
public:
	enum State {
		PENDING,       /* Promise still pending */
		REJECTED,      /* Promise finished with failure */
		RESOLVED,      /* Promise finished successfully */
	};
	typedef std::function<void()> callback;

	Promise();
	~Promise();

	/* User callbacks, multiple functions can be registered */
	Promise& always(callback func);
	Promise& done(callback func);
	Promise& fail(callback func);

	/* State changes. It is undefined behavior to call multiple times. */
	void resolve();
	void reject();

	/**
	 * Current state.
	 */
	enum State state() const;

	/**
	 * Synchronous call awaiting until the promise finished (using either resolve
	 * or reject.
	 */
	void await() const;

	/**
	 * A FD which user can use with select, poll, etc to wait for finishing.
	 */
	int fileno() const;

private:
	std::shared_ptr<PromisePimpl> impl;
};

#endif /* PROMISE_HPP */
