// https://serverfault.com/questions/405092/nice-level-not-working-on-linux
// echo "kernel.sched_autogroup_enabled = 0" >> /etc/sysctl.conf; sysctl -p

// In three terminals, run:
// 1. time nice -n 19 taskset -c 1 ./nicedemo
// 2. time taskset -c 1 ./nicedemo
// 3. top
#include<stdio.h>
#include<limits.h>
int main(void) {
	unsigned int i = 0;
	while(i < UINT_MAX) {i++;}   
	i=0;
	while(i < UINT_MAX) {i++;}   
}
