#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "promise.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

class PromisePimpl {
public:
	PromisePimpl()
		: mutex(PTHREAD_MUTEX_INITIALIZER)
		, state(Promise::PENDING) {

		pipe2(fd, O_NONBLOCK);
	}

	~PromisePimpl(){
		close(fd[0]);
		close(fd[1]);
	}

	static void exec_callback(Promise::callback func){
		try {
			func();
		} catch ( std::exception& e ){
			fprintf(stderr, "user exception: %s\n", e.what());
		}
	}

	void set_state(enum Promise::State state){
		this->state = state;
		char ch = state;
		write(fd[1], &ch, 1);
	}

	pthread_mutex_t mutex;
	enum Promise::State state;
	int fd[2];
	std::vector<Promise::callback> cb_always;
	std::vector<Promise::callback> cb_done;
	std::vector<Promise::callback> cb_fail;
};

Promise::Promise()
	: impl(new PromisePimpl){

}

Promise::~Promise(){

}

Promise& Promise::always(callback func){
	pthread_mutex_lock(&impl->mutex);
	impl->cb_always.push_back(func);
	pthread_mutex_unlock(&impl->mutex);
	return *this;
}

Promise& Promise::done(callback func){
	pthread_mutex_lock(&impl->mutex);
	impl->cb_done.push_back(func);
	pthread_mutex_unlock(&impl->mutex);
	return *this;
}

Promise& Promise::fail(callback func){
	pthread_mutex_lock(&impl->mutex);
	impl->cb_fail.push_back(func);
	pthread_mutex_unlock(&impl->mutex);
	return *this;
}

void Promise::resolve(){
	if ( impl->state != PENDING ){
		return;
	}

	pthread_mutex_lock(&impl->mutex);
	for ( auto func: impl->cb_done   ) func();
	for ( auto func: impl->cb_always ) func();
	impl->set_state(RESOLVED);
	pthread_mutex_unlock(&impl->mutex);
}

void Promise::reject(){
	if ( state() != PENDING ){
		return;
	}

	pthread_mutex_lock(&impl->mutex);
	for ( auto func: impl->cb_fail   ) func();
	for ( auto func: impl->cb_always ) func();
	impl->set_state(REJECTED);
	pthread_mutex_unlock(&impl->mutex);
}

enum Promise::State Promise::state() const {
	return impl->state;
}

void Promise::await() const {
	struct pollfd fds[1];
	fds[0].fd = impl->fd[0];
	fds[0].events = POLLRDNORM;

	if ( poll(fds, 1, -1) == -1 ){
		fprintf(stderr, "poll failed: %s\n", strerror(errno));
	}
}

int Promise::fileno() const {
	return impl->fd[0];
}
