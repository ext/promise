#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "promise.hpp"
#include <cstdio>
#include <functional>
#include <unistd.h>

static pthread_t thread;

void* sample_workload(Promise* promise){
	printf("thread starting\n");
	usleep(3000 * 1000);
	printf("thread done\n");
	promise->resolve();
	return nullptr;
}

Promise execute(void*(*func)(Promise*)){
	Promise promise;
	pthread_create(&thread, nullptr, (void*(*)(void*))func, &promise);
	return promise;
}

int main(int argc, const char* argv[]){
	auto promise = execute(sample_workload).done([](){
		printf("promise done callback\n");
	});
	printf("execute async return\n");

	promise.await();
	printf("promise awaited\n");

	pthread_join(thread, nullptr);
	return 0;
}
